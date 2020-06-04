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

#include "Stats.h"

namespace Qn {
using STAT = Qn::Stats::State;

Stats MergeBins(const Stats &lhs, const Stats &rhs) {
  if (!rhs.mergeable_) throw std::logic_error("Cannot merge Stats. Please check prior operations.");
  Stats result;
  if ((rhs.state_ == STAT::MEAN_ERROR)) {
    auto temp_l = lhs;
    auto temp_r = rhs;
    double totalweight = 0.;
    if (lhs.weight_ == 0) {
      totalweight = temp_r.weight_;
    } else {
      totalweight = temp_l.weight_ + temp_r.weight_;
    }
    result.mean_ = (temp_l.weight_*temp_l.mean_ + temp_r.weight_*temp_r.mean_)/totalweight;
    auto error_1 = temp_l.weight_/totalweight*temp_l.error_;
    auto error_2 = temp_r.weight_/totalweight*temp_r.error_;
    result.error_ = std::sqrt(error_1*error_1 + error_2*error_2);
    result.weight_ = totalweight;
    bool merge_weights = (rhs.weights_flag==Qn::Stats::Weights::OBSERVABLE);
    result.resamples_ = ReSamples::Merge(temp_l.resamples_, temp_r.resamples_, merge_weights);
    result.state_ = STAT::MEAN_ERROR;
    result.bits_ = rhs.bits_;
    result.weights_flag = rhs.weights_flag;
    result.mergeable_ = rhs.mergeable_;
  } else {
    result.statistic_ = Qn::Merge(lhs.statistic_, rhs.statistic_);
    result.resamples_ = ReSamples::MergeStatistics(lhs.resamples_, rhs.resamples_);
    result.state_ = STAT::MOMENTS;
    result.bits_ = rhs.bits_;
    result.weights_flag = rhs.weights_flag;
    result.mergeable_ = rhs.mergeable_;
  }
  return result;
}
//
Stats Merge(const Stats &lhs, const Stats &rhs) {
  if (!lhs.mergeable_ || !rhs.mergeable_) throw std::logic_error("Cannot merge Stats. Please check prior operations.");
  Stats result;
  result.state_ = lhs.state_;
  result.bits_ = lhs.bits_;
  result.weights_flag = lhs.weights_flag;
  result.mergeable_ = lhs.mergeable_;
  if (lhs.TestBit(Qn::Stats::CONCATENATE_SUBSAMPLES)) {
    result.statistic_ = lhs.statistic_;
    result.resamples_ = ReSamples::Concatenate(lhs.resamples_, rhs.resamples_);
  } else {
    result.statistic_ = Qn::Merge(lhs.statistic_, rhs.statistic_);
    result.resamples_ = ReSamples::MergeStatistics(lhs.resamples_, rhs.resamples_);
  }
  return result;
}

Stats operator+(const Stats &lhs, const Stats &rhs) {
  Stats result;
  result.state_ = STAT::MEAN_ERROR;
  result.bits_ = lhs.bits_;
  auto tlhs = lhs;
  auto trhs = rhs;
  if (tlhs.state_!=STAT::MEAN_ERROR) tlhs.CalculateMeanAndError();
  if (trhs.state_!=STAT::MEAN_ERROR) trhs.CalculateMeanAndError();
  if (tlhs.weights_flag==Stats::Weights::OBSERVABLE) result.weight_ = tlhs.weight_;
  if (trhs.weights_flag==Stats::Weights::OBSERVABLE) result.weight_ = trhs.weight_;
  result.mean_ = tlhs.mean_ + trhs.mean_;
  result.error_ = std::sqrt(tlhs.error_*tlhs.error_ + trhs.error_*trhs.error_);
  result.resamples_ = ReSamples::Addition(tlhs.resamples_, trhs.resamples_);
  result.weights_flag = lhs.weights_flag;
  result.mergeable_ = false;
  return result;
}

Stats operator-(const Stats &lhs, const Stats &rhs) {
  Stats result;
  result.state_ = STAT::MEAN_ERROR;
  result.bits_ = lhs.bits_;
  auto tlhs = lhs;
  auto trhs = rhs;
  if (tlhs.state_!=STAT::MEAN_ERROR) tlhs.CalculateMeanAndError();
  if (trhs.state_!=STAT::MEAN_ERROR) trhs.CalculateMeanAndError();
  if (tlhs.weights_flag==Stats::Weights::OBSERVABLE) result.weight_ = tlhs.weight_;
  if (trhs.weights_flag==Stats::Weights::OBSERVABLE) result.weight_ = trhs.weight_;
  result.mean_ = tlhs.mean_ - trhs.mean_;
  result.error_ = std::sqrt(tlhs.error_*tlhs.error_ + trhs.error_*trhs.error_);
  result.resamples_ = ReSamples::Subtraction(tlhs.resamples_, trhs.resamples_);
  result.weights_flag = lhs.weights_flag;
  result.mergeable_ = false;
  return result;
}

Stats operator*(const Stats &lhs, const Stats &rhs) {
  Stats result;
  result.state_ = STAT::MEAN_ERROR;
  result.bits_ = lhs.bits_;
  auto tlhs = lhs;
  auto trhs = rhs;
  if (tlhs.state_!=STAT::MEAN_ERROR) tlhs.CalculateMeanAndError();
  if (trhs.state_!=STAT::MEAN_ERROR) trhs.CalculateMeanAndError();
  if (tlhs.weights_flag==Stats::Weights::OBSERVABLE && trhs.weights_flag==Stats::Weights::REFERENCE) {
    result.weight_ = tlhs.weight_;
  } else if (trhs.weights_flag==Stats::Weights::OBSERVABLE && tlhs.weights_flag==Stats::Weights::REFERENCE) {
    result.weight_ = trhs.weight_;
  } else if (trhs.weights_flag==Stats::Weights::OBSERVABLE && tlhs.weights_flag==Stats::Weights::OBSERVABLE) {
    result.mergeable_ = false;
  }
  result.mean_ = tlhs.mean_*trhs.mean_;
  auto t1 = trhs.mean_*tlhs.error_;
  auto t2 = tlhs.mean_*trhs.error_;
  result.error_ = std::sqrt(t1*t1 + t2*t2);
  result.resamples_ = ReSamples::Multiplication(tlhs.resamples_, trhs.resamples_);
  return result;
}

Stats operator*(const Stats &stat, const double scale) {
  Stats result = stat;
  if (result.state_!=STAT::MEAN_ERROR) result.CalculateMeanAndError();
  result.mean_ *= scale;
  result.error_ *= scale;
  result.resamples_ = ReSamples::Scaling(result.resamples_, scale);
  return result;
}

Stats operator*(const Stats &stat, const std::pair<double, double> scale_error) {
  Stats result = stat;
  if (result.state_!=STAT::MEAN_ERROR) result.CalculateMeanAndError();
  result.mean_ *= scale_error.first;
  auto t1 = scale_error.first*stat.error_;
  auto t2 = stat.mean_*scale_error.second;
  result.error_ = std::sqrt(t1*t1 + t2*t2);
  result.resamples_ = ReSamples::Scaling(result.resamples_, scale_error.first);
  return result;
}

Stats operator*(const double scale, const Stats &stat) {
  Stats result = stat;
  if (result.state_!=STAT::MEAN_ERROR) result.CalculateMeanAndError();
  result.mean_ *= scale;
  result.error_ *= scale;
  result.resamples_ = ReSamples::Scaling(result.resamples_, scale);
  return result;
}

Stats operator/(const Stats &stat, const double scale) {
  Stats result = stat;
  if (scale==0) return result;
  if (result.state_!=STAT::MEAN_ERROR) result.CalculateMeanAndError();
  result.mean_ /= scale;
  result.error_ /= scale;
  result.resamples_ = ReSamples::Scaling(result.resamples_, 1./scale);
  return result;
}

Stats operator/(const Stats &num, const Stats &den) {
  Stats result;
  result.state_ = STAT::MEAN_ERROR;
  result.bits_ = num.bits_;
  auto tlhs = num;
  auto trhs = den;
  if (tlhs.state_!=STAT::MEAN_ERROR) tlhs.CalculateMeanAndError();
  if (trhs.state_!=STAT::MEAN_ERROR) trhs.CalculateMeanAndError();
  if (tlhs.weights_flag==Stats::Weights::OBSERVABLE && trhs.weights_flag==Stats::Weights::REFERENCE) {
    result.weight_ = tlhs.weight_;
  } else if (trhs.weights_flag==Stats::Weights::OBSERVABLE && tlhs.weights_flag==Stats::Weights::REFERENCE) {
    result.weight_ = trhs.weight_;
  } else if (trhs.weights_flag==Stats::Weights::OBSERVABLE && tlhs.weights_flag==Stats::Weights::OBSERVABLE) {
    result.mergeable_ = false;
  }
  result.mean_ = tlhs.mean_/trhs.mean_;
  double denominator_mean;
  if (trhs.mean_ != 0.) {
    denominator_mean = trhs.mean_;
  } else {
    denominator_mean = 1.;
  }
  auto t1 = tlhs.error_ / denominator_mean;
  auto t2 = tlhs.mean_*trhs.error_ / (denominator_mean*denominator_mean);
  result.error_ = std::sqrt(t1*t1+t2*t2);
  result.resamples_ = ReSamples::Division(tlhs.resamples_, trhs.resamples_);
  return result;
}

Stats Abs(const Stats &stat) {
  Stats result = stat;
  if (result.state_!=STAT::MEAN_ERROR) result.CalculateMeanAndError();
  auto temp_mean = result.mean_;
  result.mean_ = std::fabs(temp_mean);
  result.resamples_ = ReSamples::Abs(result.resamples_);
  return result;
}

Stats Sqrt(const Stats &stat) {
  Stats result = stat;
  if (result.state_!=STAT::MEAN_ERROR) result.CalculateMeanAndError();
  auto temp_mean = result.mean_;
  result.mean_ =
      std::signbit(temp_mean) ? -1*std::sqrt(std::fabs(temp_mean)) : std::sqrt(std::fabs(temp_mean));
  result.error_ = stat.error_/(2*std::sqrt(temp_mean));
  result.resamples_ = ReSamples::Sqrt(result.resamples_);
  return result;
}

Stats PowSqrt(const Stats & stat, unsigned int k) {
  Stats result = stat;
  if (result.state_!=STAT::MEAN_ERROR) result.CalculateMeanAndError();
  auto temp_mean = result.mean_;
  result.mean_ = std::signbit(temp_mean) ? -1*std::pow(std::fabs(temp_mean), 1./k) : std::pow(std::fabs(temp_mean), 1./k);
  result.error_  = stat.error_ * std::pow(temp_mean, 1./(k - 1)) / k;
  result.resamples_ = ReSamples::PowSqrt(result.resamples_, k);
  return result;
}

TCanvas *Stats::CIvsNSamples(const int nsteps) const {
  auto resamples(resamples_);
  resamples.CalculateMeans();
  auto pivot = resamples.CIvsNSamples(statistic_.Mean(), Qn::ReSamples::CIMethod::pivot, nsteps);
  auto percentile = resamples.CIvsNSamples(statistic_.Mean(), Qn::ReSamples::CIMethod::percentile, nsteps);
  auto normal = resamples.CIvsNSamples(statistic_.Mean(), Qn::ReSamples::CIMethod::normal, nsteps);
  auto statistical = new TGraphAsymmErrors(2);
  statistical->SetPoint(0, 0, statistic_.Mean());
  statistical->SetPointError(0, 0, 0, statistic_.MeanError(), statistic_.MeanError());
  statistical->SetPoint(1, resamples.size(), statistic_.Mean());
  statistical->SetPointError(1, 0, 0, statistic_.MeanError(), statistic_.MeanError());
  auto canvas = new TCanvas("CIvsNSamples", "CI", 600, 400);
  statistical->Draw("AL3");
  statistical->SetFillColorAlpha(kBlack, 0.2);
  statistical->SetLineWidth(2);
  statistical->SetLineColorAlpha(kBlack, 0.4);
  statistical->GetYaxis()->SetRangeUser((statistic_.Mean() - statistic_.MeanError()*2),
                                        (statistic_.Mean() + statistic_.MeanError()*2));
  statistical->GetXaxis()->SetRangeUser(0, resamples.size());
  statistical->SetNameTitle("", ";number of bootstrap samples; x");

  auto draw_ci = [](std::pair<TGraph *, TGraph *> &pair, int color) {
    pair.first->SetLineWidth(2);
    pair.second->SetLineWidth(2);
    pair.first->SetLineColorAlpha(color, 0.8);
    pair.second->SetLineColorAlpha(color, 0.8);
    pair.first->Draw("L");
    pair.second->Draw("L");
  };
  draw_ci(pivot, kRed);
  draw_ci(percentile, kGreen + 2);
  draw_ci(normal, kBlue);

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

}
