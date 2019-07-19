#ifndef QNCORRECTIONS_CORRECTIONONINPUTDATA_H
#define QNCORRECTIONS_CORRECTIONONINPUTDATA_H

/***************************************************************************
 * Package:       FlowVectorCorrections                                    *
 * Authors:       Jaap Onderwaater, GSI, jacobus.onderwaater@cern.ch       *
 *                Ilya Selyuzhenkov, GSI, ilya.selyuzhenkov@gmail.com      *
 *                Víctor González, UCM, victor.gonzalez@cern.ch            *
 *                Contributors are mentioned in the code where appropriate.*
 * Development:   2012-2016                                                *
 * See cxx source for GPL licence et. al.                                  *
 ***************************************************************************/

/// \file QnCorrectionsCorrectionOnInputData.h
/// \brief Correction steps on input data support within Q vector correction framework
///

#include <TNamed.h>
#include <TList.h>

#include "CorrectionStep.h"

namespace Qn {

/// \class QnCorrectionsCorrectionOnInputData
/// \brief Base class for correction steps applied to input data
///
/// \author Jaap Onderwaater <jacobus.onderwaater@cern.ch>, GSI
/// \author Ilya Selyuzhenkov <ilya.selyuzhenkov@gmail.com>, GSI
/// \author Víctor González <victor.gonzalez@cern.ch>, UCM
/// \date Feb 05, 2016

class CorrectionOnInputData : public CorrectionStep {
 public:
  enum Priority {
    kGainEqualization
  };
  friend class SubEventChannels;
  CorrectionOnInputData() = default;
  CorrectionOnInputData(const char *name, unsigned int prio) : CorrectionStep(name, prio) {}
  virtual ~CorrectionOnInputData() = default;
  /// Perform after calibration histograms attach actions
  /// It is used to inform the different correction step that
  /// all conditions for running the network are in place so
  /// it is time to check if their requirements are satisfied
  /// Does nothing for the time being
  virtual void AfterInputAttachAction() {}
  /// Reports if the correction step is being applied
  /// \return FALSE, input data correction step dont make use of this service, yet
  virtual Bool_t IsBeingApplied() const { return kFALSE; }
/// \cond CLASSIMP
 ClassDef(CorrectionOnInputData, 1);
/// \endcond
};
}

#endif // QNCORRECTIONS_CORRECTIONONINPUTDATA_H
