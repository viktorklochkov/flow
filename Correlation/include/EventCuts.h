// Flow Vector Correction Framework
//
// Copyright (C) 2019  Lukas Kreis Ilya Selyuzhenkov
// Contact: l.kreis@gsi.de; ilya.selyuzhenkov@gmail.com
// For a full list of contributors please see docs/Credits
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef QNEVENTCUTS_H
#define QNEVENTCUTS_H

#include <string>
#include "TTreeReaderValue.h"
#include "ROOT/RMakeUnique.hxx"
#include "ROOT/RIntegerSequence.hxx"

namespace Qn {

/**
 * Base class of the Cut.
 */
struct EventCutBase {
  virtual ~EventCutBase() = default;
  virtual bool Check() = 0;
};

/**
 * Template class of a cut applied in the correction step.
 * Number of dimensions is determined by the signature of the cut function
 * and by the size of the variables array passed to the constructor.
 * @tparam T Type of variable
 */
template<typename VAR, typename... T>
class EventCut : public EventCutBase {
 public:
  EventCut(VAR (&arr)[sizeof...(T)], std::function<bool(T...)> lambda)
      : lambda_(lambda) {
    int i = 0;
    for (auto &a : arr) {
      variables_.at(i) = std::move(a);
      ++i;
    }
  }

  /**
   * Check if the cut is passed for variables at variable id + i
   * @param i offset from the variable id.
   * @return true if the cut is passed.
   */
  bool Check() override {
    return CheckImpl(std::make_index_sequence<sizeof...(T)>{});
  }

 private:

  /**
   * Implements the evaluation of the cut.
   * @tparam I index sequence
   * @param i offset from the variable id.
   * @return true if the cut is passed.
   */
  template<std::size_t... I>
  bool CheckImpl(std::index_sequence<I...>) {
    return lambda_(*(variables_[I]->Get())...);
  }

  std::array<VAR, sizeof...(T)> variables_; /// array of the variables used in the cut.
  std::function<bool(T...)> lambda_; /// function used to evaluate the cut.
};

namespace Details {
template<std::size_t>
using Type = float &;
template<typename VAR, std::size_t N, typename FUNC, std::size_t... Is>
std::unique_ptr<EventCut<VAR, Type<Is>...>> MakeUniqueEventCutImpl(std::index_sequence<Is...>,
                                                              VAR (&arr)[N],
                                                              FUNC &&func) {
  return std::make_unique<EventCut<VAR, Type<Is>...>>(arr, std::forward<FUNC>(func));
}
}

/**
 * Function which create a unique_ptr of a cut of n variables.
 * @tparam N length of the variable array.
 * @tparam FUNC type of the cut function
 * @param arr array of variables
 * @param func cut function
 * @return Returns a unique pointer to the cut.
 */
template<std::size_t N, typename FUNC, typename VAR>
std::unique_ptr<EventCutBase> MakeUniqueEventCut(VAR (&arr)[N], FUNC &&func) {
  return Details::MakeUniqueEventCutImpl(std::make_index_sequence<N>{}, arr, std::forward<FUNC>(func));
}

class EventCuts {
 public:

  void AddCut(std::unique_ptr<EventCutBase> cut) {
    cuts_.push_back(std::move(cut));
  }

  bool CheckCuts() {
    bool passed = true;
    for (auto &cut : cuts_) {
      passed = cut->Check() && passed;
    }
    return passed;
  }

 private:
    std::vector<std::unique_ptr<EventCutBase>> cuts_;
};

}

#endif