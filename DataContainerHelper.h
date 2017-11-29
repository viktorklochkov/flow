//
// Created by Lukas Kreis on 20.11.17.
//

#ifndef FLOW_DATACONTAINERHELPER_H
#define FLOW_DATACONTAINERHELPER_H

#include <TGraph.h>
#include <TGraphErrors.h>
#include <iostream>
#include "DataContainer.h"
#include "Stats.h"

namespace Qn {

/**
 * Create a profile graph from a one dimensional DataContainerVF, which containing a distribution of floating point numbers in each bin.
 * @param data Datacontainer with one Axis to plot as a TGraphErrors.
 * @return A graph with errors corresponding to the standard error of the mean.
 */
inline TGraphErrors DataToProfileGraph(Qn::DataContainerStat data) {
  if (data.GetAxes().size() > 1) {
    std::cout << "Data container has more than one dimension. " << std::endl;
    std::cout << "Cannot draw as Graph. Use Projection() to make it one dimensional." << std::endl;
  }
  TGraphErrors graph((int) data.size());
  int ibin = 0;
  for (auto &bin : data) {
    float xhi = data.GetAxes().front().GetUpperBinEdge(ibin);
    float xlo = data.GetAxes().front().GetLowerBinEdge(ibin);
    float x = xlo + ((xhi - xlo) / 2);
    graph.SetPoint(ibin, x, bin.Mean());
    graph.SetPointError(ibin, (xhi - xlo) / 2, bin.Error());
    ibin++;
  }
  return graph;
};


/**
 * Create a error graph from a one dimensional DataContainerVF, which contains the value and error for each bin.
 * @param data Datacontainer with one Axis to plot as a TGraphErrors.
 * @return A graph with errors.
 */
inline TGraphErrors DataToGraph(Qn::DataContainerVF data) {
  if (data.GetAxes().size() > 1) {
    std::cout << "Data container has more than one dimension. " << std::endl;
    std::cout << "Cannot draw as Graph. Use Projection() to make it one dimensional." << std::endl;
  }
  TGraphErrors graph((int) data.size());
  int ibin = 0;
  for (auto &bin : data) {
    if (bin.size() > 2) throw std::logic_error("bin contains more than value and error. Cannot plot");
    float xhi = data.GetAxes().front().GetUpperBinEdge(ibin);
    float xlo = data.GetAxes().front().GetLowerBinEdge(ibin);
    float x = xlo + ((xhi - xlo) / 2);
    graph.SetPoint(ibin, x, bin[0]);
    graph.SetPointError(ibin, (xhi - xlo) / 2, bin[1]);
    ibin++;
  }
  return graph;
};

}
#endif //FLOW_DATACONTAINERHELPER_H
