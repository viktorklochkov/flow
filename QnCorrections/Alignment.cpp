/**************************************************************************************************
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

/// \file QnCorrectionsQnVectorAlignment.cxx
/// \brief Implementation of procedures for Qn vector alignment correction.
#include "ROOT/RMakeUnique.hxx"

#include "EventClassVariablesSet.h"
#include "CorrectionCalculator.h"
#include "Alignment.h"

/// \cond CLASSIMP
ClassImp(Qn::Alignment);
/// \endcond
namespace Qn {
/// Set the detector configuration used as reference for alignment
/// The detector configuration name is stored for further use.
/// \param name the name of the reference detector configuration
void Alignment::SetReferenceConfigurationForAlignment(const char *name) {
//  QnCorrectionsInfo(Form("Reference name: %s, attached to detector configuration: %s",
//                         name,
//                         ((fDetector) ? "yes" : "no")));
  fDetectorForAlignmentName = name;
  /* we could be in different situations of framework attachment */
  /* so, we do nothing for the time being */
}

/// Informs when the detector configuration has been attached to the framework manager
/// Basically this allows interaction between the different framework sections at configuration time
void Alignment::AttachedToFrameworkManager() {
//  QnCorrectionsInfo(Form("Attached! reference for alignment: %s", fDetectorConfigurationForAlignmentName.Data()));
}

