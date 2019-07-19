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

#ifndef FLOW_CORRECTIONCUTS_H
#define FLOW_CORRECTIONCUTS_H

#include <array>
#include <vector>
#include <functional>

#include "ROOT/RMakeUnique.hxx"
#include "ROOT/RIntegerSequence.hxx"

#include "InputVariableManager.h"
#include "QAHistogram.h"
#include "Cuts.h"

namespace Qn {
/**
 * Manages cuts class and allows checking if the current variables passes the cut
 */
class CorrectionCuts {
 public:
  using CutCallBack = std::function<std::unique_ptr<CutBase>(Qn::InputVariableManager*)>;
  ~CorrectionCuts() { delete[] var_values_; }
  /**
   * @brief Adds a cut to the manager.
   * @param cut pointer to the cut.
   */
  void AddCut(std::unique_ptr<CutBase> cut) {
    cuts_.push_back(std::move(cut));
  }
  void AddCutCallBack(CutCallBack callback) {
    cuts_callback_.push_back(callback);
  }

  void InitializeCuts(Qn::InputVariableManager *var) {
    for (auto & proto : cuts_callback_) {
      cuts_.push_back(proto(var));
    }
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
    ++*((cut_weight_).begin() + i);
    bool passed = true;
    for (auto &cut : cuts_) {
      bool ipass = cut->Check(i) && passed;
      if (ipass) {
        ++*cut_weight_.at(i + n_channels_*icut);
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
      auto offset = n_channels_*(cuts_.size() + 1);
      for (std::size_t i = 0; i < n_channels_; ++i) {
        for (std::size_t j = 0; j < (cuts_.size() + 1); ++j) {
          var_values_[2*offset + i + n_channels_*j] = 0;
        }
      }
    }
  }

  /**
   * @brief Initializes the cut report histogram
   * @param report_name name of the histogram.
   * @param n_channels number of channels.
   */
  void CreateCutReport(const std::string& report_name, std::size_t n_channels = 1) {
    if (!cuts_.empty()) {
      n_channels_ = n_channels;
      auto offset = n_channels_*(cuts_.size() + 1);
      cut_number = InputVariable(0, offset);
      cut_channel_ = InputVariable(offset, offset);
      cut_weight_ = InputVariable(2*offset, offset);
      var_values_ = new double[3*offset];
      cut_weight_.values_container_ = var_values_;
      cut_number.values_container_ = var_values_;
      cut_channel_.values_container_ = var_values_;
      for (std::size_t i = 0; i < n_channels_; ++i) {
        for (std::size_t j = 0; j < (cuts_.size() + 1); ++j) {
          var_values_[i + n_channels_*j] = j;
          var_values_[offset + i + n_channels_*j] = i;
          var_values_[2*offset + i + n_channels_*j] = 0;
        }
      }
      if (n_channels_==1) {
        std::string name = report_name + "Cut_Report";
        std::string title(";cuts;entries");
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
        std::array<InputVariable, 2> arr = {{cut_number, cut_weight_}};
        report_ = std::make_unique<QAHisto1DPtr>(arr, histo);
      } else {
        std::string name = report_name + "Cut_Report";
        std::string title(";cuts;channels");
        auto x_nbins = cuts_.size() + 1;
        auto y_nbins = n_channels_;
        float low = 0.;
        float x_high = cuts_.size() + 1;
        float y_high = n_channels_;
        auto histo = new TH2F(name.data(), title.data(), x_nbins, low, x_high, y_nbins, low, y_high);
        histo->GetXaxis()->SetBinLabel(1, "all");
        int icut = 2;
        for (auto &cut : cuts_) {
          histo->GetXaxis()->SetBinLabel(icut, cut->Name().data());
          ++icut;
        }
        std::array<InputVariable, 3> arr = {{cut_number, cut_channel_, cut_weight_}};
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
  std::size_t n_channels_ = 0; /// number of channels is zero in case of no report
  double *var_values_ = nullptr; /// pointer to the values which are filled to the histogram.
  InputVariable cut_number; /// Variable of saving cut number
  InputVariable cut_weight_; /// Variable saving a weight used for filling the cut histogram
  InputVariable cut_channel_; /// Variable saving the channel number
  std::vector<std::unique_ptr<CutBase>> cuts_; /// vector of cuts which are applied
  std::vector<CutCallBack> cuts_callback_; /// vector of cuts which are applied
  std::unique_ptr<QAHistoBase> report_; /// histogram of the cut report.

};

}

#endif //FLOW_CORRECTIONCUTS_H
