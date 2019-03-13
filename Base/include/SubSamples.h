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
#include <vector>
#include <algorithm>
#include <iostream>

#include "Rtypes.h"
#include "TH1F.h"

#include "Profile.h"
#include "Product.h"

namespace Qn {

struct Sample {
  double sumwy = 0;
  double sumw = 0;
  int entries = 0;

  void Fill(const Product &prod) {
    sumwy += prod.result*prod.GetWeight();
    sumw += prod.GetWeight();
    ++entries;
  }

  double Mean() const { if (sumw > 0.) { return sumwy/sumw; } else { return sumwy; }; }

  void operator+=(Sample b) {
    sumwy += b.sumwy;
    sumw += b.sumw;
  }

  void operator-=(Sample b) {
    sumwy -= b.sumwy;
    sumw -= b.sumw;
  }

  void operator*=(Sample b) {
    sumwy *= b.sumwy;
    sumw *= b.sumw;
  }

  void operator/=(Sample b) {
    sumwy /= b.sumwy;
    sumw /= b.sumw;
  }

  void merge(Sample b) {
    sumwy += b.sumwy;
    sumw += b.sumw;
  }

  void operator*=(double b) { sumwy *= b; }

  void Sqrt() {
    sumw = sqrt(sumw);
    sumwy = sqrt(sumwy);
  }

  void Print() {
    std::cout << "Sum{w x} " << sumwy << std::endl;
    std::cout << "Sum{w}   " << sumw << std::endl;
    std::cout << "Entries  " << entries << std::endl;
    std::cout << "Mean     " << Mean() << std::endl;
  }
};

/**
 * Compare two Samples based on mean. Used to find the quantiles of the subsamples.
 * @param lhs one sample
 * @param rhs the other sample
 * @return true is smaller, false if larger
 */
inline bool operator<(const Sample &lhs, const Sample &rhs) { return lhs.Mean() < rhs.Mean(); }

class SubSamples {
 public:
  using size_type = std::size_t;

  SubSamples() = default;

  explicit SubSamples(std::size_t n_samples) { samples_.resize(n_samples); }

  explicit SubSamples(std::vector<Sample> means) : samples_(std::move(means)) {}

  virtual ~SubSamples() = default;

  SubSamples(const SubSamples &subsample) : samples_(subsample.samples_) {}

  using iterator = typename std::vector<Sample>::iterator;
  using const_iterator = typename std::vector<Sample>::const_iterator;
  iterator begin() { return samples_.begin(); } ///< iterator for external use
  iterator end() { return samples_.end(); } ///< iterator for external use
  const_iterator begin() const { return samples_.cbegin(); } ///< iterator for external use
  const_iterator end() const { return samples_.cend(); } ///< iterator for external use

  void Fill(const Product &product, const std::vector<size_type> &samples) {
    for (auto &sample : samples) {
      samples_.at(sample).Fill(product);
    }
  }

  void SetNumberOfSamples(size_type nsamples) { samples_.resize(nsamples); }

  void Print(double real_mean);

  bool empty() const { return samples_.empty(); }

  size_type size() const { return samples_.size(); }

  double Mean() const {
    double ssum = 0;
    int sentries = 0;
    for (const auto &stat : samples_) {
      ssum += stat.Mean();
      sentries++;
    }
    if (sentries > 0) {
      return ssum/sentries;
    } else {
      return ssum;
    }
  }

  double ErrorHi(double mean) const {
    std::vector<Sample> temp_samples = samples_;
    return fabs(Quantiles(temp_samples, 0.84135, 1).Mean() - mean);
  }

  double ErrorLo(double mean) const {
    std::vector<Sample> temp_samples = samples_;
    return fabs(Quantiles(temp_samples, 0.15865, 0).Mean() - mean);
  }

  TH1F SubSampleMeanHisto(const std::string &name) {
    auto means = samples_;
    std::sort(means.begin(),
              means.end(),
              [](Sample a, Sample b) { return (a.Mean()) > (b.Mean()); });
    TH1F histo(name.data(), name.data(), 50, means.begin()->Mean(), (means.end() - 1)->Mean());
    for (auto sample : samples_) {
      histo.Fill(sample.Mean());
    }
    return histo;
  }

  static SubSamples MergeBinsNormal(const SubSamples &, const SubSamples &);
  static SubSamples MergeBinsPointAverage(const SubSamples &, const SubSamples &);

  static SubSamples MergeConcat(const SubSamples &, const SubSamples &);

  static SubSamples AdditionNormal(const SubSamples &, const SubSamples &);
  static SubSamples AdditionPointAverage(const SubSamples &, const SubSamples &);

  static SubSamples SubtractionNormal(const SubSamples &, const SubSamples &);
  static SubSamples SubtractionPointAverage(const SubSamples &, const SubSamples &);

  static SubSamples MultiplicationNormal(const SubSamples &, const SubSamples &);
  static SubSamples MultiplicationPointAverage(const SubSamples &, const SubSamples &);

  static SubSamples DivisionNormal(const SubSamples &, const SubSamples &);
  static SubSamples DivisionPointAverage(const SubSamples &, const SubSamples &);

  static SubSamples SqrtNormal(const SubSamples &);
  static SubSamples SqrtPointAverage(const SubSamples &);

  static SubSamples ScaleNormal(const SubSamples &, double);
  static SubSamples ScalePointAverage(const SubSamples &, double);

 private:
  std::vector<Sample> samples_;

  Sample Quantiles(std::vector<Sample> quantiles, double pos, size_type offset) const {
    size_type quant = quantiles.size()*pos;
    std::nth_element(quantiles.begin(), quantiles.begin() + quant, quantiles.end());
    return quantiles[quant + offset];
  }

  /// \cond CLASSIMP
 ClassDef(SubSamples, 3);
  /// \endcond
};

}

#endif //FLOW_SAMPLE_H
