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

#include "ReSamples.h"

namespace Qn {

ConfidenceInterval ReSamples::ConfidenceIntervalPercentile(std::vector<SampleMean> means) const {
  const double alpha = 0.3173;
  std::sort(means.begin(), means.end(), [](const SampleMean &a, const SampleMean &b) { return a.mean < b.mean; });
  auto lowerpos = std::round(means.size()*alpha/2.);
  auto upperpos = std::round(means.size()*(1 - alpha/2.));
  return ConfidenceInterval{means.at(lowerpos).mean, means.at(upperpos).mean};
}

ConfidenceInterval ReSamples::ConfidenceIntervalPivot(std::vector<SampleMean> means, const double real_mean) const {
  const double alpha = 0.3173;
  std::sort(means.begin(), means.end(), [](const SampleMean &a, const SampleMean &b) { return a.mean < b.mean; });
  auto upperpos = std::round(means.size()*alpha/2.);
  auto lowerpos = std::round(means.size()*(1 - alpha/2.));
  return ConfidenceInterval{2*real_mean - means.at(lowerpos).mean, 2*real_mean - means.at(upperpos).mean};
}

ConfidenceInterval ReSamples::ConfidenceIntervalNormal(std::vector<SampleMean> means, const double real_mean) const {
  Statistic stats;
  for (const auto &mean : means) {
    stats.Fill(mean.mean, 1.0);
  }
  auto stddev = std::sqrt(stats.Variance());
  return ConfidenceInterval{real_mean - stddev, real_mean + stddev};
}

ReSamples ReSamples::Addition(const ReSamples &a, const ReSamples &b) {
  ReSamples result = a;
  for (unsigned int i = 0; i < result.means_.size(); ++i) {
    result.means_[i].mean += b.means_[i].mean;
  }
  return result;
}

ReSamples ReSamples::Subtraction(const ReSamples &a, const ReSamples &b) {
  ReSamples result = a;
  for (unsigned int i = 0; i < result.means_.size(); ++i) {
    result.means_[i].mean -= b.means_[i].mean;
  }
  return result;
}
ReSamples ReSamples::Division(const ReSamples &a, const ReSamples &b) {
  ReSamples result = a;
  for (unsigned int i = 0; i < result.means_.size(); ++i) {
    result.means_[i].mean /= b.means_[i].mean;
  }
  return result;
}

ReSamples ReSamples::Multiplication(const ReSamples &a, const ReSamples &b) {
  ReSamples result = a;
  for (unsigned int i = 0; i < result.means_.size(); ++i) {
    result.means_[i].mean *= b.means_[i].mean;
  }
  return result;
}

ReSamples ReSamples::Scaling(const ReSamples &a, const double scale) {
  ReSamples result = a;
  for (auto &mean : result.means_) {
    mean.mean *= scale;
  }
  return result;
}

ReSamples ReSamples::Sqrt(const ReSamples &a) {
  ReSamples result = a;
  for (unsigned int i = 0; i < result.means_.size(); ++i) {
    auto root = sqrt(fabs(a.means_[i].mean));
    result.means_[i].mean = std::signbit(a.means_[i].mean) ? -1.*root : root;
  }
  return result;
}

ReSamples ReSamples::Merge(const ReSamples &a, const ReSamples &b, bool merge_weights) {
  ReSamples result = a;
  for (unsigned int i = 0; i < result.means_.size(); ++i) {
    auto a_mean = a.means_[i];
    auto b_mean = b.means_[i];
    result.means_[i].mean = (a_mean.mean*a_mean.weight + b_mean.mean*b_mean.weight)/(a_mean.weight + b_mean.weight);
    if (merge_weights) {
      result.means_[i].weight += b.means_[i].weight;
    }
  }
  return result;
}

ReSamples ReSamples::Concatenate(const ReSamples &a, const ReSamples &b) {
  ReSamples result = a;
  result.means_.insert(result.means_.end(), b.means_.begin(), b.means_.end());
  result.statistics_.insert(result.statistics_.end(), b.statistics_.begin(), b.statistics_.end());
  return result;
}

}