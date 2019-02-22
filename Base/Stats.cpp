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


#include "Stats.h"

namespace Qn {
Stats operator+(const Stats &lhs, const Stats &rhs) {
  Stats res;
  res.subsamples_ = lhs.subsamples_ + rhs.subsamples_;
  res.profile_ = lhs.profile_ + rhs.profile_;
  res.bits_ = lhs.bits_;
  return res;
}

Stats operator-(const Stats &lhs, const Stats &rhs) {
  Stats res;
  res.subsamples_ = lhs.subsamples_ - rhs.subsamples_;
  res.profile_ = lhs.profile_ - rhs.profile_;
  res.bits_ = lhs.bits_;
  return res;
}

Stats operator*(const Stats &lhs, const Stats &rhs) {
  Stats res;
  res.subsamples_ = lhs.subsamples_*rhs.subsamples_;
  res.profile_ = lhs.profile_*rhs.profile_;
  res.bits_ = lhs.bits_;
  return res;
}

Stats operator*(const Stats &lhs, double rhs) {
  Stats res;
  res.subsamples_ = lhs.subsamples_*rhs;
  res.profile_ = lhs.profile_*rhs;
  res.bits_ = lhs.bits_;
  return res;
}

Stats operator*(double lhs, const Stats &rhs) {
  Stats res;
  res.subsamples_ = rhs.subsamples_*lhs;
  res.profile_ = rhs.profile_*lhs;
  res.bits_ = rhs.bits_;
  return res;
}

Stats operator/(const Stats &num, const Stats &den) {
  Stats res;
  res.subsamples_ = num.subsamples_/den.subsamples_;
  res.profile_ = num.profile_/den.profile_;
  res.bits_ = num.bits_;
  return res;
}

Stats Merge(const Stats &lhs, const Stats &rhs) {
  Stats res;
  res.subsamples_ = lhs.subsamples_ + rhs.subsamples_;
  res.profile_ = lhs.profile_ + rhs.profile_;
  res.bits_ = lhs.bits_;
  return res;
}

void Stats::Print() {
  std::cout << std::endl;
  std::cout << "-----Bits------" << std::endl;
  std::cout << std::bitset<32>(bits_) << std::endl;
  std::cout << "CORRELATEDERRORS " << (bits_ & BIT(16) ? 1 : 0) << std::endl;
  std::cout << "PRODAVGWEIGHTS   " << (bits_ & BIT(17) ? 1 : 0) << std::endl;
  std::cout << "--SubSampling--" << std::endl;
  subsamples_.Print(profile_.Mean());
  std::cout << "----Profile----" << std::endl;
  std::cout << "Sum{w y}   " << sumwy_ << std::endl;
  std::cout << "Sum{w y^2} " << sumwy2_ << std::endl;
  std::cout << "Sum{w}     " << sumw_ << std::endl;
  std::cout << "Sum{w^2}   " << sumw2_ << std::endl;
  std::cout << "Entries    " << entries_ << std::endl;
  std::cout << "Mean       " << Mean() << std::endl;
  std::cout << "Error      " << Error() << std::endl;
}
}