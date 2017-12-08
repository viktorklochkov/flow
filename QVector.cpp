
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
                 [](const QVec qa, const QVec qb) { return qa + qb; });
  c.n_ = at.n_ + bt.n_;
  c.sum_weights_ = at.sum_weights_ + bt.sum_weights_;
  return c;
}


}