//
// Created by Lukas Kreis on 03.11.17.
//

#ifndef FLOW_CORRELATION_H
#define FLOW_CORRELATION_H

#include "DataContainer.h"

namespace Qn {

using CORR = DataContainer<std::vector<float>>;
using AXES = std::vector<Qn::Axis>;

template <typename T, typename Function>
DataContainer<CORR> Correlate(DataContainer<CORR> c, DataContainer<T> a, DataContainer<T> b, AXES axes, Function &&lambda, AXES eventaxes) {
  a.Projection(axes, lambda);
  b.Projection(axes, lambda);
  DataContainer<CORR> container;
  container.AddAxes(axes);
  container.AddAxes(eventaxes);
}

template <typename T, typename Function>
DataContainer<CORR> CreateCorrelation(const DataContainer<T> &a, const DataContainer<T> &b, AXES &axesa, AXES &axesb, Function &&lambda, AXES &eventaxes) {
  a.Projection(axesa,lambda);
  b.Projection(axesb, lambda);
  DataContainer<CORR> container;
  container.AddAxes(axesa);
  container.AddAxes(axesb);
  container.AddAxes(eventaxes);
  return container;
}

template <typename T, typename Function>
DataContainer<CORR> FillCorrelation(DataContainer<CORR> &c, const DataContainer<T> &a, const DataContainer<T> &b, Function &&lambda, AXES eventaxes) {

  DataContainer<CORR> container;
  container.AddAxes(eventaxes);
}



}



#endif //FLOW_CORRELATION_H
