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
#include "../Correlation/include/Correlation.h"

namespace Qn {
using STAT = Qn::Stats::State;

Stats MergeBins(const Stats &lhs, const Stats &rhs) {
  if (!lhs.mergeable_) throw std::logic_error("Cannot merge Stats. Please check prior operations.");
  Stats result;
  auto temp_l = lhs;
  auto temp_r = rhs;
  if (lhs.state_==STAT::MOMENTS) {
    temp_l.CalculateMeanAndError();
    temp_r.CalculateMeanAndError();
  }
  result.mean_ = (temp_l.weight_*temp_l.mean_ + temp_r.weight_*temp_r.mean_)/(temp_l.weight_ + temp_r.weight_);
  result.weight_ = temp_l.weight_ + temp_r.weight_;
  bool merge_weights = (lhs.weights_flag==Qn::Stats::Weights::OBSERVABLE);
  result.resamples_ = ReSamples::Merge(temp_l.resamples_, temp_r.resamples_, merge_weights);
  result.state_ = STAT::MEAN_ERROR;
  result.bits_ = lhs.bits_;
  result.weights_flag = lhs.weights_flag;
  result.mergeable_ = lhs.mergeable_;
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
    result.statistic_ = Statistic::Merge(lhs.statistic_, rhs.statistic_);
    bool merge_weights = (lhs.weights_flag==Qn::Stats::Weights::OBSERVABLE);
    result.resamples_ = ReSamples::Merge(lhs.resamples_, rhs.resamples_, merge_weights);
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
  result.error_ = std::sqrt(tlhs.error_*tlhs.error_ + trhs.error_*trhs.error_);
  result.resamples_ = ReSamples::Multiplication(tlhs.resamples_, trhs.resamples_);
  return result;
}
//
Stats operator*(const Stats &stat, const double scale) {
  Stats result = stat;
  if (result.state_!=STAT::MEAN_ERROR) result.CalculateMeanAndError();
  result.mean_ *= scale;
  result.error_ *= scale;
  result.resamples_ = ReSamples::Scaling(result.resamples_, scale);
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
  result.error_ = std::sqrt(tlhs.error_*tlhs.error_ + trhs.error_*trhs.error_);
  result.resamples_ = ReSamples::Division(tlhs.resamples_, trhs.resamples_);
  return result;
}

Stats Sqrt(const Stats &stat) {
  Stats result = stat;
  if (result.state_!=STAT::MEAN_ERROR) result.CalculateMeanAndError();
  result.mean_ =
      std::signbit(result.mean_) ? -1*std::sqrt(std::fabs(result.mean_)) : std::sqrt(std::fabs(result.mean_));
  result.error_ = stat.error_/(2*std::sqrt(result.mean_));
  result.resamples_ = ReSamples::Sqrt(result.resamples_);
  return result;
}

}