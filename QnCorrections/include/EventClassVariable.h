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

namespace Qn {
class EventClassVariable {
 public:
  EventClassVariable() = default;
  EventClassVariable(const EventClassVariable &ecv);
  EventClassVariable(Int_t varId, const char *varname, Int_t nbins, Double_t min, Double_t max);
  EventClassVariable(Int_t varId, const char *varname, Int_t nbins, Double_t *bins);
  EventClassVariable(Int_t varId, const char *varname, Double_t binsArray[][2]);
  virtual ~EventClassVariable() {
    delete[] fBins;
  }
  EventClassVariable &operator=(const EventClassVariable &) = delete;
  /// Gets the variable unique Id
  Int_t GetId() const { return fVarId; }
  /// Gets the variable name / label
  const char *GetLabel() const { return fLabel.data(); }
  /// Gets the number of bins
  Int_t GetNBins() const { return fNBins; }
  /// Gets the actual bins edges array
  const Double_t *GetBins() const { return fBins; }
  /// Gets the lower edge for the passed bin number
  /// \param bin bin number starting from one
  Double_t GetBinLowerEdge(Int_t bin) const { return (((bin < 1) || (bin > fNBins)) ? 0.0 : fBins[bin - 1]); }
  /// Gets the upper edge for the passed bin number
  /// \param bin bin number starting from one
  Double_t GetBinUpperEdge(Int_t bin) const { return (((bin < 1) || (bin > fNBins)) ? 0.0 : fBins[bin]); }
  /// Gets the lowest variable value considered
  Double_t GetLowerEdge() { return fBins[0]; }
  /// Gets the highest variabel value considered
  Double_t GetUpperEdge() { return fBins[fNBins]; }
 private:
  Int_t fVarId = -1;        ///< The external Id for the variable in the data bank
  Int_t fNBins = 0;        ///< The number of bins for the variable when shown in a histogram
  Int_t fNBinsPlusOne = 0; ///< the number of bins plus one. Needed for object persistence
  /// Bin edges array for the variable when shown in a histogram
  Double_t *fBins = nullptr;       //[fNBinsPlusOne]
  std::string fLabel;        ///< Label to use in an axis that shows the variable

/// \cond CLASSIMP
 ClassDef(EventClassVariable, 1);
/// \endcond
};
}
#endif /* QNCORRECTIONS_EVENTCLASSVAR_H */
