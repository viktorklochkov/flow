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

struct QAHistoBase {
  virtual ~QAHistoBase() = default;
  virtual void Fill() = 0;
  virtual void Draw(const char *option) = 0;
  virtual void Write(const char *name) = 0;
  virtual const char* Name() = 0;
  virtual void AddToList(TList*) = 0;

};

template<typename HISTO, int N, typename VAR>
class QAHisto : public QAHistoBase {
 public:
  QAHisto(std::array<VAR, N> vec, HISTO histo) : vars_(std::move(vec)), histo_(std::move(histo)) {}

  template<typename array, std::size_t... I>
  void FillImpl(const array a, std::index_sequence<I...>) {
    ptr(histo_)->FillN(a[0].length(), (a[I].begin())...);
  }

  void Fill() override {
    return FillImpl(vars_, std::make_index_sequence<N>{});
  };

  void Draw(const char *option) override { ptr(histo_)->Draw(option); }

  void Write(const char *name) override { ptr(histo_)->Write(name); }

  const char* Name() override { return ptr(histo_)->GetName(); }

  void AddToList(TList *list) override {list->Add(ptr(histo_));}


 private:
  template<typename T>
  T * ptr(T & obj) { return &obj; } //turn reference into pointer!

  template<typename T>
  T * ptr(T * obj) { return obj; } //obj is already pointer, return it!
  std::array<VAR, N> vars_;
  HISTO histo_;
};

using QAHisto1D = QAHisto<TH1F, 2, Qn::Variable>;
using QAHisto2D = QAHisto<TH2F, 3, Qn::Variable>;

using QAHisto1DPtr = QAHisto<TH1F*, 2, Qn::Variable>;
using QAHisto2DPtr = QAHisto<TH2F*, 3, Qn::Variable>;

}
#endif //FLOW_QAHISTOGRAM_H
