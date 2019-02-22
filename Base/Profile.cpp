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

#include "Profile.h"

namespace Qn {

namespace Internal {

}

Profile Merge(const Profile lhs, const Profile rhs) {
  Profile c(lhs);
  c.sumwy_ = lhs.sumwy_ + rhs.sumwy_;
  c.sumwy2_ = lhs.sumwy2_ + rhs.sumwy2_;
  c.sumw_ = lhs.sumw_ + rhs.sumw_;
  c.sumw2_ = lhs.sumw2_ + rhs.sumw2_;
  c.entries_ = lhs.entries_ + rhs.entries_;
  return c;
}

Profile operator+(const Profile lhs, const Profile rhs) {
  Profile c(lhs);
  c.sumwy_ = lhs.sumwy_ + rhs.sumwy_;
  c.mean_ = (lhs.Mean()*lhs.sumw_ + rhs.Mean()*rhs.sumw_)/c.sumwy_;
  c.var_ = lhs.sumw_*(lhs.Mean()*lhs.Mean() + lhs.Error()*lhs.Error())
      + rhs.sumw_*(rhs.Mean()*rhs.Mean() + rhs.Error()*rhs.Error()) - (c.Mean()*c.Mean());
//  c.sumwy2_ = lhs.sumwy2_ + rhs.sumwy2_;
//  c.sumw_ = lhs.sumw_ + rhs.sumw_;
//  c.sumw2_ = lhs.sumw2_ + rhs.sumw2_;
//  c.entries_ = lhs.entries_ + rhs.entries_;
  return c;
}

Profile operator-(const Profile lhs, const Profile rhs) {
  Profile c(lhs);
  c.sumwy_ = lhs.sumwy_ + rhs.sumwy_;
  c.mean_ = (lhs.Mean()*lhs.sumw_ - rhs.Mean()*rhs.sumw_)/c.sumwy_;
  c.var_ = lhs.sumw_*(lhs.Mean()*lhs.Mean() + lhs.Error()*lhs.Error())
      - rhs.sumw_*(rhs.Mean()*rhs.Mean() + rhs.Error()*rhs.Error()) - (c.Mean()*c.Mean());
////  c.sumwy_ = lhs.sumwy_ - rhs.sumwy_;
//  c.sumwy2_ = lhs.sumwy2_ - rhs.sumwy2_;
//  c.sumw_ = lhs.sumw_ - rhs.sumw_;
//  c.sumw2_ = lhs.sumw2_ - rhs.sumw2_;
//  c.entries_ = lhs.entries_ - rhs.entries_;
  return c;
}

Profile Sqrt(const Profile a) {
  Profile c(a);
  c.sumwy_ = sqrt(a.sumwy_);
  c.sumwy2_ = sqrt(c.sumwy2_);
  c.sumw_ = sqrt(c.sumw_);
  c.sumw2_ = sqrt(c.sumw2_);
  return c;
}

Profile operator*(const Profile lhs, double rhs) {
  Profile c(lhs);
  c.sumwy_ = lhs.sumwy_*rhs;
  c.sumwy2_ = lhs.sumwy2_*rhs;
  return c;
}

Profile operator*(const Profile lhs, const Profile rhs) {
  Profile c(lhs);
  c.sumwy_ = lhs.sumwy_*rhs.sumwy_;
  c.sumwy2_ = lhs.sumwy2_*rhs.sumwy2_;
  c.sumw_ = lhs.sumw_*rhs.sumw_;
  c.sumw2_ = lhs.sumw2_*rhs.sumw2_;
  c.entries_ = lhs.entries_*rhs.entries_;
  return c;
}

Profile operator/(const Profile num, const Profile den) {
  Profile c(num);
  c.sumwy_ = num.sumwy_/den.sumwy_;
  c.sumwy2_ = num.sumwy2_/den.sumwy2_;
  c.sumw_ = num.sumw_/den.sumw_;
  c.sumw2_ = num.sumw2_/den.sumw2_;
  c.entries_ = num.entries_/den.entries_;
  return c;
}

}