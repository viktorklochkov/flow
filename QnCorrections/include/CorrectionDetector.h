#ifndef QNCORRECTIONS_DETECTOR_H
#define QNCORRECTIONS_DETECTOR_H

/***************************************************************************
 * Package:       FlowVectorCorrections                                    *
 * Authors:       Jaap Onderwaater, GSI, jacobus.onderwaater@cern.ch       *
 *                Ilya Selyuzhenkov, GSI, ilya.selyuzhenkov@gmail.com      *
 *                Víctor González, UCM, victor.gonzalez@cern.ch            *
 *                Contributors are mentioned in the code where appropriate.*
 * Development:   2012-2016                                                *
 * See cxx source for GPL licence et. al.                                  *
 ***************************************************************************/

/// \file QnCorrectionsDetector.h
/// \brief Detector and detector configuration classes for Q vector correction framework
///

#include "DetectorConfiguration.h"
namespace Qn {
class DetectorConfigurationsSet;
class DetectorConfiguration;
class CorrectionDetector;
class CorrectionCalculator;

/// \class QnCorrectionsDetector
/// \brief Detector class within Q vector correction framework
///
/// The roles of the detector class are: to store its unique name and Id,
/// and to store and handle the list of the different
/// configurations defined for the involved detector.
///
/// The detector Id should only be obtained at creation time and
/// the object, once created, does not allow its modification.
///
/// The detector object is in the path of the whole control flow and
/// as such it should distribute the different commands to the
/// defined detector configurations.
///
/// \author Jaap Onderwaater <jacobus.onderwaater@cern.ch>, GSI
/// \author Ilya Selyuzhenkov <ilya.selyuzhenkov@gmail.com>, GSI
/// \author Víctor González <victor.gonzalez@cern.ch>, UCM
/// \date Feb 08, 2016

class CorrectionDetector : public TNamed {
 public:
  CorrectionDetector();
  CorrectionDetector(const char *name, Int_t id);
  virtual ~CorrectionDetector();

  /// Gets the detector Id
  ///
  /// \return detector Id
  Int_t GetId() { return fDetectorId; }

  void CreateSupportDataStructures();
  Bool_t CreateSupportHistograms(TList *list);
  Bool_t CreateQAHistograms(TList *list);
  Bool_t CreateNveQAHistograms(TList *list);
  Bool_t AttachCorrectionInputs(TList *list);
  virtual void AfterInputsAttachActions();
  Bool_t ProcessCorrections(const double *variableContainer);
  Bool_t ProcessDataCollection(const double *variableContainer);
  void IncludeQnVectors(TList *list);

  /// Get the Pointer to input data bank.
  /// Makes it available for input corrections steps.
  /// \param index index of the configuration
  /// \return pointer to the input data bank
  std::vector<Qn::CorrectionDataVector> &GetInputDataBank() {
    return fConfiguration->GetInputDataBank();
  }

  void AttachCorrectionsManager(CorrectionCalculator *manager);
  void AddDetectorConfiguration(DetectorConfiguration *detectorConfiguration);
  DetectorConfiguration *GetDetectorConfiguration();
  void FillDetectorConfigurationNameList(TList *list) const;
  void FillOverallInputCorrectionStepList(TList *list) const;
  void FillOverallQnVectorCorrectionStepList(TList *list) const;
  virtual void ReportOnCorrections(TList *steps, TList *calib, TList *apply) const;


  virtual void ClearDetector();

 private:
  Int_t fDetectorId;            ///< detector Id
  std::unique_ptr<DetectorConfiguration> fConfiguration;
  CorrectionCalculator *fCorrectionsManager; ///< the framework correction manager

 private:
  /// Copy constructor
  /// Not allowed. Forced private.
  CorrectionDetector(const CorrectionDetector &);
  /// Assignment operator
  /// Not allowed. Forced private.
  CorrectionDetector &operator=(const CorrectionDetector &);

/// \cond CLASSIMP
 ClassDef(CorrectionDetector, 3);
/// \endcond
};

/// Ask for processing corrections for the involved detector
///
/// The request is transmitted to the attached detector configurations
/// \return kTRUE if everything went OK
inline Bool_t CorrectionDetector::ProcessCorrections(const double *variableContainer) {
  return fConfiguration->ProcessCorrections(variableContainer);
}

/// Ask for processing corrections data collection for the involved detector
///
/// The request is transmitted to the attached detector configurations
/// \return kTRUE if everything went OK
inline Bool_t CorrectionDetector::ProcessDataCollection(const double *variableContainer) {
  return fConfiguration->ProcessDataCollection(variableContainer);
}

/// Clean the detector to accept a new event
///
/// Transfers the order to the detector configurations
inline void CorrectionDetector::ClearDetector() {
  fConfiguration->ClearConfiguration();
}
}
#endif // QNCORRECTIONS_DETECTOR_H
