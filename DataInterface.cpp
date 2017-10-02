//
// Created by Lukas Kreis on 09.08.17.
//

#include <iostream>
#include "DataInterface.h"
#include <array>

#define VAR AliReducedVarManager

namespace Qn {
namespace Interface {
void SetVariables(std::vector<VAR::Variables> vars) {
  for (auto var : vars) {
    AliReducedVarManager::SetUseVariable(var);
  }
}

void FillTpc(std::unique_ptr<Qn::DataContainerDataVector> &datacontainer, AliReducedEventInfo &event) {
  auto values = new float[AliReducedVarManager::Variables::kNVars];
  AliReducedTrackInfo *track = nullptr;
  auto trackList = event.GetTracks();
  TIter next(trackList);
  while ((track = (AliReducedTrackInfo *) next()) != nullptr) {
    if (!track->TestQualityFlag(15)) continue;
    VAR::FillTrackInfo(track, values);
    auto axices = datacontainer->GetAxes();
    std::vector<float> trackparams;
    trackparams.reserve(axices.size());
    for (const auto &axis : axices) {
      trackparams.push_back(values[axis.Id()]);
    }
    try {
      auto &element = datacontainer->ModifyElement(trackparams);
      element.emplace_back(values[VAR::kPhi], values[VAR::kPt]);
    }
    catch (std::exception &) {
    }
  }
  delete[] values;
  trackList->Clear();
}

void FillVZEROA(std::unique_ptr<Qn::DataContainerDataVector> &datacontainer, AliReducedEventInfo &event) {
  const std::array<double, 8> kX = {0.92388, 0.38268, -0.38268, -0.92388, -0.92388, -0.38268, 0.38268, 0.92388};
  const std::array<double, 8> kY = {0.38268, 0.92388, 0.92388, 0.38268, -0.38268, -0.92388, -0.92388, -0.38268};
  const std::array<float, 4> etaborders = {4.8, 4.2, 3.65, 3.1};
  for (int ich = 0; ich < 32; ich++) {
    double weight = event.MultChannelVZERO(ich);
    if (weight < 0.01) weight = 0.0;
    auto axes = datacontainer->GetAxes();
    std::vector<float> eta;
    eta.reserve(axes.size());
    for (const auto &axis : axes) {
      float etavalue = 0;
      if (axis.IsIntegrated()) {
        auto &element = datacontainer->ModifyElement({0.5});
        element.emplace_back(std::atan2(kY[ich % 8], kX[ich % 8]), weight);
      } else {
        etavalue = etaborders[ich / 8];
        eta.push_back(etavalue);
        auto &element = datacontainer->ModifyElement(eta);
        element.emplace_back(std::atan2(kY[ich % 8], kX[ich % 8]), weight);
      }
    }
  }
}

void FillVZEROC(std::unique_ptr<Qn::DataContainerDataVector> &datacontainer, AliReducedEventInfo &event) {
  const std::array<double, 8> kX = {0.92388, 0.38268, -0.38268, -0.92388, -0.92388, -0.38268, 0.38268, 0.92388};
  const std::array<double, 8> kY = {0.38268, 0.92388, 0.92388, 0.38268, -0.38268, -0.92388, -0.92388, -0.38268};
  const std::array<float, 4> etaborders = {-3.45, -2.95, -2.45, -1.95};
  for (int ich = 32; ich < 64; ich++) {
    double weight = event.MultChannelVZERO(ich);
    if (weight < 0.01) weight = 0.0;
    auto axes = datacontainer->GetAxes();
    std::vector<float> eta;
    eta.reserve(axes.size());
    for (const auto &axis : axes) {
      float etavalue = 0;
      if (axis.IsIntegrated()) {
        auto &element = datacontainer->ModifyElement({0.5});
        element.emplace_back(std::atan2(kY[ich % 8], kX[ich % 8]), weight);
      } else {
        etavalue = etaborders[(ich-32) / 8];
        eta.push_back(etavalue);
        auto &element = datacontainer->ModifyElement(eta);
        element.emplace_back(std::atan2(kY[ich % 8], kX[ich % 8]), weight);
      }
    }
  }
}

void FillDetectors(Qn::Internal::DetectorMap &map, AliReducedEventInfo &event) {
  if (map.find((int) Configuration::DetectorId::TPC) != map.end())
    FillTpc(std::get<1>(map[(int) Configuration::DetectorId::TPC]), event);
  if (map.find((int) Configuration::DetectorId::VZEROA_reference) != map.end())
    FillVZEROA(std::get<1>(map[(int) Configuration::DetectorId::VZEROA_reference]), event);
  if (map.find((int) Configuration::DetectorId::VZEROC_reference) != map.end())
    FillVZEROC(std::get<1>(map[(int) Configuration::DetectorId::VZEROC_reference]), event);
  if (map.find((int) Configuration::DetectorId::VZEROA) != map.end())
    FillVZEROA(std::get<1>(map[(int) Configuration::DetectorId::VZEROA]), event);
  if (map.find((int) Configuration::DetectorId::VZEROC) != map.end())
    FillVZEROC(std::get<1>(map[(int) Configuration::DetectorId::VZEROC]), event);
}


}
}




