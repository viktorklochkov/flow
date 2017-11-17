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

using CORR = DataContainer<std::vector<float>>;
using AXES = std::vector<Qn::Axis>;

template<typename Proj, typename Corr>
class Correlation {
  using INPUT = std::pair<std::shared_ptr<TTreeReaderValue<DataContainerQVector>>, AXES>;
 public:
  Correlation() = default;
  Correlation(std::vector<INPUT> &inputs, AXES &event, Proj project, Corr correlate) :
      inputs_(inputs),
      axes_event_(event),
      project_(project),
      correlate_(correlate) {
    CreateCorrelationContainer();
  }
 private:
  DataContainerVF data_correlation_;
  std::vector<INPUT> inputs_;
  AXES axes_event_;
  Proj project_;
  Corr correlate_;

  void CreateCorrelationContainer() {
    int i = 0;
    for (auto &input : inputs_) {
      auto axes = input.second;
      for (auto axis : axes) {
        axis.SetName(axis.Name() + i);
      }
      data_correlation_.AddAxes(axes);
      ++i;
    }
    data_correlation_.AddAxes(axes_event_);
  }

  void FillCorrelation(std::vector<long> eventindex, std::vector<long> index, int iteration) {
    if (iteration == inputs_.size()-1) {
      auto i = inputs_.begin()+iteration;
      int ibin = 0;
      for (auto & bin : (**i->first)) {
        auto binindex = (**i->first).GetIndex(ibin);
        if ((**i->first).size() != 1) index.insert(std::end(index), std::begin(binindex), std::end(binindex));
        index.insert(std::end(index), std::begin(eventindex), std::end(eventindex));
        for (auto ii : index) {
          std::cout << ii << " " ;
        }
        std::endl;
      }
    }
    for (auto i = inputs_.begin()+iteration; i < inputs_.end(); ++i) {
      int ibin = 0;
      for (auto & bin : (**i->first)) {
        auto binindex = (**i->first).GetIndex(ibin);
        if ((**i->first).size() != 1) index.insert(std::end(index), std::begin(binindex), std::end(binindex));
        FillCorrelation(eventindex, index, iteration+1);
      }
    }
  }
};

template<typename T, typename Function>
DataContainerVF CreateCorrelation(const DataContainer <T> &a,
                                  const DataContainer <T> &b,
                                  AXES axesa,
                                  AXES axesb,
                                  Function &&lambda,
                                  const AXES &eventaxes) {
  auto at = a.Projection(axesa, lambda);
  auto bt = b.Projection(axesb, lambda);
  DataContainerVF container;
  for (auto &aa : axesa) {
    aa.SetName(aa.Name() + "a");
  }
  container.AddAxes(axesa);
  for (auto &bb : axesb) {
    bb.SetName(bb.Name() + "b");
  }
  container.AddAxes(axesb);
  container.AddAxes(eventaxes);
  return container;
}

inline std::vector<long> CalculateEventBin(AXES &eventaxes, std::vector<float> eventvars) {
  std::vector<long> index;
  int ie = 0;
  for (const auto &ae : eventaxes) {
    index.push_back(ae.FindBin(eventvars[ie]));
    ie++;
  }
  return index;
}

/**
 * Fill Correlation matrix
 * @tparam T datatype from which the correlation is calculated
 * @tparam Function function to calculate correlation
 * @param c Datacontainer in which correlation matrix is filled into
 * @param a Data which is correlated C(a,b)
 * @param b Data which is correlated C(a,b)
 * @param lambda function which correlates
 * @param eventindex multidimensional index calculated by CalculateEventBin used to determine the bin in the Event variables.
 */
template<typename T, typename Function>
void FillCorrelation(DataContainerVF &c,
                     DataContainer <T> &a,
                     DataContainer <T> &b,
                     Function &&lambda,
                     std::vector<long> &eventindex) {
  int ia = 0;
  for (const auto &bina : a) {
    int ib = 0;
    for (const auto &binb : b) {
      std::vector<long> index;
      auto indexa = a.GetIndex(ia);
      auto indexb = b.GetIndex(ib);
      if (a.size() != 1) index.insert(std::end(index), std::begin(indexa), std::end(indexa));
      if (a.size() != 1) index.insert(std::end(index), std::begin(indexb), std::end(indexb));
      index.insert(std::end(index), std::begin(eventindex), std::end(eventindex));
      c.CallOnElement(index,
                      [lambda, bina, binb](std::vector<float> &element) {
                        element.push_back(lambda(bina, binb));
                      });
      ++ib;
    }
    ++ia;
  }
}

}

#endif //FLOW_CORRELATION_H
