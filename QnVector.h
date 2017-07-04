//
// Created by Lukas Kreis on 30.06.17.
//

#ifndef FLOW_QNVECTOR_H
#define FLOW_QNVECTOR_H

#include <array>

class QnVector {
 public:
  QnVector() = default;
  ~QnVector() = default;
  const float qx(int n) const {return qx[n];}
  const float qy(int n) const {return qy[n];}
 private:
  std::array<float,8> qx;
  std::array<float,8> qy;
  int multiplicity;

};

#endif //FLOW_QNVECTOR_H
