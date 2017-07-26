//
// Created by Lukas Kreis on 18.07.17.
//

#ifndef FLOW_CORRECTIONSINTERFACE_H
#define FLOW_CORRECTIONSINTERFACE_H

#include <iostream>
#include <array>
#include <QnCorrections/QnCorrectionsCutAbove.h>
#include <QnCorrections/QnCorrectionsCutBelow.h>
#include <QnCorrections/QnCorrectionsDetectorConfigurationTracks.h>
#include <QnCorrections/QnCorrectionsProfile3DCorrelations.h>
#include <QnCorrections/QnCorrectionsQnVectorRecentering.h>
#include <QnCorrections/QnCorrectionsQnVectorTwistAndRescale.h>
#include <QnCorrections/QnCorrectionsCutWithin.h>

#include "AliReducedVarManager.h"
#include "QnCorrectionsManager.h"
#include "AliReducedEventInfo.h"
#include "AliReducedBaseTrack.h"
#include "AliReducedTrackInfo.h"
#include "AliReducedFMDInfo.h"
#include "DataContainer.h"

#define VAR AliReducedVarManager

namespace Qn {

enum class DetectorId : int {
  TPC,
  VZERO,
  TZERO,
  ZDC,
  FMD
};

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
    if ((manager.FindDetector((int) DetectorId::ZDC))) FillZDC(manager, event);
    if ((manager.FindDetector((int) DetectorId::TPC))) FillTPC(manager, event);
    if ((manager.FindDetector((int) DetectorId::FMD))) FillFMD(manager, event);
    if ((manager.FindDetector((int) DetectorId::VZERO))) FillVZERO(manager, event);
    if ((manager.FindDetector((int) DetectorId::TZERO))) FillTZERO(manager, event);
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
        manager.AddDataVector((int) DetectorId::TZERO, std::atan2(kY[ich], kX[ich]), weight, ich);
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
      manager.AddDataVector((int) DetectorId::FMD, fmd->Phi(), fmd->Multiplicity(), std::abs(fmd->Id()));
    }
  }

  static void FillZDC(QnCorrectionsManager &manager, AliReducedEventInfo &event) {
    const double kX[10] = {0.0, -1.75, 1.75, -1.75, 1.75, 0.0, 1.75, -1.75, 1.75, -1.75};
    const double kY[10] = {0.0, -1.75, -1.75, 1.75, 1.75, 0.0, -1.75, -1.75, 1.75, 1.75};
    double weight = 0.0;
    for (Int_t ich = 0; ich < 10; ich++) {
      weight = event.EnergyZDCnTree(ich);
      if (weight < 0.01) weight = 0.;
      manager.AddDataVector((int) DetectorId::ZDC, std::atan2(kY[ich], kX[ich]), weight, ich);
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
      manager.AddDataVector((int) DetectorId::TPC, values[VAR::kPhi]);
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
      else manager.AddDataVector((int) DetectorId::VZERO, std::atan2(kY[ich % 8], kX[ich % 8]), weight, ich);
    }
  }

  static void ConfigureBins(QnCorrectionsManager &manager,
                            std::unique_ptr<DataContainerQn> const &data,
                            DetectorId id) {
    auto *detector = new QnCorrectionsDetector("TPC", (int) id);
    auto size = data->size();
    const auto &axices = data->GetAxices();
    for (int index = 0; index < size; ++index) {
      auto indices = data->GetIndex(index);
      auto configuration = SetDetectorConfiguration(std::to_string(index).data(), detector);
      int i = 0;
      std::array<int, 2> enumarray = {VAR::kEta, VAR::kPhi};
      QnCorrectionsCutsSet *bincuts = new QnCorrectionsCutsSet();
      bincuts->SetOwner(kTRUE);
      for (const auto &axis : axices) {
        bincuts->Add(new QnCorrectionsCutAbove(enumarray[i], axis.GetLowerBinEdge(indices[i])));
        bincuts->Add(new QnCorrectionsCutBelow(enumarray[i], axis.GetUpperBinEdge(indices[i])));
        ++i;
      }
      configuration->SetCuts(bincuts);
      detector->AddDetectorConfiguration(configuration);
    }
    manager.AddDetector(detector);
  }

  static QnCorrectionsDetectorConfigurationTracks *SetDetectorConfiguration(std::string name,
                                                                            QnCorrectionsDetector *detector) {
    const int dimension = 2;
    QnCorrectionsEventClassVariablesSet *correventclass = new QnCorrectionsEventClassVariablesSet(dimension);
    double vtxzbins[][2] = {{-10.0, 4}, {-7.0, 1}, {7.0, 8}, {10.0, 1}};
    double centbins[][2] = {{0.0, 2}, {100.0, 100}};
    correventclass->Add(new QnCorrectionsEventClassVariable(VAR::kVtxZ, VAR::GetVarName(VAR::kVtxZ), vtxzbins));
    correventclass->Add(new QnCorrectionsEventClassVariable(VAR::kCentVZERO,
                                                            Form("Centrality (%s)",
                                                                 VAR::GetVarName(VAR::kCentVZERO).Data()),
                                                            centbins));
    int harmonics[4] = {1, 2, 3, 4};
    auto configuration = new QnCorrectionsDetectorConfigurationTracks(name.data(), correventclass, 4, harmonics);
    configuration->SetQVectorNormalizationMethod(QnCorrectionsQnVector::QVNORM_QoverM);
    configuration->AddCorrectionOnQnVector(new QnCorrectionsQnVectorRecentering());
    auto twistscale = new QnCorrectionsQnVectorTwistAndRescale();
    twistscale->SetApplyTwist(kTRUE);
    twistscale->SetApplyRescale(kFALSE);
    twistscale->SetTwistAndRescaleMethod(QnCorrectionsQnVectorTwistAndRescale::TWRESCALE_doubleHarmonic);
    configuration->AddCorrectionOnQnVector(static_cast<QnCorrectionsCorrectionOnQvector *>(twistscale));
    return configuration;
  }

  static void FillTree(QnCorrectionsManager &manager, std::unique_ptr<DataContainerQn> const &data,
                       DetectorId id) {
    auto detector = manager.FindDetector((int) id);
    auto size = data->size();
    for (long index = 0; index < size; ++index) {
      auto vector = manager.GetDetectorQnVector(std::to_string(index).data());
      if (!vector) return;
      auto vectorcopy = new QnCorrectionsQnVector(*vector);
//      if (vector != nullptr) std::cout << "success" << std::endl;
      std::unique_ptr<const QnCorrectionsQnVector> element(std::move(vectorcopy));
      data->SetElement(element, index);
    }
  }

};
}
#endif //FLOW_CORRECTIONSINTERFACE_H
