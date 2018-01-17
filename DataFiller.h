//
// Created by Lukas Kreis on 17.01.18.
//

#ifndef FLOW_DATAFILLER_H
#define FLOW_DATAFILLER_H

#include <ReducedEvent/AliReducedEventInfo.h>
#include <ReducedEvent/AliReducedTrackInfo.h>
#include <ReducedEvent/AliReducedVarManager.h>
#include "TList.h"
#include "DataContainer.h"
#include "Detector.h"
#define VAR AliReducedVarManager

namespace Qn {
namespace Differential {
namespace Interface {

class DataFiller {

 public:
  explicit DataFiller(AliReducedEventInfo *event) : event_(event) {}

  void Fill(std::string name, Detector &det) {
    if (name == "TPC") FillTPC(det, *event_);
  }

  void SetVariables(std::vector<VAR::Variables> vars) {
    for (auto var : vars) {
      AliReducedVarManager::SetUseVariable(var);
    }
  }

  void FillTPC(Qn::Detector &detector, AliReducedEventInfo &event) {
    auto values = new float[AliReducedVarManager::Variables::kNVars];
    AliReducedTrackInfo *track = nullptr;
    auto trackList = event.GetTracks();
    TIter next(trackList);
    next.Reset();
    auto &datacontainer = detector.GetDataContainer();
    auto &axes = datacontainer->GetAxes();
    std::vector<float> trackparams;
    trackparams.reserve(axes.size());
    long ntracks = trackList->GetSize();
    std::for_each(datacontainer->begin(),
                  datacontainer->end(),
                  [ntracks](std::vector<DataVector> &vector) { vector.reserve(ntracks); });
    while ((track = (AliReducedTrackInfo *) next()) != nullptr) {
      if (!track->TestQualityFlag(15)) continue;
      VAR::FillTrackInfo(track, values);
      if (values[VAR::kEta] > 0.8 || values[VAR::kEta] < -0.8) continue;
      if (values[VAR::kPt] < 0.2 || values[VAR::kPt] > 10.0) continue;
      values[-1] = 0;
      for (const auto num : detector.GetEnums()) {
        trackparams.push_back(values[num]);
      }
      try {
        datacontainer->CallOnElement(trackparams, [values](std::vector<DataVector> &vector) {
          vector.emplace_back(values[VAR::kPhi]);
        });
      }
      catch (std::exception &) {
        continue;
      }
      trackparams.clear();
    }
    delete[] values;
  }

 private:
  AliReducedEventInfo *event_;
};

}
}
}
#endif //FLOW_DATAFILLER_H
