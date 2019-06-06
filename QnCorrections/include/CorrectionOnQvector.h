#ifndef QNCORRECTIONS_CORRECTIONONQNVECTOR_H
#define QNCORRECTIONS_CORRECTIONONQNVECTOR_H

/***************************************************************************
 * Package:       FlowVectorCorrections                                    *
 * Authors:       Jaap Onderwaater, GSI, jacobus.onderwaater@cern.ch       *
 *                Ilya Selyuzhenkov, GSI, ilya.selyuzhenkov@gmail.com      *
 *                Víctor González, UCM, victor.gonzalez@cern.ch            *
 *                Contributors are mentioned in the code where appropriate.*
 * Development:   2012-2016                                                *
 * See cxx source for GPL licence et. al.                                  *
 ***************************************************************************/

/// \file QnCorrectionsCorrectionOnQvector.h
/// \brief Correction steps on Qn vectors support within Q vector correction framework
///

#include <TNamed.h>
#include "TList.h"
#include "CorrectionQnVector.h"
#include "CorrectionStep.h"

/// \class QnCorrectionsCorrectionOnQvector
/// \brief Base class for correction steps applied to a Q vector
///
/// \author Jaap Onderwaater <jacobus.onderwaater@cern.ch>, GSI
/// \author Ilya Selyuzhenkov <ilya.selyuzhenkov@gmail.com>, GSI
/// \author Víctor González <victor.gonzalez@cern.ch>, UCM
/// \date Feb 05, 2016

namespace Qn {
class CorrectionOnQvector : public CorrectionStep {
 public:

  enum Priority {
    kRecentering,
    kAlignment,
    kTwistAndRescale
  };

  CorrectionOnQvector() = default;
  CorrectionOnQvector(const char *name, unsigned int prio) : CorrectionStep(name, prio) {}
  virtual ~CorrectionOnQvector() = default;
  /// Copy constructor deleted
  CorrectionOnQvector(CorrectionOnQvector &) = delete;
  /// Assignment operator deleted
  CorrectionOnQvector &operator=(const CorrectionOnQvector &) = delete;
  /// Informs when the detector configuration has been attached to the framework manager
  /// Basically this allows interaction between the different framework sections at configuration time
  /// Pure virtual function
  virtual void AttachedToFrameworkManager() = 0;
  /// Attaches the needed input information to the correction step
  ///
  /// Pure virtual function
  /// \param list list where the inputs should be found
  /// \return kTRUE if everything went OK
  virtual Bool_t AttachInput(TList *list) = 0;
  /// Perform after calibration histograms attach actions
  /// It is used to inform the different correction step that
  /// all conditions for running the network are in place so
  /// it is time to check if their requirements are satisfied
  ///
  /// Pure virtual function
  virtual void AfterInputsAttachActions() = 0;
  /// Asks for support data structures creation
  ///
  /// Pure virtual function
  virtual void CreateSupportDataStructures() = 0;
  /// Asks for support histograms creation
  ///
  /// Pure virtual function
  /// \param list list where the histograms should be incorporated for its persistence
  /// \return kTRUE if everything went OK
  virtual Bool_t CreateSupportHistograms(TList *list) = 0;
  /// Processes the correction step
  ///
  /// Pure virtual function
  /// \return kTRUE if everything went OK
  virtual Bool_t ProcessCorrections(const double *variableContainer) = 0;
  /// Processes the correction step data collection
  ///
  /// Pure virtual function
  /// \return kTRUE if everything went OK
  virtual Bool_t ProcessDataCollection(const double *variableContainer) = 0;
  /// Gets the corrected Qn vector
  /// \return the corrected Qn vector
  const CorrectionQnVector *GetCorrectedQnVector() const { return fCorrectedQnVector.get(); }
  /// Include the new corrected Qn vector into the passed list
  ///
  /// Adds the Qn vector to the passed list
  /// if the correction step is in correction states.
  /// \param list list where the corrected Qn vector should be added
  void IncludeCorrectedQnVector(TList *list) {
    switch (fState) {
      case State::CALIBRATION:
        /* collect the data needed to further produce correction parameters */
        break;
      case State::APPLYCOLLECT:
        /* collect the data needed to further produce correction parameters */
        /* and proceed to ... */
        /* FALLTHRU */
      case State::APPLY: /* apply the correction */
        list->Add(fCorrectedQnVector.get());
        break;
      default:
        break;
    }
  }
  /// Clean the correction to accept a new event
  /// Pure virtual function
  virtual void ClearCorrectionStep() = 0;
  /// Reports if the correction step is being applied
  /// Pure virtual function
  /// \return TRUE if the correction step is being applied
  virtual Bool_t IsBeingApplied() const = 0;
  /// Report on correction usage
  /// Pure virtual function
  /// Correction step should incorporate its name in calibration
  /// list if it is producing information calibration in the ongoing
  /// step and in the apply list if it is applying correction in
  /// the ongoing step.
  /// \param calibrationList list containing the correction steps producing calibration information
  /// \param applyList list containing the correction steps applying corrections
  /// \return kTRUE if the correction step is being applied
  virtual Bool_t ReportUsage(TList *calibrationList, TList *applyList) = 0;
 protected:
  std::unique_ptr<CorrectionQnVector> fCorrectedQnVector; //!<! the step corrected Qn vector
  const CorrectionQnVector *fInputQnVector = nullptr; //!<! the previous step corrected Qn vector
/// \cond CLASSIMP
 ClassDef(CorrectionOnQvector, 2);
/// \endcond
};
}
#endif // QNCORRECTIONS_CORRECTIONONQVECTORS_H
