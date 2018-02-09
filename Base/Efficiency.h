//
// Created by Lukas Kreis on 01.02.18.
//

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
