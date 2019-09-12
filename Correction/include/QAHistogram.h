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

#include "TH1.h"
#include "TH2.h"
#include "TH3F.h"
#include "TList.h"
#include "ROOT/RMakeUnique.hxx"
#include "ROOT/RIntegerSequence.hxx"

#include "Axis.h"
#include "InputVariableManager.h"

namespace Qn {

/**
 * Base class of a QA histogram
 */
struct QAHistoBase {
  virtual ~QAHistoBase() = default;
  virtual void Fill() = 0;
  virtual void AddToList(TList *) = 0;
  static inline void FillHistogram(std::unique_ptr<QAHistoBase> &histo) { histo->Fill(); }
};

class QAHistogram {
 public:
  enum class Type {
    OneDim,
    TwoDim,
    TwoDimArray
  };

  QAHistogram() = default;
  virtual ~QAHistogram() = default;
  QAHistogram(QAHistogram &&hist) = default;
  QAHistogram &operator=(QAHistogram &&hist) = default;
  QAHistogram(std::string name, AxisD axis, std::string weight) :
      name_(name), weight_(weight), type_(Type::OneDim) { axes_.push_back(axis); }
  QAHistogram(std::string name, std::vector<AxisD> axes, std::string weight) :
      name_(name), axes_(axes), weight_(weight), type_(Type::TwoDim) {}
  QAHistogram(std::string name, std::vector<AxisD> axes, std::string weight, Qn::AxisD histoaxis) :
      name_(name), axes_(axes), weight_(weight), type_(Type::TwoDimArray), histoaxis_(histoaxis) {}
  void Initialize(InputVariableManager *var) {
    if (type_==Type::OneDim) {
      histogram_ = MakeHisto1D(var);
    } else if (type_==Type::TwoDim) {
      histogram_ = MakeHisto2D(var);
    } else if (type_==Type::TwoDimArray) {
      histogram_ = MakeHisto2DArray(var);
    }
  }
  void Fill() { histogram_->Fill(); }
  void AddToList(TList *list) { histogram_->AddToList(list); }
 private:

  std::unique_ptr<QAHistoBase> histogram_; //!<!
  std::string name_;
  std::vector<AxisD> axes_;
  std::string weight_;
  Type type_;
  AxisD histoaxis_;

  std::unique_ptr<QAHistoBase> MakeHisto1D(InputVariableManager *var);
  std::unique_ptr<QAHistoBase> MakeHisto2D(InputVariableManager *var);
  std::unique_ptr<QAHistoBase> MakeHisto2DArray(InputVariableManager *var);
};

class QAHistograms {
 public:
  QAHistograms() = default;
  virtual ~QAHistograms() = default;
  QAHistograms(QAHistograms &&hist) = default;
  QAHistograms &operator=(QAHistograms &&hist) = default;
  template<typename... Args>
  void Add(Args &&... args) {
    histograms_.emplace_back(std::forward<Args>(args)...);
  }
  void AddToList(TList *list) {
    for (auto &histo : histograms_) {
      histo.AddToList(list);
    }
  }

  void Initialize(InputVariableManager *var) {
    for (auto &histo : histograms_) {
      histo.Initialize(var);
    }
  }

  void Fill() {
    for (auto &histo : histograms_) {
      histo.Fill();
    }
  }
 private:
  std::vector<QAHistogram> histograms_;

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
  QAHisto(std::array<VAR, N> vec, HISTO histo, std::unique_ptr<Qn::AxisD> axis, Qn::InputVariable axisvar) :
      vars_(std::move(vec)),
      axis_(std::move(axis)),
      axisvar_(axisvar) {
    if (axis_) {
      for (unsigned int i = 0; i < axis_->size(); ++i) {
        auto histname = histo->GetName();
        auto binname = std::string(histname) + "_" + axis_->GetBinName(i);
        auto binhisto = (HISTO) ptr(histo)->Clone(binname.data());
        histo_.push_back(binhisto);
      }
      name_ = ptr(histo)->GetName();
      delete histo;
    } else {
      histo_.push_back(histo);
    }
  }

  QAHisto(std::array<VAR, N> vec, HISTO histo) :
      vars_(std::move(vec)) {
    histo_.push_back(histo);
  }

  /**
   * Implementation of the fill function
   * @tparam VARS type of array
   * @tparam I index sequence
   * @param vars Array of variables used for filling the histogram
   */
  template<typename VARS, std::size_t... I>
  void FillImpl(const VARS &variables, std::index_sequence<I...>) {
    auto bin = 0;
    if (axis_) {
      bin = axis_->FindBin(*axisvar_.begin());
      if (bin > -1) ptr(histo_.at(bin))->FillN(variables[0].size(), (variables[I].begin())...);
    } else {
      ptr(histo_.at(bin))->FillN(variables[0].size(), (variables[I].begin())...);
    }
  }

  /**
   * Fill function.
   */
  void Fill() override {
    return FillImpl(vars_, std::make_index_sequence<N>{});
  };

  /**
   * Add the histogram to the list.
   * @param list pointer to the list. Lifetime of the histogram hast to be managed by the list.
   */
  void AddToList(TList *list) override {
    if (axis_) {
      auto dir = new TList();
      dir->SetName(name_.data());
      for (auto &histo : histo_) {
        dir->Add(ptr(histo));
      }
      list->Add(dir);
    } else {
      for (auto &histo : histo_) {
        list->Add(ptr(histo));
      }
    }
  }

 private:

  template<typename T>
  T *ptr(T &obj) { return &obj; } ///Turns a reference into pointer.
  template<typename T>
  T *ptr(T *obj) { return obj; } /// The Object is already pointer. Returns it.

  std::array<VAR, N> vars_; /// Array of variables to be filled in the histogram.
  std::vector<HISTO> histo_; /// Histogram (e.g. TH1, TH2) which support the filling with FillN(...).
  std::unique_ptr<Qn::AxisD> axis_ = nullptr; // Creates a histogram for each bin of the axis
  VAR axisvar_;
  std::string name_;
};

/// specializations used in the framework
using QAHisto1DPtr = QAHisto<TH1F *, 2, Qn::InputVariable>;
using QAHisto2DPtr = QAHisto<TH2F *, 3, Qn::InputVariable>;

}

#endif //FLOW_QAHISTOGRAM_H
