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

namespace Qn {
/**
 * @brief      Correlation of several input data containers.
 */
class Correlation {
  using size_type = std::size_t;
  using AXES = std::vector<Qn::Axis>;
  using DataContainers = std::vector<DataContainerQVector>;
 public:
  Correlation() = default;

  Correlation(const DataContainers &input,
              AXES event,
              std::function<double(std::vector<Qn::QVector> &)> lambda) :
      data_correlation_(),
      axes_event_(std::move(event)),
      function_(std::move(lambda)) {
    CreateCorrelationContainer(input);
  }

  Correlation(std::vector<std::string> names,
              const DataContainers &input,
              AXES event,
              std::function<double(std::vector<Qn::QVector> &)> lambda) :
      data_correlation_(),
      axes_event_(std::move(event)),
      function_(std::move(lambda)),
      names_(std::move(names)) {
    CreateCorrelationContainer(input);
  }

  void ConfigureCorrelation(const DataContainers &input,
                            const std::vector<Qn::Axis> &event,
                            std::function<double(std::vector<Qn::QVector> &)> function,
                            const std::vector<std::string> &names,
                            std::vector<bool> weights) {
    axes_event_ = event;
    names_ = names;
    function_ = function;
    use_weights_ = weights;
    CreateCorrelationContainer(input);
  }

  DataContainerProduct* GetCorrelation() { return &data_correlation_; }
  AXES GetEventAxes() const { return axes_event_; }
  inline double &At(size_type index) { return data_correlation_.At(index).result; }
 private:
  std::vector<std::vector<std::vector<size_type>>> index_; ///< map of indices of all inputs
  std::vector<size_type> c_index_; ///< flattened index of correlation
  DataContainerProduct data_correlation_; ///<  datacontainer containing the correlations
  AXES axes_event_; ///< vector of event axes used in the correlation
  std::function<double(std::vector<Qn::QVector> &)> function_; ///< correlation function
  std::vector<std::string> names_; ///< vector of input names
  std::vector<bool> use_weights_;
  std::vector<QVector> contents_;

/**
 * Create the correlation function. Automatically called at creation of Correlation object.
 */
  void CreateCorrelationContainer(const DataContainers &);

 public:

/**
 * Fill Correlation container with specified inputs.
 * Recursive function called N times, where N is the number of correlated datacontainers.
 * @param input vector of all input containers in correlation
 * @param eventindex of the used for the event axes
 * @param lambda correlation function
 */
  void Fill(const DataContainers &input, const std::vector<unsigned long> &eventindex);
/**
 * Fill correlation recursive function
 * @param eventindex event index of event axes
 * @param index index in each new axis
 * @param contents content of each correlated datacontainer for one bin
 * @param iteration iterationstep
 * @param lambda function which is used for the correlation
 * @param cindex compacted index of the bin in the correlation datacontainer.
 */
  void FillCorrelation(const std::vector<unsigned long> &eventindex,
                       int iterationoffset,
                       u_int iteration,
                       const DataContainers &input);
};
}

#endif //FLOW_CORRELATION_H
