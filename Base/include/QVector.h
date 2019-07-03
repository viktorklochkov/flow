// Flow Vector Correction Framework
//
// Copyright (C) 2018  Lukas Kreis, Ilya Selyuzhenkov
// Contact: l.kreis@gsi.de; ilya.selyuzhenkov@gmail.com
// For a full list of contributors please see docs/Credits
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef FLOW_QVECTOR_H
#define FLOW_QVECTOR_H

#include <array>
#include <bitset>
#include <cmath>

#include "Rtypes.h"

namespace Qn {

struct QVec {
  QVec() = default;
  QVec(float x, float y) : x(x), y(y) {}
  float x{0.};
  float y{0.};
};

inline QVec operator+(QVec a, QVec b) { return {a.x + b.x, a.y + b.y}; }
inline QVec operator-(QVec a, QVec b) { return {a.x - b.x, a.y - b.y}; }
inline QVec operator/(QVec a, float s) { return {a.x/s, a.y/s}; }
inline QVec operator*(QVec a, float s) { return {a.x*s, a.y*s}; }
inline float norm(QVec a) { return std::sqrt(a.x*a.x + a.y*a.y); }

class QVector {
 public:
  static constexpr int kmaxharmonics = 8;
  static constexpr float kminimumweight = 1e-6;
  enum CorrectionStep {
    RAW,
    PLAIN,
    RECENTERED,
    TWIST,
    RESCALED,
    ALIGNED
  };
  enum class Normalization {
    NONE,       ///< \f$ \mbox{Q'} = \mbox{Q}\f$
    SQRT_M,     ///< \f$ \mbox{Q'} = \frac{\mbox{Q}}{\sqrt{\mbox{M}}} \f$
    M,          ///< \f$ \mbox{Q'} = \frac{\mbox{Q}}{\mbox{M}} \f$
    MAGNITUDE   ///< \f$ \mbox{Q'} = \frac{\mbox{Q}}{|\mbox{Q}|} \f$
  };

  QVector() = default;
  virtual ~QVector() = default;

  QVector(std::bitset<kmaxharmonics> bits, CorrectionStep step) :
      correction_step_(step),
      bits_(bits) {
    q_.resize(bits.count());
    maxharmonic_ = highestharmonic();
  }

  QVector(const QVector &ref) = default;

  void CopyHarmonics(const QVector &qvec) {
    this->bits_ = qvec.bits_;
    this->q_.resize(qvec.q_.size());
  }

  void Reset() {
    n_ = 0;
    sum_weights_ = 0.;
    quality_ = false;
  }
  void GetHarmonicsMap(Int_t *store) const {
    unsigned int iharmonics = 0;
    for (unsigned char h = 1; h <= maxharmonic_; h++) {
      if (bits_.test(h - 1)) {
        store[iharmonics] = h;
        iharmonics++;
      }
    }
  }
  std::bitset<kmaxharmonics> GetHarmonics() const { return bits_; }
  unsigned int GetNoOfHarmonics() const { return bits_.count(); }
  void SetHarmonicMultiplier(unsigned char mult) { harmonic_multiplier_ = mult; }
  unsigned char GetHarmonicMultiplier() const { return harmonic_multiplier_; }
  void SetGood(bool quality) { quality_ = quality; }
  bool IsGoodQuality() const { return quality_; }
  void CheckQuality() { quality_ = 0 < n_; }
  void ActivateHarmonic(unsigned int i) {
    bits_.set(i - 1);
    q_.resize(static_cast<size_t>(bits_.count()));
  }
  void SetHarmonics(std::bitset<kmaxharmonics> bits) {
    bits_ = bits;
    q_.resize(static_cast<size_t>(bits.count()));
  }
  void SetNormalization(Normalization norm) { norm_ = norm; }
  void SetCorrectionStep(CorrectionStep step) { correction_step_ = step; }
  CorrectionStep GetCorrectionStep() const { return correction_step_; }
  inline float Qx(const unsigned int i) const {
    if (bits_.test(i - 1)) {
      return q_[std::bitset<kmaxharmonics>(bits_ & std::bitset<kmaxharmonics>((1UL << (i + 1)) - 1)).count() - 1].x;
    } else {
      throw std::out_of_range("harmonic not in range.");
    }
  }
  inline float Qy(const unsigned int i) const {
    if (bits_.test(i - 1)) {
      return q_[std::bitset<kmaxharmonics>(bits_ & std::bitset<kmaxharmonics>((1UL << (i + 1)) - 1)).count() - 1].y;
    } else {
      throw std::out_of_range("harmonic not in range.");
    }
  }
  inline void SetQx(const unsigned int i, double x) {
    q_[std::bitset<kmaxharmonics>(bits_ & std::bitset<kmaxharmonics>((1UL << (i + 1)) - 1)).count() - 1].x = x;
  }
  inline void SetQy(const unsigned int i, double y) {
    q_[std::bitset<kmaxharmonics>(bits_ & std::bitset<kmaxharmonics>((1UL << (i + 1)) - 1)).count() - 1].y = y;
  }
  void SetCurrentEvent(const QVector &qvec) {
    norm_ = qvec.norm_;
    n_ = qvec.n_;
    sum_weights_ = qvec.sum_weights_;
  }

