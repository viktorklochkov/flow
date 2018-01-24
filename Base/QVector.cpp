
// Created by Lukas Kreis on 14.11.17.
//

#include "QVector.h"
namespace Qn {

QVector operator+(const QVector a, const QVector b) {
  QVector at = a.DeNormal();
  QVector bt = b.DeNormal();
  QVector c;
  for (int i = 0; i < 4; ++i) {
    c.q_[i] = a.q_[i] + b.q_[i];
  }
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
      for (int i = 0; i < 4; ++i) {
        c.q_[i] = add(c.q_[i]);
      }
      break;
    }
    case (Normalization::QOVERSQRTM): {
      auto add = [this](const QVec q) {
        return q/std::sqrt(sum_weights_);
      };
      for (int i = 0; i < 4; ++i) {
        c.q_[i] = add(c.q_[i]);
      }
      break;
    }
    case (Normalization::QOVERNORMQ): {
      auto add = [](const QVec q) {
        if (Qn::norm(q)!=0) {
          return q/Qn::norm(q);
        }
        return q;
      };
      for (int i = 0; i < 4; ++i) {
        c.q_[i] = add(c.q_[i]);
      }
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
      for (int i = 0; i < 4; ++i) {
        c.q_[i] = q_[i]*sum_weights_;
      }
      break;
    }
    case (Normalization::QOVERSQRTM): {
      for (int i = 0; i < 4; ++i) {
        c.q_[i] = q_[i]*std::sqrt(sum_weights_);
      }
      break;
    }
    case (Normalization::QOVERNORMQ): {
      for (int i = 0; i < 4; ++i) {
        c.q_[i] = q_[i]*Qn::norm(q_[i]);
      }
      break;
    }
  }
  c.norm_ = Normalization::NOCALIB;
  return c;
}

}