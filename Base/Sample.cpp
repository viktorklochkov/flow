// Flow Vector Correction Framework
//
// Copyright (C) 2018  Lukas Kreis, Ilya Selyuzhenkov
// Contact: l.kreis@gsi.de; ilya.selyuzhenkov@gmail.com
// For a full list of contributors please see docs/Credits
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "Sample.h"

namespace Qn {

Sample Merge(const Sample &lhs, const Sample &rhs) {
  std::vector<StatisticMean> sums(lhs.samples_);
  int i = 0;
  if (!rhs.samples_.empty()) {
    for (auto &sum : sums) {
      sum.merge(rhs.samples_[i]);
      ++i;
    }
  }
  Sample c(sums);
  return c;
}

Sample operator+(const Sample &lhs, const Sample &rhs) {
  std::vector<StatisticMean> sums(lhs.samples_);
  int i = 0;
  if (!rhs.samples_.empty()) {
    for (auto &sum : sums) {
      sum += rhs.samples_[i];
      ++i;
    }
  }
  Sample c(sums);
  return c;
}

Sample operator-(const Sample &lhs, const Sample &rhs) {
  std::vector<StatisticMean> sums(lhs.samples_);
  int i = 0;
  if (!rhs.samples_.empty()) {
    for (auto &sum : sums) {
      sum -= rhs.samples_[i];
      ++i;
    }
  }
  Sample c(sums);
  return c;
}

Sample operator*(const Sample &lhs, const Sample &rhs) {
  std::vector<StatisticMean> sums(lhs.samples_);
  int i = 0;
  if (!rhs.samples_.empty()) {
    for (auto &sum : sums) {
      sum *= rhs.samples_[i];
      ++i;
    }
  }
  Sample c(sums);
  return c;
}

Sample operator/(const Sample &num, const Sample &den) {
  std::vector<StatisticMean> sums(num.samples_);
  int i = 0;
  if (!den.samples_.empty()) {
    for (auto &sum : sums) {
      sum /= den.samples_[i];
      ++i;
    }
  }
  Sample c(sums);
  return c;
}

Sample operator*(const Sample &lhs, const double rhs) {
  std::vector<StatisticMean> sums(lhs.samples_);
  for (auto &sum : sums) {
    sum *= rhs;
  }
  Sample c(sums);
  return c;
}

Sample Sqrt(const Sample &a) {
  std::vector<StatisticMean> sums(a.samples_);
  for (auto &sum : sums) {
    sum.Sqrt();
  }
  Sample c(sums);
  return c;
}

void Sample::Print(double real_mean) {
  int isample = samples_.size()/3;
  std::cout << "SAMPLES EXCERPT: " << isample << ", " << isample*2 << ", " << isample*3 << std::endl;
  std::cout << "S" << isample << " ";
  samples_[isample].Print();
  std::cout << "S" << isample*2 << " ";
  samples_[isample*2].Print();
  std::cout << "S" << isample*3 << " ";
  samples_[isample*3].Print();
  std::cout << "SUMMARY   " << std::endl;
  std::cout << "N_samples " << samples_.size() << std::endl;
  std::cout << "Mean      " << Mean() << std::endl;
  std::cout << "Error Hi  " << ErrorHi(real_mean) << std::endl;
  std::cout << "Error Lo  " << ErrorLo(real_mean) << std::endl;
  std::cout << "Error Sym " << (ErrorHi(real_mean) + ErrorLo(real_mean))/2 << std::endl;
}

}