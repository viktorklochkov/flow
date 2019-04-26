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

#ifndef FLOW_VARIABLECUTBASE_H
#define FLOW_VARIABLECUTBASE_H

#include <array>
#include <vector>
#include <functional>

#include "VariableManager.h"
#include "QAHistogram.h"

#include "ROOT/RMakeUnique.hxx"
#include "ROOT/RIntegerSequence.hxx"

namespace Qn {
/**
 * Base class of the Cut.
 */
struct VariableCutBase {
  virtual ~VariableCutBase() = default;
  virtual bool Check(int i) = 0;
  virtual int GetVariableLength() const = 0;
  virtual std::string Name() const = 0;
};
/**
 * Template class of a cut applied in the correction step.
 * Number of dimensions is determined by the signature of the cut function
 * and by the size of the variables array passed to the constructor.
 * @tparam T Type of variable
 */
template<typename... T>
class VariableCutNDim : public VariableCutBase {
 public:
  VariableCutNDim<T...>(Variable const (&arr)[sizeof...(T)], std::function<bool(T...)> lambda)
      : lambda_(lambda) {
    int i = 0;
    for (const auto &a : arr) {
      variables_[i] = a;
      ++i;
    }
  }

  /**
   * Check if the cut is passed for variables at variable id + i
   * @param i offset from the variable id.
   * @return true if the cut is passed.
   */
  bool Check(int i) override {
    return CheckImpl(i, std::make_index_sequence<sizeof...(T)>{});
  }

  /**
   * Get the length of the variable at position 0.
   * @return length of the variables.
   */
  int GetVariableLength() const override { return variables_[0].length(); }

 private:

  /**
   * Implements the evaluation of the cut.
   * @tparam I index sequence
   * @param i offset from the variable id.
   * @return true if the cut is passed.
   */
  template<std::size_t... I>
  bool CheckImpl(int i, std::index_sequence<I...>) {
    return lambda_(*(variables_[I].begin() + i)...);
  }

  /**
   * Returns the name of all variables used in the cut.
   * @return name of all variables.
   */
  inline std::string Name() const override {
    std::string name;
    for (auto var : variables_) {
      name += var.Name();
    }
    return name;
  }

