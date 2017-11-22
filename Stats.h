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

inline float Sigma(const float mean, const float sum2, const int n) {
  float variance = TMath::Abs(sum2 / (float) n - mean * mean);
  return sqrt(variance) / sqrt((float) n);
}

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
