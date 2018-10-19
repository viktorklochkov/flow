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

#ifndef FLOW_EFFICIENCY_H
#define FLOW_EFFICIENCY_H

#include <cmath>
#include <Math/QuantFunc.h>

namespace Qn {
class Efficiency {
 public:

  enum class Passed : bool {
    Yes = true,
    No = false
  };

  void Update(Efficiency::Passed pass) {
    total++;
    if ((bool) pass) passed++;
  }

  float GetEfficiency() {
    CalculateEfficiency();
    CalculateUncertainties();
    return efficiency;
  }

  float GetErrorUp() {
    CalculateEfficiency();
    CalculateUncertainties();
    return up_error;
  }

  float GetErrorLow() {
    CalculateEfficiency();
    CalculateUncertainties();
    return down_error;
  }

 private:
  void CalculateEfficiency() {
    efficiency = (total) ? (passed/total) : 0.0;
  }

  double CalculateNormal(float level, bool upper) const {
    double alpha = (1.0 - level)/2.0;
    if (total==0) return (upper) ? 1.0 : 0.0;
    double sigma = std::sqrt(efficiency*(1.0 - efficiency)/total);
    double delta = ROOT::Math::normal_quantile(1.0 - alpha, sigma);

    if (upper)
      return ((efficiency + delta) > 1.0) ? 1.0 : (efficiency + delta);
    else
      return ((efficiency - delta) < 0.0) ? 0.0 : (efficiency - delta);
  }

  void CalculateUncertainties() {
    float level = 0.68;
    up_error = CalculateNormal(level, true) - efficiency;
    down_error = efficiency - CalculateNormal(level, false);
  }

  float passed = 0;
  float total = 0;
  float efficiency = 0;
  float up_error = 0;
  float down_error = 0;
};
}

#endif //FLOW_EFFICIENCY_H
