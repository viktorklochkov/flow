//
// Created by Lukas Kreis on 15.11.17.
//

#ifndef FLOW_STATISTICS_H
#define FLOW_STATISTICS_H

#include <iterator>
#include <algorithm>
#include <TMathBase.h>
#include <TMath.h>
namespace Qn {
namespace Stats {

inline std::tuple<float,float,float,int> Mean(std::vector<float> vector) {
  float sum = 0;
  float sum2 = 0;
  int entries = 0;
  std::for_each(vector.begin(), vector.end(), [&](float q) {
    sum += q;
    sum2 += q*q;
    ++entries;
  });
  float mean = sum / entries;
  return std::make_tuple(mean,sum,sum2,entries);

}

inline float Sigma(float mean, float sum2, int n) {
  float variance = TMath::Abs(sum2 / (float) n - mean * mean);
  return TMath::Sqrt(variance) / sqrt((float) n);
}


inline float Error(std::vector<float> vector) {
  auto stats = Mean(vector);
  float mean = std::get<0>(stats);
  float sum2 = std::get<2>(stats);
  int n = std::get<3>(stats);
  return Sigma(mean,sum2,n);

}
}
}


#endif //FLOW_STATISTICS_H
