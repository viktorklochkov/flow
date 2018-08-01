//
// Created by Lukas Kreis on 03.11.17.
//

#ifndef FLOW_CORRELATION_H
#define FLOW_CORRELATION_H

#include <iostream>
#include <utility>
#include <TGraphErrors.h>
#include "Base/DataContainer.h"
#include "Base/QVector.h"

namespace Qn {
/**
 * @brief      Correlation of several input data containers.
 */
class Correlation {
  using AXES = std::vector<Qn::Axis>;
  using CONTAINERS = DataContainerQVector;
 public:
  Correlation() = default;
  Correlation(const std::vector<CONTAINERS> &input,
              AXES event,
              std::function<double(std::vector<Qn::QVector> &)> lambda) :
      data_correlation_(),
      axes_event_(std::move(event)),
      function_(std::move(lambda)) {
//    for (int i = 0; i < input.size(); ++i) {
//      names_.push_back(std::to_string(i));
//    }
    CreateCorrelationContainer(input);
  }
  Correlation(std::vector<std::string> names,
              const std::vector<CONTAINERS> &input,
              AXES event,
              std::function<double(std::vector<Qn::QVector> &)> lambda) :
      data_correlation_(),
      axes_event_(std::move(event)),
      function_(std::move(lambda)),
      names_(std::move(names)) {
    CreateCorrelationContainer(input);
  }
  void ConfigureCorrelation(const std::vector<DataContainerQVector> &input,
                            const std::vector<Qn::Axis> &event,
                            std::function<double(std::vector<Qn::QVector> &)> function,
                            std::vector<std::string> names) {
    axes_event_ = event;
    names_ = names;
    function_ = function;
    CreateCorrelationContainer(input);
  }
  DataContainerFB GetCorrelation() const { return data_correlation_; }
  AXES GetEventAxes() const { return axes_event_; }
  inline float &At(int index) {return data_correlation_.At(index).second; }
 private:
  std::vector<std::vector<std::vector<unsigned long>>>
      index_; ///< map of indices of all inputs used for calculating the correlations
  std::vector<unsigned long> c_index_; ///< flattened index of correlation
  DataContainerFB data_correlation_; ///<  datacontainer containing the correlations
  AXES axes_event_; ///< vector of event axes used in the correlation
  std::function<double(std::vector<Qn::QVector> &)> function_; ///< correlation function
  std::vector<std::string> names_; ///< vector of input names

/**
 * Create the correlation function. Automatically called at creation of Correlation object.
 */
  void CreateCorrelationContainer(const std::vector<Correlation::CONTAINERS> &);

 public:

/**
 * Fill Correlation container with specified inputs.
 * Recursive function called N times, where N is the number of correlated datacontainers.
 * @param input vector of all input containers in correlation
 * @param eventindex of the used for the event axes
 * @param lambda correlation function
 */
  void Fill(const std::vector<CONTAINERS> &input, const std::vector<unsigned long> &eventindex);
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
                       std::vector<QVector> &contents,
                       int iterationoffset,
                       u_int iteration,
                       const std::vector<Correlation::CONTAINERS> &input);
};
}

#endif //FLOW_CORRELATION_H
