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

#ifndef FLOW_STATS_H
#define FLOW_STATS_H

#include <vector>
#include <iostream>
#include <bitset>
#include <TLegend.h>

#include "Rtypes.h"

#include "Statistic.h"
#include "ReSamples.h"
#include "TCanvas.h"
#include "TGraphAsymmErrors.h"
#include "TAxis.h"


#include "Types.h"

namespace Qn {

class Stats {
 public:

  enum Settings {
    CORRELATEDERRORS = BIT(16),
    CONCATENATE_SUBSAMPLES = BIT(17),
    ASYMMERRORS = BIT(18)
  };

  enum class State {
    MOMENTS,
    MEAN_ERROR
  };

  enum class Weights {
    REFERENCE,
    OBSERVABLE
  };

  using size_type = std::size_t;

  Stats() = default;
  virtual ~Stats() = default;

  double N() const { return statistic_.N(); }

  double Neff() const { return statistic_.Neff(); }

  double SumWeights() const { return statistic_.SumWeights(); }

  double Mean() const {
    switch (state_) {
      case State::MOMENTS :return statistic_.Mean();
        break;
      case State::MEAN_ERROR :return mean_;
        break;
    }
  }

  double Error() const {
    if (bits_ & Settings::CORRELATEDERRORS) {
      return resamples_.GetConfidenceInterval(mean_,ReSamples::CIMethod::pivot).Uncertainty();
    } else {
      switch (state_) {
        case State::MOMENTS :return statistic_.MeanError();
          break;
        case State::MEAN_ERROR :return error_;
          break;
      }
    }
  }

  void CalculateMeanAndError() {
    state_ = State::MEAN_ERROR;
    mergeable_ = false;
    mean_ = statistic_.Mean();
    error_ = statistic_.MeanError();
    resamples_.CalculateMeans();
  }

  friend Stats Merge(const Stats &, const Stats &);
  friend Stats MergeBins(const Stats &, const Stats &);
  friend Stats operator+(const Stats &, const Stats &);
  friend Stats operator-(const Stats &, const Stats &);
  friend Stats operator*(const Stats &, const Stats &);
  friend Stats operator*(const Stats &, double);
  friend Stats operator/(const Stats &, double);
  friend Stats operator*(double, const Stats &);
  friend Stats operator/(const Stats &, const Stats &);
  friend Stats Sqrt(const Stats &);

  inline void Fill(const CorrelationResult &product, const std::vector<size_type> &samples) {
    if (product.validity) {
      resamples_.Fill(product, samples);
      statistic_.Fill(product);
    }
  }

  void SetNumberOfSubSamples(size_type nsamples) {
    resamples_.SetNumberOfSamples(nsamples);
  }

  void SetWeights(Weights weights) { weights_flag = weights; }
  State GetWeights() const { return state_; }

  bool TestBit(unsigned int bit) const { return static_cast<bool>(bits_ & bit); }
  void SetBits(unsigned int bits) { bits_ = bits; }
  void ResetBits(unsigned int bits) { bits_ &= ~(bits & 0x00ffffff); }

  const Statistic &GetStatistic() const {return statistic_;}

  size_type GetNSamples() const { return resamples_.size(); }
  const ReSamples &GetReSamples() const { return resamples_; }

  TCanvas* CIvsNSamples(int nsteps = 10) const {
    auto pivot = resamples_.CIvsNSamples(statistic_.Mean(), Qn::ReSamples::CIMethod::pivot, nsteps);
    auto percentile = resamples_.CIvsNSamples(statistic_.Mean(), Qn::ReSamples::CIMethod::percentile,nsteps);
    auto normal = resamples_.CIvsNSamples(statistic_.Mean(), Qn::ReSamples::CIMethod::normal, nsteps);
    auto statistical = new TGraphAsymmErrors(2);
    statistical->SetPoint(0, 0, statistic_.Mean());
    statistical->SetPointError(0, 0, 0, statistic_.MeanError(), statistic_.MeanError());
    statistical->SetPoint(1, resamples_.size(), statistic_.Mean());
    statistical->SetPointError(1, 0, 0, statistic_.MeanError(), statistic_.MeanError());
    auto canvas = new TCanvas("CIvsNSamples", "CI", 600, 400);
    statistical->Draw("AL3");
    statistical->SetFillColorAlpha(kBlack, 0.2);
    statistical->SetLineWidth(2);
    statistical->SetLineColorAlpha(kBlack, 0.4);
    statistical->GetYaxis()->SetRangeUser((statistic_.Mean() - statistic_.MeanError()*2), (statistic_.Mean() + statistic_.MeanError()*2));
    statistical->GetXaxis()->SetRangeUser(0, resamples_.size());
    statistical->SetNameTitle("", ";number of bootstrap samples; x");
    auto style = [](std::pair<TGraph*, TGraph*> &pair, int color) {
      pair.first->SetLineWidth(2);
      pair.second->SetLineWidth(2);
      pair.first->SetLineColorAlpha(color, 0.8);
      pair.second->SetLineColorAlpha(color, 0.8);
    };
    style(pivot, kRed);
    pivot.first->GetYaxis()->SetRangeUser(0.8, 1.2);
    pivot.first->Draw("L");
    pivot.second->Draw("L");
    style(percentile, kGreen + 2);
    percentile.first->Draw("L");
    percentile.second->Draw("L");
    style(normal, kBlue);
    normal.first->Draw("L");
    normal.second->Draw("L");
    auto legend = new TLegend(0.15, 0.15, 0.3, 0.25);
    legend->AddEntry(statistical, "standard", "AL");
    legend->AddEntry(pivot.first, "bs pivot", "L");
    legend->AddEntry(percentile.first, "bs percentile", "L");
    legend->AddEntry(normal.first, "bs normal", "L");
    legend->SetFillStyle(4000);
    legend->SetLineWidth(0);
    legend->Draw();
    return canvas;
  }


 private:
  ReSamples resamples_;
  Statistic statistic_;
  unsigned int bits_ = 0 | Qn::Stats::CORRELATEDERRORS;
  State state_ = State::MOMENTS;
  Weights weights_flag = Weights::REFERENCE;
  bool mergeable_ = true;

  double mean_ = 0.;
  double error_ = 0.;
  double weight_ = 1.;

  /// \cond CLASSIMP
 ClassDef(Stats, 3);
  /// \endcond
};

Stats MergeBins(const Stats &, const Stats &);
Stats Merge(const Stats &, const Stats &);
Stats operator+(const Stats &, const Stats &);
Stats operator-(const Stats &, const Stats &);
Stats operator*(const Stats &, const Stats &);
Stats operator*(const Stats &, double);
Stats operator/(const Stats &, double);
Stats operator*(double, const Stats &);
Stats operator/(const Stats &, const Stats &);
Stats Sqrt(const Stats &);
}

#endif //FLOW_STATS_H