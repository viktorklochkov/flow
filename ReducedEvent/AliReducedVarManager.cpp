/*
***********************************************************
  Implementation of AliVarManager
  Contact: iarsene@cern.ch
  2015/04/16
  *********************************************************
*/

#ifndef ALIREDUCEDVARMANAGER_H
#include "AliReducedVarManager.h"
#endif

#include <iostream>
using std::cout;
using std::endl;
//using std::flush;
//using std::ifstream;
//#include <fstream>

#include <TString.h>
#include <TMath.h>
#include <TLorentzVector.h>
#include <TChain.h>
#include <TH2F.h>
#include <TProfile.h>
#include <TRandom.h>
#include <TProfile2D.h>
#include <TFile.h>
#include <THashList.h>

#include "AliReducedBaseEvent.h"
#include "AliReducedEventInfo.h"
#include "AliReducedBaseTrack.h"
#include "AliReducedEventPlaneInfo.h"
#include "AliReducedTrackInfo.h"
#include "AliReducedPairInfo.h"
#include "AliReducedCaloClusterInfo.h"

#define BASEEVENT AliReducedBaseEvent
#define EVENT AliReducedEventInfo
#define TRACK AliReducedTrackInfo
#define PAIR  AliReducedPairInfo
#define EVENTPLANE AliReducedEventPlaneInfo
#define BASETRACK AliReducedBaseTrack
#define CLUSTER AliReducedCaloClusterInfo

ClassImp(AliReducedVarManager)

const Float_t AliReducedVarManager::fgkParticleMass[AliReducedVarManager::kNSpecies] = {
    0.000511,     // electron
    0.13957,      // pion+-
    0.493677,     // kaon+-
    0.938272,     // proton
    0.497614,     // K0
    1.019455      // phi(1020) meson
};

const Float_t AliReducedVarManager::fgkPairMass[AliReducedPairInfo::kNMaxCandidateTypes] =
    {
        0.0, // gamma
        0.497614, //K0s
        1.11568, //Lambda
        1.11568, //ALambda
        1.019455, // Phi
        3.09691599, //Jpsi
        9.460300, //Upsilon
        1.86962, // D+-
        1.86962, // D+-
        1.86962, // D+-
        1.86962, // D+-
        1.86962, // D+-
        1.86962, // D+-
        1.86484, // D0
        1.86962, // D+-
        1.96850, // Ds
    };

const Char_t *AliReducedVarManager::fgkTrackingStatusNames[AliReducedVarManager::kNTrackingStatus] = {
    "kITSin", "kITSout", "kITSrefit", "kITSpid",
    "kTPCin", "kTPCout", "kTPCrefit", "kTPCpid",
    "kTRDin", "kTRDout", "kTRDrefit", "kTRDpid",
    "kTOFin", "kTOFout", "kTOFrefit", "kTOFpid", "kTOFmismatch",
    "kHMPIDout", "kHMPIDpid",
    "kEMCALmatch", "kPHOSmatch",
    "kTRDbackup", "kTRDStop",
    "kESDpid", "kTIME", "kGlobalMerge",
    "kITSpureSA",
    "kMultInV0",
    "kMultSec",
    "kTRDnPlanes",
    "kEMCALNoMatch"
};

const Char_t *AliReducedVarManager::fgkOfflineTriggerNames[64] = {
    "MB/INT1", "INT7", "MUON", "HighMult/HighMultSPD",
    "EMC1", "CINT5/INT5", "CMUS5/MUSPB/INT7inMUON", "MuonSingleHighPt7/MUSH7/MUSHPB",
    "MuonLikeLowPt7/MUL7/MuonLikePB", "MuonUnlikeLowPt7/MUU7/MuonUnlikePB", "EMC7/EMC8", "MUS7/MuonSingleLowPt7",
    "PHI1", "PHI7/PHI8/PHOSPb", "EMCEJE", "EMCEGA",
    "Central/HighMultV0", "SemiCentral", "DG/DG5", "ZED",
    "SPI7/SPI", "INT8", "MuonSingleLowPt8", "MuonSingleHighPt8",
    "MuonLikeLowPt8", "MuonUnlikeLowPt8", "MuonUnlikeLowPt0/INT6", "UserDefined",
    "TRD", "N/A", "FastOnly", "N/A",
    "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A"
};

// radii of VZERO channels centers (in cm)
const Double_t AliReducedVarManager::fgkVZEROChannelRadii[64] = {
    6.0567, 6.0567, 6.0567, 6.0567, 6.0567, 6.0567, 6.0567, 6.0567,
    9.6977, 9.6977, 9.6977, 9.6977, 9.6977, 9.6977, 9.6977, 9.6977,
    15.9504, 15.9504, 15.9504, 15.9504, 15.9504, 15.9504, 15.9504, 15.9504,
    26.4031, 26.4031, 26.4031, 26.4031, 26.4031, 26.4031, 26.4031, 26.4031,
    5.9347, 5.9347, 5.9347, 5.9347, 5.9347, 5.9347, 5.9347, 5.9347,
    10.685, 10.685, 10.685, 10.685, 10.685, 10.685, 10.685, 10.685,
    18.116, 18.116, 18.116, 18.116, 18.116, 18.116, 18.116, 18.116,
    31.84, 31.84, 31.84, 31.84, 31.84, 31.84, 31.84, 31.84
};
const Double_t AliReducedVarManager::fgkVZEROAz = 340.0;   // cm
const Double_t AliReducedVarManager::fgkVZEROCz = 90.0;    // cm
const Double_t AliReducedVarManager::fgkVZEROminMult = 0.5;   // minimum VZERO channel multiplicity
Float_t  AliReducedVarManager::fgBeamMomentum = 1380.;   // beam momentum in GeV/c

Int_t      AliReducedVarManager::fgCurrentRunNumber = -1;
TString AliReducedVarManager::fgVariableNames[AliReducedVarManager::kNVars] = {""};
TString AliReducedVarManager::fgVariableUnits[AliReducedVarManager::kNVars] = {""};
AliReducedBaseEvent *AliReducedVarManager::fgEvent = 0x0;
AliReducedEventPlaneInfo *AliReducedVarManager::fgEventPlane = 0x0;
Bool_t AliReducedVarManager::fgUsedVars[AliReducedVarManager::kNVars] = {kFALSE};
TH2F *AliReducedVarManager::fgTPCelectronCentroidMap = 0x0;
TH2F *AliReducedVarManager::fgTPCelectronWidthMap = 0x0;
AliReducedVarManager::Variables AliReducedVarManager::fgVarDependencyX = kNothing;
AliReducedVarManager::Variables AliReducedVarManager::fgVarDependencyY = kNothing;
TH2F *AliReducedVarManager::fgPairEffMap = 0x0;
AliReducedVarManager::Variables AliReducedVarManager::fgEffMapVarDependencyX = kNothing;
AliReducedVarManager::Variables AliReducedVarManager::fgEffMapVarDependencyY = kNothing;
TH1F *AliReducedVarManager::fgRunTotalLuminosity = 0x0;
TH1F *AliReducedVarManager::fgRunTotalIntensity0 = 0x0;
TH1F *AliReducedVarManager::fgRunTotalIntensity1 = 0x0;
TH1I *AliReducedVarManager::fgRunLHCFillNumber = 0x0;
TH1I *AliReducedVarManager::fgRunDipolePolarity = 0x0;
TH1I *AliReducedVarManager::fgRunL3Polarity = 0x0;
TH1I *AliReducedVarManager::fgRunTimeStart = 0x0;
TH1I *AliReducedVarManager::fgRunTimeEnd = 0x0;
std::vector<Int_t>  AliReducedVarManager::fgRunNumbers;
Int_t AliReducedVarManager::fgRunID = -1;
TH1 *AliReducedVarManager::fgAvgMultVsVtxGlobal[kNMultiplicityEstimators] = {0x0};
TH1 *AliReducedVarManager::fgAvgMultVsVtxRunwise[kNMultiplicityEstimators] = {0x0};
TH1 *AliReducedVarManager::fgAvgMultVsRun[kNMultiplicityEstimators] = {0x0};
TH2 *AliReducedVarManager::fgAvgMultVsVtxAndRun[kNMultiplicityEstimators] = {0x0};

