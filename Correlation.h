//
// Created by Lukas Kreis on 03.11.17.
//

#ifndef FLOW_CORRELATION_H
#define FLOW_CORRELATION_H

#include <iostream>
#include <utility>
#include <TGraphErrors.h>
#include "DataContainer.h"
#include "TTreeReaderValue.h"

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
  DataContainerStat GetCorrelation() const { return data_correlation_; }
 private:
  DataContainerStat data_correlation_; ///<  datacontainer containing the correlations
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
  void Fill(const std::vector<CONTAINERS> &input, const std::vector<long> &eventindex) {
    std::vector<std::vector<long>> index;
    std::vector<QVector> contents;
    contents.resize(input.size());
    inputs_ = input;
    uint iteration = 0;
    std::vector<long> cindex;
    cindex.reserve(10);
    index.reserve(10);
    FillCorrelation(eventindex, index, contents, iteration, cindex);
  }
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
                       std::vector<long> &cindex) {
    auto &datacontainer = *(inputs_.begin() + iteration);
    if (iteration + 1 == inputs_.size()) {
      int ibin = 0;
      for (auto &bin : datacontainer) {
        auto binindex = datacontainer.GetIndex(ibin);
        if (!datacontainer.IsIntegrated()) index.push_back(binindex);
        std::for_each(std::begin(index), std::end(index), [&cindex](const std::vector<long> &element) {
          for (const auto &a : element) { cindex.push_back(a); }
        });
        contents.at(iteration) = bin;
        data_correlation_.CallOnElement(cindex,
                                        [this, &contents](Qn::Statistics &a) {
                                          if (std::all_of(contents.begin(), contents.end(), [](QVector qv) { return qv.n() != 0; })) {
                                            a.Update(function_(contents));
                                          }
                                        });
        if (!datacontainer.IsIntegrated()) index.erase(index.end() - 1);
        ++ibin;
        cindex.clear();
      }
      index.clear();
      return;
    }
    int ibin = 0;
    for (auto &bin : datacontainer) {
      index.push_back(eventindex);
      auto binindex = datacontainer.GetIndex(ibin);
      if (!datacontainer.IsIntegrated()) index.push_back(binindex);
      contents.at(iteration) = bin;
      FillCorrelation(eventindex, index, contents, iteration + 1, cindex);
      ++ibin;
    }
  }
};

/**
 * Calculates the bin indices of the event axes.
 * @param eventaxes vector of event axes.
 * @param eventvars vector of event variables
 * @return vector of event axes.
 */
inline std::vector<long> CalculateEventBin(const std::vector<Qn::Axis> &eventaxes, const std::vector<float> &eventvars) {
  std::vector<long> index;
  int ie = 0;
  for (const auto &ae : eventaxes) {
    index.push_back(ae.FindBin(eventvars[ie]));
    ie++;
  }
  return index;
}

}

#endif //FLOW_CORRELATION_H
