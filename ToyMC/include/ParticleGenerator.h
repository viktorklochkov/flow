// Flow Vector Correction Framework
//
// Copyright (C) 2019  Lukas Kreis Ilya Selyuzhenkov
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
#ifndef FLOW_TOYMC_INCLUDE_PARTICLEGENERATOR_H_
#define FLOW_TOYMC_INCLUDE_PARTICLEGENERATOR_H_

#include <array>
#include <cmath>
#include <random>

class ParticleGenerator {
  static constexpr std::size_t n_harmonics_ = 4;
  static constexpr std::size_t nphi_slices = 100;
  static constexpr double kPi = M_PI;
 public:
  ParticleGenerator(std::array<double, n_harmonics_> harmonics) :
      vns_(harmonics),
      phi_dist_(nphi_slices, 0, 2*kPi, [&](const double x){return PhiPdf(x);}) {}
  double GeneratePhi() {return phi_dist_(random_engine_);}
  double GetPhi(double psi) {
    auto phi = GeneratePhi() + psi;
    if (phi > 2*M_PI) phi -= 2*M_PI;
    if (phi < 0) phi += 2*M_PI;
    return phi;
  }
 private:
  std::default_random_engine random_engine_;
  std::array<double,n_harmonics_> vns_;
  double psi_ = 0.;
  std::piecewise_linear_distribution<> phi_dist_;

  double PhiPdf(double phi) {
    double value = 1.;
    for (unsigned int n = 1; n < n_harmonics_; ++n) {
      value += 2 * vns_[n-1] * std::cos(n *(phi - psi_));
    }
    return value;
  }

};

#endif //FLOW_TOYMC_INCLUDE_PARTICLEGENERATOR_H_
