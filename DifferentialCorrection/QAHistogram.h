//
// Created by Lukas Kreis on 28.08.18.
//

#ifndef FLOW_QAHISTOGRAM_H
#define FLOW_QAHISTOGRAM_H

#include <utility>
#include <vector>
#include <TH1.h>
#include <TH2.h>
#include "VariableManager.h"
namespace Qn {

struct QAHistoBase {
  virtual ~QAHistoBase() = default;
  virtual void Fill() = 0;
  virtual void Draw(const char *option) = 0;
  virtual void Write(const char *name) = 0;
  virtual const char* Name() = 0;

};

template<typename HISTO, int N, typename VAR>
class QAHisto : public QAHistoBase {
 public:
  QAHisto(std::array<VAR, N> vec, HISTO histo) : vars_(std::move(vec)), histo_(histo) {}

  template<typename array, std::size_t... I>
  auto FillImpl(const array a, std::index_sequence<I...>) {
    histo_.FillN(std::size(a[0]), (a[I].begin())...);
  }

  void Fill() override {
    return FillImpl(vars_, std::make_index_sequence<N>{});
  };

  void Draw(const char *option) override { histo_.Draw(option); }

  void Write(const char *name) override { histo_.Write(name); }

  const char* Name() override { return histo_.GetName(); }

 private:
  std::array<VAR, N> vars_;
  HISTO histo_;
};

using QAHisto1D = QAHisto<TH1F, 2, Variable>;
using QAHisto2D = QAHisto<TH2F, 3, Variable>;

}
#endif //FLOW_QAHISTOGRAM_H
