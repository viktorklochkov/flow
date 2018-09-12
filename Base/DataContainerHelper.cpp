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

void DataContainerHelper::SampleBrowse(Qn::DataContainer<Qn::Sample> &data, TBrowser *b) {
  TString opt = gEnv->GetValue("TGraph.BrowseOption", "");
  if (opt.IsNull()) {
    opt = b ? b->GetDrawOption() : "AlP PLC PMC Z";
    opt = (opt=="") ? "ALP PMC PLC Z" : opt.Data();
    gEnv->SetValue("TGraph.BrowseOption", opt.Data());
  }
  if (!data.list_) data.list_ = new TList();
  data.list_->SetOwner(true);
  for (auto &axis : data.axes_) {
    auto proj = data.Projection({axis.Name()});
    auto graph = Qn::Internal::DataContainerHelper::DataToProfileGraph(proj);
    graph->SetName(axis.Name().data());
    graph->SetTitle(axis.Name().data());
    data.list_->Add(graph);
  }
  if (data.dimension_ > 1) {
    auto list = new TList();
    for (auto iaxis = std::begin(data.axes_); iaxis < std::end(data.axes_); ++iaxis) {
      for (auto jaxis = std::begin(data.axes_); jaxis < std::end(data.axes_); ++jaxis) {
        if (iaxis != jaxis) {
          auto proj = data.Projection({iaxis->Name(), jaxis->Name()});
          auto mgraph = Qn::Internal::DataContainerHelper::DataToMultiGraph(proj, iaxis->Name());
          auto name = (jaxis->Name() + ":" + iaxis->Name());
          mgraph->SetName(name.data());
          mgraph->SetTitle(name.data());
          mgraph->GetXaxis()->SetTitle(jaxis->Name().data());
          mgraph->GetYaxis()->SetTitle("Correlation");
          list->Add(mgraph);
        }
      }
    }
    list->SetName("2D");
    list->SetOwner(true);
    data.list_->Add(list);
  }
  for (int i = 0; i<data.list_->GetSize(); ++i) {
    b->Add(data.list_->At(i));
  }
}

void DataContainerHelper::EventShapeBrowse(Qn::DataContainer<Qn::EventShape> &data, TBrowser *b) {
  if (!data.list_) data.list_ = new TList();
  int i = 0;
  auto hlist = new TList();
  hlist->SetName("histos");
  auto slist = new TList();
  slist->SetName("splines");
  auto ilist = new TList();
  ilist->SetName("integrals");
  for (auto &bin : data) {
    auto name = data.GetBinDescription(i);
    bin.histo_->SetName((std::string("H_")+(name)).data());
    bin.histo_->SetTitle((std::string("H_")+(name)).data());
    bin.histo_->GetXaxis()->SetTitle("|Q|^{2}");
    hlist->Add(bin.histo_);
    bin.spline_->SetName((std::string("S_")+(name)).data());
    bin.spline_->SetTitle((std::string("S_")+(name)).data());
    slist->Add(bin.spline_);
    bin.integral_->SetName((std::string("I_")+(name)).data());
    bin.integral_->SetTitle((std::string("I_")+(name)).data());
    bin.integral_->GetXaxis()->SetTitle("|Q|^{2}");
    ilist->Add(bin.integral_);
    ++i;
  }
  data.list_->Add(hlist);
  data.list_->Add(slist);
  data.list_->Add(ilist);
  data.list_->SetOwner(true);
  for (int j = 0; i<data.list_->GetSize(); ++j) {
    b->Add(data.list_->At(j));
  }
}
void DataContainerHelper::NDraw(Qn::DataContainer<Qn::Sample> &data, Option_t *option, std::string axis_name) {
  if (data.axes_.size()==1) {
    auto graph = Qn::Internal::DataContainerHelper::DataToProfileGraph(data);
//    graph->SetTitle(data.GetTitle());
    graph->Draw(option);
  } else if (data.axes_.size()==2) {
    auto mgraph = Qn::Internal::DataContainerHelper::DataToMultiGraph(data, axis_name);
    mgraph->Draw(option);
  }
}

}
}