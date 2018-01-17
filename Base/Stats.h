//
// Created by Lukas Kreis on 15.11.17.
//

#ifndef FLOW_STATISTICS_H
#define FLOW_STATISTICS_H

//#include "Base/DataContainer.h"

#include <iostream>
#include <iterator>
#include <algorithm>
#include <utility>
#include <TMathBase.h>
#include <TMath.h>
#include <array>

namespace Qn {

namespace Stats {

inline double Sigma(const double mean, const double sum2, const int n) {
  double variance = TMath::Abs(sum2 / (double) n - mean * mean);
  return sqrt(variance) / sqrt((double) n);
}

}

class Profile {
 public:
  Profile() = default;
  Profile(double mean, double sum, double sum2, double error, int entries) :
      mean_(mean),
      sum_(sum),
      sum2_(sum2),
      entries_(entries),
      error_(error) {}

  inline void Update(double value) {
    sum_ = sum_ + value;
    ++entries_;
    mean_ = sum_ / (entries_);
    sum2_ = sum2_ + value * value;
    error_ = Qn::Stats::Sigma(mean_, sum2_, entries_);
  };
  inline double Mean() const { return mean_; }
  inline double Sum() const { return sum_; }
  inline double Sum2() const { return sum2_; }
  inline double Error() const { return error_; }
  inline int Entries() const { return entries_; }

  inline Profile Sqrt() const {
    Profile a(*this);
    a.mean_ = std::sqrt(std::abs(mean_));
    a.sum_ = std::sqrt(std::abs(sum_));
    a.sum2_ = std::sqrt(std::abs(sum2_));
    a.error_ = 1./2. * a.error_ / a.mean_;
    return a;
  }

  friend Qn::Profile operator+(Qn::Profile a, Qn::Profile b);
  friend Qn::Profile operator-(Qn::Profile a, Qn::Profile b);
  friend Qn::Profile operator*(Qn::Profile a, Qn::Profile b);
  friend Qn::Profile operator/(Qn::Profile a, Qn::Profile b);
  friend Qn::Profile operator*(Qn::Profile a, double b);
 private:
  double mean_ = 0;
  double sum_ = 0;
  double sum2_ = 0;
  int entries_ = 0;
  double error_ = 0;
};

inline Qn::Profile operator*(Qn::Profile a, double b) {
  double nsum = a.sum_ * b;
  double nsum2 = a.sum2_ * b;
  int nentries = a.entries_;
  double nmean = a.mean_ * b;
  double nerror = a.error_ * fabs(b);
  Qn::Profile c(nmean, nsum, nsum2, nerror, nentries);
  return c;
}

inline Qn::Profile operator+(Qn::Profile a, Qn::Profile b) {
  int nentries = a.entries_ + b.entries_;
  double nsum = (a.sum_ * a.mean_ + b.sum_ * b.mean_) / nentries;
  double nsum2 = (a.sum2_ * a.entries_+ b.sum2_ *b.entries_) / nentries;
  double nmean = (a.entries_ * a.mean_ + b.entries_ * b.mean_) / nentries;
//  double nerror = std::sqrt(a.error_* a.error_ + b.error_ * b.error_)/2;
  double nerror = std::sqrt((a.error_* a.error_*a.entries_ + b.error_ * b.error_ * a.entries_ - 2* a.mean_ * b.mean_)/nentries);
  Qn::Profile c(nmean, nsum, nsum2, nerror, nentries);
  return c;
}


inline Qn::Profile operator-(Qn::Profile a, Qn::Profile b) {
  int nentries = a.entries_ + b.entries_;
  double nsum = (a.sum_ * a.mean_ - b.sum_ * b.mean_) / nentries;
  double nsum2 = (a.sum2_ * a.entries_- b.sum2_ *b.entries_) / nentries;
  double nmean = (a.entries_ * a.mean_ - b.entries_ * b.mean_) / nentries;
  double nerror = std::sqrt(a.error_* a.error_ + b.error_ * b.error_);
  Qn::Profile c(nmean, nsum, nsum2, nerror, nentries);
  return c;
}

inline Qn::Profile operator*(Qn::Profile a, Qn::Profile b) {
  double nmean = a.mean_ * b.mean_;
  int nentries = a.entries_ + b.entries_;
  double nsum2 = a.mean_ * a.mean_ * b.error_ * b.error_ + b.mean_ * b.mean_ * a.error_ * a.error_;
  double nerror = std::sqrt(a.mean_ * a.mean_ * b.error_ * b.error_ + b.mean_ * b.mean_ * a.error_ * a.error_);
  double nsum = 0;
  Qn::Profile c(nmean, nsum, nsum2, nerror, nentries);
  return c;

}

inline Qn::Profile operator/(Qn::Profile a, Qn::Profile b) {
  double nmean = 0;
  double nsum2 = 0;
  double nerror = 0;
  if (std::abs(b.Mean() - 0) > 10e-8) {
    nmean = a.Mean() / b.Mean();
    nsum2 = a.error_ * a.error_ / (b.mean_ * b.mean_) + a.mean_ * a.mean_ / (b.mean_ * b.mean_ * b.mean_ * b.mean_) * b.error_ * b.error_;
    nerror = std::sqrt(a.error_ * a.error_ / (b.mean_ * b.mean_) + a.mean_ * a.mean_ / (b.mean_ * b.mean_ * b.mean_ * b.mean_) * b.error_ * b.error_);
  }
  int nentries = a.Entries() + b.Entries();
  double nsum = nmean * nentries;
  Qn::Profile c(nmean, nsum, nsum2, nerror, nentries);
  return c;
}

}

#endif //FLOW_STATISTICS_H
