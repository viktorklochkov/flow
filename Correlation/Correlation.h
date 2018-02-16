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
  Correlation(const std::vector<CONTAINERS> &input,
              AXES event,
              std::function<double(std::vector<Qn::QVector> &)> lambda) :
      axes_event_(std::move(event)),
      function_(std::move(lambda)) {
    CreateCorrelationContainer(input);
  }
  DataContainerF GetCorrelation() const { return data_correlation_; }
 private:
  std::vector<std::vector<std::vector<long>>> index_;
  std::vector<long> c_index_;
  DataContainerF data_correlation_; ///<  datacontainer containing the correlations
//  std::vector<CONTAINERS> inputs_; ///< vector of input datacontainers
  AXES axes_event_; ///< vector of event axes used in the correlation
  std::function<double(std::vector<Qn::QVector> &)> function_;

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
                     std::vector<QVector> &contents,
                     std::vector<long> &binindex,
                     int pos,
                     u_int iteration,
                     std::vector<long> &cindex,
                     const std::vector<Correlation::CONTAINERS> &input);



  template<typename FF>
  void FillCorrelation(const std::vector<long> &eventindex,
                       std::vector<QVector> &contents,
                       std::vector<long> &binindex,
                       int ipos,
                       u_int iteration,
                       std::vector<long> &cindex,
                       const std::vector<Correlation::CONTAINERS> &input,
                       FF&& lambda) {
    const auto &datacontainer = input[iteration];
    ipos += iteration;
    if (iteration + 1==input.size()) {
      int ibin = 0;
      for (auto &bin : datacontainer) {
        if (bin.n()==0) continue;
//      datacontainer.GetIndex(binindex, ibin);
        if (!datacontainer.IsIntegrated()) {
          int ii = 0;
          for (auto i : index_[iteration][ibin]) {
            int pos = ii + ipos;
            cindex[pos] = i;
            ++ii;
          }
        }
        contents[iteration] = bin;
        data_correlation_.At(cindex) = lambda(contents);
//      if (!datacontainer.IsIntegrated()) index.erase(index.end() - 1);
        ++ibin;
      }
      return;
    }
    int ibin = 0;
    for (const auto &bin : datacontainer) {
      int pos = ipos;
      if (bin.n()==0) continue;
      if (!datacontainer.IsIntegrated()) {
        int ii = 0;
        for (auto i : index_[iteration][ibin]) {
          pos += ii;
          cindex[pos] = i;
          ++ii;
        }
      }
//    if (!datacontainer.IsIntegrated()) index.push_back(binindex);
      contents[iteration] = bin;
      FillCorrelation(eventindex, contents,binindex, pos, iteration + 1, cindex, input, lambda);
      ++ibin;
    }
  }
  template <typename FF>
  void Fill(const std::vector<Correlation::CONTAINERS> &input, const std::vector<long> &eventindex, FF&& lambda) {
    std::vector<QVector> contents;
    contents.resize(input.size());
    uint iteration = 0;
    std::vector<long> cindex;
    std::vector<long> binindex;
    binindex.reserve(input[0].size());
    int size = eventindex.size();
    for (auto i : input) {
      size += i.GetAxes().size();
    }
    cindex.resize(size);
    int ii = 0;
    for (auto eventind : eventindex) {
      cindex[ii] = eventind;
      ++ii;
    }
    FillCorrelation(eventindex, contents, binindex, ii, iteration, cindex, input, lambda);
  }
};

}

#endif //FLOW_CORRELATION_H
