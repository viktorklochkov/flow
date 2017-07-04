//
// Created by Lukas Kreis on 30.06.17.
//

#ifndef FLOW_QNVECTOR_H
#define FLOW_QNVECTOR_H

#include <complex>
#include <array>

class QnVector {
 public:
  QnVector() = default;
  ~QnVector() = default;
 private:
  std::array<float,8> qx;
  std::array<float,8> qy;
  int multiplicity;

};

#endif //FLOW_QNVECTOR_H
