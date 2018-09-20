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

#ifndef FLOW_DATAFILLER_H
#define FLOW_DATAFILLER_H

#include "DifferentialCorrection/Detector.h"

namespace Qn {
class DataFiller {
 public:
  using MapDetectors = std::map<std::string, std::unique_ptr<DetectorBase>>;
  void FillEventInfo(const std::shared_ptr<VariableManager> &var_manager) {
    //Fill event info into variable manager here.
  }
  void FillDetectors(MapDetectors &channel, MapDetectors  &tracking,
                     const std::shared_ptr<VariableManager> &var_manager) {
    for (auto &dp : channel) { dp.second->FillData(); }
    for (int i= 0; i < 100;++i) {
    //Fill Track info into variable manager here.
      for (auto &dp : tracking) { dp.second->FillData(); }
    }
  }
 private:
};
}
#endif //FLOW_DATAFILLER_H
