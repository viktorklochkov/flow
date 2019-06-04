#ifndef QNCORRECTIONS_DATAVECTORS_H
#define QNCORRECTIONS_DATAVECTORS_H

/***************************************************************************
 * Package:       FlowVectorCorrections                                    *
 * Authors:       Jaap Onderwaater, GSI, jacobus.onderwaater@cern.ch       *
 *                Ilya Selyuzhenkov, GSI, ilya.selyuzhenkov@gmail.com      *
 *                Víctor González, UCM, victor.gonzalez@cern.ch            *
 *                Contributors are mentioned in the code where appropriate.*
 * Development:   2012-2016                                                *
 * See cxx source for GPL licence et. al.                                  *
 ***************************************************************************/

/// \file QnCorrectionsDataVector.h
/// \brief Class that model data vectors from detectors within the Q vector correction framework
///
/// As it is today, a data vector is just an azimuthal angle. As
/// it is intended to be incorporated into an array of clones the
/// constructor is really simple and instead the setters are used
/// to initialize its members
///

#include <Rtypes.h>
namespace Qn {
/// \class QnCorrectionsDataVector
/// \brief Class that models and encapsulates a data vector
///
/// \author Jaap Onderwaater <jacobus.onderwaater@cern.ch>, GSI
/// \author Ilya Selyuzhenkov <ilya.selyuzhenkov@gmail.com>, GSI
/// \author Víctor González <victor.gonzalez@cern.ch>, UCM
/// \date Feb 01, 2016

class CorrectionDataVector {
 public:
  CorrectionDataVector() = default;
  CorrectionDataVector(Int_t id, Float_t phi, Float_t weight) :
      fId(id),
      fPhi(phi),
      fWeight(weight),
      fEqualizedWeight(weight) {}
  ~CorrectionDataVector() = default;

  /// Sets the data vector azimuthal angle
  /// \param phi the azimuthal angle
  inline void SetPhi(const Float_t phi) { fPhi = phi; }
  /// Sets the channel id associated with the data vector
  /// \param id channel id
  inline void SetId(const Int_t id) { fId = id; }
  /// Sets the raw weight
  /// \param weight raw weight from the detector channel
  inline void SetWeight(const Float_t weight) { fWeight = weight; }
  /// Sets the equalized weight
  /// \param weight equalized weight after channel equalization
  inline void SetEqualizedWeight(const Float_t weight) { fEqualizedWeight = weight; }
  /// Gets the channel id associated with the data vector
  /// \return the channel id
  constexpr Int_t GetId() const { return fId; }
  /// Gets the azimuthal angle for the data vector
  /// \return phi
  constexpr Float_t Phi() const { return fPhi; }
  /// Gets the weight for the data vector
  /// \return defaults to 1.0
  constexpr Float_t Weight() const { return fWeight; }
  /// Gets the equalized weight for the data vector
  /// \return defaults to weights
  constexpr Float_t EqualizedWeight() const { return fEqualizedWeight; }

 protected:
  Int_t   fId;                  //!<! the id associated with the data vector
  Float_t fPhi;                 //!<! the azimuthal angle of the data vector
  Float_t fWeight;              //!<! raw weight assigned to the data vector
  Float_t fEqualizedWeight;     //!<! eq weight assigned to the data vector

  static constexpr Float_t fMinimumSignificantValue = 1.e-6;  ///< the minimum value that will be considered
};
}
#endif /* QNCORRECTIONS_DATAVECTORS_H */
