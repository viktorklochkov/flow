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

#ifndef FLOW_PROFILE_H
#define FLOW_PROFILE_H

#include <cmath>
#include <iostream>

#include "Rtypes.h"

#include "Product.h"

namespace Qn {

class Profile {
 public:

  Profile() = default;
  Profile(const Profile &a) : sumwy_(a.sumwy_), sumwy2_(a.sumwy2_), sumw_(a.sumw_), sumw2_(a.sumw2_) {}
  virtual ~Profile() = default;

  void Fill(const Product &prod) {
    sumwy_ += prod.GetProdWeight()*prod.result;
    sumwy2_ += prod.GetProdWeight()*prod.result*prod.result;
    sumw_ += prod.GetProdWeight();
    sumw2_ += prod.GetProdWeight()*prod.GetProdWeight();
    ++entries_;
  }

  inline double Mean() const {
    if (sumw_ > 0.) {
      return sumwy_/sumw_;
    } else { return sumwy_; }
  }

  inline
  double Error() const {
    double mean = sumwy_/sumw_;
    double variance = fabs(sumwy2_/sumw_ - mean*mean);
    double neff = sumw_*sumw_/sumw2_;
    if (neff > 0.) {
      return std::sqrt(variance/neff);
    } else { return 0.; }
  }

  void Print() {
    std::cout << "Sum{w y}   " << sumwy_ << std::endl;
    std::cout << "Sum{w y^2} " << sumwy2_ << std::endl;
    std::cout << "Sum{w}     " << sumw_ << std::endl;
    std::cout << "Sum{w^2}   " << sumw2_ << std::endl;
    std::cout << "Entries    " << entries_ << std::endl;
    std::cout << "Mean       " << Mean() << std::endl;
    std::cout << "Error      " << Error() << std::endl;
  }

  friend Profile operator+(Profile, Profile);
  friend Profile operator-(Profile, Profile);
  friend Profile operator*(Profile, double);
  friend Profile operator*(Profile, Profile);
  friend Profile operator/(Profile, Profile);
  friend Profile Merge(Profile, Profile);
  friend Profile Sqrt(Profile);

 private:
  double sumwy_ = 0.;
  double sumwy2_ = 0.;
  double sumw_ = 0.;
  double sumw2_ = 0.;
  int entries_ = 0;
  double mean_ = 0.;
  double var_ = 0.;

///< /// \cond CLASSIMP
 ClassDef(Profile, 5);
/// \endcond
};
}

#endif //FLOW_STATISTICS_H
