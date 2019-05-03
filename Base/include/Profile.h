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

/**
 * @brief Profile keeps sums
 *  sum w A B;
 *  sum w (A B)^2;
 *  sum w;
 *  sum w^2
 */
class Profile {
 public:

  Profile() = default;

  virtual ~Profile() = default;

  void Fill(const Product &prod) {
    sumwy_ += prod.GetWeight()*prod.result;
    sumwy2_ += prod.GetWeight()*prod.result*prod.result;
    sumw_ += prod.GetWeight();
    sumw2_ += prod.GetWeight()*prod.GetWeight();
    ++entries_;
  }

  inline double Mean() const {
    if (sumw_ > 0.) {
      return sumwy_/sumw_;
    } else { return sumwy_; }
  }

  inline double Error() const {
    double mean = sumwy_/sumw_;
    double variance = fabs(sumwy2_/sumw_ - mean*mean);
    double neff = sumw_*sumw_/sumw2_;
    if (neff > 0.) {
      return std::sqrt(variance/neff);
    } else { return 0.; }
  }

  void CalculatePointAverage() {
    mean_ = Mean();
    var_ = Error();
  }

  inline double MeanPA() const { return mean_;}
  inline double ErrorPA() const { return var_;}

  void Print() {
    std::cout << "Sum{w y}   " << sumwy_ << std::endl;
    std::cout << "Sum{w y^2} " << sumwy2_ << std::endl;
    std::cout << "Sum{w}     " << sumw_ << std::endl;
    std::cout << "Sum{w^2}   " << sumw2_ << std::endl;
    std::cout << "Entries    " << entries_ << std::endl;
    std::cout << "Mean       " << Mean() << std::endl;
    std::cout << "Error      " << Error() << std::endl;
  }

  static Profile MergeNormal(const Profile &, const Profile &);
  static Profile MergePointAverage(const Profile &, const Profile &);

  static Profile AdditionNormal(const Profile &, const Profile &);
  static Profile AdditionPointAverage(const Profile &, const Profile &);

  static Profile SubtractionNormal(const Profile &, const Profile &);
  static Profile SubtractionPointAverage(const Profile &, const Profile &);

  static Profile MultiplicationNormal(const Profile &, const Profile &);
  static Profile MultiplicationPointAverage(const Profile &, const Profile &);

  static Profile DivisionNormal(const Profile &, const Profile &);
  static Profile DivisionPointAverage(const Profile &, const Profile &);

  static Profile SqrtNormal(const Profile &);
  static Profile SqrtPointAverage(const Profile &);

  static Profile ScaleNormal(const Profile &, double);
  static Profile ScalePointAverage(const Profile &, double);

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
