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
      element.emplace_back(values[VAR::kPhi], 1);
    }
    catch (std::exception&) {
    }
  }
  delete [] values;
  trackList->Clear();
}