Double_t AliReducedVarManager::fgRefMultVsVtxGlobal[kNMultiplicityEstimators][kNReferenceMultiplicities] = {{0.}};
Double_t AliReducedVarManager::fgRefMultVsVtxRunwise[kNMultiplicityEstimators][kNReferenceMultiplicities] = {{0.}};
Double_t AliReducedVarManager::fgRefMultVsRun[kNMultiplicityEstimators][kNReferenceMultiplicities] = {{0.}};
Double_t AliReducedVarManager::fgRefMultVsVtxAndRun[kNMultiplicityEstimators][kNReferenceMultiplicities] = {{0.}};

TString AliReducedVarManager::fgVZEROCalibrationPath = "";
TProfile2D *AliReducedVarManager::fgAvgVZEROChannelMult[64] = {0x0};
TProfile2D *AliReducedVarManager::fgVZEROqVecRecentering[4] = {0x0};
Bool_t AliReducedVarManager::fgOptionCalibrateVZEROqVec = kFALSE;
Bool_t AliReducedVarManager::fgOptionRecenterVZEROqVec = kFALSE;

//__________________________________________________________________
AliReducedVarManager::AliReducedVarManager() :
    TObject() {
  //
  // constructor
  //
//  SetDefaultVarNames();
}

//__________________________________________________________________
AliReducedVarManager::AliReducedVarManager(const Char_t *name) :
    TObject() {
  std::cout << kNVars << std::endl;
  //
  // named constructor
  //
//  SetDefaultVarNames();
}

//__________________________________________________________________
AliReducedVarManager::~AliReducedVarManager() {
  //
  // destructor
  //
}

//__________________________________________________________________
void AliReducedVarManager::SetVariableDependencies() {
  //
  // Set as used those variables on which other variables calculation depends
  //
  if (fgUsedVars[kDeltaVtxZ]) {
    fgUsedVars[kVtxZ] = kTRUE;
    fgUsedVars[kVtxZtpc] = kTRUE;
  }
  if (fgUsedVars[kRap]) {
    fgUsedVars[kMass] = kTRUE;
    fgUsedVars[kP] = kTRUE;
  }
  if (fgUsedVars[kEta]) fgUsedVars[kP] = kTRUE;

  for (Int_t ih = 0; ih < 6; ++ih) {
    if (fgUsedVars[kVZEROQvecX + 2*6 + ih]) {
      fgUsedVars[kVZEROQvecX + 0*6 + ih] = kTRUE;
      fgUsedVars[kVZEROQvecX + 1*6 + ih] = kTRUE;
    }
    if (fgUsedVars[kVZEROQvecY + 2*6 + ih]) {
      fgUsedVars[kVZEROQvecY + 0*6 + ih] = kTRUE;
      fgUsedVars[kVZEROQvecY + 1*6 + ih] = kTRUE;
    }
    if (fgUsedVars[kVZERORP + 2*6 + ih]) {
      fgUsedVars[kVZEROQvecX + 2*6 + ih] = kTRUE;
      fgUsedVars[kVZEROQvecY + 2*6 + ih] = kTRUE;
      fgUsedVars[kVZEROQvecX + 0*6 + ih] = kTRUE;
      fgUsedVars[kVZEROQvecX + 1*6 + ih] = kTRUE;
      fgUsedVars[kVZEROQvecY + 0*6 + ih] = kTRUE;
      fgUsedVars[kVZEROQvecY + 1*6 + ih] = kTRUE;
    }
    if (fgUsedVars[kVZEROQaQcSP + ih] || fgUsedVars[kVZEROQaQcSPsine + ih]) {
      fgUsedVars[kVZERORP + 0*6 + ih] = kTRUE;
      fgUsedVars[kVZERORP + 1*6 + ih] = kTRUE;
      fgUsedVars[kVZEROQvecX + 0*6 + ih] = kTRUE;
      fgUsedVars[kVZEROQvecX + 1*6 + ih] = kTRUE;
      fgUsedVars[kVZEROQvecY + 0*6 + ih] = kTRUE;
      fgUsedVars[kVZEROQvecY + 1*6 + ih] = kTRUE;
    }
    if (fgUsedVars[kRPXtpcXvzeroa + ih]) {
      fgUsedVars[kTPCQvecX + ih] = kTRUE;
      fgUsedVars[kVZEROQvecX + ih] = kTRUE;
    }
    if (fgUsedVars[kRPXtpcXvzeroc + ih]) {
      fgUsedVars[kTPCQvecX + ih] = kTRUE;
      fgUsedVars[kVZEROQvecX + 6 + ih] = kTRUE;
    }
    if (fgUsedVars[kRPYtpcYvzeroa + ih]) {
      fgUsedVars[kTPCQvecY + ih] = kTRUE;
      fgUsedVars[kVZEROQvecY + ih] = kTRUE;
    }
    if (fgUsedVars[kRPYtpcYvzeroc + ih]) {
      fgUsedVars[kTPCQvecY + ih] = kTRUE;
      fgUsedVars[kVZEROQvecY + 6 + ih] = kTRUE;
    }
    if (fgUsedVars[kRPXtpcYvzeroa + ih]) {
      fgUsedVars[kTPCQvecX + ih] = kTRUE;
      fgUsedVars[kVZEROQvecY + ih] = kTRUE;
    }
    if (fgUsedVars[kRPXtpcYvzeroc + ih]) {
      fgUsedVars[kTPCQvecX + ih] = kTRUE;
      fgUsedVars[kVZEROQvecY + 6 + ih] = kTRUE;
    }
    if (fgUsedVars[kRPYtpcXvzeroa + ih]) {
      fgUsedVars[kTPCQvecY + ih] = kTRUE;
      fgUsedVars[kVZEROQvecX + ih] = kTRUE;
    }
    if (fgUsedVars[kRPYtpcXvzeroc + ih]) {
      fgUsedVars[kTPCQvecY + ih] = kTRUE;
      fgUsedVars[kVZEROQvecX + 6 + ih] = kTRUE;
    }
    if (fgUsedVars[kRPdeltaVZEROAtpc + ih]) {
      fgUsedVars[kVZERORP + 0*6 + ih] = kTRUE;
      fgUsedVars[kTPCRP + ih] = kTRUE;
    }
    if (fgUsedVars[kRPdeltaVZEROCtpc + ih]) {
      fgUsedVars[kVZERORP + 1*6 + ih] = kTRUE;
      fgUsedVars[kTPCRP + ih] = kTRUE;
    }
    if (fgUsedVars[kTPCsubResCos + ih]) {
      fgUsedVars[kTPCRPleft + ih] = kTRUE;
      fgUsedVars[kTPCRPright + ih] = kTRUE;
    }
    for (Int_t iVZEROside = 0; iVZEROside < 3; ++iVZEROside) {
      if (fgUsedVars[kVZEROFlowVn + iVZEROside*6 + ih] || fgUsedVars[kVZEROFlowSine + iVZEROside*6 + ih] ||
          fgUsedVars[kVZEROuQ + iVZEROside*6 + ih] || fgUsedVars[kVZEROuQsine + iVZEROside*6 + ih]) {
        fgUsedVars[kPhi] = kTRUE;
        fgUsedVars[kVZERORP + iVZEROside*6 + ih] = kTRUE;
        if (iVZEROside < 2
            && (fgUsedVars[kVZEROuQ + iVZEROside*6 + ih] || fgUsedVars[kVZEROuQsine + iVZEROside*6 + ih])) {
          fgUsedVars[kVZEROQvecX + 0*6 + ih] = kTRUE;
          fgUsedVars[kVZEROQvecX + 1*6 + ih] = kTRUE;
          fgUsedVars[kVZEROQvecY + 0*6 + ih] = kTRUE;
          fgUsedVars[kVZEROQvecY + 1*6 + ih] = kTRUE;
        }
        if (iVZEROside==2) {
          fgUsedVars[kVZEROQvecX + 2*6 + ih] = kTRUE;
          fgUsedVars[kVZEROQvecY + 2*6 + ih] = kTRUE;
          fgUsedVars[kVZEROQvecX + 0*6 + ih] = kTRUE;
          fgUsedVars[kVZEROQvecX + 1*6 + ih] = kTRUE;
          fgUsedVars[kVZEROQvecY + 0*6 + ih] = kTRUE;
          fgUsedVars[kVZEROQvecY + 1*6 + ih] = kTRUE;
        }
      }
    }
    if (fgUsedVars[kTPCFlowVn + ih] || fgUsedVars[kTPCFlowSine + ih] || fgUsedVars[kTPCuQ + ih]
        || fgUsedVars[kTPCuQsine + ih]) {
      fgUsedVars[kPhi] = kTRUE;
      fgUsedVars[kTPCQvecXtotal + ih] = kTRUE;
      fgUsedVars[kTPCQvecYtotal + ih] = kTRUE;
    }

  } // end loop over harmonics
  for (Int_t ich = 0; ich < 64; ++ich) {
    if (fgUsedVars[kVZEROflowV2TPC + ich]) {
      fgUsedVars[kVZEROChannelMult + ich] = kTRUE;
      fgUsedVars[kTPCRP + 1] = kTRUE;
    }
  }
  if (fgUsedVars[kPtSquared]) fgUsedVars[kPt] = kTRUE;
  if (fgUsedVars[kTPCnSigCorrected + kElectron]) {
    fgUsedVars[kTPCnSig + kElectron] = kTRUE;
    fgUsedVars[fgVarDependencyX] = kTRUE;
    fgUsedVars[fgVarDependencyY] = kTRUE;
  }
  if (fgUsedVars[kPairEff] || fgUsedVars[kOneOverPairEff] || fgUsedVars[kOneOverPairEffSq]) {
    fgUsedVars[fgEffMapVarDependencyX] = kTRUE;
    fgUsedVars[fgEffMapVarDependencyY] = kTRUE;
  }
  if (fgUsedVars[kNTracksITSoutVsSPDtracklets] || fgUsedVars[kNTracksTPCoutVsSPDtracklets] ||
      fgUsedVars[kNTracksTOFoutVsSPDtracklets] || fgUsedVars[kNTracksTRDoutVsSPDtracklets])
    fgUsedVars[kSPDntracklets] = kTRUE;

  if (fgUsedVars[kRapMC]) fgUsedVars[kMassMC] = kTRUE;

  if (fgUsedVars[kPairPhiV]) {
    fgUsedVars[kL3Polarity] = kTRUE;
  }
  if (fgUsedVars[kMassDcaPtCorr]) {
    fgUsedVars[kMass] = kTRUE;
    fgUsedVars[kPt] = kTRUE;
    fgUsedVars[kPairDcaXYSqrt] = kTRUE;
  }
  if (fgUsedVars[kOpAngDcaPtCorr]) {
    fgUsedVars[kPairOpeningAngle] = kTRUE;
    fgUsedVars[kOneOverSqrtPt] = kTRUE;
    fgUsedVars[kPt] = kTRUE;
    fgUsedVars[kPairDcaXYSqrt] = kTRUE;
  }
}

