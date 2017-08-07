//
// Created by Lukas Kreis on 03.08.17.
//

#include "CorrectionInterface.h"
namespace Qn {
namespace Internal {

void FillDataToFramework(QnCorrectionsManager &manager, std::map<int, Qn::DataContainerDataVector> &pairs) {
  int tempid = 0;
  for (auto pair : pairs) {
    int ibin = 0;
    auto detector = pair.second;
    auto nbins = detector.size();
    for (auto bin : detector) {
      auto detectorid = nbins * tempid + ibin;
      ++ibin;
      for (auto data : bin) {
        manager.AddDataVector(detectorid, data.phi, data.weight);
      }
    }
    ++tempid;
  }
}
void AddDetectorToFramework(QnCorrectionsManager &manager,
                            DetectorType type,
                            std::map<int, Qn::DataContainerDataVector> &pairs) {
  int tempid = 0;
  for (auto pair : pairs) {
    auto detectorbinning = pair.second;
    DetectorGenerator generator;
    auto *a = new QnCorrectionsEventClassVariablesSet(1);
    double centbins[][2] = {{0.0, 2}, {100.0, 100}};
    a->Add(new QnCorrectionsEventClassVariable(1, "centrality", centbins));
    generator.SetEventVariables(a);
    int ibin = 0;
    for (const auto &bin : detectorbinning) {
      auto detectorid = detectorbinning.size() * tempid + ibin;
      ++ibin;
      auto frameworkdetector = generator.GenerateDetector(detectorid, type);
      manager.AddDetector(frameworkdetector);
    }
    ++tempid;
  }
}

void GetQnFromFramework(QnCorrectionsManager &manager, std::map<int, std::unique_ptr<Qn::DataContainerQn>> &pairs) {
  int id = 0;
  for (auto &pair : pairs) {
    auto &detector = pair.second;
    auto ibin = 0;
    for (auto &bin : *detector) {
      auto detectorid = detector->size() * id + ibin;
      ++ibin;
      auto vector = manager.GetDetectorQnVector(std::to_string(detectorid).data(), "latest", "latest");
      if ((!vector)) continue;
      QnCorrectionsQnVector element;
      bin = *vector;
    }
    ++id;
  }
}

void SaveToTree(TTree &tree, std::map<int, std::unique_ptr<Qn::DataContainerQn>> &pairs) {
 for (auto &pair : pairs) {
   tree.Branch(std::to_string(pair.first).data(),pair.second.get());
 }
}

}
}
