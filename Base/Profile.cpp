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

Profile Profile::MergeNormal(const Profile &lhs, const Profile &rhs) {
  Profile result;
  result.sumwy_ = lhs.sumwy_ + rhs.sumwy_;
  result.sumwy2_ = lhs.sumwy2_ + rhs.sumwy2_;
  result.sumw_ = lhs.sumw_ + rhs.sumw_;
  result.sumw2_ = lhs.sumw2_ + rhs.sumw2_;
  result.entries_ = lhs.entries_ + rhs.entries_;
  return result;
}
Profile Profile::MergePointAverage(const Profile &lhs, const Profile &rhs) {
  Profile result;
  (void)lhs; (void) rhs;
  std::cout << "not mergeable" << std::endl;
  return result;
}
Profile Profile::AdditionNormal(const Profile &lhs, const Profile &rhs) {
  Profile result;
  result.sumwy_ = lhs.sumwy_ + rhs.sumwy_;
  result.sumwy2_ = lhs.sumwy2_ + rhs.sumwy2_;
  result.sumw_ = lhs.sumw_ + rhs.sumw_;
  result.sumw2_ = lhs.sumw2_ + rhs.sumw2_;
  result.entries_ = lhs.entries_ + rhs.entries_;
  return result;
}
Profile Profile::AdditionPointAverage(const Profile &lhs, const Profile &rhs) {
  Profile result;
  result.mean_ = lhs.mean_ + rhs.mean_;
  result.var_ =  sqrt(lhs.var_*lhs.var_ + rhs.var_*rhs.var_);
  return result;
}
Profile Profile::SubtractionNormal(const Profile &lhs, const Profile &rhs) {
  Profile result;
  result.sumwy_ = lhs.sumwy_ - rhs.sumwy_;
  result.sumwy2_ = lhs.sumwy2_ - rhs.sumwy2_;
  result.sumw_ = lhs.sumw_ - rhs.sumw_;
  result.sumw2_ = lhs.sumw2_ - rhs.sumw2_;
  result.entries_ = lhs.entries_ - rhs.entries_;
  return result;
}

Profile Profile::SubtractionPointAverage(const Profile &lhs, const Profile &rhs) {
  Profile result;
  result.mean_ = lhs.mean_ - rhs.mean_;
  result.var_ =  sqrt(lhs.var_*lhs.var_ + rhs.var_*rhs.var_);
  return result;
}

Profile Profile::MultiplicationNormal(const Profile &lhs, const Profile &rhs) {
  Profile result;
  result.sumwy_ = lhs.sumwy_ * rhs.sumwy_;
  result.sumwy2_ = lhs.sumwy2_ * rhs.sumwy2_;
  result.sumw_ = lhs.sumw_ * rhs.sumw_;
  result.sumw2_ = lhs.sumw2_ * rhs.sumw2_;
  result.entries_ = lhs.entries_ + rhs.entries_;
  return result;
}

Profile Profile::MultiplicationPointAverage(const Profile &lhs, const Profile &rhs) {
  Profile result;
  result.mean_ = lhs.mean_ * rhs.mean_;
  result.var_ =  sqrt(rhs.mean_*rhs.mean_*lhs.var_*lhs.var_+lhs.mean_*lhs.mean_*rhs.var_*rhs.var_);
  return result;
}

Profile Profile::DivisionNormal(const Profile &num, const Profile &den) {
  Profile result;
  result.sumwy_ = num.sumwy_ / den.sumwy_;
  result.sumwy2_ = num.sumwy2_ / den.sumwy2_;
  result.sumw_ = num.sumw_ / den.sumw_;
  result.sumw2_ = num.sumw2_ / den.sumw2_;
  result.entries_ = num.entries_;
  return result;
}

Profile Profile::DivisionPointAverage(const Profile &num, const Profile &den) {
  Profile result;
  result.mean_ = num.mean_ / den.mean_;
  double den2 = den.mean_ * den.mean_;
  result.var_ =  sqrt((den2*num.var_*num.var_+num.mean_*num.mean_*den.var_*den.var_)/(den2*den2));
  return result;
}

Profile Profile::SqrtNormal(const Profile &prof) {
  Profile result;
  result.sumwy_ = std::signbit(prof.sumwy_) ? -1*sqrt(fabs(prof.sumwy_)) : sqrt(fabs(prof.sumwy_));
  result.sumwy2_ = sqrt(prof.sumwy2_);
  result.sumw_ = sqrt(prof.sumw_);
  result.sumw2_ = sqrt(prof.sumw2_);
  result.entries_ = prof.entries_;
  return result;
}

Profile Profile::SqrtPointAverage(const Profile &prof) {
  Profile result;
  result.mean_ = sqrt(prof.mean_);
  result.var_ =  sqrt(prof.var_);
  return result;
}

Profile Profile::ScaleNormal(const Profile &lhs, double scale) {
  Profile result;
  result.sumwy_ = lhs.sumwy_ * scale;
  result.sumwy2_ = lhs.sumwy2_ * scale;
  result.sumw_ = lhs.sumw_;
  result.sumw2_ = lhs.sumw2_;
  result.entries_ = lhs.entries_;
  return result;
}

Profile Profile::ScalePointAverage(const Profile &lhs, double scale) {
  Profile result;
  result.mean_ = scale*lhs.mean_;
  result.var_ =  scale*lhs.var_;
  return result;
}

}