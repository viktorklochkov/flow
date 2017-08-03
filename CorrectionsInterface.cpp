//
// Created by Lukas Kreis on 18.07.17.
//

#include "CorrectionsInterface.h"

namespace Qn {

namespace Internal {
QnCorrectionsDetectorConfigurationTracks *SetDetectorConfiguration(std::string name) {
  const int dimension = 1;
  QnCorrectionsEventClassVariablesSet *correventclass = new QnCorrectionsEventClassVariablesSet(dimension);
//  double vtxzbins[][2] = {{-10.0, 4}, {-7.0, 1}, {7.0, 8}, {10.0, 1}};
  double centbins[][2] = {{0.0, 2}, {100.0, 100}};
//  correventclass->Add(new QnCorrectionsEventClassVariable(VAR::kVtxZ, VAR::GetVarName(VAR::kVtxZ), vtxzbins));
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
}

void SetVariables(std::array<VAR::Variables, VAR::kNVars> variables) {
  for (auto var : variables) {
    AliReducedVarManager::SetUseVariable(var);
  }
}

void FillTZERO(QnCorrectionsManager &manager, AliReducedEventInfo &event) {
  double weight = 0.0;
  const double kX[24] =
      {0.905348, 0.571718, 0.0848977, -0.424671, -0.82045, -0.99639, -0.905348, -0.571718, -0.0848977,
       0.424671, 0.82045, 0.99639, 0.99995, 0.870982, 0.508635, 0.00999978, -0.491315,
       -0.860982, -0.99995, -0.870982, -0.508635, -0.0100001, 0.491315, 0.860982};
  const double kY[24] =
      {0.424671, 0.82045, 0.99639, 0.905348, 0.571718, 0.0848976, -0.424671, -0.82045, -0.99639,
       -0.905348, -0.571719, -0.0848975, -0.00999983, 0.491315, 0.860982, 0.99995, 0.870982,
       0.508635, 0.00999974, -0.491315, -0.860982, -0.99995, -0.870982, -0.508635};
  for (int ich = 0; ich < 24; ich++) {
    weight = event.AmplitudeTZEROch(ich);
    if (weight < 0.01) weight = 0.;
    manager.AddDataVector((int) DetectorId::TZERO, std::atan2(kY[ich], kX[ich]), weight, ich);
  }
}

void FillFMD(QnCorrectionsManager &manager, AliReducedEventInfo &event) {
  auto fmdList = event.GetFMD();
  TIter nextTrack(fmdList);
  AliReducedFMDInfo *fmd = nullptr;
  while ((fmd = (AliReducedFMDInfo *) nextTrack())) {
    std::cout << fmd->Phi() << " " << fmd->Multiplicity() << std::endl;
    manager.AddDataVector((int) DetectorId::FMD, fmd->Phi(), fmd->Multiplicity(), std::abs(fmd->Id()));
  }
}

void FillZDC(QnCorrectionsManager &manager, AliReducedEventInfo &event) {
  const double kX[10] = {0.0, -1.75, 1.75, -1.75, 1.75, 0.0, 1.75, -1.75, 1.75, -1.75};
  const double kY[10] = {0.0, -1.75, -1.75, 1.75, 1.75, 0.0, -1.75, -1.75, 1.75, 1.75};
  double weight = 0.0;
  for (Int_t ich = 0; ich < 10; ich++) {
    weight = event.EnergyZDCnTree(ich);
    if (weight < 0.01) weight = 0.;
    manager.AddDataVector((int) DetectorId::ZDC, std::atan2(kY[ich], kX[ich]), weight, ich);
  }
}

void FillTPC(QnCorrectionsManager &manager, AliReducedEventInfo &event) {
  auto values = manager.GetDataContainer();
  AliReducedTrackInfo *track = nullptr;
  auto trackList = event.GetTracks();
  TIter next(trackList);
  while ((track = (AliReducedTrackInfo *) next())) {
    if (!track->TestQualityFlag(15)) continue;
    VAR::FillTrackInfo(track, values);
    manager.AddDataVector((int) DetectorId::TPC, values[VAR::kPhi]);
  }
  trackList->Clear();
}

void FillVZERO(QnCorrectionsManager &manager, AliReducedEventInfo &event) {
  const double kX[8] = {0.92388, 0.38268, -0.38268, -0.92388, -0.92388, -0.38268, 0.38268, 0.92388};
  const double kY[8] = {0.38268, 0.92388, 0.92388, 0.38268, -0.38268, -0.92388, -0.92388, -0.38268};
  for (int ich = 0; ich < 64; ich++) {
    double weight = event.MultChannelVZERO(ich);
    if (weight < 0.01) weight = 0.0;
    manager.AddDataVector((int) DetectorId::VZERO, std::atan2(kY[ich % 8], kX[ich % 8]), weight, ich);
  }
}

void ConfigureBins(QnCorrectionsManager &manager,
                   DataContainerTest &data,
                   DetectorId id, std::string name, std::vector<int> cutvariables) {
  auto detector = new QnCorrectionsDetector(name.data(), (int) id);
  auto size = data.size();
  const auto &axices = data.GetAxices();
  for (int index = 0; index < size; ++index) {
    auto indices = data.GetIndex(index);
    auto configuration = Internal::SetDetectorConfiguration(std::to_string(index).data());
    int i = 0;
    auto bincuts = new QnCorrectionsCutsSet();
    bincuts->SetOwner(kTRUE);
    for (const auto &axis : axices) {
      bincuts->Add(new QnCorrectionsCutAbove(cutvariables[i], axis.GetLowerBinEdge(indices[i])));
      bincuts->Add(new QnCorrectionsCutBelow(cutvariables[i], axis.GetUpperBinEdge(indices[i])));
      ++i;
    }
    configuration->SetCuts(bincuts);
    detector->AddDetectorConfiguration(configuration);
  }
  manager.AddDetector(detector);
}

void FillTree(QnCorrectionsManager &manager, DataContainerTest &data,
              DetectorId id) {
  auto detector = manager.FindDetector((int) id);
  auto size = data.size();
  for (long index = 0; index < size; ++index) {
    auto vector = manager.GetDetectorQnVector(std::to_string(index).data(),"latest","latest");
    if(!vector) continue;
    QnCorrectionsQnVector element;
    element = *vector;
    if (element.IsGoodQuality()) data.SetElement(element,index);
  }
}

void FillData(QnCorrectionsManager &manager, AliReducedEventInfo &event) {
  float *values = manager.GetDataContainer();
  AliReducedVarManager::FillEventInfo(&event, values);
  if ((manager.FindDetector((int) DetectorId::ZDC))) FillZDC(manager, event);
  if ((manager.FindDetector((int) DetectorId::TPC))) FillTPC(manager, event);
  if ((manager.FindDetector((int) DetectorId::FMD))) FillFMD(manager, event);
  if ((manager.FindDetector((int) DetectorId::VZERO))) FillVZERO(manager, event);
  if ((manager.FindDetector((int) DetectorId::TZERO))) FillTZERO(manager, event);
}

}