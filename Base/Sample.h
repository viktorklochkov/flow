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

#ifndef FLOW_SAMPLE_H
#define FLOW_SAMPLE_H

#include <utility>

#include "Profile.h"

namespace Qn {

struct StatisticMean {
  double mean = 0;
  double sum = 0;
  int n = 0;

  StatisticMean() = default;

  void Update(double value) {
    sum += value;
    n++;
    mean = sum/(float) n;
  }

  void operator+=(StatisticMean b) {
    sum += b.sum;
    n += b.n;
    mean = sum/(float) n;
  }

  void operator*=(StatisticMean b) {
    sum *= b.sum;
    n += b.n;
    mean *= b.mean;
  }

  void operator/=(StatisticMean b) {
    sum /= b.sum;
    n += b.n;
    mean /= b.mean;
  }

  void operator-=(StatisticMean b) {
    sum -= b.sum;
    n -= b.n;
    mean = sum/(float) n;
  }

  void operator*=(double b) {
    sum *= b;
    mean *= b;
  }
};

class Sample : public Profile {
 public:
  using size_type = std::size_t;
  Sample() = default;

  virtual ~Sample() = default;

  explicit Sample(std::size_t n_samples) { samples_stat_.resize(n_samples); }

  Sample(Profile a, std::vector<StatisticMean> means) :
      Profile(a),
      samples_stat_(std::move(means)) {}

  void Fill(const double value, const std::vector<size_type> &samples) {
    Profile::Update(value);
    for (const auto sample : samples) {
      samples_stat_[sample].Update(value);
    }
  }

  void SetNumberOfSamples(size_type nsamples) { samples_stat_.resize(nsamples); }

  inline  double SampleMean(size_type isample) const { return samples_stat_[isample].mean; }
//
  void CalculateCorrelatedError() {
    subsample_sum = 0;
    subsample_sum2_ = 0;
    subsample_entries_ = 0;
    for (const auto stat : samples_stat_) {
      subsample_sum += stat.mean;
      subsample_sum2_ += stat.mean*stat.mean;
      subsample_entries_++;
    }
    correlated_error_ = Statistics::Sigma(subsample_sum/subsample_entries_, subsample_sum2_, subsample_entries_);
  }

  inline double CorrelatedError() const { return correlated_error_; }
//
  friend Sample operator+(const Sample &a, const Sample &b);
  friend Sample operator-(const Sample &a, const Sample &b);
  friend Sample operator*(const Sample &a, const Sample &b);
  friend Sample operator/(const Sample &a, const Sample &b);
  friend Sample operator*(const Sample &a, double b);
  friend Sample Merge(const Sample &a, const Sample &b);
//
  inline Sample Sqrt() const {
    Sample a(*this);
    a.mean_ = std::sqrt(std::abs(mean_));
    a.sum_ = std::sqrt(std::abs(sum_));
    a.sum2_ = std::sqrt(std::abs(sum2_));
    a.binentries_ = std::sqrt(std::abs(binentries_));
//    a.error_ = 1./2.*a.error_/a.mean_;
    return a;
  }

 private:
  std::vector<StatisticMean> samples_stat_;
  int subsample_entries_ = 0;
  double subsample_sum = 0.;
  double subsample_sum2_ = 0.;
  double correlated_error_ = 0.;
  /// \cond CLASSIMP
 ClassDef(Sample, 1);
  /// \endcond
};

inline Sample operator+(const Sample &a, const Sample &b) {
  std::vector<StatisticMean> sums(a.samples_stat_);
  int i = 0;
  if (!b.samples_stat_.empty()) {
    for (auto &sum : sums) {
      sum += b.samples_stat_[i];
      ++i;
    }
  }
  Sample c(operator+((Profile) a, (Profile) b), sums);
  c.CalculateCorrelatedError();
  return c;
}

inline Sample operator-(const Sample &a, const Sample &b) {
  std::vector<StatisticMean> sums(a.samples_stat_);
  int i = 0;
  if (!b.samples_stat_.empty()) {
    for (auto &sum : sums) {
      sum -= b.samples_stat_[i];
      ++i;
    }
  }
  Sample c(operator-((Profile) a, (Profile) b), sums);
  c.CalculateCorrelatedError();
  return c;
}

inline Sample operator*(const Sample &a, const Sample &b) {
  std::vector<StatisticMean> sums(a.samples_stat_);
  int i = 0;
  if (!b.samples_stat_.empty()) {
    for (auto &sum : sums) {
      sum *= b.samples_stat_[i];
      ++i;
    }
  }
  Sample c(operator*((Profile) a, (Profile) b), sums);
  c.CalculateCorrelatedError();
  return c;
}

inline Sample operator/(const Sample &a, const Sample &b) {
  std::vector<StatisticMean> sums(a.samples_stat_);
  int i = 0;
  if (!b.samples_stat_.empty()) {
    for (auto &sum : sums) {
      sum /= b.samples_stat_[i];
      ++i;
    }
  }
  Sample c(operator/((Profile) a, (Profile) b), sums);
  c.CalculateCorrelatedError();
  return c;
}

inline Sample operator*(const Sample &a, double b) {
  std::vector<StatisticMean> sums(a.samples_stat_);
  int i = 0;
  for (auto &sum : sums) {
    sum *= b;
    ++i;
  }
  Sample c(operator*((Profile) a, b), sums);
  c.CalculateCorrelatedError();
  return c;
}

inline Sample Merge(const Sample &a, const Sample &b) {
  return a + b;
}

}

#endif //FLOW_SAMPLE_H
