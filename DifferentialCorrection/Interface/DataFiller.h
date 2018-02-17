//
// Created by Lukas Kreis on 17.01.18.
//

#ifndef FLOW_DATAFILLER_H
#define FLOW_DATAFILLER_H

#include "TList.h"
#include "Base/DataContainer.h"
#include "DifferentialCorrection/Detector.h"
//#include "DifferentialCorrection/Interface/VarManager.h"
//#include "VarManager.h"

namespace Qn {
namespace Differential {
namespace Interface {

class DataFiller {

 public:
  explicit DataFiller() = default;

  void Fill(std::map<std::string, Detector> &detectors) {
    FillDetector(detectors.at("DET1"));
  }

  void FillDetector(Qn::Detector &detector) {
//    const std::array<double, 10> X = {{-1.75, 1.75, -1.75, 1.75}};
//    const std::array<double, 10> Y = {{-1.75, -1.75, 1.75, 1.75}};
//    auto &datacontainer = detector.GetDataContainer();
//    auto values = new float[VarManager::Values::kNMax];
//    VarManager::FillEventInfo(values);
//    for (u_short ich = 0; ich < 5; ich++) {
//      double weight = values[ich+VarManager::Values::kSignal];
//      if (weight > 100) {
//        datacontainer->CallOnElement([ich, Y, X, weight](std::vector<DataVector> &vector) {
//          vector.emplace_back(TMath::ATan2(Y[ich], X[ich]), weight);
//        });
      }
};

}
}
}
#endif //FLOW_DATAFILLER_H
