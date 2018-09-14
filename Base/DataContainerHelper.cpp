//
// Created by Lukas Kreis on 20.11.17.
//

#include <iostream>
#include <TGraphAsymmErrors.h>

#include "DataContainerHelper.h"
#include "DataContainer.h"

namespace Qn {

TGraphAsymmErrors *DataContainerHelper::DataToProfileGraphShifted(const DataContainerSample &data,
                                                                  int i,
                                                                  int maxi, Errors drawerrors = Errors::Yonly) {
  if (data.GetAxes().size() > 1) {
    std::cout << "Data container has more than one dimension. " << std::endl;
    std::cout << "Cannot draw as Graph. Use Projection() to make it one dimensional." << std::endl;
    return nullptr;
  }
  auto graph = new TGraphAsymmErrors((int) data.size());
  unsigned int ibin = 0;
  for (auto &bin : data) {
    float xhi = data.GetAxes().front().GetUpperBinEdge(ibin);
    float xlo = data.GetAxes().front().GetLowerBinEdge(ibin);
    float x = xlo + ((xhi - xlo)*static_cast<float>(i)/maxi);
    float exl = 0;
    float exh = 0;
    if (drawerrors==Errors::XandY) {
      exl = x - xlo;
      exh = xhi - x;
    }
    graph->SetPoint(ibin, x, bin.Mean());
    graph->SetPointError(ibin, exl, exh, bin.Error(), bin.Error());
    graph->SetMarkerStyle(kFullCircle);
    ibin++;
  }
  return graph;
};

TGraphAsymmErrors *DataContainerHelper::DataToProfileGraph(const DataContainerSample &data,
                                                           Errors drawerrors = Errors::Yonly) {
  return DataToProfileGraphShifted(data, 1, 2, drawerrors);
};

/**
 * Create a TMultiGraph containing profile graphs for each bin of the specified axis.
 * @param data datacontainer to be graphed.
 * @param axisname name of the axis which is used for the multigraph.
 * @return graph containing a profile graph for each bin.
 */
TMultiGraph *DataContainerHelper::DataToMultiGraph(const DataContainerSample &data,
                                                   const std::string &axisname,
                                                   Errors drawerrors = Errors::Yonly) {
  auto multigraph = new TMultiGraph();
  if (data.GetAxes().size()!=2) {
    std::cout << "Data Container dimension has wrong dimension " << data.GetAxes().size() << "\n";
    return nullptr;
  }
  Axis axis;
  try { axis = data.GetAxis(axisname); }
  catch (std::exception &) {
    throw std::logic_error("axis not found");
  }
  for (unsigned int ibin = 0; ibin < axis.size(); ++ibin) {
    auto subdata = data.Select({axisname, {axis.GetLowerBinEdge(ibin), axis.GetUpperBinEdge(ibin)}});
    auto subgraph = DataToProfileGraphShifted(subdata, ibin, axis.size(), drawerrors);
    subgraph->SetTitle(Form("%.2f - %.2f", axis.GetLowerBinEdge(ibin), axis.GetUpperBinEdge(ibin)));
    subgraph->SetMarkerStyle(kFullCircle);
//    subgraph->SetMarkerSize(static_cast<Size_t>(0.5+(1./axis.size()*ibin)));
    multigraph->Add(subgraph);
  }
  return multigraph;
}

void DataContainerHelper::SampleBrowse(DataContainer<Sample> *data, TBrowser *b) {
  using DrawErrorGraph = Internal::ProjectionDrawable<TGraphAsymmErrors *>;
  using DrawMultiGraph = Internal::ProjectionDrawable<TMultiGraph *>;
  if (!data->list_) data->list_ = new TList();
  data->list_->SetOwner(true);
  for (auto &axis : data->axes_) {
    auto proj = data->Projection({axis.Name()});
    auto graph = DataContainerHelper::DataToProfileGraph(proj, Errors::Yonly);
    graph->SetName(axis.Name().data());
    graph->SetTitle(axis.Name().data());
    graph->GetXaxis()->SetTitle(axis.Name().data());
    auto *drawable = new DrawErrorGraph(graph);
    data->list_->Add(drawable);
  }
  if (data->dimension_ > 1) {
    auto list2d = new TList();
    for (auto iaxis = std::begin(data->axes_); iaxis < std::end(data->axes_); ++iaxis) {
      for (auto jaxis = std::begin(data->axes_); jaxis < std::end(data->axes_); ++jaxis) {
        if (iaxis!=jaxis) {
          auto proj = data->Projection({iaxis->Name(), jaxis->Name()});
          auto mgraph = DataContainerHelper::DataToMultiGraph(proj, iaxis->Name(), Errors::Yonly);
          auto name = (jaxis->Name() + ":" + iaxis->Name());
          mgraph->SetName(name.data());
          mgraph->SetTitle(name.data());
          mgraph->GetXaxis()->SetTitle(jaxis->Name().data());
          mgraph->GetYaxis()->SetTitle("Correlation");
          auto *drawable = new DrawMultiGraph(mgraph);
          list2d->Add(drawable);
        }
      }
    }
    list2d->SetName("2D");
    list2d->SetOwner(true);
    data->list_->Add(list2d);
  }
  for (int i = 0; i < data->list_->GetSize(); ++i) {
    b->Add(data->list_->At(i));
  }
}

void DataContainerHelper::EventShapeBrowse(DataContainer<EventShape> *data, TBrowser *b) {
  if (!data->list_) data->list_ = new TList();
  int i = 0;
  auto hlist = new TList();
  hlist->SetName("histos");
  auto slist = new TList();
  slist->SetName("splines");
  auto ilist = new TList();
  ilist->SetName("integrals");
  for (auto &bin : *data) {
    auto name = data->GetBinDescription(i);
    bin.histo_->SetName((std::string("H_") + (name)).data());
    bin.histo_->SetTitle((std::string("H_") + (name)).data());
    bin.histo_->GetXaxis()->SetTitle("|Q|^{2}");
    hlist->Add(bin.histo_);
    bin.spline_->SetName((std::string("S_") + (name)).data());
    bin.spline_->SetTitle((std::string("S_") + (name)).data());
    slist->Add(bin.spline_);
    bin.integral_->SetName((std::string("I_") + (name)).data());
    bin.integral_->SetTitle((std::string("I_") + (name)).data());
    bin.integral_->GetXaxis()->SetTitle("|Q|^{2}");
    ilist->Add(bin.integral_);
    ++i;
  }
  data->list_->Add(hlist);
  data->list_->Add(slist);
  data->list_->Add(ilist);
  data->list_->SetOwner(true);
  for (int j = 0; j < data->list_->GetSize(); ++j) {
    b->Add(data->list_->At(j));
  }
}
void DataContainerHelper::NDraw(DataContainer<Sample> &data,
                                std::string option,
                                const std::string &axis_name = {}) {
  option = (std::empty(option)) ? std::string("ALP PMC PLC Z") : option;
  Errors err = Errors::Yonly;
  auto foundoption = option.find("XErrors");
  if (foundoption!=std::string::npos) {
    err = Errors::XandY;
    option.erase(foundoption, std::size("XErrors"));
  }
  if (data.axes_.size()==1) {
    auto graph = DataContainerHelper::DataToProfileGraph(data, err);
    graph->Draw(option.data());
  } else if (data.axes_.size()==2) {
    try {
      auto mgraph = DataContainerHelper::DataToMultiGraph(data, axis_name, err);
      mgraph->Draw(option.data());
    } catch (std::exception &) {
      std::string axes_in_dc = "Axis \"" + axis_name + "\" not found.\n";
      axes_in_dc += "Avaiable axes:";
      axes_in_dc += " \"" + data.axes_.at(0).Name() + "\"";
      for (size_t i = 1; i < data.axes_.size(); ++i) {
        axes_in_dc += ", \"" + data.axes_.at(i).Name() + "\"";
      }
      std::cout << axes_in_dc << std::endl;
    }
  } else {
    std::cout << "Not drawn because the DataContainer has dimension: " << data.dimension_ << std::endl;
    std::cout << "Only DataContainers with dimension <=2 can be drawn." << std::endl;
  }
}

}