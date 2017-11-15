
// Created by Lukas Kreis on 14.11.17.
//

#include "QVector.h"
namespace Qn {

QVector operator+(const QVector a, const QVector b) {
  QVector at(a);
  QVector bt(b);
  QVector c;
  at.DeNormal();
  bt.DeNormal();
  auto add = [](const std::complex<float> qa, const std::complex<float> qb) {
    return qa + qb;
  };
  std::transform(a.q_.begin(), a.q_.end(), b.q_.begin(), c.q_.begin(), add);
  c.n_ = a.n_ + b.n_;
  c.sum_weights_ = a.sum_weights_ + b.sum_weights_;
  return c;
}
float Qn::operator*(QVector a, QVector b) {
  return a.q_*;
}


//QVector operator+(const QVector a, const QVector b) {
//  QVector c;
//  switch (a.norm_) {
//    case (QVector::Normalization::NOCALIB): {
//      auto add = [](const std::complex<float> qa, const std::complex<float> qb) {
//        return qa + qb;
//      };
//      std::transform(a.q_.begin(), a.q_.end(), b.q_.begin(), c.q_.begin(), add);
//      c.n_ = a.n_ + b.n_;
//      c.sum_weights_ = a.sum_weights_ + b.sum_weights_;
//      break;
//    }
//    case (QVector::Normalization::QOVERSQRTM): {
//      auto add = [a, b](const std::complex<float> qa, const std::complex<float> qb) {
//        return (qa * std::sqrt((float) a.n_) + qb * std::sqrt((float) b.n_)) / std::sqrt((float) a.n_ + (float) b.n_);
//      };
//      std::transform(a.q_.begin(), a.q_.end(), b.q_.begin(), c.q_.begin(), add);
//      c.n_ = a.n_ + b.n_;
//      c.sum_weights_ = a.sum_weights_ + b.sum_weights_;
//      break;
//    }
//    case (QVector::Normalization::QOVERM): {
//      auto add = [a, b](const std::complex<float> qa, const std::complex<float> qb) {
//        return ((qa * (float) a.n_) + (qb * (float) b.n_)) / (float) (a.n_ + b.n_);
//      };
//      std::transform(a.q_.begin(), a.q_.end(), b.q_.begin(), c.q_.begin(), add);
//      c.n_ = a.n_ + b.n_;
//      c.sum_weights_ = a.sum_weights_ + b.sum_weights_;
//      break;
//    }
//    case (QVector::Normalization::QOVERNORMQ): {
//      auto add = [a, b](const std::complex<float> qa, const std::complex<float> qb) {
//        float absa = std::abs(qa);
//        float absb = std::abs(qb);
//        auto sum = (qa * absa + qb * absb);
//        return sum / std::abs(sum);
//      };
//      std::transform(a.q_.begin(), a.q_.end(), b.q_.begin(), c.q_.begin(), add);
//      c.n_ = a.n_ + b.n_;
//      c.sum_weights_ = a.sum_weights_ + b.sum_weights_;
//      break;
//    }
//  }
//  return c;
//}
}