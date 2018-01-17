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
#include "TTreeReaderValue.h"
#include "Base/Stats.h"

namespace Qn {
/**
 * @brief      Correlation of several input data containers.
 */
class Correlation {
  using AXES = std::vector<Qn::Axis>;
  using CONTAINERS = DataContainerQVector;
 public:
  Correlation() = default;
  Correlation(std::vector<CONTAINERS> input, AXES event, std::function<double (std::vector<Qn::QVector>&)> lambda) :
      inputs_(std::move(input)),
      axes_event_(std::move(event)),
      function_(lambda) {
    CreateCorrelationContainer();
  }
  DataContainerProfile GetCorrelation() const { return data_correlation_; }
 private:
  DataContainerProfile data_correlation_; ///<  datacontainer containing the correlations
  std::vector<CONTAINERS> inputs_; ///< vector of input datacontainers
  AXES axes_event_; ///< vector of event axes used in the correlation
  std::function<double (std::vector<Qn::QVector>&)> function_;

/**
 * Create the correlation function. Automatically called at creation of Correlation object.
 */
  void CreateCorrelationContainer() {
    int i = 0;
    data_correlation_.AddAxes(axes_event_);
    for (auto &input : inputs_) {
      if (!input.IsIntegrated()) {
        auto axes = input.GetAxes();
        for (auto &axis : axes) {
          axis.SetName(std::to_string(i) + axis.Name());
        }
        data_correlation_.AddAxes(axes);
        ++i;
      }
    }
  }

 public:

/**
 * Fill Correlation container with specified inputs.
 * Recursive function called N times, where N is the number of correlated datacontainers.
 * @param input vector of all input containers in correlation
 * @param eventindex of the used for the event axes
 * @param lambda correlation function
 */
  void Fill(const std::vector<CONTAINERS> &input, const std::vector<long> &eventindex);
/**
 * Fill correlation recursive function
 * @param eventindex event index of event axes
 * @param index index in each new axis
 * @param contents content of each correlated datacontainer for one bin
 * @param iteration iterationstep
 * @param lambda function which is used for the correlation
 * @param cindex compacted index of the bin in the correlation datacontainer.
 */
  void FillCorrelation(const std::vector<long> &eventindex,
                       std::vector<std::vector<long>> &index,
                       std::vector<QVector> &contents,
                       u_int iteration,
                       std::vector<long> &cindex);
};

}

#endif //FLOW_CORRELATION_H
