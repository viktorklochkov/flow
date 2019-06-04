#ifndef QNCORRECTIONS_DETECTORCONFCHANNEL_H
#define QNCORRECTIONS_DETECTORCONFCHANNEL_H

/***************************************************************************
 * Package:       FlowVectorCorrections                                    *
 * Authors:       Jaap Onderwaater, GSI, jacobus.onderwaater@cern.ch       *
 *                Ilya Selyuzhenkov, GSI, ilya.selyuzhenkov@gmail.com      *
 *                Víctor González, UCM, victor.gonzalez@cern.ch            *
 *                Contributors are mentioned in the code where appropriate.*
 * Development:   2012-2016                                                *
 * See cxx source for GPL licence et. al.                                  *
 ***************************************************************************/

/// \file QnCorrectionsDetectorConfigurationChannels.h
/// \brief Channel detector configuration class for Q vector correction framework
///

#include "CorrectionsSet.h"
#include "SubEvent.h"
#include "CorrectionLog.h"
#include "CorrectionDataVector.h"

namespace Qn {
class CorrectionProfileComponents;

/// \class QnCorrectionsDetectorConfigurationChannels
/// \brief Channel detector configuration within Q vector correction framework
///
/// A channel detector within the Q vector correction framework is defined
/// as one for which its data vectors involve azimuthal angles and channels
/// susceptible of weighting and / or grouping and / or calibration, etc.
///
/// According to that, the proper channelized data vector is used and an extra
/// Q vector builder is incorporated.
///
/// \author Jaap Onderwaater <jacobus.onderwaater@cern.ch>, GSI
/// \author Ilya Selyuzhenkov <ilya.selyuzhenkov@gmail.com>, GSI
/// \author Víctor González <victor.gonzalez@cern.ch>, UCM
/// \date Feb 08, 2016

class SubEventChannels : public SubEvent {
 public:
  friend class CorrectionStep;
  friend class SubEvent;
  SubEventChannels() = default;
  SubEventChannels(const char *name,
                                EventClassVariablesSet *eventClassesVariables,
                                Int_t nNoOfChannels,
                                Int_t nNoOfHarmonics,
                                Int_t *harmonicMap = NULL);
  virtual ~SubEventChannels();
  SubEventChannels(const SubEventChannels &) = delete;
  SubEventChannels &operator=(const SubEventChannels &) = delete;

  /// Gets the number of channels
  /// \return the number of channels of the associated detector
  Int_t GetNoOfChannels() { return fNoOfChannels; }
  /// Gets the used channels mask
  /// \return the used channels mask
  const Bool_t *GetUsedChannelsMask() const { return fUsedChannel; }
  /// Gets the channels groups
  /// \return the group associated to each channel
  const Int_t *GetChannelsGroups() const { return fChannelGroup; }
  /// Gets the hard coded group weights
  /// \return the groups hard coded weights
  const Float_t *GetHardCodedGroupWeights() const { return fHardCodedGroupWeights; }
  /// Get if the detector configuration is own by a tracking detector
  /// \return FALSE, this is a hit / channel detector configuration
  virtual Bool_t GetIsTrackingDetector() const { return kFALSE; }

  virtual void SetChannelsScheme(Bool_t *bUsedChannel,
                                 Int_t *nChannelGroup = nullptr,
                                 Float_t *hardCodedGroupWeights = nullptr);

  /* QA section */
  /// Sets the variable id used for centrality in QA histograms.
  /// It must be one of the event class variables.
  /// \param id id for the variable used for centrality
  void SetQACentralityVar(Int_t id) { fQACentralityVarId = id; }
  /// Sets the characteristics of the multiplicity axis in QA histograms
  /// \param nbins the number of bins
  /// \param min minimum multiplicity value
  /// \param max maximum multiplicity value
  void SetQAMultiplicityAxis(Int_t nbins, Float_t min, Float_t max) {
    fQAnBinsMultiplicity = nbins;
    fQAMultiplicityMin = min;
    fQAMultiplicityMax = max;
  }

  virtual void AttachCorrectionsManager(CorrectionCalculator *manager);

  virtual void CreateSupportDataStructures();
  virtual Bool_t CreateSupportHistograms(TList *list);
  virtual Bool_t CreateQAHistograms(TList *list);
  virtual Bool_t CreateNveQAHistograms(TList *list);

  /// Activate the processing for the passed harmonic
  /// \param harmonic the desired harmonic number to activate
  virtual void ActivateHarmonic(Int_t harmonic) {
    SubEvent::ActivateHarmonic(harmonic);
    fRawQnVector.ActivateHarmonic(harmonic);
  }
  virtual Bool_t AttachCorrectionInputs(TList *list);
  virtual void AfterInputsAttachActions();
  virtual Bool_t ProcessCorrections(const double *variableContainer);
  virtual Bool_t ProcessDataCollection(const double *variableContainer);
  virtual void AddCorrectionOnInputData(CorrectionOnInputData *correctionOnInputData);
  void BuildRawQnVector();
  virtual void IncludeQnVectors(TList *list);
  virtual void FillOverallInputCorrectionStepList(std::set<CorrectionStep*, CompareSteps> &set) const;
  virtual void FillOverallQnVectorCorrectionStepList(std::set<CorrectionStep*, CompareSteps> &set) const;
  virtual void ReportOnCorrections(TList *steps, TList *calib, TList *apply) const;
  virtual void ClearDetector();