//__________________________________________________________________
void AliReducedVarManager::FillEventInfo(double *values) {
  //
  // Fill event information
  //
  FillEventInfo(fgEvent, values, fgEventPlane);
}

//__________________________________________________________________
void AliReducedVarManager::FillEventInfo(BASEEVENT *baseEvent, double *values, EVENTPLANE *eventF/*=0x0*/) {
  //
  // fill event wise info
  //
  // Basic event information

  values[kUnity] = 1;
  values[kVtxX] = baseEvent->Vertex(0);
  values[kVtxY] = baseEvent->Vertex(1);
  values[kVtxZ] = baseEvent->Vertex(2);
  values[kNVtxContributors] = baseEvent->VertexNContributors();

  values[kCentVZERO] = baseEvent->CentralityVZERO();
  values[kCentSPD] = baseEvent->CentralitySPD();
  values[kCentTPC] = baseEvent->CentralityTPC();
  values[kCentZDC] = baseEvent->CentralityZEMvsZDC();
  values[kCentVZEROA] = baseEvent->CentralityVZEROA();
  values[kCentVZEROC] = baseEvent->CentralityVZEROC();
  values[kCentZNA] = baseEvent->CentralityZNA();
  values[kCentQuality] = baseEvent->CentralityQuality();

  values[kNV0total] = baseEvent->NV0CandidatesTotal();
  values[kNV0selected] = baseEvent->NV0Candidates();
  values[kNtracksTotal] = baseEvent->NTracksTotal();
//  values[kNtracksSelected] = baseEvent->NTracks();
  int selected = 0;
  int tpcout = 0;
  auto arr = baseEvent->GetTracks();
  auto ntracks = arr->GetEntries();
  for (int i = 0; i < ntracks; ++i) {
    auto track = (AliReducedTrackInfo *) arr->At(i);
    if (track->TestQualityFlag(15 + 7) || track->TestQualityFlag(15 + 8)) {
      selected++;
      if (track->CheckTrackStatus(kTPCout)) tpcout++;
    }

  }
  values[kNtracksSelected] = selected;
  values[kNTracksPerTrackingStatus + kTPCout] = tpcout;

  if (baseEvent->IsA()!=EVENT::Class()) return;

  EVENT *event = (EVENT *) baseEvent;

  // Update run wise information if available (needed for the first event filled and whenever the run changes)
  if (fgCurrentRunNumber!=baseEvent->RunNo()) {
    fgCurrentRunNumber = baseEvent->RunNo();
    // GRP and LHC information
    if (fgRunTotalLuminosity)
      values[kTotalLuminosity] =
          fgRunTotalLuminosity->GetBinContent(fgRunTotalLuminosity->GetXaxis()->FindBin(Form("%d",
                                                                                             fgCurrentRunNumber)));
    if (fgRunTotalIntensity0)
      values[kBeamIntensity0] = fgRunTotalIntensity0->GetBinContent(fgRunTotalIntensity0->GetXaxis()->FindBin(Form("%d",
                                                                                                                   fgCurrentRunNumber)));
    if (fgRunTotalIntensity1)
      values[kBeamIntensity1] = fgRunTotalIntensity1->GetBinContent(fgRunTotalIntensity1->GetXaxis()->FindBin(Form("%d",
                                                                                                                   fgCurrentRunNumber)));
    if (fgRunLHCFillNumber)
      values[kLHCFillNumber] = fgRunLHCFillNumber->GetBinContent(fgRunLHCFillNumber->GetXaxis()->FindBin(Form("%d",
                                                                                                              fgCurrentRunNumber)));
    if (fgRunDipolePolarity)
      values[kDipolePolarity] = fgRunDipolePolarity->GetBinContent(fgRunDipolePolarity->GetXaxis()->FindBin(Form("%d",
                                                                                                                 fgCurrentRunNumber)));
    if (fgRunL3Polarity)
      values[kL3Polarity] = fgRunL3Polarity->GetBinContent(fgRunL3Polarity->GetXaxis()->FindBin(Form("%d",
                                                                                                     fgCurrentRunNumber)));
    if (fgRunTimeStart)
      values[kRunTimeStart] = fgRunTimeStart->GetBinContent(fgRunTimeStart->GetXaxis()->FindBin(Form("%d",
                                                                                                     fgCurrentRunNumber)));
    if (fgRunTimeEnd)
      values[kRunTimeEnd] = fgRunTimeEnd->GetBinContent(fgRunTimeEnd->GetXaxis()->FindBin(Form("%d",
                                                                                               fgCurrentRunNumber)));
  }

//  if (fgUsedVars[kRunID] && fgRunNumbers.size() && fgRunID < 0) {
//    for (fgRunID = 0; fgRunNumbers[fgRunID]!=fgCurrentRunNumber && fgRunID < (Int_t) fgRunNumbers.size(); ++fgRunID);
//  }
//  for (int iEstimator = 0; iEstimator < kNMultiplicityEstimators; ++iEstimator) {
//    if (fgAvgMultVsVtxAndRun[iEstimator]) {
//      Bool_t fillGlobal = !fgAvgMultVsVtxGlobal[iEstimator];
//      fgAvgMultVsVtxRunwise[iEstimator] =
//          fgAvgMultVsVtxAndRun[iEstimator]->ProfileY(Form("AvgMultVsVtxRunwise%d", iEstimator), fgRunID, fgRunID);
//      if (fillGlobal) {
//        fgAvgMultVsVtxGlobal[iEstimator] =
//            fgAvgMultVsVtxAndRun[iEstimator]->ProfileY(Form("AvgMultVsVtxGlobal%d", iEstimator));
//        fgAvgMultVsRun[iEstimator] = fgAvgMultVsVtxAndRun[iEstimator]->ProfileX(Form("AvgMultVsRun%d", iEstimator));
//      }
//      for (int iReference = 0; iReference < kNReferenceMultiplicities; ++iReference) {
//        Double_t refVsVtx, refVsVtxGlobal, refVsRun;
//        switch (iReference) {
//          case kMaximumMultiplicity :refVsVtx = fgAvgMultVsVtxRunwise[iEstimator]->GetMaximum();
//            if (fillGlobal) {
//              refVsVtxGlobal = fgAvgMultVsVtxGlobal[iEstimator]->GetMaximum();
//              refVsRun = fgAvgMultVsVtxAndRun[iEstimator]->GetMaximum();
//            }
//            break;
//          case kMinimumMultiplicity :refVsVtx = fgAvgMultVsVtxRunwise[iEstimator]->GetMinimum();
//            if (fillGlobal) {
//              refVsVtxGlobal = fgAvgMultVsVtxGlobal[iEstimator]->GetMinimum();
//              refVsRun = fgAvgMultVsVtxAndRun[iEstimator]->GetMinimum();
//            }
//            break;
//          case kMeanMultiplicity :
//            refVsVtx = 0.5
//                *(fgAvgMultVsVtxRunwise[iEstimator]->GetMaximum() + fgAvgMultVsVtxRunwise[iEstimator]->GetMinimum());
//            if (fillGlobal) {
//              refVsVtxGlobal =
//                  0.5*(fgAvgMultVsVtxGlobal[iEstimator]->GetMaximum() + fgAvgMultVsVtxGlobal[iEstimator]->GetMinimum());
//              refVsRun =
//                  0.5*(fgAvgMultVsVtxAndRun[iEstimator]->GetMaximum() + fgAvgMultVsVtxAndRun[iEstimator]->GetMinimum());
//            }
//            break;
//        }
//        fgRefMultVsVtxRunwise[iEstimator][iReference] = refVsVtx;
//        if (fillGlobal) {
//          fgRefMultVsVtxGlobal[iEstimator][iReference] = refVsVtxGlobal;
//          fgRefMultVsRun[iEstimator][iReference] = refVsRun;
//        }
//      }
//    }
//  }

  values[kRunNo] = fgCurrentRunNumber;
  values[kRunID] = fgRunID;

  values[kEventNumberInFile] = event->EventNumberInFile();
  values[kBC] = event->BC();
  values[kTimeStamp] = event->TimeStamp();
  if (fgUsedVars[kTimeRelativeSOR]) values[kTimeRelativeSOR] = (event->TimeStamp() - values[kRunTimeStart])/60.;
  if (fgUsedVars[kTimeRelativeSORfraction] &&
      (values[kRunTimeEnd] - values[kRunTimeStart]) > 1.)   // the run should be longer than 1 second ...
    values[kTimeRelativeSORfraction] =
        (event->TimeStamp() - values[kRunTimeStart])/(values[kRunTimeEnd] - values[kRunTimeStart]);
  values[kEventType] = event->EventType();
  values[kTriggerMask] = event->TriggerMask();
  values[kINT7Triggered] = event->TriggerMask() & kINT7 ? 1 : 0;
  values[kHighMultV0Triggered] = event->TriggerMask() & kHighMultV0 ? 1 : 0;
  values[kIsPhysicsSelection] = (event->IsPhysicsSelection() ? 1.0 : 0.0);
  values[kIsSPDPileup] = event->IsSPDPileup();
  values[kIsSPDPileup5] = event->EventTag(11);
  values[kIsPileupMV] = event->EventTag(1);
  values[kIsSPDPileupMultBins] = event->IsSPDPileupMultBins();
  values[kNSPDpileups] = event->NpileupSPD();
  values[kNTrackPileups] = event->NpileupTracks();
  values[kIRIntClosestIntMap] = event->IRIntClosestIntMap(0);
  values[kIRIntClosestIntMap + 1] = event->IRIntClosestIntMap(1);
  values[kNPMDtracks] = event->NPMDtracks();
  values[kNTRDtracks] = event->NTRDtracks();
  values[kNTRDtracklets] = event->NTRDtracklets();
  values[kNVtxTPCContributors] = event->VertexTPCContributors();
  values[kVtxXtpc] = event->VertexTPC(0);
  values[kVtxYtpc] = event->VertexTPC(1);
  values[kVtxZtpc] = event->VertexTPC(2);
  values[kNVtxTPCContributors] = event->VertexTPCContributors();
  values[kVtxXspd] = event->VertexSPD(0);
  values[kVtxYspd] = event->VertexSPD(1);
  values[kVtxZspd] = event->VertexSPD(2);
  values[kNVtxSPDContributors] = event->VertexSPDContributors();

  if (fgUsedVars[kDeltaVtxZ]) values[kDeltaVtxZ] = values[kVtxZ] - values[kVtxZtpc];
  if (fgUsedVars[kDeltaVtxZspd]) values[kDeltaVtxZspd] = values[kVtxZ] - values[kVtxZspd];

  //for(Int_t iflag=0;iflag<32;++iflag)
  //  values[kNTracksPerTrackingStatus+iflag] = event->TracksPerTrackingFlag(iflag);

//  // set the fgUsedVars to true as these might have been set to false in the previous event
//  fgUsedVars[kNTracksTPCoutVsITSout] = kTRUE;
//  fgUsedVars[kNTracksTRDoutVsITSout] = kTRUE;
//  fgUsedVars[kNTracksTOFoutVsITSout] = kTRUE;
//  fgUsedVars[kNTracksTRDoutVsTPCout] = kTRUE;
//  fgUsedVars[kNTracksTOFoutVsTPCout] = kTRUE;
//  fgUsedVars[kNTracksTOFoutVsTRDout] = kTRUE;
//  if (TMath::Abs(values[kNTracksPerTrackingStatus + kITSout]) > 0.01) {
//    values[kNTracksTPCoutVsITSout] =
//        values[kNTracksPerTrackingStatus + kTPCout]/values[kNTracksPerTrackingStatus + kITSout];
//    values[kNTracksTRDoutVsITSout] =
//        values[kNTracksPerTrackingStatus + kTRDout]/values[kNTracksPerTrackingStatus + kITSout];
//    values[kNTracksTOFoutVsITSout] =
//        values[kNTracksPerTrackingStatus + kTOFout]/values[kNTracksPerTrackingStatus + kITSout];
//  } else {
//    // if these values are undefined, set fgUsedVars as false such that the values are not filled in histograms
//    fgUsedVars[kNTracksTPCoutVsITSout] = kFALSE;
//    fgUsedVars[kNTracksTRDoutVsITSout] = kFALSE;
//    fgUsedVars[kNTracksTOFoutVsITSout] = kFALSE;
//  }

//  if (TMath::Abs(values[kNTracksPerTrackingStatus + kTPCout]) > 0.01) {
//    values[kNTracksTRDoutVsTPCout] =
//        values[kNTracksPerTrackingStatus + kTRDout]/values[kNTracksPerTrackingStatus + kTPCout];
//    values[kNTracksTOFoutVsTPCout] =
//        values[kNTracksPerTrackingStatus + kTOFout]/values[kNTracksPerTrackingStatus + kTPCout];
//  } else {
//    fgUsedVars[kNTracksTRDoutVsTPCout] = kFALSE;
//    fgUsedVars[kNTracksTOFoutVsTPCout] = kFALSE;
//  }
//
//  if (TMath::Abs(values[kNTracksPerTrackingStatus + kTRDout]) > 0.01)
//    values[kNTracksTOFoutVsTRDout] =
//        values[kNTracksPerTrackingStatus + kTOFout]/values[kNTracksPerTrackingStatus + kTRDout];
//  else
//    fgUsedVars[kNTracksTOFoutVsTRDout] = kFALSE;

  // Multiplicity estimators

  if (event->GetFMD()!=nullptr) {
    auto fmd = event->GetFMD();
    TIter next(fmd);
    int iA = 0;
    int iC = 0;
    for (int i = 0; i < 1200; ++i) {
      values[kFMDAWeight + i] = 0;
      values[kFMDAPhi + i] = 0;
      values[kFMDAEta + i] = 0;
      values[kFMDCPhi + i] = 0;
      values[kFMDCWeight + i] = 0;
      values[kFMDCEta + i] = 0;
    }
    while (auto *info = (AliReducedFMDInfo *) next()) {
      if (info->Eta() > 0) {
        values[kFMDAWeight + iA] = info->Multiplicity();
        values[kFMDAPhi + iA] = info->Phi();
        values[kFMDAEta + iA] = info->Eta();
        ++iA;
      } else if (info->Eta() < 0) {
        values[kFMDCWeight + iC] = info->Multiplicity();
        values[kFMDCPhi + iC] = info->Phi();
        values[kFMDCEta + iC] = info->Eta();
        ++iC;
      }

    }
  }

  values[kVZEROATotalMult] = event->MultVZEROA();
  values[kVZEROCTotalMult] = event->MultVZEROC();
  values[kVZEROTotalMult] = event->MultVZERO();

  values[kVZEROACTotalMult] = event->MultVZEROA() + event->MultVZEROC();

  values[kSPDntracklets] = event->SPDntracklets();
  values[kSPDntracklets08] = 0.;
  values[kSPDntracklets16] = 0.;
  values[kSPDntrackletsOuterEta] = 0.;
  values[kSPDnTracklets10EtaVtxCorr] = 0.;

//  for (Int_t ieta = 0; ieta < 32; ++ieta) {
//    values[kSPDntrackletsEtaBin + ieta] = event->SPDntracklets(ieta);
//    if (ieta > 7 && ieta < 24) values[kSPDntracklets08] += event->SPDntracklets(ieta);
//    if (ieta < 7 || ieta > 24) values[kSPDntrackletsOuterEta] += event->SPDntracklets(ieta);
//  }

//  for (Int_t iEstimator = 0; iEstimator < kNMultiplicityEstimators; ++iEstimator) {
//    Int_t estimator = kMultiplicity + iEstimator;
//    if (estimator==kVZEROACTotalMult || estimator==kSPDnTracklets10EtaVtxCorr) {
//      if (fgAvgMultVsVtxAndRun[iEstimator]) {
//        for (Int_t iCorrection = 0; iCorrection < kNCorrections; ++iCorrection) {
//          for (Int_t iReference = 0; iReference < kNReferenceMultiplicities; ++iReference) {
//            Int_t indexNotSmeared = GetCorrectedMultiplicity(estimator, iCorrection, iReference, kNoSmearing);
//            Int_t indexSmeared = GetCorrectedMultiplicity(estimator, iCorrection, iReference, kPoissonSmearing);
//            values[indexNotSmeared] = 0.;
//            values[indexSmeared] = 0.;
//            if (estimator==kSPDnTracklets10EtaVtxCorr) {
//              for (Int_t ieta = 6; ieta < 26; ++ieta) {
//                Int_t indexBinNotSmeared =
//                    GetCorrectedMultiplicity(kSPDntrackletsEtaBin + ieta, iCorrection, iReference, kNoSmearing);
//                Int_t indexBinSmeared =
//                    GetCorrectedMultiplicity(kSPDntrackletsEtaBin + ieta, iCorrection, iReference, kPoissonSmearing);
//
//                if (fgUsedVars[indexBinNotSmeared]) values[indexNotSmeared] += values[indexBinNotSmeared];
//                if (fgUsedVars[indexBinSmeared]) values[indexSmeared] += values[indexBinSmeared];
//              }
//            } else {
//              Int_t indexAnotSmeared = GetCorrectedMultiplicity(kVZEROATotalMult, iCorrection, iReference, kNoSmearing);
//              Int_t indexCnotSmeared = GetCorrectedMultiplicity(kVZEROCTotalMult, iCorrection, iReference, kNoSmearing);
//
//              Int_t
//                  indexAsmeared = GetCorrectedMultiplicity(kVZEROATotalMult, iCorrection, iReference, kPoissonSmearing);
//              Int_t
//                  indexCsmeared = GetCorrectedMultiplicity(kVZEROCTotalMult, iCorrection, iReference, kPoissonSmearing);
//
//              values[indexNotSmeared] = values[indexAnotSmeared] + values[indexCnotSmeared];
//              values[indexSmeared] = values[indexAsmeared] + values[indexCsmeared];
//            }
//          }
//        }
//      }
//    }  // end if VZEROAC || kSPDnTracklets10EtaVtxCorr estimators

//    else {
//      if (fgAvgMultVsVtxAndRun[iEstimator]) {
//        Int_t vtxBin = fgAvgMultVsVtxAndRun[iEstimator]->GetYaxis()->FindBin(values[kVtxZ]);
//        Int_t runBin = fgAvgMultVsVtxAndRun[iEstimator]->GetXaxis()->FindBin(values[kRunID]);
//        Double_t multRaw = values[estimator];
//        for (Int_t iCorrection = 0; iCorrection < kNCorrections; ++iCorrection) {
//          for (Int_t iReference = 0; iReference < kNReferenceMultiplicities; ++iReference) {
//            Int_t indexNotSmeared = GetCorrectedMultiplicity(estimator, iCorrection, iReference, kNoSmearing);
//            Int_t indexSmeared = GetCorrectedMultiplicity(estimator, iCorrection, iReference, kPoissonSmearing);
//            Double_t multCorr = multRaw;
//            Double_t multCorrSmeared = multRaw;
//            // apply vertex and gain loss correction simultaneously
//            if (iCorrection==kVertexCorrection2D) {
//              Double_t localAvg = fgAvgMultVsVtxAndRun[iEstimator]->GetBinContent(vtxBin, runBin);
//              Double_t refMult = fgRefMultVsVtxAndRun[iEstimator][iReference];
//              multCorr *= localAvg ? refMult/localAvg : 1.;
//              Double_t deltaM = localAvg ? multRaw*(refMult/localAvg - 1) : 0.;
//              multCorrSmeared += (deltaM > 0 ? 1. : -1.)*gRandom->Poisson(TMath::Abs(deltaM));
//            } else {
//              // first apply vertex correction
//              Double_t localAvgVsVtx, refMultVsVtx;
//              switch (iCorrection) {
//                case kVertexCorrectionGlobal:
//                case kVertexCorrectionGlobalGainLoss:
//                  localAvgVsVtx = fgAvgMultVsVtxGlobal[iEstimator]->GetBinContent(vtxBin);
//                  refMultVsVtx = fgRefMultVsVtxGlobal[iEstimator][iReference];
//                  break;
//                case kVertexCorrectionRunwise:
//                case kVertexCorrectionRunwiseGainLoss:
//                  localAvgVsVtx = fgAvgMultVsVtxRunwise[iEstimator]->GetBinContent(vtxBin);
//                  refMultVsVtx = fgRefMultVsVtxRunwise[iEstimator][iReference];
//                  break;
//              }
//              multCorr *= localAvgVsVtx ? refMultVsVtx/localAvgVsVtx : 1.;
//              Double_t deltaM = localAvgVsVtx ? multRaw*(refMultVsVtx/localAvgVsVtx - 1) : 0.;
//              multCorrSmeared += (deltaM > 0 ? 1. : -1.)*gRandom->Poisson(TMath::Abs(deltaM));
//              // then apply gain loss correction
//              if (iCorrection==kVertexCorrectionGlobalGainLoss || iCorrection==kVertexCorrectionRunwiseGainLoss) {
//                Double_t localAvgVsRun = fgAvgMultVsRun[iEstimator]->GetBinContent(runBin);
//                Double_t refMultVsRun = fgRefMultVsRun[iEstimator][iReference];
//                multCorr *= localAvgVsRun ? refMultVsRun/localAvgVsRun : 1.;
//                deltaM = localAvgVsRun ? multCorrSmeared*(refMultVsRun/localAvgVsRun - 1) : 0;
//                multCorrSmeared += (deltaM > 0 ? 1. : -1.)*gRandom->Poisson(TMath::Abs(deltaM));
//              }
//            }
//            values[indexNotSmeared] = multCorr;
//            values[indexSmeared] = multCorrSmeared;
//            fgUsedVars[indexNotSmeared] = kTRUE;
//            fgUsedVars[indexSmeared] = kTRUE;
//
//          }
//        }
//      }
//    }  // end else
//  }  // end loop over multiplicity estimators

  fgUsedVars[kNTracksITSoutVsSPDtracklets] = kTRUE;
  fgUsedVars[kNTracksTPCoutVsSPDtracklets] = kTRUE;
  fgUsedVars[kNTracksTRDoutVsSPDtracklets] = kTRUE;
  fgUsedVars[kNTracksTOFoutVsSPDtracklets] = kTRUE;
  if (values[kSPDntracklets] > 0.01) {
    values[kNTracksITSoutVsSPDtracklets] = values[kNTracksPerTrackingStatus + kITSout]/values[kSPDntracklets];
    values[kNTracksTPCoutVsSPDtracklets] = values[kNTracksPerTrackingStatus + kTPCout]/values[kSPDntracklets];
    values[kNTracksTRDoutVsSPDtracklets] = values[kNTracksPerTrackingStatus + kTRDout]/values[kSPDntracklets];
    values[kNTracksTOFoutVsSPDtracklets] = values[kNTracksPerTrackingStatus + kTOFout]/values[kSPDntracklets];
  } else {
    fgUsedVars[kNTracksITSoutVsSPDtracklets] = kFALSE;
    fgUsedVars[kNTracksTPCoutVsSPDtracklets] = kFALSE;
    fgUsedVars[kNTracksTRDoutVsSPDtracklets] = kFALSE;
    fgUsedVars[kNTracksTOFoutVsSPDtracklets] = kFALSE;
  }

  values[kNCaloClusters] = event->GetNCaloClusters();
  values[kNTPCclusters] = event->NTPCClusters();

  for (Int_t i = 0; i < 2; ++i) values[kSPDFiredChips + i] = event->SPDFiredChips(i + 1);
  for (Int_t i = 0; i < 6; ++i) values[kITSnClusters + i] = event->ITSClusters(i + 1);
  values[kSPDnSingleClusters] = event->SPDnSingleClusters();

  //VZERO detector information
  fgUsedVars[kNTracksTPCoutVsVZEROTotalMult] = kTRUE;
  if (values[kVZEROTotalMult] > 1.0e-5)
    values[kNTracksTPCoutVsVZEROTotalMult] = values[kNTracksPerTrackingStatus + kTPCout]/values[kVZEROTotalMult];
  else
    fgUsedVars[kNTracksTPCoutVsVZEROTotalMult] = kFALSE;

  values[kVZEROAemptyChannels] = 0;
  values[kVZEROCemptyChannels] = 0;
  for (Int_t ich = 0; ich < 64; ++ich) fgUsedVars[kVZEROChannelMult + ich] = kTRUE;
  Float_t theta = 0.0;
  const std::array<double, 8> X = {{0.92388, 0.38268, -0.38268, -0.92388, -0.92388, -0.38268, 0.38268, 0.92388}};
  const std::array<double, 8> Y = {{0.38268, 0.92388, 0.92388, 0.38268, -0.38268, -0.92388, -0.92388, -0.38268}};
  for (Int_t ich = 0; ich < 64; ++ich) {
    if (fgUsedVars[kVZEROChannelMult + ich]) {
      values[kVZEROChannelPhi + ich] = TMath::Pi() - (float) TMath::ATan2(Y[ich%8], X[ich%8]);
      values[kVZEROChannelRing + ich] = ich < 32 ? ich/8 : (ich - 32)/8;
      values[kVZEROChannelMult + ich] = event->MultChannelVZERO(ich);
      if (values[kVZEROChannelMult + ich] < fgkVZEROminMult) {
        fgUsedVars[kVZEROChannelMult + ich] = kFALSE;   // will not be filled in histograms by the histogram manager
        if (ich < 32) values[kVZEROCemptyChannels] += 1;
        else values[kVZEROAemptyChannels] += 1;
      }
    }
    if (fgUsedVars[kVZEROChannelEta + ich]) {
      if (ich < 32) theta = TMath::ATan(fgkVZEROChannelRadii[ich]/(fgkVZEROCz - values[kVtxZ]));
      else theta = TMath::Pi() - TMath::ATan(fgkVZEROChannelRadii[ich]/(fgkVZEROAz - values[kVtxZ]));
      values[kVZEROChannelEta + ich] = -1.0*TMath::Log(TMath::Tan(theta/2.0));
    }
  }

  fgUsedVars[kNTracksTPCoutFromPileup] = kTRUE;
  if (values[kVZEROTotalMult] > 0.0)
    values[kNTracksTPCoutFromPileup] = values[kNTracksPerTrackingStatus + kTPCout]
        - (-2.55 + TMath::Sqrt(2.55*2.55 + 4.0e-5*values[kVZEROTotalMult]))/2.0e-5;
  else fgUsedVars[kNTracksTPCoutFromPileup] = kFALSE;

  constexpr std::array<double, 4> zdcX = {{1.75, -1.75, 1.75, -1.75}};
  constexpr std::array<double, 4> zdcY = {{-1.75, -1.75, 1.75, 1.75}};
  for (Int_t izdc = 0; izdc < 10; ++izdc) values[kZDCnEnergyCh + izdc] = event->EnergyZDCnTree(izdc);
  for (int izdc = 1; izdc < 5; ++izdc) values[kZDCPhi + izdc] = TMath::ATan2(zdcY[izdc - 1], zdcX[izdc - 1]);
  for (int izdc = 6; izdc < 10; ++izdc) values[kZDCPhi + izdc] = TMath::ATan2(zdcY[izdc - 6], zdcX[izdc - 6]);
  for (Int_t izdc = 0; izdc < 10; ++izdc) values[kZDCpEnergyCh + izdc] = event->EnergyZDCpTree(izdc);

  constexpr Double_t kt0X[24] =
      {/* Cside */ 0.905348, 0.571718, 0.0848977, -0.424671, -0.82045, -0.99639, -0.905348, -0.571718, -0.0848977,
                   0.424671, 0.82045, 0.99639,
          /* Aside */ 0.99995, 0.870982, 0.508635, 0.00999978, -0.491315, -0.860982, -0.99995, -0.870982, -0.508635,
                   -0.0100001, 0.491315, 0.860982};
  constexpr Double_t kt0Y[24] =
      {/* Cside */ 0.424671, 0.82045, 0.99639, 0.905348, 0.571718, 0.0848976, -0.424671, -0.82045, -0.99639, -0.905348,
                   -0.571719, -0.0848975,
          /* Aside */ -0.00999983, 0.491315, 0.860982, 0.99995, 0.870982, 0.508635, 0.00999974, -0.491315, -0.860982,
                   -0.99995, -0.870982, -0.508635};
  const Double_t
      t0phi[24] = {TMath::ATan2(kt0Y[0], kt0X[0]), TMath::ATan2(kt0Y[1], kt0X[1]), TMath::ATan2(kt0Y[2], kt0X[2]),
                   TMath::ATan2(kt0Y[3], kt0X[3]), TMath::ATan2(kt0Y[4], kt0X[4]), TMath::ATan2(kt0Y[5], kt0X[5]),
                   TMath::ATan2(kt0Y[6], kt0X[6]), TMath::ATan2(kt0Y[7], kt0X[7]),
                   TMath::ATan2(kt0Y[8], kt0X[8]), TMath::ATan2(kt0Y[9], kt0X[9]), TMath::ATan2(kt0Y[10], kt0X[10]),
                   TMath::ATan2(kt0Y[11], kt0X[11]), TMath::ATan2(kt0Y[12], kt0X[12]), TMath::ATan2(kt0Y[13], kt0X[13]),
                   TMath::ATan2(kt0Y[14], kt0X[14]), TMath::ATan2(kt0Y[15], kt0X[15]),
                   TMath::ATan2(kt0Y[16], kt0X[16]), TMath::ATan2(kt0Y[17], kt0X[17]), TMath::ATan2(kt0Y[18], kt0X[18]),
                   TMath::ATan2(kt0Y[19], kt0X[19]), TMath::ATan2(kt0Y[20], kt0X[20]), TMath::ATan2(kt0Y[21], kt0X[21]),
                   TMath::ATan2(kt0Y[22], kt0X[22]), TMath::ATan2(kt0Y[23], kt0X[23])};
  for (Int_t itzero = 0; itzero < 24; ++itzero) {
    values[kTZEROAmplitudeCh + itzero] = event->AmplitudeTZEROch(itzero);
    values[kTZEROPhiCh + itzero] = t0phi[itzero];
  }

//  values[kMultEstimatorOnlineV0M] = event->MultEstimatorOnlineV0M();
//  values[kMultEstimatorOnlineV0A] = event->MultEstimatorOnlineV0A();
//  values[kMultEstimatorOnlineV0C] = event->MultEstimatorOnlineV0C();
//  values[kMultEstimatorADM] = event->MultEstimatorADM();
//  values[kMultEstimatorADA] = event->MultEstimatorADA();
//  values[kMultEstimatorADC] = event->MultEstimatorADC();
//  values[kMultEstimatorSPDClusters] = event->MultEstimatorSPDClusters();
//  values[kMultEstimatorSPDTracklets] = event->MultEstimatorSPDTracklets();
//  values[kMultEstimatorRefMult05] = event->MultEstimatorRefMult05();
//  values[kMultEstimatorRefMult08] = event->MultEstimatorRefMult08();

//  values[kMultEstimatorPercentileOnlineV0M] = event->MultEstimatorPercentileOnlineV0M();
//  values[kMultEstimatorPercentileOnlineV0A] = event->MultEstimatorPercentileOnlineV0A();
//  values[kMultEstimatorPercentileOnlineV0C] = event->MultEstimatorPercentileOnlineV0C();
//  values[kMultEstimatorPercentileADM] = event->MultEstimatorPercentileADM();
//  values[kMultEstimatorPercentileADA] = event->MultEstimatorPercentileADA();
//  values[kMultEstimatorPercentileADC] = event->MultEstimatorPercentileADC();
//  values[kMultEstimatorPercentileSPDClusters] = event->MultEstimatorPercentileSPDClusters();
//  values[kMultEstimatorPercentileSPDTracklets] = event->MultEstimatorPercentileSPDTracklets();
//  values[kMultEstimatorPercentileRefMult05] = event->MultEstimatorPercentileRefMult05();
//  values[kMultEstimatorPercentileRefMult08] = event->MultEstimatorPercentileRefMult08();

}

