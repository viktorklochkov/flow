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
#include <map>
#include "TList.h"
#include "QVector.h"
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
  enum Step {
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
  /// Perform after calibration histograms attach actions
  /// It is used to inform the different correction step that
  /// all conditions for running the network are in place so
  /// it is time to check if their requirements are satisfied
  ///
  /// Pure virtual function
  virtual void AfterInputAttachAction() = 0;
  /// Gets the corrected Qn vector
  /// \return the corrected Qn vector
  const QVector *GetCorrectedQnVector() const { return fCorrectedQnVector.get(); }
  /// Include the new corrected Qn vector into the passed list
  ///
  /// Adds the Qn vector to the passed list
  /// if the correction step is in correction states.
  /// \param list list where the corrected Qn vector should be added
  virtual void IncludeCorrectedQnVector(std::map<QVector::CorrectionStep, QVector *> &qvectors) const {
    switch (fState) {
      case State::CALIBRATION:
        /* collect the data needed to further produce correction parameters */
        break;
      case State::APPLYCOLLECT:
        /* collect the data needed to further produce correction parameters */
        /* and proceed to ... */
        /* FALLTHRU */
      case State::APPLY: /* apply the correction */
        qvectors.emplace(fCorrectedQnVector->GetCorrectionStep(), fCorrectedQnVector.get());
        break;
      default:break;
    }
  }
  /// Reports if the correction step is being applied
  /// Pure virutal function
  /// \return TRUE if the correction step is being applied
  virtual Bool_t IsBeingApplied() const {
    bool applied = false;
    switch (fState) {
      case State::CALIBRATION: break;
      case State::APPLYCOLLECT:
        /* FALLTHRU */
      case State::APPLY: applied = true; break;
      case State::PASSIVE: break;
    }
    return applied;
  }
 protected:
  std::unique_ptr<QVector> fCorrectedQnVector; //!<! the step corrected Qn vector
  const QVector *fInputQnVector = nullptr; //!<! the previous step corrected Qn vector
/// \cond CLASSIMP
 ClassDef(CorrectionOnQvector, 2);
/// \endcond
};
}
#endif // QNCORRECTIONS_CORRECTIONONQVECTORS_H
