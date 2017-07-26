//
// Created by Lukas Kreis on 29.06.17.
//

#ifndef FLOW_QNTASK_H
#define FLOW_QNTASK_H

#include <vector>
#include <array>
#include "QnCorrections/QnCorrectionsProfile3DCorrelations.h"
#include "QnCorrections/QnCorrectionsProfileCorrelationComponents.h"
#include "QnCorrections/QnCorrectionsDetectorConfigurationChannels.h"
#include "QnCorrections/QnCorrectionsDetectorConfigurationBase.h"
#include <QnCorrections/QnCorrectionsDetectorConfigurationTracks.h>
#include <QnCorrections/QnCorrectionsQnVectorRecentering.h>
#include <QnCorrections/QnCorrectionsQnVectorTwistAndRescale.h>
#include <QnCorrections/QnCorrectionsCutSetBit.h>
#include <QnCorrections/QnCorrectionsCutWithin.h>
#include <QnCorrections/QnCorrectionsInputGainEqualization.h>
#include <QnCorrections/QnCorrectionsQnVectorAlignment.h>
#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"

#include "QnCorrections/QnCorrectionsManager.h"
#include "DataContainer.h"
#include "EventInfo.h"
#include "ReducedEvent/AliReducedEventInfo.h"
#include "ReducedEvent/AliReducedVarManager.h"
#include "ReducedEvent/AliReducedTrackInfo.h"
#include "ReducedEvent/AliReducedBaseTrack.h"
#include "CorrectionsInterface.h"

#define VAR AliReducedVarManager

namespace Qn {
class Task {
 public:
  Task() = default;
  Task(std::array<std::shared_ptr<TFile>, 4> files);
  ~Task() = default;
  void Run();
 private:
  /**
   * Initializes task;
   */
  void Initialize();
  /**
   * Processes one event;
   */
  void Process();
  /**
   * Finalizes task. Called after processing is done.
   */
  void Finalize();

 private:
  std::shared_ptr<TFile> in_file_;
  std::shared_ptr<TFile> out_file_;
  std::shared_ptr<TFile> in_calibration_file_;
  std::shared_ptr<TFile> out_calibration_file_;
  std::unique_ptr<TTree> in_tree_;
  std::unique_ptr<TTree> out_tree_;
  TTreeReader tree_reader_;
  TTreeReaderValue<AliReducedEventInfo> event_;
  std::unique_ptr<Qn::DataContainerQn> qn_data_;
  std::unique_ptr<Qn::EventInfoF> qn_eventinfo_f_;
  std::unique_ptr<Qn::EventInfoI> qn_eventinfo_i_;
  QnCorrectionsManager qn_manager_;

  void AddTPC(QnCorrectionsManager &manager) {
//    const Int_t nTPCdim = 2;
//    QnCorrectionsEventClassVariablesSet *CorrEventClasses = new QnCorrectionsEventClassVariablesSet(nTPCdim);
//    Double_t VtxZbinning[][2] = {{-10.0, 4}, {-7.0, 1}, {7.0, 8}, {10.0, 1}};
//    Double_t Ctbinning[][2] = {{0.0, 2}, {100.0, 100}};
//    CorrEventClasses->Add(new QnCorrectionsEventClassVariable(VAR::kVtxZ, VAR::GetVarName(VAR::kVtxZ), VtxZbinning));
//    CorrEventClasses->Add(new QnCorrectionsEventClassVariable(VAR::kCentVZERO,
//                                                              Form("Centrality (%s)",
//                                                                   VAR::GetVarName(VAR::kCentVZERO).Data()),
//                                                              Ctbinning));
    ////////// end of binning
//    QnCorrectionsDetectorConfigurationTracks
//        *TPCconf = new QnCorrectionsDetectorConfigurationTracks("TPC", CorrEventClasses, 4);
//    TPCconf->SetQVectorNormalizationMethod(QnCorrectionsQnVector::QVNORM_QoverM);
//    TPCconf->AddCorrectionOnQnVector(new QnCorrectionsQnVectorRecentering());
//    QnCorrectionsQnVectorTwistAndRescale *twScale = new QnCorrectionsQnVectorTwistAndRescale();
//    twScale->SetApplyTwist(kTRUE);
//    twScale->SetApplyRescale(kFALSE);
//    twScale->SetTwistAndRescaleMethod(QnCorrectionsQnVectorTwistAndRescale::TWRESCALE_doubleHarmonic);
//    TPCconf->AddCorrectionOnQnVector(dynamic_cast<QnCorrectionsCorrectionOnQvector *>(twScale));
//    QnCorrectionsCutsSet *cutsTPC = new QnCorrectionsCutsSet();
//    cutsTPC->Add(new QnCorrectionsCutWithin(VAR::kEta, -0.8, 0.8));
//    cutsTPC->Add(new QnCorrectionsCutWithin(VAR::kPt, 0.2, 5.));
//    TPCconf->SetCuts(cutsTPC);
//    TPC->AddDetectorConfiguration(TPCconf);
//    manager.AddDetector(TPC);
//    return TPCconf;
  }

