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
  inline const float Qx(int n) const {return qx[n];}
  inline const float Qy(int n) const {return qy[n];}
  inline const float Multiplicity() const {return multiplicity;}
 private:
  std::array<float,8> qx;
  std::array<float,8> qy;
  int multiplicity;

};

#endif //FLOW_QNVECTOR_H
