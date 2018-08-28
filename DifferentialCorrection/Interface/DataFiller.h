//
// Created by Lukas Kreis on 17.01.18.
//

#ifndef FLOW_DATAFILLER_H
#define FLOW_DATAFILLER_H

#include "DifferentialCorrection/Detector.h"

namespace Qn {
class DataFiller {
 public:
  explicit DataFiller() {}
  void Fill(std::map<std::string, Detector> &channel, std::map<std::string, Detector> &tracking, const std::shared_ptr<VariableManager> &var_manager) {
    for (auto &dp : channel) {
      dp.second.FillData();
    }
    for (int i= 0; i < 100;++i) {
      for (auto &dp : tracking) {
        dp.second.FillData();
      }
    }
  }
 private:
};
}
#endif //FLOW_DATAFILLER_H
