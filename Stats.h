//
// Created by Lukas Kreis on 15.11.17.
//

#ifndef FLOW_STATISTICS_H
#define FLOW_STATISTICS_H

#include <iostream>
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
  Statistics(float mean, float sum, float sum2, float error, int entries) :
      mean_(mean),
      sum_(sum),
      sum2_(sum2),
      entries_(entries),
      error_(error) {}

  inline void Update(float value) {
    mean_ = (mean_ * entries_ + value) / (entries_ + 1);
    sum_ = sum_ + value;
    sum2_ = sum2_ + value * value;
    error_ = Qn::Stats::Sigma(mean_, sum2_, entries_);
    ++entries_;
  };
  inline float Mean() const { return mean_; }
  inline float Sum() const { return sum_; }
  inline float Sum2() const { return sum2_; }
  inline float Error() const { return error_; }
  inline int Entries() const { return entries_; }

  inline Statistics Sqrt() {
    mean_ = std::sqrt(std::abs(mean_));
    return *this;
  }

  friend Qn::Statistics operator+(Qn::Statistics a, Qn::Statistics b);
  friend Qn::Statistics operator*(Qn::Statistics a, Qn::Statistics b);
  friend Qn::Statistics operator/(Qn::Statistics a, Qn::Statistics b);
 private:
  float mean_ = 0;
  float sum_ = 0;
  float sum2_ = 0;
  int entries_ = 0;
  float error_ = 0;
};

inline Qn::Statistics operator+(Qn::Statistics a, Qn::Statistics b) {
  float nsum = a.Sum() + b.Sum();
  float nsum2 = a.Sum2() + b.Sum2();
  int nentries = a.Entries() + b.Entries();
  float nmean = nsum / (float) nentries;
  float nerror = Qn::Stats::Sigma(nmean, nsum2, nentries);
  Qn::Statistics c(nmean, nsum, nsum2, nerror, nentries);
  return c;
}

inline Qn::Statistics operator*(Qn::Statistics a, Qn::Statistics b) {
  float nmean = a.Mean() * b.Mean();
  int nentries = a.Entries() + b.Entries();
  std::cout << a.error_ << " " << b.error_ << "\n";
  std::cout << a.mean_ << " " << b.mean_ << "\n";
  float nsum2 = a.Mean() * a.Mean() * b.Error() * b.Error() + a.Mean() * a.Mean() * b.Error() * b.Error();
  float nerror = std::sqrt(nsum2);
  float nsum = 0;
  Qn::Statistics c(nmean, nsum, nsum2, nerror, nentries);
  return c;

}

inline Qn::Statistics operator/(Qn::Statistics a, Qn::Statistics b) {
  float nmean;
  float nsum2;
  float nerror;
  if (std::abs(b.Mean()- 0) > 10e-4) {
    nmean = a.Mean() / b.Mean();
    nsum2 = a.Mean() * a.Mean() * b.Error() * b.Error()
        + a.Mean() * a.Mean() * a.Error() * a.Error() / (b.Mean() * b.Mean() * b.Mean() * b.Mean());
    nerror = std::sqrt(nsum2);

  } else {
    nmean = 0;
    nsum2 = 0;
    nerror = 0;

  }
  int nentries = a.Entries() + b.Entries();
  float nsum = 0;
  Qn::Statistics c(nmean, nsum, nsum2, nerror, nentries);
  return c;
}
}

#endif //FLOW_STATISTICS_H
