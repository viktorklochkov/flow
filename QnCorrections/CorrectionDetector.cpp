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

/// \file QnCorrectionsDetector.cxx
/// \brief Detector class implementation

#include "CorrectionDetector.h"
#include "CorrectionLog.h"
/// \cond CLASSIMP
ClassImp(Qn::CorrectionDetector);
/// \endcond
namespace Qn {

/// Default constructor
CorrectionDetector::CorrectionDetector() : TNamed(),
                                           fConfiguration() {

  fDetectorId = -1;
  fCorrectionsManager = nullptr;
}

/// Normal constructor
/// \param name the name of the detector
/// \param id detector Id
CorrectionDetector::CorrectionDetector(const char *name, Int_t id) :
    TNamed(name, name),
    fConfiguration(nullptr) {

  fDetectorId = id;
  fCorrectionsManager = nullptr;
}

/// Default destructor
/// The detector class does not own anything
CorrectionDetector::~CorrectionDetector() = default;

/// Asks for support data structures creation
///
/// The request is transmitted to the attached detector configurations
void CorrectionDetector::CreateSupportDataStructures() {
  fConfiguration->CreateSupportDataStructures();
}

/// Asks for support histograms creation
///
/// The request is transmitted to the attached detector configurations
/// \param list list where the histograms should be incorporated for its persistence
/// \return kTRUE if everything went OK
Bool_t CorrectionDetector::CreateSupportHistograms(TList *list) {
  return fConfiguration->CreateSupportHistograms(list);
}

/// Asks for QA histograms creation
///
/// The request is transmitted to the attached detector configurations
/// \param list list where the histograms should be incorporated for its persistence
/// \return kTRUE if everything went OK
Bool_t CorrectionDetector::CreateQAHistograms(TList *list) {
  return fConfiguration->CreateQAHistograms(list);
}

/// Asks for non validated entries QA histograms creation
///
/// The request is transmitted to the attached detector configurations
/// \param list list where the histograms should be incorporated for its persistence
/// \return kTRUE if everything went OK
Bool_t CorrectionDetector::CreateNveQAHistograms(TList *list) {
  return fConfiguration->CreateNveQAHistograms(list);
}

/// Asks for attaching the needed input information to the correction steps
///
/// The request is transmitted to the attached detector configurations
/// \param list list where the input information should be found
/// \return kTRUE if everything went OK
Bool_t CorrectionDetector::AttachCorrectionInputs(TList *list) {
  return fConfiguration->AttachCorrectionInputs(list);
}

/// Perform after calibration histograms attach actions
/// It is used to inform the different correction step that
/// all conditions for running the network are in place so
/// it is time to check if their requirements are satisfied
///
/// The request is transmitted to the attached detector configurations
void CorrectionDetector::AfterInputsAttachActions() {
  fConfiguration->AfterInputsAttachActions();
}

/// Stores the framework manager pointer and transmits it to the incorporated detector configurations if any
///
/// \param manager the framework manager
void CorrectionDetector::AttachCorrectionsManager(CorrectionCalculator *manager) {
  fCorrectionsManager = manager;
  fConfiguration->AttachCorrectionsManager(manager);
}

/// Adds a new detector configuration to the current detector
///
/// Raise an execution error if the configuration detector reference
/// is not empty and if the detector configuration
/// is already incorporated to the detector.
/// \param detectorConfiguration pointer to the configuration to be added
void CorrectionDetector::AddDetectorConfiguration(DetectorConfiguration *detectorConfiguration) {
  detectorConfiguration->SetDetectorOwner(this);
  detectorConfiguration->AttachCorrectionsManager(fCorrectionsManager);
  fConfiguration.reset(detectorConfiguration);
}

/// Searches for a concrete detector configuration by name
/// \param name the name of the detector configuration to find
/// \return pointer to the found detector configuration (NULL if not found)
DetectorConfiguration *CorrectionDetector::GetDetectorConfiguration() {
  return fConfiguration.get();
}

/// Include the the list of Qn vector associated to the detector
/// into the passed list
///
/// The request is transmitted to the attached detector configurations
/// \param list list where the corrected Qn vector should be added
void CorrectionDetector::IncludeQnVectors(TList *list) {
  fConfiguration->IncludeQnVectors(list);
}

/// Include the name of each detector configuration into the passed list
///
/// \param list the list where to incorporate detector configurations name
void CorrectionDetector::FillDetectorConfigurationNameList(TList *list) const {
  list->Add(new TObjString(fConfiguration->GetName()));
}

/// Include the name of the input correction steps on each detector
/// configuration into the passed list
///
/// The request is transmitted to the attached detector configurations
/// \param list list where the corrected Qn vector should be added
void CorrectionDetector::FillOverallInputCorrectionStepList(TList *list) const {
  fConfiguration->FillOverallInputCorrectionStepList(list);
}

/// Include the name of the Qn vector correction steps on each detector
/// configuration into the passed list
///
/// The request is transmitted to the attached detector configurations
/// \param list list where the corrected Qn vector should be added
void CorrectionDetector::FillOverallQnVectorCorrectionStepList(TList *list) const {
  fConfiguration->FillOverallQnVectorCorrectionStepList(list);
}

/// Provide information about assigned corrections on each of the detector configurations
///
/// The request is transmitted to the attached detector configurations
/// \param steps list for incorporating the list of assigned correction steps
/// \param calib list for incorporating the list of steps in calibrating status
/// \param apply list for incorporating the list of steps in applying status
void CorrectionDetector::ReportOnCorrections(TList *steps, TList *calib, TList *apply) const {
  fConfiguration->ReportOnCorrections(steps, calib, apply);
}

}

