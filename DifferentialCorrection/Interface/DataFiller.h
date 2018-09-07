//
// Created by Lukas Kreis on 17.01.18.
//

#ifndef FLOW_DATAFILLER_H
#define FLOW_DATAFILLER_H

#include "DifferentialCorrection/Detector.h"

namespace Qn {
class DataFiller {
 public:
  using MapDetectors = std::map<std::string, Detector>;
  explicit DataFiller()  {}
  void FillEventInfo(const std::shared_ptr<VariableManager> &var_manager) {
    //Fill event info into variable manager here.
  }
  void FillDetectors(MapDetectors &channel, MapDetectors  &tracking,
                     const std::shared_ptr<VariableManager> &var_manager) {
    for (auto &dp : channel) { dp.second.FillData(); }
    for (int i= 0; i < 100;++i) {
    //Fill Track info into variable manager here.
      for (auto &dp : tracking) { dp.second.FillData(); }
    }
  }
 private:
};
}
#endif //FLOW_DATAFILLER_H
