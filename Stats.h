//
// Created by Lukas Kreis on 15.11.17.
//

#ifndef FLOW_STATISTICS_H
#define FLOW_STATISTICS_H

#include <iterator>
#include <algorithm>
#include <utility>
#include <TMathBase.h>
#include <TMath.h>
#include <array>

namespace Qn {

namespace Stats {

inline std::tuple<float,float,float,int> Mean(const std::vector<float> &vector) {
  float sum = 0;
  float sum2 = 0;
  int entries = 0;
  std::for_each(vector.begin(), vector.end(), [&](float q) {
    sum += q;
    sum2 += q*q;
    ++entries;
  });
  if (entries == 0) return std::make_tuple(0.0,0.0,0.0,0);
  auto mean = sum / entries;
  return std::make_tuple(mean,sum,sum2,entries);
}

inline float Sigma(const float mean, const float sum2, const int n) {
  float variance = TMath::Abs(sum2 / (float) n - mean * mean);
  return sqrt(variance) / sqrt((float) n);
}


inline float Error(const std::vector<float> &vector) {
  auto stats = Mean(vector);
  float mean = std::get<0>(stats);
  float sum2 = std::get<2>(stats);
  int n = std::get<3>(stats);
  return Sigma(mean,sum2,n);
}

inline std::array<float,2> MeanAndError(const std::vector<float> &vector) {
  return {{std::get<0>(Mean(vector)),Error(vector)}};
};

}

class Statistics {
 public:
  Statistics() = default;
  inline void Update(float value) {
    mean_ = (mean_ * entries_ + value) / (entries_+1);
    sum_ = sum_ + value;
    sum2_ = sum2_ + value*value;
    error_ = Qn::Stats::Sigma(mean_,sum2_,entries_);
    ++entries_;
  };
  inline float Mean() const {return mean_;}
  inline float Sum() const {return sum_;}
  inline float Sum2() const {return sum2_;}
  inline float Error() const {return error_;}
  inline float Entries() const {return entries_;}
 private:
  float mean_ = 0;
  float sum_ = 0;
  float sum2_ = 0;
  int entries_ = 0;
  float error_ = 0;
};
}


#endif //FLOW_STATISTICS_H