//

//_________________________________________________________________
void AliReducedVarManager::FillTrackInfo(BASETRACK *p, double *values) {
  //
  // fill track information
  //

  // Fill base track information
  if (fgUsedVars[kPt]) {
    values[kPt] = p->Pt();
    values[kQualityTrackFlags] = p->GetQualityFlags();
  }
  if (fgUsedVars[kPtSquared]) values[kPtSquared] = values[kPt]*values[kPt];
  if (fgUsedVars[kOneOverSqrtPt]) {
    values[kOneOverSqrtPt] = values[kPt] > 0. ? 1./TMath::Sqrt(values[kPt]) : 999.;
  }
  for (int i = 0; i < 10; ++i) {
    if (p->TestQualityFlag(15 + i)) { values[kFilterBit + i] = 1; } else { values[kFilterBit + i] = 0; }
  }

  if (fgUsedVars[kP]) values[kP] = p->P();
  if (fgUsedVars[kPx]) values[kPx] = p->Px();
  if (fgUsedVars[kPy]) values[kPy] = p->Py();
  if (fgUsedVars[kPz]) values[kPz] = p->Pz();
  if (fgUsedVars[kTheta]) values[kTheta] = p->Theta();
  if (fgUsedVars[kPhi]) values[kPhi] = p->Phi();
  if (fgUsedVars[kEta]) values[kEta] = p->Eta();
  for (Int_t ih = 1; ih <= 6; ++ih) {
    if (fgUsedVars[kCosNPhi + ih - 1]) values[kCosNPhi + ih - 1] = TMath::Cos(p->Phi()*ih);
    if (fgUsedVars[kSinNPhi + ih - 1]) values[kSinNPhi + ih - 1] = TMath::Sin(p->Phi()*ih);
  }

  //pair efficiency variables
  if ((fgUsedVars[kPairEff] || fgUsedVars[kOneOverPairEff] || fgUsedVars[kOneOverPairEffSq]) && fgPairEffMap) {
    Int_t binX = fgPairEffMap->GetXaxis()->FindBin(values[fgEffMapVarDependencyX]);
    if (binX==0) binX = 1;
    if (binX==fgPairEffMap->GetXaxis()->GetNbins() + 1) binX -= 1;
    Int_t binY = fgPairEffMap->GetYaxis()->FindBin(values[fgEffMapVarDependencyY]);
    if (binY==0) binY = 1;
    if (binY==fgPairEffMap->GetYaxis()->GetNbins() + 1) binY -= 1;
    Float_t pairEff = fgPairEffMap->GetBinContent(binX, binY);
    Float_t oneOverPairEff = 1;
    if (pairEff > 1.0e-6) oneOverPairEff = 1/pairEff;
    values[kPairEff] = pairEff;
    values[kOneOverPairEff] = oneOverPairEff;
    values[kOneOverPairEffSq] = oneOverPairEff*oneOverPairEff;
  }

  // Fill VZERO flow variables
  for (Int_t iVZEROside = 0; iVZEROside < 3; ++iVZEROside) {
    for (Int_t ih = 0; ih < 6; ++ih) {
      if (fgUsedVars[kVZEROFlowVn + iVZEROside*6 + ih])
        values[kVZEROFlowVn + iVZEROside*6 + ih] =
            TMath::Cos((values[kPhi] - values[kVZERORP + iVZEROside*6 + ih])*(ih + 1));
      if (fgUsedVars[kVZEROFlowSine + iVZEROside*6 + ih])
        values[kVZEROFlowSine + iVZEROside*6 + ih] =
            TMath::Sin((values[kPhi] - values[kVZERORP + iVZEROside*6 + ih])*(ih + 1));
      if (iVZEROside < 2) {
        if (fgUsedVars[kVZEROuQ + iVZEROside*6 + ih]) {
          values[kVZEROuQ + iVZEROside*6 + ih] =
              TMath::Cos((values[kPhi] - values[kVZERORP + iVZEROside*6 + ih])*(ih + 1));
          values[kVZEROuQ + iVZEROside*6 + ih] *=
              TMath::Sqrt(values[kVZEROQvecX + iVZEROside*6 + ih]*values[kVZEROQvecX + iVZEROside*6 + ih] +
                  values[kVZEROQvecY + iVZEROside*6 + ih]*values[kVZEROQvecY + iVZEROside*6 + ih]);
        }
        if (fgUsedVars[kVZEROuQsine + iVZEROside*6 + ih]) {
          values[kVZEROuQsine + iVZEROside*6 + ih] =
              TMath::Sin((values[kPhi] - values[kVZERORP + iVZEROside*6 + ih])*(ih + 1));
          values[kVZEROuQsine + iVZEROside*6 + ih] *=
              TMath::Sqrt(values[kVZEROQvecX + iVZEROside*6 + ih]*values[kVZEROQvecX + iVZEROside*6 + ih] +
                  values[kVZEROQvecY + iVZEROside*6 + ih]*values[kVZEROQvecY + iVZEROside*6 + ih]);
        }
      }
    }  // end loop over harmonics
  }  // end loop over VZERO sides

  // Fill TPC flow variables
  // Subtract the q vector of the track or of the pair legs from the event q-vector
  Bool_t tpcEPUsed = kFALSE;
  for (Int_t ih = 0; ih < 6; ++ih) {
    if (fgUsedVars[kTPCFlowVn + ih]) {
      tpcEPUsed = kTRUE;
      break;
    }
    if (fgUsedVars[kTPCFlowSine + ih]) {
      tpcEPUsed = kTRUE;
      break;
    }
    if (fgUsedVars[kTPCuQ + ih]) {
      tpcEPUsed = kTRUE;
      break;
    }
    if (fgUsedVars[kTPCuQsine + ih]) {
      tpcEPUsed = kTRUE;
      break;
    }
  }

  if (p->IsA()!=TRACK::Class()) return;
  TRACK *pinfo = (TRACK *) p;

  values[kPtTPC] = pinfo->PtTPC();
  values[kTrackLength] = pinfo->TrackLength();
  values[kChi2TPCConstrainedVsGlobal] = pinfo->Chi2TPCConstrainedVsGlobal();
  values[kMassUsedForTracking] = pinfo->MassForTracking();
  values[kPhiTPC] = pinfo->PhiTPC();
  values[kEtaTPC] = pinfo->EtaTPC();
  values[kPin] = pinfo->Pin();
  values[kDcaXY] = pinfo->DCAxy();
  values[kDcaZ] = pinfo->DCAz();
  values[kDcaXYTPC] = pinfo->DCAxyTPC();
  values[kDcaZTPC] = pinfo->DCAzTPC();
  values[kCharge] = pinfo->Charge();

  if (fgUsedVars[kITSncls]) values[kITSncls] = pinfo->ITSncls();
  values[kITSsignal] = pinfo->ITSsignal();
  values[kITSchi2] = pinfo->ITSchi2();

  if (fgUsedVars[kITSnclsShared]) values[kITSnclsShared] = pinfo->ITSnSharedCls();
  values[kTPCncls] = pinfo->TPCncls();

  if (fgUsedVars[kNclsSFracITS])
    values[kNclsSFracITS] = (pinfo->ITSncls() > 0 ? Float_t(pinfo->ITSnSharedCls())/Float_t(pinfo->ITSncls()) : 0.0);
  if (fgUsedVars[kTPCnclsRatio])
    values[kTPCnclsRatio] =
        (pinfo->TPCFindableNcls() > 0 ? Float_t(pinfo->TPCncls())/Float_t(pinfo->TPCFindableNcls()) : 0.0);
  if (fgUsedVars[kTPCnclsRatio2])
    values[kTPCnclsRatio2] =
        (pinfo->TPCCrossedRows() > 0 ? Float_t(pinfo->TPCncls())/Float_t(pinfo->TPCCrossedRows()) : 0.0);

  if (fgUsedVars[kTPCcrossedRowsOverFindableClusters]) {
    if (pinfo->TPCFindableNcls() > 0)
      values[kTPCcrossedRowsOverFindableClusters] = Float_t(pinfo->TPCCrossedRows())/Float_t(pinfo->TPCFindableNcls());
    else
      values[kTPCcrossedRowsOverFindableClusters] = 0.0;
  }
  if (fgUsedVars[kTPCnclsSharedRatio]) {
    if (pinfo->TPCncls() > 0)
      values[kTPCnclsSharedRatio] = Float_t(pinfo->TPCnclsShared())/Float_t(pinfo->TPCncls());
    else
      values[kTPCnclsSharedRatio] = 0.0;
  }

  if (fgUsedVars[kTPCnclsRatio3])
    values[kTPCnclsRatio3] =
        (pinfo->TPCFindableNcls() > 0 ? Float_t(pinfo->TPCCrossedRows())/Float_t(pinfo->TPCFindableNcls()) : 0.0);

  values[kTPCnclsF] = pinfo->TPCFindableNcls();
  values[kTPCnclsShared] = pinfo->TPCnclsShared();
  values[kTPCcrossedRows] = pinfo->TPCCrossedRows();
  values[kTPCsignal] = pinfo->TPCsignal();
  values[kTPCsignalN] = pinfo->TPCsignalN();
  values[kTPCchi2] = pinfo->TPCchi2();
  if (fgUsedVars[kTPCNclusBitsFired]) values[kTPCNclusBitsFired] = pinfo->TPCClusterMapBitsFired();
  if (fgUsedVars[kTPCclustersPerBit]) {
    Int_t nbits = pinfo->TPCClusterMapBitsFired();
    values[kTPCclustersPerBit] = (nbits > 0 ? values[kTPCncls]/Float_t(nbits) : 0.0);
  }

  values[kTOFbeta] = pinfo->TOFbeta();
  values[kTOFdeltaBC] = pinfo->TOFdeltaBC();
  values[kTOFtime] = pinfo->TOFtime();
  values[kTOFdx] = pinfo->TOFdx();
  values[kTOFdz] = pinfo->TOFdz();
  values[kTOFmismatchProbability] = pinfo->TOFmismatchProbab();
  values[kTOFchi2] = pinfo->TOFchi2();

  for (Int_t specie = kElectron; specie <= kProton; ++specie) {
    values[kITSnSig + specie] = pinfo->ITSnSig(specie);
    values[kTPCnSig + specie] = pinfo->TPCnSig(specie);
    values[kTOFnSig + specie] = pinfo->TOFnSig(specie);
    values[kBayes + specie] = pinfo->GetBayesProb(specie);
  }
  if (fgUsedVars[kTPCnSigCorrected + kElectron] && fgTPCelectronCentroidMap && fgTPCelectronWidthMap) {
    Int_t binX = fgTPCelectronCentroidMap->GetXaxis()->FindBin(values[fgVarDependencyX]);
    if (binX==0) binX = 1;
    if (binX==fgTPCelectronCentroidMap->GetXaxis()->GetNbins() + 1) binX -= 1;
    Int_t binY = fgTPCelectronCentroidMap->GetYaxis()->FindBin(values[fgVarDependencyY]);
    if (binY==0) binY = 1;
    if (binY==fgTPCelectronCentroidMap->GetYaxis()->GetNbins() + 1) binY -= 1;
    Float_t centroid = fgTPCelectronCentroidMap->GetBinContent(binX, binY);
    Float_t width = fgTPCelectronWidthMap->GetBinContent(binX, binY);
    if (TMath::Abs(width) < 1.0e-6) width = 1.;
    values[kTPCnSigCorrected + kElectron] = (values[kTPCnSig + kElectron] - centroid)/width;
  }

  values[kTRDpidProbabilitiesLQ1D] = pinfo->TRDpidLQ1D(0);
  values[kTRDpidProbabilitiesLQ1D + 1] = pinfo->TRDpidLQ1D(1);
  values[kTRDpidProbabilitiesLQ2D] = pinfo->TRDpidLQ2D(0);
  values[kTRDpidProbabilitiesLQ2D + 1] = pinfo->TRDpidLQ2D(1);
  values[kTRDntracklets] = pinfo->TRDntracklets(0);
  values[kTRDntrackletsPID] = pinfo->TRDntracklets(1);

  if (fgUsedVars[kEMCALmatchedEnergy] || fgUsedVars[kEMCALmatchedEOverP]) {
    values[kEMCALmatchedClusterId] = pinfo->CaloClusterId();
    if (fgEvent && (fgEvent->IsA()==EVENT::Class())) {
      CLUSTER *cluster = ((EVENT *) fgEvent)->GetCaloCluster(pinfo->CaloClusterId());
      values[kEMCALmatchedEnergy] = (cluster ? cluster->Energy() : -999.0);
      Float_t mom = pinfo->P();
      values[kEMCALmatchedEOverP] = (TMath::Abs(mom) > 1.e-8 && cluster ? values[kEMCALmatchedEnergy]/mom : -999.0);
    }
  }

//  FillTrackingStatus(pinfo, values);
  //FillTrackingFlags(pinfo,values);

  if (fgUsedVars[kPtMC]) values[kPtMC] = pinfo->PtMC();
  if (fgUsedVars[kPMC]) values[kPMC] = pinfo->PMC();
  values[kPxMC] = pinfo->MCmom(0);
  values[kPyMC] = pinfo->MCmom(1);
  values[kPzMC] = pinfo->MCmom(2);
  if (fgUsedVars[kThetaMC]) values[kThetaMC] = pinfo->ThetaMC();
  if (fgUsedVars[kEtaMC]) values[kEtaMC] = pinfo->EtaMC();
  if (fgUsedVars[kPhiMC]) values[kPhiMC] = pinfo->PhiMC();
  //TODO: add also the massMC and RapMC
  values[kPdgMC] = pinfo->MCPdg(0);
  values[kPdgMC + 1] = pinfo->MCPdg(1);
  values[kPdgMC + 2] = pinfo->MCPdg(2);
  values[kPdgMC + 3] = pinfo->MCPdg(3);
}