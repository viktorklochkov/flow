//
// Created by Lukas Kreis on 09.08.17.
//

#include <iostream>
#include "DataInterface.h"

#define VAR AliReducedVarManager

void Qn::Interface::SetVariables(std::vector<VAR::Variables> vars) {
  for (auto var : vars) {
    AliReducedVarManager::SetUseVariable(var);
  }
}

void Qn::Interface::FillTpc(std::unique_ptr<Qn::DataContainerDataVector> &datacontainer, AliReducedEventInfo &event) {
  auto values = new float[AliReducedVarManager::Variables::kNVars];
  AliReducedTrackInfo *track = nullptr;
  auto trackList = event.GetTracks();
  TIter next(trackList);
  while ((track = (AliReducedTrackInfo *) next()) != nullptr) {
    if (!track->TestQualityFlag(15)) continue;
    VAR::FillTrackInfo(track, values);
    auto axices = datacontainer->GetAxices();
    std::vector<float> trackparams;
    trackparams.reserve(axices.size());
    for (const auto &axis : axices) {
      trackparams.push_back(values[axis.Id()]);
    }
    try {
      auto &element = datacontainer->ModifyElement(trackparams);
      element.emplace_back(values[VAR::kPhi], values[VAR::kPt]);
    }
    catch (std::exception&) {
    }
  }
  delete [] values;
  trackList->Clear();
}

void Qn::Interface::FillVZERO(std::unique_ptr<Qn::DataContainerDataVector> &datacontainer, AliReducedEventInfo &event) {
  const std::array<double, 8 > kX = {0.92388, 0.38268, -0.38268, -0.92388, -0.92388, -0.38268, 0.38268, 0.92388};
  const std::array<double, 8 > kY = {0.38268, 0.92388, 0.92388, 0.38268, -0.38268, -0.92388, -0.92388, -0.38268};
  for (int ich = 0; ich < 64; ich++) {
    double weight = event.MultChannelVZERO(ich);
    if (weight < 0.01) weight = 0.0;
    auto axices = datacontainer->GetAxices();
    for (auto &element : *datacontainer) {
      element.emplace_back(std::atan2(kY[ich % 8], kX[ich % 8]), weight);
    }
//    manager.AddDataVector((int) DetectorId::VZERO, std::atan2(kY[ich % 8], kX[ich % 8]), weight, ich);
  }
}


