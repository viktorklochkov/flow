// Flow Vector Correction Framework
//
// Copyright (C) 2018  Lukas Kreis, Ilya Selyuzhenkov
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

#ifndef FLOW_QAHISTOGRAM_H
#define FLOW_QAHISTOGRAM_H

#include <utility>
#include <vector>
#include <TH1.h>
#include <TH2.h>
#include "TList.h"
#include "VariableManager.h"
#include "ROOT/RMakeUnique.hxx"
#include "ROOT/RIntegerSequence.hxx"

namespace Qn {
/**
 * Base class of a QA histogram
 */
struct QAHistoBase {
  virtual ~QAHistoBase() = default;
  virtual void Fill() = 0;
  virtual void Draw(const char *option) = 0;
  virtual void Write(const char *name) = 0;
  virtual const char *Name() = 0;
  virtual void AddToList(TList *) = 0;

};
/**
 * Wrapper for a ROOT histogram, which allows it to be filled by the correction manager.
 * @tparam HISTO type of histogram.
 * @tparam N number of dimensions
 * @tparam VAR Type of the variable
 */
template<typename HISTO, int N, typename VAR>
class QAHisto : public QAHistoBase {
 public:
  QAHisto(std::array<VAR, N> vec, HISTO histo) : vars_(std::move(vec)), histo_(std::move(histo)) {}

  /**
   * Implementation of the fill function
   * @tparam array type of array
   * @tparam I index sequence
   * @param a Array of variables used for filling the histogram
   */
  template<typename array, std::size_t... I>
  void FillImpl(const array a, std::index_sequence<I...>) {
    ptr(histo_)->FillN(a[0].length(), (a[I].begin())...);
  }

  /**
   * Fill function.
   */
  void Fill() override {
    return FillImpl(vars_, std::make_index_sequence<N>{});
  };

  void Draw(const char *option) override { ptr(histo_)->Draw(option); }

  void Write(const char *name) override { ptr(histo_)->Write(name); }

  const char *Name() override { return ptr(histo_)->GetName(); }

  /**
   * Add the histogram to the list.
   * @param list pointer to the list. Lifetime of the histogram hast to be managed by the list.
   */
  void AddToList(TList *list) override { list->Add(ptr(histo_)); }

 private:
  template<typename T>
  T *ptr(T &obj) { return &obj; } ///Turns a reference into pointer.
  template<typename T>
  T *ptr(T *obj) { return obj; } /// The Object is already pointer. Returns it.
  std::array<VAR, N> vars_; /// Array of variables to be filled in the histogram.
  HISTO histo_; /// Histogram (e.g. TH1, TH2) which support the filling with FillN(...).
};

/// specializations used in the framework
using QAHisto1DPtr = QAHisto<TH1F *, 2, Qn::Variable>;
using QAHisto2DPtr = QAHisto<TH2F *, 3, Qn::Variable>;

}

#endif //FLOW_QAHISTOGRAM_H
