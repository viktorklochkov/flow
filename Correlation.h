//
// Created by Lukas Kreis on 03.11.17.
//

#ifndef FLOW_CORRELATION_H
#define FLOW_CORRELATION_H

#include <iostream>
#include <TGraphErrors.h>
#include "DataContainer.h"

namespace Qn {

using CORR = DataContainer<std::vector<float>>;
using AXES = std::vector<Qn::Axis>;

template<typename T, typename Function>
DataContainerVF
CreateCorrelation(const DataContainer <T> &a,
                  const DataContainer <T> &b,
                  AXES axesa,
                  AXES axesb,
                  Function &&lambda,
                  const AXES eventaxes) {
  a.Projection(axesa, lambda);
  b.Projection(axesb, lambda);
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
      c.CallOnElement(index, [lambda, bina, binb](std::vector<float> &element) {
        element.push_back(lambda(bina, binb));
        return element;
      });
      ++ib;
    }
    ++ia;
  }
}

}

#endif //FLOW_CORRELATION_H
