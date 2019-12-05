// Flow Vector Correction Framework
//
// Copyright (C) 2019  Lukas Kreis Ilya Selyuzhenkov
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
#ifndef FLOW_BASE_INCLUDE_EQUALENTRIESBINNER_H_
#define FLOW_BASE_INCLUDE_EQUALENTRIESBINNER_H_

#include <vector>
#include <numeric>
#include "Math/Interpolator.h"
namespace Qn {
class EqualEntriesBinner {
 public:
  std::vector<double> CalculateBins(std::vector<double> data, unsigned int nbins) {
    std::sort(data.begin(), data.end());
    auto npoints = data.size();
    std::vector<double> x(npoints);
    std::iota(x.begin(), x.end(), 0);
    ROOT::Math::Interpolator interpolator(x, data, ROOT::Math::Interpolation::Type::kLINEAR);
    auto evalat = linespace(0, npoints, nbins + 1);
    std::vector<double> bin_edges(nbins + 1);
    evalat[nbins] = evalat[nbins] - 1;
    for (unsigned int ibin = 0; ibin < nbins + 1; ++ibin) {
      bin_edges[ibin] = interpolator.Eval(evalat[ibin]);
    }
    double epsilon = 1e-5;
    bin_edges[nbins] = data.back() + epsilon;
    return bin_edges;
  }
  std::vector<double> CalculateBins(std::vector<double> data, unsigned int nbins, double low, double high) {
    std::sort(data.begin(), data.end());
    if (low < data.front()) { data.insert(data.begin(),low); }
    if (high > data.back()) { data.push_back(high); }
    auto npoints = data.size();
    auto lowiter = std::lower_bound(data.begin(), data.end(), low);
    auto highiter = std::lower_bound(data.begin(), data.end(), high);
    auto position_low = std::distance(data.begin(), lowiter);
    auto position_high = std::distance(data.begin(), highiter);
    std::vector<double> x(npoints);
    std::iota(x.begin(), x.end(), 0);
    ROOT::Math::Interpolator interpolator(x, data, ROOT::Math::Interpolation::Type::kLINEAR);
    auto evalat = linespace(position_low, position_high, nbins + 1);
    std::vector<double> bin_edges(nbins + 1);
    for (unsigned int ibin = 0; ibin < nbins + 1; ++ibin) {
      bin_edges[ibin] = interpolator.Eval(evalat[ibin]);
    }
    bin_edges.front() = low;
    bin_edges.back() = high;
    return bin_edges;
  }

 private:
  // Linear interpolation following MATLAB linspace
  std::vector<double> linespace(double start, double ed, int num) {
    int partitions = num - 1;
    std::vector<double> pts;
    // length of each segment
    double length = (ed - start)/partitions;
    // first, not to change
    pts.push_back(start);
    for (int i = 1; i < num - 1; i++) {
      pts.push_back(start + i*length);
    }
    // last, not to change
    pts.push_back(ed);
    return pts;
  }
};
}

#endif //FLOW_BASE_INCLUDE_EQUALENTRIESBINNER_H_