  std::array<Variable, sizeof...(T)> variables_; /// array of the variables used in the cut.
  std::function<bool(T...)> lambda_; /// function used to evaluate the cut.
};

namespace Details {
template<std::size_t>
using Type = double &;
template<std::size_t N, typename FUNC, std::size_t... Is>
std::unique_ptr<VariableCutNDim<Type<Is>...>> CreateNDimCutImpl(std::index_sequence<Is...>,
                                                                Variable const (&arr)[N],
                                                                FUNC &&func) {
  auto pp = std::make_unique<VariableCutNDim<Type<Is>...>>(arr, std::forward<FUNC>(func));
  return pp;
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
template<std::size_t N, typename FUNC>
std::unique_ptr<VariableCutBase> MakeUniqueNDimCut(Variable const (&arr)[N], FUNC &&func) {
  return Details::CreateNDimCutImpl(std::make_index_sequence<N>{}, arr, std::forward<FUNC>(func));
}

/**
 * Manages cuts class and allows checking if the current variables passes the cut
 */
class Cuts {
 public:
  ~Cuts() { delete[] var_values_; }

  /**
   * @brief Adds a cut to the manager.
   * @param cut pointer to the cut.
   */
  void AddCut(std::unique_ptr<VariableCutBase> cut) {
    cuts_.push_back(std::move(cut));
  }

  /**
   * Checks if the current variables pass the cuts
   * Creates entries in the cut report
   * @param i offset of the variable in case it has a length longer than 1
   * @return Returns true if the cut was passed.
   */
  inline bool CheckCuts(int i) {
    int icut = 1;
    if (cuts_.empty()) return true;
    ++*((cut_weight_).begin() + i);// = *((cut_weight_).begin() + i) + 1.0;
    bool passed = true;
    for (auto &cut : cuts_) {
      bool ipass = cut->Check(i) && passed;
      if (ipass) {
        ++*cut_weight_.at(i + nchannels_*icut);
      }
      passed = ipass;
      ++icut;
    }
    return passed;
  }

  /**
   * @brief Fills the cut report.
   */
  void FillReport() {
    if (report_) {
      report_->Fill();
      // reset binvalues container before filling next event.
      auto offset = nchannels_*(cuts_.size() + 1);
      for (std::size_t i = 0; i < nchannels_; ++i) {
        for (std::size_t j = 0; j < (cuts_.size() + 1); ++j) {
          var_values_[2*offset + i + nchannels_*j] = 0;
        }
      }
    }
  }

  /**
   * @brief Initializes the cut report histogram
   * @param detname name of the histogram.
   * @param nchannels number of channels.
   */
  void CreateCutReport(std::string detname, std::size_t nchannels = 1) {
    if (!cuts_.empty()) {
      nchannels_ = nchannels;
      auto offset = nchannels*(cuts_.size() + 1);
      cut_i_ = Variable(0, offset);
      cut_channel_ = Variable(offset, offset);
      cut_weight_ = Variable(2*offset, offset);
      var_values_ = new double[3*offset];
      cut_weight_.var_container = var_values_;
      cut_i_.var_container = var_values_;
      cut_channel_.var_container = var_values_;
      for (std::size_t i = 0; i < nchannels; ++i) {
        for (std::size_t j = 0; j < (cuts_.size() + 1); ++j) {
          var_values_[i + nchannels*j] = j;
          var_values_[offset + i + nchannels*j] = i;
          var_values_[2*offset + i + nchannels*j] = 0;
        }
      }
      if (nchannels==1) {
        std::string name = detname + "Cut_Report";
        std::string title = std::string(";cuts;entries");
        auto nbins = cuts_.size() + 1;
        float low = 0.;
        float high = cuts_.size() + 1;
        auto histo = new TH1F(name.data(), title.data(), nbins, low, high);
        int icut = 2;
        histo->GetXaxis()->SetBinLabel(1, "all");
        for (auto &cut : cuts_) {
          histo->GetXaxis()->SetBinLabel(icut, cut->Name().data());
          ++icut;
        }
        std::array<Variable, 2> arr = {{cut_i_, cut_weight_}};
        report_ = std::make_unique<QAHisto1DPtr>(arr, histo);
      } else {
        std::string name = detname + "Cut_Report";
        std::string title = std::string(";cuts;channels");
        auto x_nbins = cuts_.size() + 1;
        auto y_nbins = nchannels_;
        float low = 0.;
        float x_high = cuts_.size() + 1;
        float y_high = nchannels_;
        auto histo = new TH2F(name.data(), title.data(), x_nbins, low, x_high, y_nbins, low, y_high);
        histo->GetXaxis()->SetBinLabel(1, "all");
        int icut = 2;
        for (auto &cut : cuts_) {
          histo->GetXaxis()->SetBinLabel(icut, cut->Name().data());
          ++icut;
        }
        std::array<Variable, 3> arr = {{cut_i_, cut_channel_, cut_weight_}};
        report_ = std::make_unique<QAHisto2DPtr>(arr, histo);
      }
    }
  }

  /**
   * @brief Adds the cut report to the list.
   * Lifetime of the list and the histogram has to be managed by the user.
   * @param list list containing output histograms.
   */
  void AddToList(TList *list) {
    if (report_) report_->AddToList(list);
  }

 private:
  std::size_t nchannels_ = 0; /// number of channels is zero in case of no report
  double *var_values_ = nullptr; /// pointer to the values which are filled to the histogram.
  Variable cut_i_; /// Variable of saving cut number
  Variable cut_weight_; /// Variable saving a weight used for filling the cut histogram
  Variable cut_channel_; /// Variable saving the channel number
  std::vector<std::unique_ptr<VariableCutBase>> cuts_; /// vector of cuts which are applied
  std::unique_ptr<QAHistoBase> report_ = nullptr; /// histogram of the cut report.

};

}

#endif //FLOW_VARIABLECUTBASE_H
