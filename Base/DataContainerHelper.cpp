//
// Created by Lukas Kreis on 20.11.17.
//

#include <iostream>

#include "DataContainerHelper.h"
#include "DataContainer.h"

namespace Qn {

namespace Internal {

TGraphErrors *DataContainerHelper::DataToProfileGraph(const Qn::DataContainerSample &data) {
  if (data.GetAxes().size() > 1) {
    std::cout << "Data container has more than one dimension. " << std::endl;
    std::cout << "Cannot draw as Graph. Use Projection() to make it one dimensional." << std::endl;
    return nullptr;
  }
  auto graph = new TGraphErrors((int) data.size());
  unsigned int ibin = 0;
  for (auto &bin : data) {
    float xhi = data.GetAxes().front().GetUpperBinEdge(ibin);
    float xlo = data.GetAxes().front().GetLowerBinEdge(ibin);
    float x = xlo + ((xhi - xlo)/2);
    graph->SetPoint(ibin, x, bin.Mean());
    graph->SetPointError(ibin, 0, bin.Error());
    graph->SetMarkerStyle(kFullCircle);
    ibin++;
  }
  return graph;
};

TMultiGraph *DataContainerHelper::DataToMultiGraph(const Qn::DataContainerSample &data, const std::string &axisname) {
  auto multigraph = new TMultiGraph();
  if (data.GetAxes().size()!=2) {
    std::cout << "Data Container dimension has wrong dimension " << data.GetAxes().size() << "\n";
    return nullptr;
  }
  Qn::Axis axis;
  try { axis = data.GetAxis(axisname); }
  catch (std::exception &) {
    std::cout << "axis not found" << "\n";
    return nullptr;
  }
  for (unsigned int ibin = 0; ibin < axis.size(); ++ibin) {
    auto subdata = data.Select({axisname, {axis.GetLowerBinEdge(ibin), axis.GetUpperBinEdge(ibin)}});
    auto subgraph = DataToProfileGraph(subdata);
    subgraph->SetTitle(Form("%.2f - %.2f", axis.GetLowerBinEdge(ibin), axis.GetUpperBinEdge(ibin)));
    subgraph->SetMarkerStyle(kFullCircle);
    multigraph->Add(subgraph);
  }
  return multigraph;
}

}
}