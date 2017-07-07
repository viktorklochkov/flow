//
// Created by Lukas Kreis on 06.07.17.
//

#ifndef FLOW_CORRELATOR_H
#define FLOW_CORRELATOR_H

#include <iostream>
#include <algorithm>
#include <vector>
#include <tuple>
#include "DataContainer.h"
#include "QnCorrections/QnCorrectionsQnVector.h"
#include "Correlation.h"

namespace Qn {
enum class Coord {
  X,
  Y
};

namespace internal {
template<typename T>
inline Correlation correlate(int n, T last) {
  const auto & element = *(std::get<0>(last).begin() + n);
  int harmonic = std::get<1>(last);
  float qx = element->Qx(harmonic);
  float qy = element->Qy(harmonic);
  std::array<float,2> tmp= {qx,qy};
  Correlation b(tmp);
  return b;
};

template<typename T, typename... Args>
inline Correlation correlate(int n, T first, Args ...args) {
  const auto & element = *(std::get<0>(first).begin() + n);
  int harmonic = std::get<1>(first);
  float qx = element->Qx(harmonic);
  float qy = element->Qy(harmonic);
  std::array<float,2> tmp= {qx,qy};
  Correlation b(tmp);
  return b * correlate(n, args...);
};
}
template<typename T, typename... Args>
void Correlate(DataContainerC &c, const T &x, Args ...args) {
  int n = 0;
  c.ClearData();
  for (const auto &i : std::get<0>(x)) {
    auto result = internal::correlate(n, x, args...);
    std::unique_ptr<Correlation> a(new Correlation(result));
    c.SetElement(a,n);
    n++;
  }
}

void Correlate(std::tuple<const DataContainerQn &, int> qn1, std::tuple<const DataContainerQn &, int> qn2) {
  int n = 0;
  for (const auto &i : std::get<0>(qn1)) {
    const auto &ta = i;
    const auto &tb = std::get<0>(qn2).begin() + n;
    float qx1 = ta->Qx(std::get<1>(qn1));
    float qx2 = ta->Qx(std::get<1>(qn1));
    float qxx = qx1 * qx2;
  }
}

}

#endif //FLOW_CORRELATOR_H
