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
#include <exception>

#include "DataContainer.h"
#include "Sampler.h"
#include "Correlation.h"

namespace Qn {

/**
 * @brief Correlation of several input Q-Vector DataContainers.
 */
class StatsResult {

 public:
  using CorrelationFunction = Correlation::function_type;
  using size_type = std::size_t;

  /**
   * @brief Constructs the Correlation step before information of the Q-Vectors is read from the tree.
   * @param names input Q-Vector names.
   * @param lambda correlation function
   * @param use_resampling enable resampling (configured in the CorrelationManager).
   */
  StatsResult(Qn::Sampler::Resample use_resample, Qn::Correlation *ptr) :
  use_resampling_((use_resample==Sampler::Resample::ON)),
  correlation_current_event(ptr) {}

  void ConfigureStats(Qn::Sampler *sampler);

  /**
   * @brief Returns the result of the correlation.
   * @return Average over all events.
   */
  DataContainerStats GetResult() const { return result_; }

  /**
   * @brief Fill Correlation container with specified inputs.
   * Recursive function called N times, where N is the number of correlated datacontainers.
   * @param eventindices of the used for the event axes
   * @param event_id id of the current event used for resampling.
   */
  void Fill(size_type event_id);

 private:
  bool use_resampling_ = false; ///< resampling flag
  Correlation *correlation_current_event = nullptr; ///< Pointer to the correlation result.
  Qn::Sampler *resampler_ = nullptr; ///< Pointer to the central Resampler. CorrelationManager manages lifetime.
  DataContainerStats result_; ///< averaged result of the correlation over all events
};

struct NoResamplerException : public std::exception {
  const char *what() const noexcept override {
    return "correlation requested resampling, but it has not been configured. Resampling DISABLED!";
  }
};

}

#endif //FLOW_CORRELATION_H
