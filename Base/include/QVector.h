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
  friend QVec operator+(QVec a, QVec b);
  friend QVec operator-(QVec a, QVec b);
  friend QVec operator/(QVec a, float s);
  friend QVec operator*(QVec a, float s);
  friend float norm(QVec a);
};

inline QVec operator+(QVec a, QVec b) { return {a.x + b.x, a.y + b.y}; }
inline QVec operator-(QVec a, QVec b) { return {a.x - b.x, a.y - b.y}; }
inline QVec operator/(QVec a, float s) { return {a.x/s, a.y/s}; }
inline QVec operator*(QVec a, float s) { return {a.x*s, a.y*s}; }
inline float norm(QVec a) { return sqrt(a.x*a.x + a.y*a.y); }

class QVector {

 public:

  using Normalization = CorrectionQnVector::Normalization;

//  enum class Normalization : short {
//    NOCALIB,
//    QOVERSQRTM,
//    QOVERM,
//    QOVERNORMQ
//  };

  QVector() = default;
  virtual ~QVector() = default;

  QVector(Normalization norm, int n, float sum, std::vector<QVec> q) :
      norm_(norm),
      n_(n),
      sum_weights_(sum),
      q_(q) {
    for (unsigned int i = 0; i < q.size(); ++i){
      bits_.set(i);
    }
  }

  void CopyHarmonics(const QVector &qvec) {
    this->bits_ = qvec.bits_;
    this->q_.resize(qvec.q_.size());
  }

  double Mean() const {return 0;}

  QVector(Normalization norm, const CorrectionQnVector *vector, std::bitset<8> bits);

  inline float x(const unsigned int i) const {
    if (bits_.test(i)) {
      return q_[std::bitset<8>(bits_ & std::bitset<8>((1 << (i + 1)) - 1)).count() - 1].x;
    }
    else {
      throw std::out_of_range("harmonic not in range.");
    }
  }

  inline float y(const unsigned int i) const {
    if (bits_.test(i)) {
      return q_[std::bitset<8>(bits_ & std::bitset<8>((1 << (i + 1)) - 1)).count() - 1].y;
    }
    else {
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
  int n_ = 0;                                   ///< number of data vectors contributing to the q vector
  float sum_weights_ = 0.0;                     ///< sum of weights
  std::bitset<8> bits_{};                       ///< Bitset for keeping track of the harmonics
  std::vector<QVec> q_;                         ///< array of qvectors for the different harmonics
  /// \cond CLASSIMP
 ClassDef(QVector, 7);
  /// \endcond
};

}
#endif //FLOW_QVECTOR_H
