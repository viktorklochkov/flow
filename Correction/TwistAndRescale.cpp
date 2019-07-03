/************************************************************************************************
 *                                                                                                *
 * Package:       FlowVectorCorrections                                                           *
 * Authors:       Jaap Onderwaater, GSI, jacobus.onderwaater@cern.ch                              *
 *                Ilya Selyuzhenkov, GSI, ilya.selyuzhenkov@gmail.com                             *
 *                Víctor González, UCM, victor.gonzalez@cern.ch                                   *
 *                Contributors are mentioned in the code where appropriate.                       *
 * Development:   2012-2016                                                                       *
 *                                                                                                *
 * This file is part of FlowVectorCorrections, a software package that corrects Q-vector          *
 * measurements for effects of nonuniform detector acceptance. The corrections in this package    *
 * are based on publication:                                                                      *
 *                                                                                                *
 *  [1] "Effects of non-uniform acceptance in anisotropic flow measurements"                      *
 *  Ilya Selyuzhenkov and Sergei Voloshin                                                         *
 *  Phys. Rev. C 77, 034904 (2008)                                                                *
 *                                                                                                *
 * The procedure proposed in [1] is extended with the following steps:                            *
 * (*) alignment correction between subevents                                                     *
 * (*) possibility to extract the twist and rescaling corrections                                 *
 *      for the case of three detector subevents                                                  *
 *      (currently limited to the case of two “hit-only” and one “tracking” detectors)            *
 * (*) (optional) channel equalization                                                            *
 * (*) flow vector width equalization                                                             *
 *                                                                                                *
 * FlowVectorCorrections is distributed under the terms of the GNU General Public License (GPL)   *
 * (https://en.wikipedia.org/wiki/GNU_General_Public_License)                                     *
 * either version 3 of the License, or (at your option) any later version.                        *
 *                                                                                                *
 **************************************************************************************************/

/// \file QnCorrectionsQnVectorTwistAndRescale.cxx
/// \brief Implementation of procedures for Qn vector twist and rescale corrections.
#include "EventClassVariablesSet.h"
#include "CorrectionProfileComponents.h"
#include "CorrectionProfile3DCorrelations.h"
#include "CorrectionHistogramSparse.h"
#include "CorrectionCalculator.h"
#include "TwistAndRescale.h"
#include "ROOT/RMakeUnique.hxx"
#include "Detector.h"
#include "DetectorList.h"