/// Asks for support data structures creation
///
/// Locates the reference detector configuration for alignment if its name has been previously stored
/// Creates the recentered Qn vector
void Alignment::CreateSupportDataStructures() {
  /* now, definitely, we should have the reference detector configurations */
  if (!fDetectorForAlignmentName.empty()) {
    if (fDetector->GetCorrectionsManager()->FindDetectorConfiguration(
        fDetectorForAlignmentName)) {
      fDetectorForAlignment = fDetector->GetCorrectionsManager()->FindDetectorConfiguration(
          fDetectorForAlignmentName);
    } else {
//      QnCorrectionsFatal(Form("Wrong reference detector configuration %s for %s alignment correction step",
//                              fDetectorConfigurationForAlignmentName.Data(),
//                              fDetector->GetName()));
    }
  } else {
//    QnCorrectionsFatal(Form("Missing reference detector configuration for %s alignment correction step",
//                            fDetector->GetName()));
  }
  Int_t nNoOfHarmonics = fDetector->GetNoOfHarmonics();
  auto harmonicsMap = new Int_t[nNoOfHarmonics];
  /* make sure the alignment harmonic processing is active */
  fDetector->ActivateHarmonic(fHarmonicForAlignment);
  /* in both configurations */
  fDetectorForAlignment->ActivateHarmonic(fHarmonicForAlignment);
  /* and now create the corrected Qn vector */
  fDetector->GetHarmonicMap(harmonicsMap);
  auto name = szCorrectedQnVectorName;
  fCorrectedQnVector = std::make_unique<CorrectionQnVector>(name, nNoOfHarmonics, harmonicsMap);
  fInputQnVector = fDetector->GetPreviousCorrectedQnVector(this);
  delete[] harmonicsMap;
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
Bool_t Alignment::CreateSupportHistograms(TList *list) {
  TString histoNameAndTitle = Form("%s %s#times%s ",
                                   szSupportHistogramName,
                                   fDetector->GetName(),
                                   fDetectorForAlignment->GetName());
  fInputHistograms =
      std::make_unique<CorrectionProfileCorrelationComponents>(histoNameAndTitle.Data(), histoNameAndTitle.Data(),
                                                    fDetector->GetEventClassVariablesSet());
  fInputHistograms->SetNoOfEntriesThreshold(fMinNoOfEntriesToValidate);
  fCalibrationHistograms =
      std::make_unique<CorrectionProfileCorrelationComponents>(histoNameAndTitle.Data(), histoNameAndTitle.Data(),
                                                    fDetector->GetEventClassVariablesSet());
  fCalibrationHistograms->CreateCorrelationComponentsProfileHistograms(list);
  return kTRUE;
}

/// Attaches the needed input information to the correction step
/// \param list list where the inputs should be found
/// \return kTRUE if everything went OK
Bool_t Alignment::AttachInput(TList *list) {
  if (fInputHistograms->AttachHistograms(list)) {
    fState = State::APPLYCOLLECT;
    return kTRUE;
  }
  return kFALSE;
}

/// Asks for QA histograms creation
///
/// Allocates the histogram objects and creates the QA histograms.
/// \param list list where the histograms should be incorporated for its persistence
/// \return kTRUE if everything went OK
Bool_t Alignment::CreateQAHistograms(TList *list) {
  fQAQnAverageHistogram = std::make_unique<CorrectionProfileComponents>(
      Form("%s %s", szQAQnAverageHistogramName, fDetector->GetName()),
      Form("%s %s", szQAQnAverageHistogramName, fDetector->GetName()),
      fDetector->GetEventClassVariablesSet());
  /* get information about the configured harmonics to pass it for histogram creation */
  Int_t nNoOfHarmonics = fDetector->GetNoOfHarmonics();
  auto harmonicsMap = new Int_t[nNoOfHarmonics];
  fDetector->GetHarmonicMap(harmonicsMap);
  fQAQnAverageHistogram->CreateComponentsProfileHistograms(list, nNoOfHarmonics, harmonicsMap);
  delete[] harmonicsMap;
  return kTRUE;
}

/// Asks for non validated entries QA histograms creation
///
/// Allocates the histogram objects and creates the non validated entries QA histograms.
/// \param list list where the histograms should be incorporated for its persistence
/// \return kTRUE if everything went OK
Bool_t Alignment::CreateNveQAHistograms(TList *list) {
  fQANotValidatedBin = std::make_unique<CorrectionHistogramSparse>(
      Form("%s %s", szQANotValidatedHistogramName, fDetector->GetName()),
      Form("%s %s", szQANotValidatedHistogramName, fDetector->GetName()),
      fDetector->GetEventClassVariablesSet());
  fQANotValidatedBin->CreateHistogram(list);
  return kTRUE;
}

/// Processes the correction step
///
/// Apply the correction step
/// \return kTRUE if the correction step was applied
Bool_t Alignment::ProcessCorrections(const double *variableContainer) {
  switch (fState) {
    case State::CALIBRATION:
      /* collect the data needed to further produce correction parameters if both current Qn vectors are good enough */
      /* we have not perform any correction yet */
      return kFALSE;
    case State::APPLYCOLLECT:
      /* collect the data needed to further produce correction parameters if both current Qn vectors are good enough */
      /* and proceed to ... */
    case State::APPLY: /* apply the correction if the current Qn vector is good enough */
      /* logging */
//      QnCorrectionsInfo(Form("Alignment process in detector %s with reference %s: applying correction.",
//                             fDetector->GetName(),
//                             fDetectorConfigurationForAlignment->GetName()));
      if (fDetector->GetCurrentQnVector()->IsGoodQuality()) {
        /* we get the properties of the current Qn vector but its name */
        fCorrectedQnVector->Set(fDetector->GetCurrentQnVector(), kFALSE);
        /* let's check the correction histograms */
        Long64_t bin = fInputHistograms->GetBin(variableContainer);
        if (fInputHistograms->BinContentValidated(bin)) {
          /* the bin content is validated so, apply the correction */
          Double_t XX = fInputHistograms->GetXXBinContent(bin);
          Double_t YY = fInputHistograms->GetYYBinContent(bin);
          Double_t XY = fInputHistograms->GetXYBinContent(bin);
          Double_t YX = fInputHistograms->GetYXBinContent(bin);
          Double_t eXY = fInputHistograms->GetXYBinError(bin);
          Double_t eYX = fInputHistograms->GetYXBinError(bin);
          Double_t deltaPhi = -TMath::ATan2((XY - YX), (XX + YY))*(1.0/fHarmonicForAlignment);
          /* significant correction? */
          if (!(TMath::Sqrt((XY - YX)*(XY - YX)/(eXY*eXY + eYX*eYX)) < 2.0)) {
            Int_t harmonic = fDetector->GetCurrentQnVector()->GetFirstHarmonic();
            while (harmonic!=-1) {
              fCorrectedQnVector->SetQx(harmonic,
                                        fDetector->GetCurrentQnVector()->Qx(harmonic)
                                            *TMath::Cos(((Double_t) harmonic)*deltaPhi)
                                            + fDetector->GetCurrentQnVector()->Qy(harmonic)
                                                *TMath::Sin(((Double_t) harmonic)*deltaPhi));
              fCorrectedQnVector->SetQy(harmonic,
                                        fDetector->GetCurrentQnVector()->Qy(harmonic)
                                            *TMath::Cos(((Double_t) harmonic)*deltaPhi)
                                            - fDetector->GetCurrentQnVector()->Qx(harmonic)
                                                *TMath::Sin(((Double_t) harmonic)*deltaPhi));
              harmonic = fDetector->GetCurrentQnVector()->GetNextHarmonic(harmonic);
            }
          } /* if the correction is not significant we leave the Q vector untouched */
        } /* if the correction bin is not validated we leave the Q vector untouched */
        else {
          if (fQANotValidatedBin) fQANotValidatedBin->Fill(variableContainer, 1.0);
        }
      } else {
        /* not done! input Q vector with bad quality */
        fCorrectedQnVector->SetGood(kFALSE);
      }
      /* and update the current Qn vector */
      fDetector->UpdateCurrentQnVector(fCorrectedQnVector.get());
      break;
    default:
      /* we are in passive state waiting for proper conditions, no corrections applied */
      return kFALSE;
  }
  /* if we reached here is because we applied the correction */
  return kTRUE;
}

/// Processes the correction step data collection
///
/// Collect data for the correction step.
/// \return kTRUE if the correction step was applied
Bool_t Alignment::ProcessDataCollection(const double *variableContainer) {
  switch (fState) {
    case State::CALIBRATION:
      /* logging */
//      QnCorrectionsInfo(Form("Alignment process in detector %s with reference %s: collecting data.",
//                             fDetector->GetName(),
//                             fDetectorConfigurationForAlignment->GetName()));
      /* collect the data needed to further produce correction parameters if both current Qn vectors are good enough */
      if ((fInputQnVector->IsGoodQuality()) &&
          (fDetectorForAlignment->GetCurrentQnVector()->IsGoodQuality())) {
        fCalibrationHistograms->FillXX(variableContainer,
                                       fInputQnVector->Qx(fHarmonicForAlignment)
                                           *fDetectorForAlignment->GetCurrentQnVector()->Qx(
                                               fHarmonicForAlignment));
        fCalibrationHistograms->FillXY(variableContainer,
                                       fInputQnVector->Qx(fHarmonicForAlignment)
                                           *fDetectorForAlignment->GetCurrentQnVector()->Qy(
                                               fHarmonicForAlignment));
        fCalibrationHistograms->FillYX(variableContainer,
                                       fInputQnVector->Qy(fHarmonicForAlignment)
                                           *fDetectorForAlignment->GetCurrentQnVector()->Qx(
                                               fHarmonicForAlignment));
        fCalibrationHistograms->FillYY(variableContainer,
                                       fInputQnVector->Qy(fHarmonicForAlignment)
                                           *fDetectorForAlignment->GetCurrentQnVector()->Qy(
                                               fHarmonicForAlignment));
      }
      /* we have not perform any correction yet */
      return kFALSE;
    case State::APPLYCOLLECT:
      /* logging */
//      QnCorrectionsInfo(Form("Alignment process in detector %s with reference %s: collecting data.",
//                             fDetector->GetName(),
//                             fDetectorConfigurationForAlignment->GetName()));
      /* collect the data needed to further produce correction parameters if both current Qn vectors are good enough */
      if ((fInputQnVector->IsGoodQuality()) &&
          (fDetectorForAlignment->GetCurrentQnVector()->IsGoodQuality())) {
        fCalibrationHistograms->FillXX(variableContainer,
                                       fInputQnVector->Qx(fHarmonicForAlignment)
                                           *fDetectorForAlignment->GetCurrentQnVector()->Qx(
                                               fHarmonicForAlignment));
        fCalibrationHistograms->FillXY(variableContainer,
                                       fInputQnVector->Qx(fHarmonicForAlignment)
                                           *fDetectorForAlignment->GetCurrentQnVector()->Qy(
                                               fHarmonicForAlignment));
        fCalibrationHistograms->FillYX(variableContainer,
                                       fInputQnVector->Qy(fHarmonicForAlignment)
                                           *fDetectorForAlignment->GetCurrentQnVector()->Qx(
                                               fHarmonicForAlignment));
        fCalibrationHistograms->FillYY(variableContainer,
                                       fInputQnVector->Qy(fHarmonicForAlignment)
                                           *fDetectorForAlignment->GetCurrentQnVector()->Qy(
                                               fHarmonicForAlignment));
      }
      /* and proceed to ... */
      /* FALLTHRU */
    case State::APPLY: /* apply the correction if the current Qn vector is good enough */
      /* provide QA info if required */
      if (fQAQnAverageHistogram) {
        Int_t harmonic = fCorrectedQnVector->GetFirstHarmonic();
        while (harmonic!=-1) {
          fQAQnAverageHistogram->FillX(harmonic, variableContainer, fCorrectedQnVector->Qx(harmonic));
          fQAQnAverageHistogram->FillY(harmonic, variableContainer, fCorrectedQnVector->Qy(harmonic));
          harmonic = fCorrectedQnVector->GetNextHarmonic(harmonic);
        }
      }
      break;
    default:
      /* we are in passive state waiting for proper conditions, no corrections applied */
      return kFALSE;
  }
  /* if we reached here is because we applied the correction */
  return kTRUE;
}

/// Clean the correction to accept a new event
void Alignment::ClearCorrectionStep() {
  fCorrectedQnVector->Reset();
}

/// Reports if the correction step is being applied
/// Returns TRUE if in the proper state for applying the correction step
/// \return TRUE if the correction step is being applied
Bool_t Alignment::IsBeingApplied() const {
  switch (fState) {
    case State::CALIBRATION:
      /* we are collecting */
      /* but not applying */
      return kFALSE;
    case State::APPLYCOLLECT:
      /* we are collecting */
    case State::APPLY:
      /* and applying */
      return kTRUE;
    default:
      break;
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
Bool_t Alignment::ReportUsage(TList *calibrationList, TList *applyList) {
  switch (fState) {
    case State::CALIBRATION:
      /* we are collecting */
      calibrationList->Add(new TObjString(szCorrectionName));
      /* but not applying */
      return kFALSE;
    case State::APPLYCOLLECT:
      /* we are collecting */
      calibrationList->Add(new TObjString(szCorrectionName));
      /* FALLTHRU */
    case State::APPLY:
      /* and applying */
      applyList->Add(new TObjString(szCorrectionName));
      break;
    default:
      return kFALSE;
  }
  return kTRUE;
}
}
