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
#include <iostream>

#include "Rtypes.h"
#include "TH1F.h"

#include "Profile.h"
#include "Product.h"

namespace Qn {

struct StatisticMean {
  double sumwy = 0;
  double sumw = 0;
  int entries = 0;

  void Fill(const Product &prod) {
    sumwy += prod.result;
    sumw += prod.GetProdWeight();
    ++entries;
  }

  double Mean() const { if (sumw > 0.) { return sumwy/sumw; } else { return sumwy; }; }

  void operator+=(StatisticMean b) {
    sumwy += b.sumwy;
    sumw  += b.sumw;
  }

  void operator-=(StatisticMean b) {
    sumwy -= b.sumwy;
    sumw  -= b.sumw;
  }

  void operator*=(StatisticMean b) {
    sumwy *= b.sumwy;
    sumw *= b.sumw;
  }

  void operator/=(StatisticMean b) {
    sumwy /= b.sumwy;
    sumw /= b.sumw;
  }

  void merge(StatisticMean b) {
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

class Sample {
 public:
  using size_type = std::size_t;
  Sample() = default;

  virtual ~Sample() = default;

  explicit Sample(std::size_t n_samples) { samples_.resize(n_samples); }

  explicit Sample(std::vector<StatisticMean> means) :
      samples_(std::move(means)) {}

  void Fill(const Product &product, const std::vector<size_type> &samples) {
    for (auto &sample : samples) {
      samples_.at(sample).Fill(product);
    }
  }

  void SetNumberOfSamples(size_type nsamples) { samples_.resize(nsamples); }

  void Print(double real_mean);

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

  double ErrorHi(double real_mean) const {
    std::vector<StatisticMean> means = samples_;
    return fabs(quantiles(means, real_mean, 0.84135,1));
  }

  double ErrorLo(double real_mean) const {
    std::vector<StatisticMean> means = samples_;
    return fabs(quantiles(means, real_mean, 0.15865,0));
  }

  TH1F SampleMeanHisto(std::string name) {
    auto means = samples_;
    std::sort(means.begin(),
              means.end(),
              [](StatisticMean a, StatisticMean b) { return (a.Mean()) > (b.Mean()); });
    TH1F histo(name.data(), name.data(), 50, means.begin()->Mean(), (means.end() - 1)->Mean());
    for (auto sample : samples_) {
      histo.Fill(sample.Mean());
    }
    return histo;
  }

  friend Sample operator+(const Sample &, const Sample &);
  friend Sample operator-(const Sample &, const Sample &);
  friend Sample operator*(const Sample &, const Sample &);
  friend Sample operator/(const Sample &, const Sample &);
  friend Sample operator*(const Sample &, double);
  friend Sample Sqrt(const Sample &);
  friend Sample Merge(const Sample &, const Sample &);

 private:
  std::vector<StatisticMean> samples_;
  template<typename T>
  double quantiles(std::vector<T> &quantiles, double rmean, double pos, int  off=0) const {
    auto const quant = quantiles.size()*pos;
    std::nth_element(quantiles.begin(),
                     quantiles.begin() + quant,
                     quantiles.end(),
                     [rmean](T a, T b) { return (a.Mean() - rmean) > (b.Mean() - rmean); });
    return quantiles[quant+off].Mean() - rmean;
  }
  /// \cond CLASSIMP
 ClassDef(Sample, 3);
  /// \endcond
};

}

#endif //FLOW_SAMPLE_H
