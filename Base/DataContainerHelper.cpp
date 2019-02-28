// Flow Vector Correction Framework
//
// Copyright (C) 2018  Lukas Kreis, Ilya Selyuzhenkov
// Contact: l.kreis@gsi.de; ilya.selyuzhenkov@gmail.com
// For a full list of contributors please see docs/Credits
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <iostream>
#include <iterator>

#include "TGraphAsymmErrors.h"

#include "DataContainerHelper.h"
#include "DataContainer.h"

namespace Qn {

TGraphAsymmErrors *DataContainerHelper::ToTGraphShifted(const DataContainerStats &data,
                                                        int i,
                                                        int maxi, Errors drawerrors) {
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
    graph->SetPointError(ibin, exl, exh, bin.ErrorLo(), bin.ErrorHi());
    graph->SetMarkerStyle(kFullCircle);
    ibin++;
  }
  return graph;
};

TGraphAsymmErrors *DataContainerHelper::ToTGraph(const DataContainerStats &data,
                                                 Errors drawerrors) {
  return ToTGraphShifted(data, 1, 2, drawerrors);
};

/**
 * Create a TMultiGraph containing profile graphs for each bin of the specified axis.
 * @param data datacontainer to be graphed.
 * @param axisname name of the axis which is used for the multigraph.
 * @return graph containing a profile graph for each bin.
 */
TMultiGraph *DataContainerHelper::ToTMultiGraph(const DataContainerStats &data,
                                                const std::string &axisname,
                                                Errors drawerrors) {
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
    auto subgraph = ToTGraphShifted(subdata, ibin, axis.size(), drawerrors);
    subgraph->SetTitle(Form("%.2f - %.2f", axis.GetLowerBinEdge(ibin), axis.GetUpperBinEdge(ibin)));
    subgraph->SetMarkerStyle(kFullCircle);
    multigraph->Add(subgraph);
  }
  return multigraph;
}

void DataContainerHelper::StatsBrowse(DataContainer<Stats> *data, TBrowser *b) {
  using DrawErrorGraph = Internal::ProjectionDrawable<TGraphAsymmErrors *>;
  using DrawMultiGraph = Internal::ProjectionDrawable<TMultiGraph *>;
  if (!data->list_) data->list_ = new TList();
  data->list_->SetOwner(true);
  for (auto &axis : data->axes_) {
    auto proj = data->Projection({axis.Name()});
    auto graph = DataContainerHelper::ToTGraph(proj, Errors::Yonly);
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
          auto mgraph = DataContainerHelper::ToTMultiGraph(proj, iaxis->Name(), Errors::Yonly);
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
void DataContainerHelper::NDraw(DataContainer<Stats> &data,
                                std::string option,
                                const std::string &axis_name = {}) {
  option = (option.empty()) ? std::string("ALP PMC PLC Z") : option;
  Errors err = Errors::Yonly;
  auto foundoption = option.find("XErrors");
  if (foundoption!=std::string::npos) {
    err = Errors::XandY;
    option.erase(foundoption, std::string("XErrors").size());
  }
  if (data.axes_.size()==1) {
    auto graph = DataContainerHelper::ToTGraph(data, err);
    graph->Draw(option.data());
  } else if (data.axes_.size()==2) {
    try {
      auto mgraph = DataContainerHelper::ToTMultiGraph(data, axis_name, err);
      mgraph->Draw(option.data());
    } catch (std::exception &) {
      std::string axes_in_dc = "Axis \"" + axis_name + "\" not found.\n";
      axes_in_dc += "Available axes:";
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