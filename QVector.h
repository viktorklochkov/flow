//
// Created by Lukas Kreis on 14.11.17.
//

#ifndef FLOW_QVECTOR_H
#define FLOW_QVECTOR_H
#include <iostream>
#include <array>
#include <functional>
#include <complex>
#include "QnCorrectionsQnVector.h"
#include "Rtypes.h"

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
inline QVec operator/(QVec a, float s) { return {a.x / s, a.y / s}; }
inline QVec operator*(QVec a, float s) { return {a.x * s, a.y * s}; }
inline float norm(QVec a) { return sqrt(a.x * a.x + a.y * a.y); }

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
      q_(q) {}

  QVector(Normalization norm, const QnCorrectionsQnVector &vector) :
      norm_(norm),
      n_(vector.GetN()),
      sum_weights_(vector.GetSumOfWeights()) {
    for (int i = 0; i < 4; i++) {
      q_[i] = isnan(vector.Qx(i)) || isnan(vector.Qy(i)) ? QVec(0, 0) : QVec(vector.Qx(i), vector.Qy(i));
    }
  }

  float x(int i) const { return q_[i].x; }
  float n() const { return n_; }
  float y(int i) const { return q_[i].y; }

  friend QVector operator+(QVector a, QVector b);

  void Add(const QVector &a) {
    *this + a;
  }

  QVector Normal(const Normalization norm) const {
    QVector c(*this);
    if (norm_ != Normalization::NOCALIB) {
      c = c.DeNormal();
    }
    switch (norm) {
      case (Normalization::NOCALIB): {
        break;
      }
      case (Normalization::QOVERM): {
        auto add = [this](const QVec q) {
          return q / sum_weights_;
        };
        std::transform(c.q_.begin(), c.q_.end(), c.q_.begin(), add);
        break;
      }
      case (Normalization::QOVERSQRTM): {
        auto add = [this](const QVec q) {
          return q / std::sqrt(sum_weights_);
        };
        std::transform(c.q_.begin(), c.q_.end(), c.q_.begin(), add);
        break;
      }
      case (Normalization::QOVERNORMQ): {
        auto add = [](const QVec q) {
          if (Qn::norm(q) != 0) {
            return q / Qn::norm(q);
          }
          return q;
        };
        std::transform(c.q_.begin(), c.q_.end(), c.q_.begin(), add);
        break;
      }
    }
    c.norm_ = norm;
    return c;
  }

  QVector DeNormal() const {
    QVector c(*this);
    switch (norm_) {
      case (Normalization::NOCALIB): {
        break;
      }
      case (Normalization::QOVERM): {
        std::transform(q_.begin(), q_.end(), c.q_.begin(), [this](const QVec q) { return q * sum_weights_; });
        break;
      }
      case (Normalization::QOVERSQRTM): {
        std::transform(q_.begin(), q_.end(), c.q_.begin(),
                       [this](const QVec q) { return q * std::sqrt(sum_weights_); });
        break;
      }
      case (Normalization::QOVERNORMQ): {
        std::transform(q_.begin(), q_.end(), c.q_.begin(), [](const QVec q) { return q * Qn::norm(q); });
        break;
      }
    }
    c.norm_ = Normalization::NOCALIB;
    return c;
  }

 private:
  Normalization norm_ = Normalization::NOCALIB;
  int n_ = 0;
  float sum_weights_ = 0;
  std::array<QVec, 4> q_;
  /// \cond CLASSIMP
 ClassDef(QVector, 2);
  /// \endcond
};

inline std::vector<float> Mult(std::vector<float> temp, QVector vec, std::vector<int> harmonics) {
  std::vector<float> result;
  for (auto &value : temp) {
    result.push_back(value * vec.x(2));
    result.push_back(value * vec.y(2));
  }
  return result;
}

inline std::vector<float> Multiply(std::vector<QVector> vectors, std::vector<int> harmonics) {
  std::vector<float> result;
  // case for iteration = 0
  result.push_back(vectors[0].x(harmonics[0]) * vectors[1].x(harmonics[1]));
  result.push_back(vectors[0].x(harmonics[0]) * vectors[1].y(harmonics[1]));
  result.push_back(vectors[0].y(harmonics[0]) * vectors[1].x(harmonics[1]));
  result.push_back(vectors[0].y(harmonics[0]) * vectors[1].y(harmonics[1]));
  for (auto vec = vectors.begin() + 2; vec < vectors.end(); ++vec) {
    result = Mult(result, *vec, harmonics);
  }
  return result;
}

}

////

#endif //FLOW_QVECTOR_H
