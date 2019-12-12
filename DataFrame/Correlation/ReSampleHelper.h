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
#ifndef FLOW_RESAMPLEHELPER_H_
#define FLOW_RESAMPLEHELPER_H_

#include <random>
#include <vector>
#include <algorithm>

#include "RtypesCore.h"
#include "ROOT/RVec.hxx"

namespace Qn {
namespace Correlation {

/**
 * Class for creating samples in the DataFrame using
 */
template<typename DataFrame>
class ReSampleHelper {
 public:
  /**
   * Constructor
   * @param df Dataframe to which the samples are added.
   * @param n number of samples which are to be used for error estimation
   */
  ReSampleHelper(DataFrame df, std::size_t n=100) :
      dataframe_(df),
      n_(n),
      generator_(std::random_device{}()),
      poisson_(1) {
  }

  /**
   * Defines the tiems a single event enters in the different samples.
   * This function is called for each event during the event loop.
   * @return returns the vector of multiplicity for the different samples.
   */
  ROOT::RVec<ULong64_t> operator()() {
    ROOT::RVec<ULong64_t> vec(n_);
    for (auto &entry : vec) { entry = poisson_(generator_); }
    return vec;
  }

  /**
   * Adds the samples to the RDataFrame and returns the resulting dataframe.
   * @return The DataFrame containing the definitions.
   */
  auto Define() { return dataframe_.Define("samples", *this, {}); }

 private:
  DataFrame dataframe_; /// Input data
  const std::size_t n_; /// Number of samples
  std::mt19937 generator_; /// Random number generator
  std::poisson_distribution<> poisson_; /// distribution of events per sample.
};

/**
 * Helper function to add the samples to the RDataFrame.
 * The Event loop is lazily executed as soon as the "samples" branch information is consumed by a correlation.
 * @tparam DataFrame type of the RDataFrame.
 * @param df RDataFrame wrapping the input data.
 * @param n Number of samples.
 * @return The resulting RDataFrame with the sample definition.
 */
template <typename DataFrame>
auto Resample(DataFrame df, std::size_t n) {
  return ReSampleHelper<DataFrame>(df,n).Define();
}

}
}
#endif //RESAMPLEHELPER_H_
