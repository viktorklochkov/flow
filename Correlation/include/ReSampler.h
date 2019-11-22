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
#ifndef FLOW_DATAFRAMECORRELATION_INCLUDE_DATAFRAMERESAMPLER_H_
#define FLOW_DATAFRAMECORRELATION_INCLUDE_DATAFRAMERESAMPLER_H_

#include <random>
#include <vector>
#include <algorithm>
#include "RtypesCore.h"
#include "ROOT/RVec.hxx"

namespace Qn {
namespace Correlation {

class ReSampler {
 public:
  ReSampler() = default;
  ReSampler(std::size_t n) :
      n_(n),
      poisson_(1) {
    std::random_device rd;
    generator_ = std::mt19937{rd()};
  }

  ROOT::RVec<ULong64_t> operator()() {
    ROOT::RVec<ULong64_t> vec(n_);
    for (auto &entry : vec) {
      entry = poisson_(generator_);
    }
    return vec;
  }

  std::size_t N() const { return n_; }
 private:
  std::size_t n_{10};
  std::mt19937 generator_;
  std::poisson_distribution<> poisson_;
};

}
}
#endif //FLOW_DATAFRAMECORRELATION_INCLUDE_DATAFRAMERESAMPLER_H_