 private:
  static const char
      *szRawQnVectorName;   ///< the name of the raw Qn vector from raw data without input data corrections
  CorrectionQnVector fRawQnVector;     ///< Q vector from input data before pre-processing
  Int_t fNoOfChannels = 0;                    ///< The number of channels associated
  /// array, which of the detector channels is used for this configuration
  Bool_t *fUsedChannel = nullptr;                   //[fNoOfChannels]
  /// array, mapping external to internal channel id. TODO: this has to go to a more generic histogram support
  Int_t *fChannelMap = nullptr;                     //[fNoOfChannels]
  /// array, the group to which the channel pertains
  Int_t *fChannelGroup = nullptr;                   //[fNoOfChannels]
  /// array, group hard coded weight
  Float_t *fHardCodedGroupWeights = nullptr;         //[fNoOfChannels]
  CorrectionsSetOnInputData fInputDataCorrections; ///< set of corrections to apply on input data vectors

  /* QA section */
  void FillQAHistograms(const double *variableContainer);
  static const char *szQAMultiplicityHistoName; ///< QA multiplicity histograms name
  Int_t fQACentralityVarId = -1;   ///< the id of the variable used for centrality in QA histograms
  Int_t fQAnBinsMultiplicity = 100; ///< number of bins for multiplicity in QA histograms
  Float_t fQAMultiplicityMin = 0.0; ///< minimum multiplicity value
  Float_t fQAMultiplicityMax = 1000.0; ///< maximum multiplicity value
  TH3F *fQAMultiplicityBefore3D = nullptr; //!<! 3D channel multiplicity histogram before input equalization
  TH3F *fQAMultiplicityAfter3D = nullptr;  //!<! 3D channel multiplicity histogram after input equalization

/// \cond CLASSIMP
 ClassDef(SubEventChannels, 2);
/// \endcond
};

/// Builds raw Qn vector before Q vector corrections and before input
/// data corrections but considering the chosen calibration method.
/// This is a channelized configuration so this Q vector will NOT be
/// the one to be used for subsequent Q vector corrections.
inline void SubEventChannels::BuildRawQnVector() {
  fTempQnVector.Reset();
  for (const auto &dataVector : fDataVectorBank) {
    fTempQnVector.Add(dataVector.Phi(), dataVector.Weight());
  }
  fTempQnVector.CheckQuality();
  fTempQnVector.Normalize(fNormalizationMethod);
  fRawQnVector.Set(&fTempQnVector, kFALSE);
}

/// Ask for processing corrections for the involved detector configuration
///
/// The request is transmitted to the incoming data correction steps
/// and then to Q vector correction steps.
/// The first not applied correction step breaks the loop and kFALSE is returned
/// \return kTRUE if all correction steps were applied
inline Bool_t SubEventChannels::ProcessCorrections(const double *variableContainer) {

  /* first we build the raw Q vector with the chosen calibration */
  BuildRawQnVector();

  /* then we transfer the request to the input data correction steps */
  for (auto &correction : fInputDataCorrections) {
    if (correction->ProcessCorrections(variableContainer)) {
      continue;
    } else {
      return kFALSE;
    }
  }

  /* input corrections were applied so let's build the Q vector with the chosen calibration */
  BuildQnVector();

  /* now let's propagate it to Q vector corrections */
  for (auto &correction : fQnVectorCorrections) {
    if (correction->ProcessCorrections(variableContainer))
      continue;
    else
      return kFALSE;
  }
  /* all correction steps were applied */
  return kTRUE;
}

/// Ask for processing corrections data collection for the involved detector configuration
///
/// The request is transmitted to the incoming data correction steps
/// and then to Q vector correction steps.
/// The first not applied correction step should break the loop after collecting the data and kFALSE is returned
/// \return kTRUE if all correction steps were applied
inline Bool_t SubEventChannels::ProcessDataCollection(const double *variableContainer) {
  /* we transfer the request to the input data correction steps */
  for (auto &correction : fInputDataCorrections) {
    if (correction->ProcessDataCollection(variableContainer)) {
      continue;
    } else {
      return kFALSE;
    }
  }
  /* check whether QA histograms must be filled */
  FillQAHistograms(variableContainer);
  /* now let's propagate it to Q vector corrections */
  for (auto &correction : fQnVectorCorrections) {
    if (correction->ProcessDataCollection(variableContainer))
      continue;
    else
      return kFALSE;
  }
  /* all correction steps were applied */
  return kTRUE;
}

/// Clean the configuration to accept a new event
///
/// Transfers the order to the Q vector correction steps then
/// to the input data correction steps and finally
/// cleans the own Q vector and the input data vector bank
/// for accepting the next event.
inline void SubEventChannels::ClearDetector() {
  for (auto &correction : fQnVectorCorrections) {
    correction->ClearCorrectionStep();
  }
  for (auto &correction : fInputDataCorrections) {
    correction->ClearCorrectionStep();
  }
  /* clean the raw Q vector */
  fRawQnVector.Reset();
  /* clean the own Q vector */
  fPlainQnVector.Reset();
  fPlainQ2nVector.Reset();
  fCorrectedQnVector.Reset();
  fCorrectedQ2nVector.Reset();
  /* and now clear the the input data bank */
  fDataVectorBank.clear();
}

}
#endif // QNCORRECTIONS_DETECTORCONFCHANNEL_H
