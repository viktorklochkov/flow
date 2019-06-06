#ifndef QNCORRECTIONS_EVENTCLASSVAR_H
#define QNCORRECTIONS_EVENTCLASSVAR_H
/***************************************************************************
 * Package:       FlowVectorCorrections                                    *
 * Authors:       Jaap Onderwaater, GSI, jacobus.onderwaater@cern.ch       *
 *                Ilya Selyuzhenkov, GSI, ilya.selyuzhenkov@gmail.com      *
 *                Víctor González, UCM, victor.gonzalez@cern.ch            *
 *                Contributors are mentioned in the code where appropriate.*
 * Development:   2012-2016                                                *
 * See cxx source for GPL licence et. al.                                  *
 ***************************************************************************/

/// \file QnCorrectionsEventClassVariable.h
/// \brief Class that models variables used for defining an event class within the Q vector correction framework

/// \class QnCorrectionsEventClassVariable
/// \brief One variable used for defining an event class
///
/// Class defining one variable and its associated binning allowing
/// its use for the definition of event classes within the Q vector
/// correction framework.
///
/// \author Jaap Onderwaater <jacobus.onderwaater@cern.ch>, GSI
/// \author Ilya Selyuzhenkov <ilya.selyuzhenkov@gmail.com>, GSI
/// \author Víctor González <victor.gonzalez@cern.ch>, UCM
/// \date Jan 4, 2016


#include <TObject.h>
#include <TObjArray.h>
#include <Axis.h>

namespace Qn {
class EventClassVariable {
 public:
  EventClassVariable() = default;
  EventClassVariable(Int_t id, AxisD axis) : id_(id), axis_(axis) {}
  virtual ~EventClassVariable() = default;
  EventClassVariable &operator=(const EventClassVariable &) = delete;
  /// Gets the variable unique Id
  Int_t GetId() const { return id_; }
  /// Gets the variable name / label
  const char *GetLabel() const { return axis_.Name().data(); }
  /// Gets the number of bins
  Int_t GetNBins() const { return axis_.GetNBins(); }
  /// Gets the actual bins edges array
  const Double_t *GetBins() const { return axis_.GetPtr(); }
  /// Gets the lower edge for the passed bin number
  /// \param bin bin number starting from one
  Double_t GetBinLowerEdge(Int_t bin) const { return axis_.GetLowerBinEdge(bin); }
  /// Gets the upper edge for the passed bin number
  /// \param bin bin number starting from one
  Double_t GetBinUpperEdge(Int_t bin) const { return axis_.GetUpperBinEdge(bin); }
  /// Gets the lowest variable value considered
  Double_t GetLowerEdge() const { return axis_.GetFirstBinEdge(); }
  /// Gets the highest variabel value considered
  Double_t GetUpperEdge() const { return axis_.GetLastBinEdge(); }
 private:
  Int_t id_ = -1;        ///< The external Id for the variable in the data bank
  AxisD axis_;

/// \cond CLASSIMP
 ClassDef(EventClassVariable, 1);
/// \endcond
};
}
#endif /* QNCORRECTIONS_EVENTCLASSVAR_H */
