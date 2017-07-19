//
// Created by Lukas Kreis on 18.07.17.
//

#ifndef FLOW_CORRECTIONSINTERFACE_H
#define FLOW_CORRECTIONSINTERFACE_H

#include <iostream>
#include <array>

#include "AliReducedVarManager.h"
#include "QnCorrectionsManager.h"
#include "AliReducedEventInfo.h"
#include "AliReducedBaseTrack.h"
#include "AliReducedTrackInfo.h"
#include "AliReducedFMDInfo.h"

#define VAR AliReducedVarManager

namespace Qn {

class CorrectionsInterface {
 public:
  ~CorrectionsInterface() = default;

  static void SetVariables(std::array<VAR::Variables, VAR::kNVars> variables) {
    for (auto var : variables) {
      AliReducedVarManager::SetUseVariable(var);
    }
  }

  static void FillData(QnCorrectionsManager &manager, AliReducedEventInfo &event) {
    float *values = manager.GetDataContainer();
    AliReducedVarManager::FillEventInfo(&event, values);
    if ((manager.FindDetector(VAR::kZDC))) FillZDC(manager, event);
    if ((manager.FindDetector(VAR::kTPC))) FillTPC(manager, event);
    if ((manager.FindDetector(VAR::kFMD))) FillFMD(manager, event);
    if ((manager.FindDetector(VAR::kVZERO))) FillVZERO(manager, event);
    if ((manager.FindDetector(VAR::kTZERO))) FillTZERO(manager, event);
  }

  static void FillTZERO(QnCorrectionsManager &manager, AliReducedEventInfo &event) {
    Double_t weight = 0.0;
    const Double_t kX[24] =
        {0.905348, 0.571718, 0.0848977, -0.424671, -0.82045, -0.99639, -0.905348, -0.571718, -0.0848977,
         0.424671, 0.82045, 0.99639, 0.99995, 0.870982, 0.508635, 0.00999978, -0.491315,
         -0.860982, -0.99995, -0.870982, -0.508635, -0.0100001, 0.491315, 0.860982};
    const Double_t kY[24] =
        {0.424671, 0.82045, 0.99639, 0.905348, 0.571718, 0.0848976, -0.424671, -0.82045, -0.99639,
         -0.905348, -0.571719, -0.0848975, -0.00999983, 0.491315, 0.860982, 0.99995, 0.870982,
         0.508635, 0.00999974, -0.491315, -0.860982, -0.99995, -0.870982, -0.508635};
    for (Int_t ich = 0; ich < 24; ich++) {
      weight = event.AmplitudeTZEROch(ich);
      if (weight < 0.01) weight = 0.;
      else
        manager.AddDataVector(VAR::kTZERO, std::atan2(kY[ich], kX[ich]), weight, ich);
    }
  }

  static void FillFMD(QnCorrectionsManager &manager, AliReducedEventInfo &event) {
    int nTrack = -1;
    AliReducedFMDInfo *fmd = 0x0;
    TClonesArray *fmdList = event.GetFMD();
    TIter nextTrack(fmdList);
    for (int it = 0; it < fmdList->GetEntriesFast(); ++it) {
      fmd = (AliReducedFMDInfo *) nextTrack();
      if (!fmd) continue;
      manager.AddDataVector(VAR::kFMD, fmd->Phi(), fmd->Multiplicity(), std::abs(fmd->Id()));
    }
  }

  static void FillZDC(QnCorrectionsManager &manager, AliReducedEventInfo &event) {
    const double kX[10] = {0.0, -1.75, 1.75, -1.75, 1.75, 0.0, 1.75, -1.75, 1.75, -1.75};
    const double kY[10] = {0.0, -1.75, -1.75, 1.75, 1.75, 0.0, -1.75, -1.75, 1.75, 1.75};
    double weight = 0.0;
    for (Int_t ich = 0; ich < 10; ich++) {
      weight = event.EnergyZDCnTree(ich);
      if (weight < 0.01) weight = 0.;
      manager.AddDataVector(VAR::kZDC, std::atan2(kY[ich], kX[ich]), weight, ich);
    }
  }

  static void FillTPC(QnCorrectionsManager &manager, AliReducedEventInfo &event) {
    float *values = manager.GetDataContainer();
    AliReducedTrackInfo *track = nullptr;
    TClonesArray *trackList = event.GetTracks();
    TIter next(trackList);
    while ((track = (AliReducedTrackInfo *) next())) {
      if (!track->TestQualityFlag(15)) continue;
      VAR::FillTrackInfo(track, values);
      manager.AddDataVector(VAR::kTPC, values[VAR::kPhi]);
    }
    trackList->Clear();
  }

  static void FillVZERO(QnCorrectionsManager &manager, AliReducedEventInfo &event) {
    double weight = 0.;
    const double kX[8] = {0.92388, 0.38268, -0.38268, -0.92388, -0.92388, -0.38268, 0.38268, 0.92388};
    const double kY[8] = {0.38268, 0.92388, 0.92388, 0.38268, -0.38268, -0.92388, -0.92388, -0.38268};
    for (int ich = 0; ich < 64; ich++) {
      weight = event.MultChannelVZERO(ich);
      if (weight < 0.01) weight = 0.;
      else manager.AddDataVector(VAR::kVZERO, std::atan2(kY[ich % 8], kX[ich % 8]), weight, ich);
    }
  }
};
}
#endif //FLOW_CORRECTIONSINTERFACE_H
