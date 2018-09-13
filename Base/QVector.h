//
// Created by Lukas Kreis on 14.11.17.
//

#ifndef FLOW_QVECTOR_H
#define FLOW_QVECTOR_H
#include <iostream>
#include <array>
#include <functional>
#include <complex>

#include "Rtypes.h"

#include "QnCorrections/QnCorrectionsQnVector.h"

namespace Qn {

struct QVec {
  QVec() = default;
  QVec(float x, float y) : x(x), y(y) {}
  float x{0};
  float y{0};
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

  enum class Normalization : short {
    NOCALIB,
    QOVERSQRTM,
    QOVERM,
    QOVERNORMQ
  };

  QVector() = default;
  virtual ~QVector() = default;

  QVector(Normalization norm, int n, float sum, std::array<QVec, 4> q) :
      norm_(norm),
      n_(n),
      sum_weights_(sum),
      q_(q) {
  }

  QVector(Normalization norm, const QnCorrectionsQnVector *vector);

  inline float x(const int i) const { return q_[i].x; }
  inline float n() const { return n_; }
  inline float y(const int i) const { return q_[i].y; }
  inline float mag(const int i) const { return sqrt(q_[i].x*q_[i].x + q_[i].y*q_[i].y); }
  inline float sumweights() const { return sum_weights_; }
  inline Normalization GetNorm() const { return norm_; }

  friend QVector operator+(QVector a, QVector b);

  inline void Add(const QVector &a) { *this + a; }
  QVector Normal(Normalization norm) const;
  QVector DeNormal() const;

 private:
  Normalization norm_ = Normalization::NOCALIB; ///< normalization method
  int n_ = 0;                                   ///< number of data vectors contributing to the q vector
  float sum_weights_ = 0.0;                     ///< sum of weights
  std::array<QVec, 4> q_;                       ///< array of qvectors for the different harmonics
  /// \cond CLASSIMP
 ClassDef(QVector, 6);
  /// \endcond
};

}

////

#endif //FLOW_QVECTOR_H
