
// Created by Lukas Kreis on 14.11.17.
//

#include "QVector.h"
namespace Qn {

QVector operator+(const QVector a, const QVector b) {
  QVector at = a.DeNormal();
  QVector bt = b.DeNormal();
  QVector c;
  std::transform(at.q_.begin(),
                 at.q_.end(),
                 bt.q_.begin(),
                 c.q_.begin(),
                 [](const QVec qa, const QVec qb) {
                   QVec ta = {0.0,0.0};
                   QVec tb = {0.0,0.0};
                   if (!(isnan(qa.x)||isnan(qa.y))) ta = qa;
                   if (!(isnan(qb.x)||isnan(qb.y))) tb = qb;
                   return ta + tb;
                 });
  c.n_ = at.n_ + bt.n_;
  c.sum_weights_ = at.sum_weights_ + bt.sum_weights_;
  return c;
}

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