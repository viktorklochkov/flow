#include <utility>

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

#ifndef FLOW_CORRELATION_H
#define FLOW_CORRELATION_H

#include <utility>

#include "DataContainer.h"
#include "QVector.h"
#include "Sampler.h"

namespace Qn {
/**
 * @brief      Correlation of several input data containers.
 */

enum class Weight {
  REFERENCE,
  OBSERVABLE
};

class Correlation {
  using size_type = std::size_t;
  using AXES = std::vector<Qn::Axis>;
  using DataContainers = std::vector<DataContainerQVector *>;
  using FUNCTION = std::function<double(const std::vector<Qn::QVector> &)>;
 public:
  Correlation() = default;

  Correlation(std::vector<std::string> names,
              FUNCTION lambda,
              bool use_resampling) :
      use_resampling_(use_resampling),
      function_(std::move(lambda)),
      names_(std::move(names)) {
    use_weights_.resize(names_.size());
    use_weights_[0] = true;
  }

  void ConfigureCorrelation(const DataContainers &input, const std::vector<Qn::Axis> &event, Qn::Sampler* sampler);

  void SetWeights(std::vector<Qn::Weight> weights) {
    for (size_type iw = 0; iw < use_weights_.size(); ++iw) {
      use_weights_[iw] = weights[iw]==Qn::Weight::OBSERVABLE;
    }
  }

  std::vector<std::string> &GetInputNames() { return names_; }

  DataContainerStats GetResult() const { return result_; }

  std::vector<DataContainerQVector *> &GetInputs() { return inputs_; }

 private:
  bool use_histo_result_ = false;
  bool use_resampling_ = false;

  Qn::Sampler *sampler_ = nullptr;

  std::vector<std::vector<std::vector<size_type>>> index_; ///< map of indices of all inputs
  std::vector<size_type> c_index_; ///< flattened index of correlation
  DataContainerProduct current_event_result_;
  DataContainerStats result_;
  DataContainer<TH1F> histo_result_;
  AXES axes_event_; ///< vector of event axes used in the correlation
  FUNCTION function_; ///< correlation function
  std::vector<std::string> names_; ///< vector of input names
  std::vector<bool> use_weights_;
  std::vector<QVector> qvectors_;
  std::vector<DataContainerQVector *> inputs_;

 public:

/**
 * Fill Correlation container with specified inputs.
 * Recursive function called N times, where N is the number of correlated datacontainers.
 * @param input vector of all input containers in correlation
 * @param eventindices of the used for the event axes
 * @param lambda correlation function
 */
  void Fill(const std::vector<unsigned long> &eventindices, std::size_t event_id);

/**
 *
 * @param initial_offset
 * @param n
 * @param event_id
 */
  void FillCorrelation(size_type initial_offset,
                       unsigned int n,
                       std::size_t event_id);

/**
 *
 * @param iterationoffset
 * @param iteration
 * @param event_id
 */
  void FillCorrelationNoResampling(size_type initial_offset,
                                   unsigned int n,
                                   std::size_t event_id);
};

}

#endif //FLOW_CORRELATION_H
