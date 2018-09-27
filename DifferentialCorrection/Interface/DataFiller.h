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

#include <iostream>
#include <fstream>

#include <ReducedEvent/AliReducedEventInfo.h>
#include <ReducedEvent/AliReducedTrackInfo.h>
#include <ReducedEvent/AliReducedVarManager.h>
#include <QnCorrections/QnCorrectionsManager.h>

#include "TList.h"
#include "Base/DataContainer.h"
#include "DifferentialCorrection/Detector.h"
#define VAR AliReducedVarManager

namespace Qn {
class DataFiller {
 public:
  using MapDetectors = std::map<std::string, std::unique_ptr<DetectorBase>>;
  explicit DataFiller(AliReducedEventInfo *event) : event_(event) {}
  void FillEventInfo(const std::shared_ptr<VariableManager> &var_manager) {
    VAR::FillEventInfo(event_, var_manager->GetVariableContainer());
  }
  void FillDetectors(MapDetectors &channel, MapDetectors  &tracking,
                     const std::shared_ptr<VariableManager> &var_manager) {
    for (auto &dp : channel) { dp.second->FillData(); }
    AliReducedTrackInfo *track = nullptr;
    auto trackList = event_->GetTracks();
    TIter next(trackList);
    next.Reset();
    while ((track = (AliReducedTrackInfo *) next())!=nullptr) {
      VAR::FillTrackInfo(track, var_manager->GetVariableContainer());
      for (auto &dp : tracking) { dp.second->FillData(); }
    }
  }
 private:
  AliReducedEventInfo *event_;
};

inline TChain *MakeChain(std::string filename, std::string treename) {
  auto chain = new TChain(treename.data());
  std::ifstream in;
  in.open(filename);
  std::string line;
  std::cout << "Adding files to chain:" << std::endl;
  while ((in >> line).good()) {
    if (!line.empty()) {
      chain->AddFile(line.data());
      std::cout << line << std::endl;
    }
  }
  return chain;
}
}
#endif //FLOW_DATAFILLER_H