  void AddVZERO(QnCorrectionsManager &manager) {
    Bool_t VZEROchannels[4][64];
    for (Int_t iv0 = 0; iv0 < 4; iv0++) for (Int_t ich = 0; ich < 64; ich++) VZEROchannels[iv0][ich] = kFALSE;
    for (Int_t ich = 32; ich < 64; ich++)
      VZEROchannels[0][ich] = kTRUE;
    for (Int_t ich = 0; ich < 32; ich++) VZEROchannels[1][ich] = kTRUE;
    Int_t channelGroups[64];
    for (Int_t ich = 0; ich < 64; ich++) channelGroups[ich] = Int_t(ich / 8);
    const Int_t nVZEROdim = 2;
    QnCorrectionsEventClassVariablesSet *CorrEventClasses = new QnCorrectionsEventClassVariablesSet(nVZEROdim);
    Double_t VtxZbinning[][2] = {{-10.0, 4}, {-7.0, 1}, {7.0, 8}, {10.0, 1}};
    Double_t Ctbinning[][2] = {{0.0, 2}, {100.0, 100}};
    CorrEventClasses->Add(new QnCorrectionsEventClassVariable(VAR::kVtxZ,
                                                              VAR::GetVarName(VAR::kVtxZ), VtxZbinning));
    CorrEventClasses->Add(new QnCorrectionsEventClassVariable(VAR::kCentVZERO,
                                                              Form("Centrty (%s)",
                                                                   VAR::GetVarName(VAR::kCentVZERO).Data()),
                                                              Ctbinning));
    auto *VZERO = new QnCorrectionsDetector("VZERO", VAR::kVZERO);
    auto *VZEROAconf = new QnCorrectionsDetectorConfigurationChannels("VZEROA", CorrEventClasses, 64, 4);
    VZEROAconf->SetChannelsScheme(VZEROchannels[0], channelGroups);
    VZEROAconf->SetQVectorNormalizationMethod(QnCorrectionsQnVector::QVNORM_QoverM);
    auto *eqA = new QnCorrectionsInputGainEqualization();
    eqA->SetEqualizationMethod(QnCorrectionsInputGainEqualization::GEQUAL_averageEqualization);
    eqA->SetShift(1.0);
    eqA->SetScale(0.1);
    eqA->SetUseChannelGroupsWeights(kTRUE);
    VZEROAconf->AddCorrectionOnInputData(eqA);
    VZEROAconf->AddCorrectionOnQnVector(new QnCorrectionsQnVectorRecentering());
    auto *alignA = new QnCorrectionsQnVectorAlignment();
    alignA->SetHarmonicNumberForAlignment(2);
    alignA->SetReferenceConfigurationForAlignment("TPC");
    VZEROAconf->AddCorrectionOnQnVector(alignA);
    VZEROAconf->SetQACentralityVar(VAR::kCentVZERO);
    VZEROAconf->SetQAMultiplicityAxis(100, 0.0, 500.0);
    auto *twScaleA = new QnCorrectionsQnVectorTwistAndRescale();
    twScaleA->SetApplyTwist(kTRUE);
    twScaleA->SetApplyRescale(kTRUE);
    twScaleA->SetTwistAndRescaleMethod(QnCorrectionsQnVectorTwistAndRescale::TWRESCALE_correlations);
    twScaleA->SetReferenceConfigurationsForTwistAndRescale("TPC", "VZEROC");
    VZEROAconf->AddCorrectionOnQnVector(twScaleA);
    VZERO->AddDetectorConfiguration(VZEROAconf);
    auto *VZEROCconf = new QnCorrectionsDetectorConfigurationChannels("VZEROC", CorrEventClasses, 64, 4);
    VZEROCconf->SetChannelsScheme(VZEROchannels[1], channelGroups);
    VZEROCconf->SetQVectorNormalizationMethod(QnCorrectionsQnVector::QVNORM_QoverM);
    auto *eqC = new QnCorrectionsInputGainEqualization();
    eqC->SetEqualizationMethod(QnCorrectionsInputGainEqualization::GEQUAL_averageEqualization);
    eqC->SetShift(1.0);
    eqC->SetScale(0.1);
    eqC->SetUseChannelGroupsWeights(kTRUE);
    VZEROCconf->AddCorrectionOnInputData(eqC);
    VZEROCconf->AddCorrectionOnQnVector(new QnCorrectionsQnVectorRecentering());
    auto *alignC = new QnCorrectionsQnVectorAlignment();
    alignC->SetHarmonicNumberForAlignment(2);
    alignC->SetReferenceConfigurationForAlignment("TPC");
    VZEROCconf->AddCorrectionOnQnVector(alignC);
    VZEROCconf->SetQACentralityVar(VAR::kCentVZERO);
    VZEROCconf->SetQAMultiplicityAxis(100, 0.0, 500.0);
    auto *twScaleC = new QnCorrectionsQnVectorTwistAndRescale();
    twScaleC->SetApplyTwist(kTRUE);
    twScaleC->SetApplyRescale(kTRUE);
    twScaleC->SetTwistAndRescaleMethod(QnCorrectionsQnVectorTwistAndRescale::TWRESCALE_correlations);
    twScaleC->SetReferenceConfigurationsForTwistAndRescale("TPC", "VZEROA");
    VZEROCconf->AddCorrectionOnQnVector(static_cast<QnCorrectionsCorrectionOnQvector *>(twScaleC));
    VZERO->AddDetectorConfiguration(VZEROCconf);
    manager.AddDetector(VZERO);
  }

};
}
#endif //FLOW_QNTASK_H
