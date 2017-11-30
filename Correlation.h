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

using AXES = std::vector<Qn::Axis>;

class Correlation {
  using CONTAINERS = DataContainerQVector;
 public:
  Correlation() = default;
  Correlation(std::vector<CONTAINERS> input, AXES &event) :
      inputs_(std::move(input)),
      axes_event_(event) {
    CreateCorrelationContainer();
  }
  DataContainerStat GetCorrelation() const { return data_correlation_; }
 private:
  DataContainerStat data_correlation_;
  std::vector<CONTAINERS> inputs_;
  AXES axes_event_;

  /**
 * Create the correlation function. Automatically called at creation of Correlation object.
 */
  void CreateCorrelationContainer() {
    int i = 0;
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
    data_correlation_.AddAxes(axes_event_);
  }

 public:

  /**
   * Fill Correlation container with specified inputs.
   * Recursive function called N times, where N is the number of correlated datacontainers.
   * @tparam Function type of lambda
   * @param input vector of all input containers in correlation
   * @param eventindex of the used for the event axes
   * @param lambda correlation function
   */
  template<typename Function>
  void Fill(const std::vector<CONTAINERS> &input, const std::vector<long> &eventindex, Function &&lambda) {
    std::vector<std::vector<long>> index;
    std::vector<QVector> contents;
    contents.resize(input.size());
    inputs_ = input;
    uint iteration = 0;
    std::vector<long> cindex;
    cindex.reserve(10);
    index.reserve(10);
    FillCorrelation(eventindex, index, contents, iteration, lambda, cindex);
  }
  template<typename Function>
  void FillCorrelation(const std::vector<long> &eventindex,
                       std::vector<std::vector<long>> &index,
                       std::vector<QVector> &contents,
                       u_int iteration,
                       Function &&lambda,
                       std::vector<long> &cindex) {
    auto &datacontainer = *(inputs_.begin() + iteration);
    if (iteration + 1 == inputs_.size()) {
      int ibin = 0;
      for (auto &bin : datacontainer) {
        auto binindex = datacontainer.GetIndex(ibin);
        if (datacontainer.size() != 1) index.push_back(binindex);
        index.push_back(eventindex);
        u_long i = 0;
        std::for_each(std::begin(index), std::end(index), [&cindex, &i](const std::vector<long> &element) {
          for (const auto &a : element) { cindex.push_back(a); }
          ++i;
        });
        contents[iteration] = bin;
        data_correlation_.CallOnElement(cindex,
                                        [&lambda, &contents](Qn::Statistics &a) { a.Update(lambda(contents)); });
        if (datacontainer.size() != 1) index.erase(index.end() - 2);
        index.erase(index.end() - 1);
        ++ibin;
        cindex.clear();
      }
      index.clear();
      return;
    }
    int ibin = 0;
    for (auto &bin : datacontainer) {
      auto binindex = datacontainer.GetIndex(ibin);
      if (datacontainer.size() != 1) index.push_back(binindex);
      contents[iteration] = bin;
      FillCorrelation(eventindex, index, contents, iteration + 1, lambda, cindex);
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
inline std::vector<long> CalculateEventBin(const AXES &eventaxes, const std::vector<float> &eventvars) {
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