  int GetFirstHarmonic() const {
    for (Int_t h = 1; h < maxharmonic_ + 1; h++) {
      if (bits_.test(h - 1)) {
        return h;
      }
    }
    return -1;
  }

  int GetNextHarmonic(unsigned char harmonic) const {
    for (unsigned char h = harmonic + 1; h < maxharmonic_ + 1; h++) {
      if (bits_.test(h - 1)) {
        return h;
      }
    }
    return -1;
  }

  inline float mag(const unsigned int i) const { return sqrt(Qx(i)*Qx(i) + Qy(i)*Qy(i)); }
  inline float sumweights() const { return sum_weights_; }
  inline float n() const { return n_; }
  inline Normalization GetNorm() const { return norm_; }
  friend QVector operator+(QVector a, QVector b);
  inline void Add(const QVector &a) { *this + a; }
  QVector Normal(Normalization norm) const;
  QVector DeNormal() const;

  inline void Add(double phi, double weight) {
    if (weight < kminimumweight) return;
    unsigned int pos = 0;
    for (unsigned int h = 1; h <= maxharmonic_; ++h) {
      if (bits_.test(h - 1)) {
        q_[pos].x += (weight*std::cos(h*harmonic_multiplier_*phi));
        q_[pos].y += (weight*std::sin(h*harmonic_multiplier_*phi));
        ++pos;
      }
    }
    sum_weights_ += weight;
    n_ += 1;
  }

  inline unsigned int highestharmonic() const {
    unsigned char val = bits_.to_ulong();
    if (val==0) return UCHAR_MAX;
    if (val==1) return 0 + 1;
    unsigned char ret = 0;
    while (val > 1) {
      val >>= 1;
      ret++;
    }
    return (ret + 1);
  }

 private:
  Normalization norm_ = Normalization::NONE; ///< normalization method
  unsigned char maxharmonic_ = 0;            ///<
  unsigned char harmonic_multiplier_ = 1;    ///<
  CorrectionStep correction_step_ = CorrectionStep::RAW; ///< correction step defined by enumerator
  int n_ = 0;                                ///< number of data vectors contributing to the q vector
  float sum_weights_ = 0.0;                  ///< sum of weights
  std::bitset<kmaxharmonics> bits_{};        ///< Bitset for keeping track of the harmonics
  std::vector<QVec> q_;                      ///< array of qvectors for the different harmonics
  bool quality_ = false;                     //!<!




  /// \cond CLASSIMP
 ClassDef(QVector, 11);
  /// \endcond
};

/**
 * @class QVector ptr
 * @brief Wrapper for QVector used in the correlation step.
 */
class QVectorPtr {
  using Normalization = QVector::Normalization;
 public:
  QVectorPtr() = default;
  // construct/copy/destroy
  explicit QVectorPtr(const QVector &ref) noexcept : qvector_(&ref) {}
  QVectorPtr(const QVectorPtr &) noexcept = default;
  // assignment
  QVectorPtr &operator=(const QVectorPtr &x) noexcept = default;

  inline float x(const unsigned int i) const { return qvector_->Qx(i); }
  inline float y(const unsigned int i) const { return qvector_->Qy(i); }
  inline float mag(const unsigned int i) const { return qvector_->mag(i); }
  inline float sumweights() const { return qvector_->sumweights(); }
  inline float n() const { return qvector_->n(); }
  inline unsigned int GetCorrectionStep() const { return qvector_->GetCorrectionStep(); }
  inline Normalization GetNorm() const { return qvector_->GetNorm(); }
  inline QVector Normal(Normalization norm) const { return qvector_->Normal(norm); }
  inline QVector DeNormal() const { return qvector_->DeNormal(); }
 private:
  const QVector *qvector_ = nullptr;
};

}
#endif //FLOW_QVECTOR_H
