#ifndef QNCORRECTIONS_QNVECTORSBUILD_H
#define QNCORRECTIONS_QNVECTORSBUILD_H

/***************************************************************************
 * Package:       FlowVectorCorrections                                    *
 * Authors:       Jaap Onderwaater, GSI, jacobus.onderwaater@cern.ch       *
 *                Ilya Selyuzhenkov, GSI, ilya.selyuzhenkov@gmail.com      *
 *                Víctor González, UCM, victor.gonzalez@cern.ch            *
 *                Contributors are mentioned in the code where appropriate.*
 * Development:   2012-2016                                                *
 * See cxx source for GPL licence et. al.                                  *
 ***************************************************************************/

/// \file CorrectionQnVectorBuild.h
/// \brief Class that models and encapsulates a Q vector set while building it within the Q vector correction framework

#include "CorrectionQnVector.h"
namespace Qn {
/// \class CorrectionQnVectorBuild
/// \brief Class that models and encapsulates a Q vector set while building it
///
/// When the Q vector is being built it needs extra support.
/// This class provides such extra support.
///
/// \author Jaap Onderwaater <jacobus.onderwaater@cern.ch>, GSI
/// \author Ilya Selyuzhenkov <ilya.selyuzhenkov@gmail.com>, GSI
/// \author Víctor González <victor.gonzalez@cern.ch>, UCM
/// \date Feb 02, 2016
class CorrectionQnVectorBuild : public CorrectionQnVector {

 public:
  CorrectionQnVectorBuild();
  CorrectionQnVectorBuild(const char *name, Int_t nNoOfHarmonics, Int_t *harmonicMap = NULL);
  CorrectionQnVectorBuild(const CorrectionQnVector &Qn);
  CorrectionQnVectorBuild(const CorrectionQnVectorBuild &Qn);
  virtual ~CorrectionQnVectorBuild();

  virtual void SetQx(Int_t harmonic, Float_t qx);
  virtual void SetQy(Int_t harmonic, Float_t qy);

  void Set(CorrectionQnVectorBuild *Qn);

  void Add(CorrectionQnVectorBuild *qvec);
  void Add(Double_t phi, Double_t weight = 1.0);

  /// Check the quality of the constructed Qn vector
  /// Current criteria is number of contributors should be at least one.
  /// If so happen, sets the good quality flag.
  void CheckQuality() { fGoodQuality = ((0 < fN) ? kTRUE : kFALSE); }
  void Normalize(Normalization method);

  void NormalizeQoverM();
  void NormalizeQoverSquareRootOfM();

  virtual void Reset();

  virtual void Print(Option_t *) const;

 private:
  /// Assignment operator
  ///
  /// Default implementation to protect against its accidental use.
  /// It will give a compilation error. Don't make it different from
  /// private!
  /// \param Qn the Q vector to assign
  CorrectionQnVectorBuild &operator=(const CorrectionQnVectorBuild &Qn);

/// \cond CLASSIMP
 ClassDef(CorrectionQnVectorBuild, 2);
/// \endcond
};

/// Adds a contribution to the build Q vector
/// A check for weight significant value is made. Not passing it ignores the contribution.
/// The process of incorporating contributions takes into account the harmonic multiplier
/// \param phi azimuthal angle contribution
/// \param weight the weight of the contribution
inline void CorrectionQnVectorBuild::Add(Double_t phi, Double_t weight) {

  if (weight < fMinimumSignificantValue) return;
  for (Int_t h = 1; h < fHighestHarmonic + 1; h++) {
    if ((fHarmonicMask & harmonicNumberMask[h])==harmonicNumberMask[h]) {
      fQnX[h] += (weight*std::cos(h*fHarmonicMultiplier*phi));
      fQnY[h] += (weight*std::sin(h*fHarmonicMultiplier*phi));
    }
  }
  fSumW += weight;
  fN += 1;
}

/// Calibrates the Q vector according to the method passed
/// \param method the method of calibration
inline void CorrectionQnVectorBuild::Normalize(Normalization method) {
  switch (method) {
    case Normalization::NONE:break;
    case Normalization::SQRT_M:NormalizeQoverSquareRootOfM();
      break;
    case Normalization::M:NormalizeQoverM();
      break;
    case Normalization::MAGNITUDE:CorrectionQnVector::Normalize();
      break;
  }
}
}

#endif /* QNCORRECTIONS_QNVECTORSBUILD_H */
