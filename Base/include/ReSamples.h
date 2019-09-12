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

#ifndef FLOW_RESAMPLES_H
#define FLOW_RESAMPLES_H

#include <utility>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <random>

#include "TGraphAsymmErrors.h"

#include "CorrelationResult.h"
#include "Statistic.h"

namespace Qn {

struct SampleMean {
  double mean;
  double weight;
};

struct ConfidenceInterval {
  double lower_limit;
  double upper_limit;
  double Uncertainty() const {
    return (upper_limit - lower_limit) / 2.;
  }
};

class ReSamples {
  using size_type = std::size_t;
 public:

  enum class CIMethod {
    percentile,
    pivot,
    normal
  };

  ReSamples() = default;
  explicit ReSamples(unsigned int size) : statistics_(size), means_(size) {}
  virtual ~ReSamples() = default;

  void SetNumberOfSamples(unsigned int i) {
    statistics_.resize(i);
    means_.resize(i);
  }

  const SampleMean &GetSampleMean(int i) const {
    return means_.at(i);
  }

  size_type size() const { return statistics_.size(); }

  ConfidenceInterval GetConfidenceInterval(const double mean, CIMethod method) const {
    return ConstructConfidenceInterval(means_, mean, method);
  }

  void Fill(const CorrelationResult &result, const std::vector<size_type> &sample_ids) {
    for (const auto &id : sample_ids) { statistics_[id].Fill(result); }
  }

  void FillPoisson(const CorrelationResult &result, const std::vector<size_type> &sample_multiplicities_) {
    for (unsigned int i = 0; i < sample_multiplicities_.size(); ++i) {
      for (unsigned int j = 0; j < sample_multiplicities_[i]; ++j) {
        statistics_[i].Fill(result);
      }
    }
  }

  void FillSample(const CorrelationResult &result, unsigned int sample) {
    statistics_[sample].Fill(result);
  }

  void CalculateMeans() {
    unsigned int i = 0;
    for (auto &statistic : statistics_) {
      double mean = statistic.Mean();
      double weight = statistic.SumWeights();
      SampleMean sm = {mean, weight};
      means_.at(i) = sm;
      ++i;
    }
  }

  std::vector<double> GetMeans() const {
    std::vector<double> means(means_.size());
    std::transform(means_.begin(), means_.end(), means.begin(), [](const SampleMean &mean) { return mean.mean; });
    return means;
  }

  std::pair<TGraph *, TGraph *> CIvsNSamples(double mean, CIMethod method, unsigned int nsteps = 10) const {
    auto graphup = new TGraph(nsteps);
    auto graphlo = new TGraph(nsteps);
    int step = means_.size()/nsteps;
    for (unsigned int i = 0; i < nsteps; ++i) {
      auto ci = ConfidenceIntervalNSamplesMethod(mean, step*(i + 1), method);
      graphlo->SetPoint(i, (i + 1)*step, ci.lower_limit);
      graphup->SetPoint(i, (i + 1)*step, ci.upper_limit);
    }
    return std::make_pair(graphlo, graphup);
  }

  void ScatterGraph(TGraph &graph, double offset, double width) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> position(offset - width, offset + width);
    for (auto mean : means_) {
      graph.SetPoint(graph.GetN(), position(gen), mean.mean);
    }
  }

  static ReSamples Addition(const ReSamples &, const ReSamples &);

  static ReSamples Subtraction(const ReSamples &, const ReSamples &);

  static ReSamples Division(const ReSamples &, const ReSamples &);

  static ReSamples Multiplication(const ReSamples &, const ReSamples &);

  static ReSamples Scaling(const ReSamples &, double);

  static ReSamples Sqrt(const ReSamples &);

  static ReSamples Merge(const ReSamples &, const ReSamples &, bool);

  static ReSamples Concatenate(const ReSamples &, const ReSamples &);

 private:

  ConfidenceInterval ConfidenceIntervalNSamplesMethod(const double mean,
                                                      const size_type nsamples,
                                                      CIMethod method = CIMethod::pivot) const {
    std::vector<SampleMean> bsmeans = means_;
    bsmeans.resize(nsamples);
    return ConstructConfidenceInterval(bsmeans, mean, method);
  }

  inline ConfidenceInterval ConstructConfidenceInterval(const std::vector<SampleMean> &means,
                                                 const double mean,
                                                 CIMethod method) const {
    ConfidenceInterval interval{};
    switch (method) {
      case CIMethod::percentile :interval = ConfidenceIntervalPercentile(means);
        break;
      case CIMethod::pivot :interval = ConfidenceIntervalPivot(means, mean);
        break;
      case CIMethod::normal :interval = ConfidenceIntervalNormal(means, mean);
        break;
    }
    return interval;
  }

  ConfidenceInterval ConfidenceIntervalPercentile(std::vector<SampleMean>) const;

  ConfidenceInterval ConfidenceIntervalPivot(std::vector<SampleMean>, double) const;

  ConfidenceInterval ConfidenceIntervalNormal(std::vector<SampleMean>, double) const;

  std::vector<Statistic> statistics_;
  std::vector<SampleMean> means_;

  /// \cond CLASSIMP
 ClassDef(ReSamples, 1);
  /// \endcond

};

}

#endif //FLOW_SAMPLE_H
