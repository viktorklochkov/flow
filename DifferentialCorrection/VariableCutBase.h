//
// Created by Lukas Kreis on 29.08.18.
//

#ifndef FLOW_VARIABLECUTBASE_H
#define FLOW_VARIABLECUTBASE_H

#include <array>
#include "VariableManager.h"

namespace Qn {

struct VariableCutBase {
  virtual ~VariableCutBase() = default;
  virtual bool Check(int i) = 0;
};

template<typename... T>
class VariableCutNDim : public VariableCutBase {
 public:
  VariableCutNDim<T...>(Variable const (&arr)[sizeof...(T)], std::function<bool(T...)> lambda)
      : lambda_(lambda) {
    int i = 0;
    for (auto a : arr) {
      variables_[i] = a;
      ++i;
    }
  }
  bool Check(int i) override {
    return CheckImpl(i, variables_, std::make_index_sequence<sizeof...(T)>{});
  }
 private:
  template<typename ARR, std::size_t... I>
  bool CheckImpl(int i, const ARR &arr, std::index_sequence<I...>) {
    return lambda_(*(arr[I].begin() + i)...);
  }

  std::array<Variable, sizeof...(T)> variables_;
  std::function<bool(T...)> lambda_;
};

namespace Details {
template<std::size_t>
using Type = double &;
template<std::size_t... Is, std::size_t N, typename FUNC>
auto CreateNDimCutImpl(std::index_sequence<Is...>, Variable const (&arr)[N], FUNC &&func) {
  auto pp = std::make_unique<VariableCutNDim<Type<Is>...>>(arr, std::forward<FUNC>(func));
  return pp;
}
}

template<std::size_t N, typename FUNC>
auto MakeUniqueNDimCut(Variable const (&arr)[N], FUNC &&func) {
  return Details::CreateNDimCutImpl(std::make_index_sequence<N>{}, arr, std::forward<FUNC>(func));
}

}

#endif //FLOW_VARIABLECUTBASE_H
