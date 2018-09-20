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

#include "QVector.h"
namespace Qn {
/**
 * Constructor
 * @param norm normalisation method
 * @param vector QnCorrectionsQnVector to construct the QVector from. It is used internally by the framework but not exposed to the user.
 */
QVector::QVector(QVector::Normalization norm, const QnCorrectionsQnVector *vector,std::bitset<8> bits) :
    norm_(norm),
    bits_(bits) {
  q_.resize(static_cast<size_t>(bits.count()));
  if (vector) {
    n_ = vector->GetN();
    sum_weights_ = vector->GetSumOfWeights();
    int *harmonicsmap = new int[8];
    vector->GetHarmonicsMap(harmonicsmap);
    for (int i = 0; i < bits.count(); i++) {
      int iharmonic = harmonicsmap[i];
      q_[i] = QVec(vector->Qx(iharmonic), vector->Qy(iharmonic));
    }
    delete[] harmonicsmap;
  } else {
    n_ = 0;
    sum_weights_ = 0;
  }
}
/**
 * Adds two Q vectors taking into account for the normalizations
 * @param a Q vector
 * @param b Q vector
 * @return unnormalized sum of the two QVectors
 */
QVector operator+(const QVector a, const QVector b) {
  QVector at = a.DeNormal();
  QVector bt = b.DeNormal();
  QVector c;
  std::transform(at.q_.begin(),
                 at.q_.end(),
                 bt.q_.begin(),
                 c.q_.begin(),
                 [](const QVec qa, const QVec qb) {
                   QVec ta = {0.0, 0.0};
                   QVec tb = {0.0, 0.0};
                   if (!(isnan(qa.x) || isnan(qa.y))) ta = qa;
                   if (!(isnan(qb.x) || isnan(qb.y))) tb = qb;
                   return ta + tb;
                 });
  c.n_ = at.n_ + bt.n_;
  c.sum_weights_ = at.sum_weights_ + bt.sum_weights_;
  return c;
}
/**
 * Normalize the Q vector with a given normalization method.
 * @param norm normalization method
 * @return normalized Q vector
 */
QVector QVector::Normal(const QVector::Normalization norm) const {
  QVector c(*this);
  if (norm_!=Normalization::NOCALIB) {
    c = c.DeNormal();
  }
  switch (norm) {
    case (Normalization::NOCALIB): {
      break;
    }
    case (Normalization::QOVERM): {
      auto add = [this](const QVec q) {
        if (sum_weights_!=0) return q/sum_weights_;
        return q;
      };
      std::transform(c.q_.begin(), c.q_.end(), c.q_.begin(), add);
      break;
    }
    case (Normalization::QOVERSQRTM): {
      auto add = [this](const QVec q) {
        return q/std::sqrt(sum_weights_);
      };
      std::transform(c.q_.begin(), c.q_.end(), c.q_.begin(), add);
      break;
    }
    case (Normalization::QOVERNORMQ): {
      auto add = [](const QVec q) {
        if (Qn::norm(q)!=0) {
          return q/Qn::norm(q);
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
/**
 * Remove normalization of Q vector
 * @return unnormalized Q vector
 */
QVector QVector::DeNormal() const {
  QVector c(*this);
  switch (norm_) {
    case (Normalization::NOCALIB): {
      break;
    }
    case (Normalization::QOVERM): {
      std::transform(q_.begin(), q_.end(), c.q_.begin(), [this](const QVec q) { return q*sum_weights_; });
      break;
    }
    case (Normalization::QOVERSQRTM): {
      std::transform(q_.begin(), q_.end(), c.q_.begin(),
                     [this](const QVec q) { return q*std::sqrt(sum_weights_); });
      break;
    }
    case (Normalization::QOVERNORMQ): {
      std::transform(q_.begin(), q_.end(), c.q_.begin(), [](const QVec q) { return q*Qn::norm(q); });
      break;
    }
  }
  c.norm_ = Normalization::NOCALIB;
  return c;
}

}