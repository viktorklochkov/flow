//
// Created by Lukas Kreis on 03.08.17.
//

#include "CorrectionInterface.h"
#include "EventInfo.h"
namespace Qn {
namespace Internal {

void FillDataToFramework(QnCorrectionsManager &manager, std::map<int, std::unique_ptr<Qn::DataContainerDataVector>> &pairs) {
  int nbinsrunning = 0;
  for (auto &pair : pairs) {
    int ibin = 0;
    auto &detector = pair.second;
    for (auto bin : *detector) {
      auto detectorid = nbinsrunning + ibin;
      std::cout << detectorid << std::endl;
      ++ibin;
      int idata = 0;
      for (auto data : bin) {
        manager.AddDataVector(detectorid, data.phi, data.weight,idata);
        ++idata;
      }
    }
    nbinsrunning += detector->size();
  }
}
void AddDetectorToFramework(QnCorrectionsManager &manager,
                            std::vector<DetectorType> type,
                            std::map<int, std::unique_ptr<Qn::DataContainerDataVector>> &pairs, QnCorrectionsEventClassVariablesSet &set, int nchannels) {
  int nbinsrunning = 0;
  int in = 0;
  for (auto &pair : pairs) {
    auto &detector = pair.second;
    DetectorGenerator generator;
    generator.SetEventVariables(&set);
    int ibin = 0;
    for (const auto &bin : *detector) {
      auto detectorid = nbinsrunning + ibin;
      auto frameworkdetector = generator.GenerateDetector(detectorid, type.at(in), nchannels);
      manager.AddDetector(frameworkdetector);
      ++ibin;
    }
    nbinsrunning += detector->size();
  }
}

void GetQnFromFramework(QnCorrectionsManager &manager, std::map<int, std::unique_ptr<Qn::DataContainerQn>> &pairs) {
  int nbinsrunning = 0;
  for (auto &pair : pairs) {
    auto &detector = pair.second;
    auto ibin = 0;
    for (auto &bin : *detector) {
      auto detectorid = nbinsrunning + ibin;
      ++ibin;
      auto vector = manager.GetDetectorQnVector(std::to_string(detectorid).data(), "latest", "latest");
      if ((!vector)) continue;
      QnCorrectionsQnVector element;
      bin = *vector;
    }
    nbinsrunning += detector->size();
  }
}

void SaveToTree(TTree &tree, std::map<int, std::unique_ptr<Qn::DataContainerQn>> &pairs) {
 for (auto &pair : pairs) {
   tree.Branch(std::to_string(pair.first).data(),pair.second.get());
 }
}

void SaveToTree(TTree &tree, std::map<int, std::unique_ptr<Qn::DataContainerDataVector>> &pairs) {
  for (auto &pair : pairs) {
    tree.Branch(std::to_string(pair.first).data(),pair.second.get());
  }
}

/**
 * Attaches input tree to event information object for reading.
 * @param tree input tree which contains the  event information in branches
 */
void SaveToTree(TTree &tree, std::unique_ptr<Qn::EventInfo<float>> &eventinfo) {
  eventinfo->SetToTree(tree);
}

}
}
