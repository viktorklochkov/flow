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

enum class Weight {
  REFERENCE,
  OBSERVABLE
};

/**
 * @brief Correlation of several input Q-Vector DataContainers.
 */
class Correlation {

  using size_type = std::size_t;
  using AXES = std::vector<Qn::Axis>;
  using INPUTS = std::vector<DataContainerQVector *>;

 public:
  using CorrelationFunction = std::function<double(const std::vector<Qn::QVectorPtr> &)>;

  /**
   * @brief Constructs the Correlation step before information of the Q-Vectors is read from the tree.
   * @param names input Q-Vector names.
   * @param lambda correlation function
   * @param use_resampling enable resampling (configured in the CorrelationManager).
   */
  Correlation(std::vector<std::string> names, CorrelationFunction lambda, std::vector<Qn::Weight> use_weights, bool use_resampling)
      :
      use_resampling_(use_resampling),
      function_(std::move(lambda)),
      names_(std::move(names)),
      use_weights_(use_weights) {
  }

  /**
   * @brief Configure the correlation using the Q-Vector inputs read from the tree.
   * @param input Q-Vector inputs
   * @param event Event variables used for this correlation
   * @param sampler Pointer to the resampler
   */
  void ConfigureCorrelation(const INPUTS &input, const std::vector<Qn::Axis> &event, Qn::Sampler *sampler);

  /**
   * @brief reference to the names of the Q-Vector inputs of the correlation.
   * @return vector of input Q-Vector names.
   */
  std::vector<std::string> &GetInputNames() { return names_; }

  /**
   * @brief Returns the result of the correlation.
   * @return Average over all events.
   */
  DataContainerStats GetResult() const { return result_; }

  /**
   * @brief Returns a reference to the pointers to the QVectors.
   * @return reference to the inputs.
   */
  std::vector<DataContainerQVector *> &GetInputs() { return inputs_; }

  /**
   * @brief Fill Correlation container with specified inputs.
   * Recursive function called N times, where N is the number of correlated datacontainers.
   * @param eventindices of the used for the event axes
   * @param event_id id of the current event used for resampling.
   */
  void Fill(const std::vector<unsigned long> &eventindices, size_type event_id);

 private:
  bool use_histo_result_ = false;
  bool use_resampling_ = false;
  CorrelationFunction function_; ///< correlation function
  AXES axes_event_; ///< vector of event axes used in the correlation
  INPUTS inputs_; ///< Q-Vector inputs during the correlation
  std::vector<std::string> names_; ///< vector of input names
  std::vector<Qn::Weight> use_weights_; ///< vector of input weights
  std::vector<QVectorPtr> qvectors_; ///< vector holding Q-Vectors during Fill step
  Qn::Sampler *resampler_ = nullptr; ///< Pointer to the central Resampler. CorrelationManager manages lifetime.
  std::vector<std::vector<std::vector<size_type>>> index_; ///< map of multi-dimensional indices of all inputs
  std::vector<size_type> c_index_; ///<  multi-dimensional indices of a bin of the resulting correlation
  DataContainerProduct current_event_result_; ///< result of the correlation of the current event
  DataContainerStats result_; ///< averaged result of the correlation over all events
  DataContainer<TH1F> histo_result_; ///< histogrammed result of the correlation

  /**
   * @brief Fills correlation using a recursive algorithm
   * @param initial_offset offset determined by the number of event axes.
   * @param n recursion step
   */
  void FillCorrelation(size_type initial_offset, unsigned int n);

  /**
   * Calculate weight for correlation;
   * @return
   */
  inline double CalculateWeight() {
    size_type i_weight = 0;
    double weight = 1.;
    for (const auto &q : qvectors_) {
      if (use_weights_[i_weight]==Qn::Weight::OBSERVABLE) {
        weight *= q.sumweights();
      }
      ++i_weight;
    }
    return weight;
  }
};

}

#endif //FLOW_CORRELATION_H
