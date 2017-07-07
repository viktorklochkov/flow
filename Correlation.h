//
// Created by Lukas Kreis on 06.07.17.
//

#ifndef FLOW_CORRELATION_H
#define FLOW_CORRELATION_H

#include <array>

namespace Qn {

class Correlation {
 public:
  Correlation() = default;
  Correlation(std::array<float, 2> corr) : correlation_(corr) {}
  ~Correlation() = default;
  Correlation operator*(Correlation x) const {
    std::array<float, 2> tmp = {correlation_[0] * x.correlation_[0], correlation_[1] * x.correlation_[1]};
    return Correlation(tmp);
  }
 private:
  std::array<float, 2> correlation_;

};
}

#endif //FLOW_CORRELATION_H
