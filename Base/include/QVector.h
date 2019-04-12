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

#include <vector>
#include <bitset>
#include <cmath>

#include "Rtypes.h"

#include "CorrectionQnVector.h"

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
inline float norm(QVec a) { return sqrt(a.x*a.x + a.y*a.y); }

class QVector {
 public:
  static constexpr int kMaxNHarmonics = 8;
  using Normalization = CorrectionQnVector::Normalization;

  QVector() = default;
  virtual ~QVector() = default;

  QVector(Normalization norm, int n, float sum, std::vector<QVec> q) :
      norm_(norm),
      n_(n),
      sum_weights_(sum),
      q_(q) {
    for (unsigned int i = 0; i < q.size(); ++i) { bits_.set(i); }
  }

  void CopyHarmonics(const QVector &qvec) {
    this->bits_ = qvec.bits_;
    this->q_.resize(qvec.q_.size());
  }

  QVector(Normalization norm, const CorrectionQnVector *vector, std::bitset<kMaxNHarmonics> bits);

  inline float x(const unsigned int i) const {
    if (bits_.test(i)) {
      return q_[std::bitset<kMaxNHarmonics>(bits_ & std::bitset<kMaxNHarmonics>((1 << (i + 1)) - 1)).count() - 1].x;
    } else {
      throw std::out_of_range("harmonic not in range.");
    }
  }

  inline float y(const unsigned int i) const {
    if (bits_.test(i)) {
      return q_[std::bitset<kMaxNHarmonics>(bits_ & std::bitset<kMaxNHarmonics>((1 << (i + 1)) - 1)).count() - 1].y;
    } else {
      throw std::out_of_range("harmonic not in range.");
    }
  }

  inline float mag(const unsigned int i) const { return sqrt(x(i)*x(i) + y(i)*y(i)); }
  inline float sumweights() const { return sum_weights_; }
  inline float n() const { return n_; }
  inline Normalization GetNorm() const { return norm_; }
  friend QVector operator+(QVector a, QVector b);
  inline void Add(const QVector &a) { *this + a; }
  QVector Normal(Normalization norm) const;
  QVector DeNormal() const;

  Normalization norm_ = Normalization::NONE; ///< normalization method
  int n_ = 0;                                ///< number of data vectors contributing to the q vector
  float sum_weights_ = 0.0;                  ///< sum of weights
  std::bitset<kMaxNHarmonics> bits_{};       ///< Bitset for keeping track of the harmonics
  std::vector<QVec> q_;                      ///< array of qvectors for the different harmonics

  /// \cond CLASSIMP
 ClassDef(QVector, 8);
  /// \endcond
};

namespace detail {
template<class T>
T &FUN(T &t) noexcept { return t; }
template<class T>
void FUN(T &&) = delete;
}

/**
 * @class QVector ptr
 * @brief Wrapper for QVector used in the correlation step.
 */
class QVectorPtr {
  using Normalization = QVector::Normalization;
 public:
  QVectorPtr() = default;
  // construct/copy/destroy
  QVectorPtr(const QVector &ref) noexcept : qvector_(&ref) {}
  QVectorPtr(const QVectorPtr &) noexcept = default;
  // assignment
  QVectorPtr &operator=(const QVectorPtr &x) noexcept = default;

  inline float x(const unsigned int i) const { return qvector_->x(i); }
  inline float y(const unsigned int i) const { return qvector_->y(i); }
  inline float mag(const unsigned int i) const { return qvector_->mag(i); }
  inline float sumweights() const { return qvector_->sumweights(); }
  inline float n() const { return qvector_->n(); }
  inline Normalization GetNorm() const { return qvector_->GetNorm(); }
  inline QVector Normal(Normalization norm) const { return qvector_->Normal(norm); }
  inline QVector DeNormal() const { return qvector_->DeNormal(); }
 private:
  const QVector *qvector_ = nullptr;
};

}
#endif //FLOW_QVECTOR_H