/// \cond CLASSIMP
ClassImp(Qn::TwistAndRescale);
/// \endcond
namespace Qn {
const Int_t TwistAndRescale::fDefaultMinNoOfEntries = 2;
const Double_t TwistAndRescale::fMaxThreshold = 99999999.0;
const char *TwistAndRescale::szTwistCorrectionName = "Twist";
const char *TwistAndRescale::szRescaleCorrectionName = "Rescale";
const char *TwistAndRescale::szDoubleHarmonicSupportHistogramName = "DH Q2n";
const char *TwistAndRescale::szCorrelationsSupportHistogramName = "3D QnQn";
const char *TwistAndRescale::szTwistCorrectedQnVectorName = "twist";
const char *TwistAndRescale::szRescaleCorrectedQnVectorName = "rescale";
const char *TwistAndRescale::szQANotValidatedHistogramName = "TwScale NvE";
const char *TwistAndRescale::szQATwistQnAverageHistogramName = "Twist Qn avg ";
const char *TwistAndRescale::szQARescaleQnAverageHistogramName = "Rescale Qn avg ";

/// Default constructor
/// Passes to the base class the identity data for the recentering and width equalization correction step
TwistAndRescale::TwistAndRescale() :
    CorrectionOnQvector(Form("%s and %s", szTwistCorrectionName, szRescaleCorrectionName), szPriority),
    fBDetectorConfigurationName(),
    fCDetectorConfigurationName() {
  fTwistAndRescaleMethod = Method::DOUBLE_HARMONIC;
  fApplyTwist = kTRUE;
  fApplyRescale = kTRUE;
  fSubEventB = nullptr;
  fSubEventC = nullptr;
  fMinNoOfEntriesToValidate = fDefaultMinNoOfEntries;
}

/// Set the detector configurations used as reference for twist and rescaling
/// The detector configurations names are stored for further use.
/// \param nameB the name of the B detector configuration
/// \param nameC the name of the C detector configuration
void TwistAndRescale::SetReferenceConfigurationsForTwistAndRescale(const char *nameB, const char *nameC) {
  fBDetectorConfigurationName = nameB;
  fCDetectorConfigurationName = nameC;
  /* we and the reference detector configurations could be in different situations of framework attachment */
  /* so, we do nothing for the time being */
}

/// Asks for support data structures creation
/// Creates the corrected Qn vectors
/// Locates the reference detector configurations for twist and rescaling if their names have been previously stored
void TwistAndRescale::CreateSupportDataStructures() {
  auto harmonics = fSubEvent->GetHarmonics();
  /* now create the corrected Qn vectors */
  fCorrectedQnVector = std::make_unique<QVector>(harmonics, QVector::CorrectionStep::TWIST);
  fTwistCorrectedQnVector =
      std::make_unique<QVector>(harmonics, QVector::CorrectionStep::TWIST);
  fRescaleCorrectedQnVector =
      std::make_unique<QVector>(harmonics, QVector::CorrectionStep::RESCALED);
  /* get the input vectors we need */
  fInputQnVector = fSubEvent->GetPreviousCorrectedQnVector(this);
  /* now, definitely, we should have the reference detector configurations */
  switch (fTwistAndRescaleMethod) {
    case Method::DOUBLE_HARMONIC: break;
    case Method::CORRELATIONS:
      if (!fBDetectorConfigurationName.empty()) {
        fSubEventB =
            fSubEvent->GetDetector()->GetDetectors()->FindDetector(fBDetectorConfigurationName).GetSubEvent(0);
        if (!fSubEventB->GetIsTrackingDetector()) {
          throw std::runtime_error(
              std::string(fSubEventB->GetName()) + "Detector is not a tracking detector");
        }
      } else {
        throw std::runtime_error(
            std::string(fSubEventB->GetName()) + "is not configured for twist and rescaling step.");
      }
      if (!fCDetectorConfigurationName.empty()) {
        fSubEventC =
            fSubEvent->GetDetector()->GetDetectors()->FindDetector(fCDetectorConfigurationName).GetSubEvent(0);
        if (!fSubEventC->GetIsTrackingDetector()) {
          throw std::runtime_error(
              std::string(fSubEventC->GetName()) + "Detector is not a tracking detector");
        } else {
          throw std::runtime_error(
              std::string(fSubEventC->GetName()) + "is not configured for twist and rescaling step.");
        }
      }
      break;
  }
}

/// Asks for support histograms creation
///
/// Allocates the histogram objects and creates the calibration histograms.
///
/// Process concurrency requires Calibration Histograms creation for all
/// concurrent processes but not for Input Histograms so, we delete previously
/// allocated ones.
/// \param list list where the histograms should be incorporated for its persistence
/// \return kTRUE if everything went OK
void TwistAndRescale::AttachSupportHistograms(TList *list) {
  Int_t *harmonicsMap;
  auto event_variables = fSubEvent->GetEventClassVariablesSet();
  std::string name;
  switch (fTwistAndRescaleMethod) {
    case Method::DOUBLE_HARMONIC:name = std::string(szDoubleHarmonicSupportHistogramName) + "_" + fSubEvent->GetName();
      fDoubleHarmonicInputHistograms =
          std::make_unique<CorrectionProfileComponents>(name, event_variables);
      fDoubleHarmonicInputHistograms->SetNoOfEntriesThreshold(fMinNoOfEntriesToValidate);
      fDoubleHarmonicCalibrationHistograms =
          std::make_unique<CorrectionProfileComponents>(name, event_variables);
      harmonicsMap = new Int_t[fCorrectedQnVector->GetNoOfHarmonics()];
      fCorrectedQnVector->GetHarmonicsMap(harmonicsMap);
      /* we duplicate the harmonics used because that will be the info stored by the profiles */
      for (unsigned int h = 0; h < fCorrectedQnVector->GetNoOfHarmonics(); h++) harmonicsMap[h] = 2*harmonicsMap[h];
      fDoubleHarmonicCalibrationHistograms->CreateComponentsProfileHistograms(list,
                                                                              fCorrectedQnVector->GetNoOfHarmonics(),
                                                                              harmonicsMap);
      delete[] harmonicsMap;
      break;
    case Method::CORRELATIONS:name = std::string(szCorrelationsSupportHistogramName) + "_" + fSubEvent->GetName();
      fCorrelationsInputHistograms =
          std::make_unique<CorrectionProfile3DCorrelations>(name,
                                                            fSubEvent->GetName(),
                                                            fSubEventB->GetName(),
                                                            fSubEventC->GetName(),
                                                            event_variables);
      fCorrelationsInputHistograms->SetNoOfEntriesThreshold(fMinNoOfEntriesToValidate);
      fCorrelationsCalibrationHistograms =
          std::make_unique<CorrectionProfile3DCorrelations>(name,
                                                            fSubEvent->GetName(),
                                                            fSubEventB->GetName(),
                                                            fSubEventC->GetName(),
                                                            event_variables);
      harmonicsMap = new Int_t[fCorrectedQnVector->GetNoOfHarmonics()];
      fCorrectedQnVector->GetHarmonicsMap(harmonicsMap);
      fCorrelationsCalibrationHistograms->CreateCorrelationComponentsProfileHistograms(list,
                                                                                       fCorrectedQnVector->GetNoOfHarmonics(),
                                                                                       1 /* harmonic multiplier */,
                                                                                       harmonicsMap);
      delete[] harmonicsMap;
      break;
  }
}

/// Attaches the needed input information to the correction step
/// \param list list where the inputs should be found
/// \return kTRUE if everything went OK
void TwistAndRescale::AttachInput(TList *list) {
  switch (fTwistAndRescaleMethod) {
    case Method::DOUBLE_HARMONIC:
      /* TODO: basically we are re producing half of the information already produce for recentering correction. Re use it! */
      if (fDoubleHarmonicInputHistograms->AttachHistograms(list)) {
        fState = State::APPLYCOLLECT;
      }
      break;
    case Method::CORRELATIONS:
      if (fCorrelationsInputHistograms->AttachHistograms(list)) {
        fState = State::APPLYCOLLECT;
      }
      break;
  }
}

/// Perform after calibration histograms attach actions
/// It is used to inform the different correction step that
/// all conditions for running the network are in place so
/// it is time to check if their requirements are satisfied
///
/// A check is done to confirm that \f$ B \f$ is applying
/// twist to correct its Qn vectors. If not the correction
/// step is set to passive
void TwistAndRescale::AfterInputsAttachActions() {
  switch (fTwistAndRescaleMethod) {
    case Method::DOUBLE_HARMONIC:
      /* nothing required */
      break;
    case Method::CORRELATIONS:
      /* require B be applying twist corrections */
      if (!fSubEventB->IsCorrectionStepBeingApplied(szTwistCorrectionName)) {
        fState = State::PASSIVE;
      }
      break;
    default:
      /* nothing required */
      break;
  }
}

/// Asks for QA histograms creation
///
/// Allocates the histogram objects and creates the QA histograms.
/// \param list list where the histograms should be incorporated for its persistence
/// \return kTRUE if everything went OK
void TwistAndRescale::AttachQAHistograms(TList *list) {
  auto name = std::string(szQATwistQnAverageHistogramName) + "DH" + "_" + fSubEvent->GetName();
  if (fApplyTwist) {
    fQATwistQnAverageHistogram =
        std::make_unique<CorrectionProfileComponents>(name, fSubEvent->GetEventClassVariablesSet());
  }
  if (fApplyRescale) {
    fQARescaleQnAverageHistogram =
        std::make_unique<CorrectionProfileComponents>(name, fSubEvent->GetEventClassVariablesSet());
  }
  if (fApplyTwist || fApplyRescale) {
    /* get information about the configured harmonics to pass it for histogram creation */
    Int_t nNoOfHarmonics = fSubEvent->GetNoOfHarmonics();
    auto harmonicsMap = new Int_t[nNoOfHarmonics];
    fSubEvent->GetHarmonicMap(harmonicsMap);
    if (fApplyTwist)
      fQATwistQnAverageHistogram->CreateComponentsProfileHistograms(list, nNoOfHarmonics, harmonicsMap);
    if (fApplyRescale)
      fQARescaleQnAverageHistogram->CreateComponentsProfileHistograms(list, nNoOfHarmonics, harmonicsMap);
    delete[] harmonicsMap;
  }
}

/// Asks for non validated entries QA histograms creation
///
/// Allocates the histogram objects and creates the non validated entries QA histograms.
/// \param list list where the histograms should be incorporated for its persistence
/// \return kTRUE if everything went OK
void TwistAndRescale::AttachNveQAHistograms(TList *list) {
  auto hname = std::string(szQANotValidatedHistogramName) + "DH" + "_" + fSubEvent->GetName();
  switch (fTwistAndRescaleMethod) {
    case Method::DOUBLE_HARMONIC:
      fQANotValidatedBin = std::make_unique<CorrectionHistogramSparse>(hname,
                                                                       fSubEvent->GetEventClassVariablesSet());
      fQANotValidatedBin->CreateHistogram(list);
      break;
    case Method::CORRELATIONS:
      fQANotValidatedBin = std::make_unique<CorrectionHistogramSparse>(hname,
                                                                       fSubEvent->GetEventClassVariablesSet());
      fQANotValidatedBin->CreateHistogram(list);
      break;
  }
}

/// Processes the correction step
///
/// Apply the correction step
/// \return kTRUE if the correction step was applied
bool TwistAndRescale::ProcessCorrections(const double *variableContainer) {
  Int_t harmonic;
  switch (fState) {
    case State::CALIBRATION:
      /* collect the data needed to further produce correction parameters if Qn vectors are good enough */
      /* we have not perform any correction yet */
      /* we check if detector B is in its proper correction step */
      return false;
    case State::APPLYCOLLECT:
      /* collect the data needed to further produce correction parameters if Qn vectors are good enough */
      /* and proceed to ... */
      /* FALLTHRU */
    case State::APPLY: { /* apply the correction if the current Qn vector is good enough */
      /* logging */
      switch (fTwistAndRescaleMethod) {
        case Method::DOUBLE_HARMONIC: {
          /* TODO: basically we are re producing half of the information already produce for recentering correction. Re use it! */
          if (fSubEvent->GetCurrentQnVector()->IsGoodQuality()) {
            fCorrectedQnVector->SetCurrentEvent(*fSubEvent->GetCurrentQnVector());
            fTwistCorrectedQnVector->SetCurrentEvent(*fCorrectedQnVector);
            fRescaleCorrectedQnVector->SetCurrentEvent(*fCorrectedQnVector);
            /* let's check the correction histograms */
            Long64_t bin = fDoubleHarmonicInputHistograms->GetBin(variableContainer);
            if (fDoubleHarmonicInputHistograms->BinContentValidated(bin)) {
              /* remember we store the profile information on a twice the harmonic number base */
              harmonic = fCorrectedQnVector->GetFirstHarmonic();
              while (harmonic!=-1) {
                Double_t X2n = fDoubleHarmonicInputHistograms->GetXBinContent(harmonic*2, bin);
                Double_t Y2n = fDoubleHarmonicInputHistograms->GetYBinContent(harmonic*2, bin);

                Double_t Aplus = 1 + X2n;
                Double_t Aminus = 1 - X2n;
                Double_t LambdaPlus = Y2n/Aplus;
                Double_t LambdaMinus = Y2n/Aminus;

                if (TMath::Abs(Aplus) > fMaxThreshold) {
                  harmonic = fCorrectedQnVector->GetNextHarmonic(harmonic);
                  continue;
                }
                if (TMath::Abs(Aminus) > fMaxThreshold) {
                  harmonic = fCorrectedQnVector->GetNextHarmonic(harmonic);
                  continue;
                }
                if (TMath::Abs(LambdaPlus) > fMaxThreshold) {
                  harmonic = fCorrectedQnVector->GetNextHarmonic(harmonic);
                  continue;
                }
                if (TMath::Abs(LambdaMinus) > fMaxThreshold) {
                  harmonic = fCorrectedQnVector->GetNextHarmonic(harmonic);
                  continue;
                }
                Double_t Qx = fTwistCorrectedQnVector->Qx(harmonic);
                Double_t Qy = fTwistCorrectedQnVector->Qy(harmonic);
                Double_t newQx = (Qx - LambdaMinus*Qy)/(1 - LambdaMinus*LambdaPlus);
                Double_t newQy = (Qy - LambdaPlus*Qx)/(1 - LambdaMinus*LambdaPlus);
                if (fApplyTwist) {
                  fCorrectedQnVector->SetQx(harmonic, newQx);
                  fCorrectedQnVector->SetQy(harmonic, newQy);
                  fTwistCorrectedQnVector->SetQx(harmonic, newQx);
                  fTwistCorrectedQnVector->SetQy(harmonic, newQy);
                  fRescaleCorrectedQnVector->SetQx(harmonic, newQx);
                  fRescaleCorrectedQnVector->SetQy(harmonic, newQy);
                }
                newQx = newQx/Aplus;
                newQy = newQy/Aminus;
                if (Aplus==0.0) {
                  harmonic = fCorrectedQnVector->GetNextHarmonic(harmonic);
                  continue;
                }
                if (Aminus==0.0) {
                  harmonic = fCorrectedQnVector->GetNextHarmonic(harmonic);
                  continue;
                }
                if (fApplyRescale) {
                  fCorrectedQnVector->SetQx(harmonic, newQx);
                  fCorrectedQnVector->SetQy(harmonic, newQy);
                  fRescaleCorrectedQnVector->SetQx(harmonic, newQx);
                  fRescaleCorrectedQnVector->SetQy(harmonic, newQy);
                }
                harmonic = fCorrectedQnVector->GetNextHarmonic(harmonic);
              }
            } else {
              if (fQANotValidatedBin) fQANotValidatedBin->Fill(variableContainer, 1.0);
            }
          } else {
            /* not done! input Q vector with bad quality */
            fCorrectedQnVector->SetGood(kFALSE);
          }
        }
          break;
        case Method::CORRELATIONS: {
          if (fSubEvent->GetCurrentQnVector()->IsGoodQuality()) {
            fCorrectedQnVector->SetCurrentEvent(*fSubEvent->GetCurrentQnVector());
            fTwistCorrectedQnVector->SetCurrentEvent(*fCorrectedQnVector);
            fRescaleCorrectedQnVector->SetCurrentEvent(*fCorrectedQnVector);
            /* let's check the correction histograms */
            Long64_t bin = fCorrelationsInputHistograms->GetBin(variableContainer);
            if (fCorrelationsInputHistograms->BinContentValidated(bin)) {
              harmonic = fCorrectedQnVector->GetFirstHarmonic();
              while (harmonic!=-1) {
                Double_t XAXC = fCorrelationsInputHistograms->GetXXBinContent("AC", harmonic, bin);
                Double_t YAYB = fCorrelationsInputHistograms->GetYYBinContent("AB", harmonic, bin);
                Double_t XAXB = fCorrelationsInputHistograms->GetXXBinContent("AB", harmonic, bin);
                Double_t XBXC = fCorrelationsInputHistograms->GetXXBinContent("BC", harmonic, bin);
                Double_t XAYB = fCorrelationsInputHistograms->GetXYBinContent("AB", harmonic, bin);
                Double_t XBYC = fCorrelationsInputHistograms->GetXYBinContent("BC", harmonic, bin);
                Double_t
                    Aplus = TMath::Sqrt(TMath::Abs(2.0*XAXC))*XAXB/TMath::Sqrt(TMath::Abs(XAXB*XBXC + XAYB*XBYC));
                Double_t
                    Aminus = TMath::Sqrt(TMath::Abs(2.0*XAXC))*YAYB/TMath::Sqrt(TMath::Abs(XAXB*XBXC + XAYB*XBYC));
                Double_t LambdaPlus = XAYB/XAXB;
                Double_t LambdaMinus = XAYB/YAYB;
                if (TMath::Abs(Aplus) > fMaxThreshold) {
                  harmonic = fCorrectedQnVector->GetNextHarmonic(harmonic);
                  continue;
                }
                if (TMath::Abs(Aminus) > fMaxThreshold) {
                  harmonic = fCorrectedQnVector->GetNextHarmonic(harmonic);
                  continue;
                }
                if (TMath::Abs(LambdaPlus) > fMaxThreshold) {
                  harmonic = fCorrectedQnVector->GetNextHarmonic(harmonic);
                  continue;
                }
                if (TMath::Abs(LambdaMinus) > fMaxThreshold) {
                  harmonic = fCorrectedQnVector->GetNextHarmonic(harmonic);
                  continue;
                }
                Double_t Qx = fTwistCorrectedQnVector->Qx(harmonic);
                Double_t Qy = fTwistCorrectedQnVector->Qy(harmonic);
                Double_t newQx = (Qx - LambdaMinus*Qy)/(1 - LambdaMinus*LambdaPlus);
                Double_t newQy = (Qy - LambdaPlus*Qx)/(1 - LambdaMinus*LambdaPlus);
                if (fApplyTwist) {
                  fCorrectedQnVector->SetQx(harmonic, newQx);
                  fCorrectedQnVector->SetQy(harmonic, newQy);
                  fTwistCorrectedQnVector->SetQx(harmonic, newQx);
                  fTwistCorrectedQnVector->SetQy(harmonic, newQy);
                  fRescaleCorrectedQnVector->SetQx(harmonic, newQx);
                  fRescaleCorrectedQnVector->SetQy(harmonic, newQy);
                }
                newQx = newQx/Aplus;
                newQy = newQy/Aminus;
                if (Aplus==0.0) {
                  harmonic = fCorrectedQnVector->GetNextHarmonic(harmonic);
                  continue;
                }
                if (Aminus==0.0) {
                  harmonic = fCorrectedQnVector->GetNextHarmonic(harmonic);
                  continue;
                }
                if (fApplyRescale) {
                  fCorrectedQnVector->SetQx(harmonic, newQx);
                  fCorrectedQnVector->SetQy(harmonic, newQy);
                  fRescaleCorrectedQnVector->SetQx(harmonic, newQx);
                  fRescaleCorrectedQnVector->SetQy(harmonic, newQy);
                }
                harmonic = fCorrectedQnVector->GetNextHarmonic(harmonic);
              }
            } else {
              if (fQANotValidatedBin) fQANotValidatedBin->Fill(variableContainer, 1.0);
            }
          } else {
            /* not done! input Q vector with bad quality */
            fCorrectedQnVector->SetGood(kFALSE);
          }
        }
          break;
      }
      /* and update the current Qn vector */
      if (fApplyTwist) {
        fSubEvent->UpdateCurrentQnVector(*fTwistCorrectedQnVector, QVector::CorrectionStep::TWIST);
      }
      if (fApplyRescale) {
        fSubEvent->UpdateCurrentQnVector(*fRescaleCorrectedQnVector, QVector::CorrectionStep::RESCALED);
      }
    }
      return true;
    default:
      /* we are in passive state waiting for proper conditions, no corrections applied */
      return false;
  }
}

/// Processes the correction step data collection
///
/// Collect data for the correction step.
/// \return kTRUE if the correction step was applied
Bool_t TwistAndRescale::ProcessDataCollection(const double *variableContainer) {
  switch (fState) {
    case State::CALIBRATION: {
      /* logging */
      switch (fTwistAndRescaleMethod) {
        case Method::DOUBLE_HARMONIC: {
          /* remember, we store in the profiles the double harmonic while the Q2n vector stores them single */
          auto plainQ2nVector = fSubEvent->GetPlainQ2nVector();
          Int_t harmonic = fCorrectedQnVector->GetFirstHarmonic();
          if (plainQ2nVector.IsGoodQuality()) {
            while (harmonic!=-1) {
              fDoubleHarmonicCalibrationHistograms->FillX(harmonic*2, variableContainer, plainQ2nVector.Qx(harmonic));
              fDoubleHarmonicCalibrationHistograms->FillY(harmonic*2, variableContainer, plainQ2nVector.Qy(harmonic));
              harmonic = fCorrectedQnVector->GetNextHarmonic(harmonic);
            }
          }
        }
          break;
        case Method::CORRELATIONS: {
          /* collect the data needed to further produce correction parameters if both current Qn vectors are good enough */
          if ((fInputQnVector->IsGoodQuality()) &&
              (fSubEventB->GetCurrentQnVector()->IsGoodQuality()) &&
              (fSubEventC->GetCurrentQnVector()->IsGoodQuality())) {
            fCorrelationsCalibrationHistograms->Fill(fInputQnVector,
                                                     fSubEventB->GetCurrentQnVector(),
                                                     fSubEventC->GetCurrentQnVector(),
                                                     variableContainer);
          }
        }
          break;
      }
      /* we have not perform any correction yet */
      return false;
    }
    case State::APPLYCOLLECT: {
      /* logging */
      switch (fTwistAndRescaleMethod) {
        case Method::DOUBLE_HARMONIC: {
          /* remember, we store in the profiles the double harmonic while the Q2n vector stores them single */
          QVector plainQ2nVector = fSubEvent->GetPlainQ2nVector();
          Int_t harmonic = fCorrectedQnVector->GetFirstHarmonic();
          if (plainQ2nVector.IsGoodQuality()) {
            while (harmonic!=-1) {
              fDoubleHarmonicCalibrationHistograms->FillX(harmonic*2, variableContainer, plainQ2nVector.Qx(harmonic));
              fDoubleHarmonicCalibrationHistograms->FillY(harmonic*2, variableContainer, plainQ2nVector.Qy(harmonic));
              harmonic = fCorrectedQnVector->GetNextHarmonic(harmonic);
            }
          }
        }
          break;
        case Method::CORRELATIONS: {
          /* collect the data needed to further produce correction parameters if both current Qn vectors are good enough */
          if ((fInputQnVector->IsGoodQuality()) &&
              (fSubEventB->GetCurrentQnVector()->IsGoodQuality()) &&
              (fSubEventC->GetCurrentQnVector()->IsGoodQuality())) {
            fCorrelationsCalibrationHistograms->Fill(fInputQnVector,
                                                     fSubEventB->GetCurrentQnVector(),
                                                     fSubEventC->GetCurrentQnVector(),
                                                     variableContainer);
          }
        }
          break;
      }
    }
      /* FALLTHRU */
    case State::APPLY: { /* apply the correction if the current Qn vector is good enough */
      /* provide QA info if required */
      if (fQATwistQnAverageHistogram) {
        Int_t harmonic = fCorrectedQnVector->GetFirstHarmonic();
        while (harmonic!=-1) {
          fQATwistQnAverageHistogram->FillX(harmonic, variableContainer, fTwistCorrectedQnVector->Qx(harmonic));
          fQATwistQnAverageHistogram->FillY(harmonic, variableContainer, fTwistCorrectedQnVector->Qy(harmonic));
          harmonic = fCorrectedQnVector->GetNextHarmonic(harmonic);
        }
      }
      if (fQARescaleQnAverageHistogram) {
        Int_t harmonic = fCorrectedQnVector->GetFirstHarmonic();
        while (harmonic!=-1) {
          fQARescaleQnAverageHistogram->FillX(harmonic, variableContainer, fRescaleCorrectedQnVector->Qx(harmonic));
          fQARescaleQnAverageHistogram->FillY(harmonic, variableContainer, fRescaleCorrectedQnVector->Qy(harmonic));
          harmonic = fCorrectedQnVector->GetNextHarmonic(harmonic);
        }
      }
    }
      return true;
    case State::PASSIVE:
      /* we are in passive state waiting for proper conditions, no corrections applied */
      return false;
  }
}

/// Clean the correction to accept a new event
void TwistAndRescale::ClearCorrectionStep() {
  fTwistCorrectedQnVector->Reset();
  fRescaleCorrectedQnVector->Reset();
  fCorrectedQnVector->Reset();
}

/// Include the corrected Qn vectors into the passed list
///
/// Adds the Qn vector to the passed list
/// if the correction step is in correction states.
/// \param list list where the corrected Qn vector should be added
void TwistAndRescale::IncludeCorrectedQnVector(std::map<QVector::CorrectionStep, QVector *> &qvectors) const {
  switch (fState) {
    case State::CALIBRATION:
      /* collect the data needed to further produce correction parameters */
      break;
    case State::APPLYCOLLECT:
      /* collect the data needed to further produce correction parameters */
      /* FALLTHRU */
    case State::APPLY: /* apply the correction */
      if (fApplyTwist)
        qvectors.emplace(QVector::CorrectionStep::TWIST, fTwistCorrectedQnVector.get());
      if (fApplyRescale)
        qvectors.emplace(QVector::CorrectionStep::RESCALED, fRescaleCorrectedQnVector.get());
      break;
    default:break;
  }
}

/// Reports if the correction step is being applied
/// Returns TRUE if in the proper state for applying the correction step
/// \return TRUE if the correction step is being applied
Bool_t TwistAndRescale::IsBeingApplied() const {
  switch (fState) {
    case State::CALIBRATION:
      /* we are collecting */
      /* but not applying */
      break;
    case State::APPLYCOLLECT:
      /* we are collecting */
    case State::APPLY:
      /* and applying */
      return kTRUE;
    default:break;
  }
  return kFALSE;
}

/// Report on correction usage
/// Correction step should incorporate its name in calibration
/// list if it is producing information calibration in the ongoing
/// step and in the apply list if it is applying correction in
/// the ongoing step.
/// \param calibrationList list containing the correction steps producing calibration information
/// \param applyList list containing the correction steps applying corrections
/// \return kTRUE if the correction step is being applied
Bool_t TwistAndRescale::ReportUsage(TList *calibrationList, TList *applyList) {
  switch (fState) {
    case State::CALIBRATION:
      /* we are collecting */
      calibrationList->Add(new TObjString(GetName()));
      /* but not applying */
      return kFALSE;
    case State::APPLYCOLLECT:
      /* we are collecting */
      calibrationList->Add(new TObjString(GetName()));
      /* FALLTHRU */
    case State::APPLY:
      /* and applying */
      applyList->Add(new TObjString(GetName()));
      return kTRUE;
    default:break;
  }
  return kFALSE;
}
}
