//
// Created by Lukas Kreis on 03.08.17.
//
//
#include "CorrectionInterface.h"
namespace Qn {
namespace Internal {

std::map<int, std::unique_ptr<Qn::DataContainerQn>> MakeQnDataContainer(const DetectorMap &map) {
  std::map<int, std::unique_ptr<Qn::DataContainerQn>> outmap;
  for (const auto &detector : map) {
    auto axes = std::get<1>(detector.second)->GetAxes();
    std::unique_ptr<Qn::DataContainerQn> datacontainer(new Qn::DataContainerQn());
    for (auto axis : axes) {
      datacontainer->AddAxis(axis);
    }
    outmap.insert(std::pair<int, std::unique_ptr<Qn::DataContainerQn>>(detector.first, std::move(datacontainer)));
  }
  return outmap;
};

void FillDataToFramework(QnCorrectionsManager &manager,
                         DetectorMap &map) {
  int nbinsrunning = 0;
  for (auto &pair : map) {
    int ibin = 0;
    auto &detector = std::get<1>(pair.second);
    for (auto bin : *detector) {
      auto detectorid = nbinsrunning + ibin;
      ++ibin;
      int idata = 0;
      for (auto data : bin) {
        manager.AddDataVector(detectorid, data.phi, data.weight, idata);
        ++idata;
      }
    }
    nbinsrunning += detector->size();
  }
}
void AddDetectorsToFramework(QnCorrectionsManager &manager,
                             DetectorMap &map,
                             QnCorrectionsEventClassVariablesSet &set) {
  int nbinsrunning = 0;
  for (auto &pair : map) {
    auto &detectortuple = pair.second;
    auto &detector = std::get<1>(pair.second);
    auto detectorid = pair.first;
    DetectorGenerator generator;
    generator.SetEventVariables(&set);
    int ibin = 0;
    for (const auto &bin : *detector) {
      auto globalid = nbinsrunning + ibin;
      auto frameworkdetector = generator.GenerateDetector(globalid, detectorid, ibin, detectortuple);
      manager.AddDetector(frameworkdetector);
      ++ibin;
    }
    nbinsrunning += detector->size();
  }
}

void GetQnFromFramework(QnCorrectionsManager &manager, std::map<int, std::unique_ptr<Qn::DataContainerQn>> &map) {
  int nbinsrunning = 0;
  for (auto &pair : map) {
    auto &detector = pair.second;
    auto ibin = 0;
    for (auto &bin : *detector) {
      auto detectorid = nbinsrunning + ibin;
      ++ibin;
      auto name = (std::string(Configuration::DetectorNames[pair.first]) + std::to_string(detectorid)).data();
//      auto detectorname = std::string(Configuration::DetectorNames[pair.first]);
//      auto binname = detector->GetBinDescription(ibin);
//      auto name = (detectorname + binname).c_str();
      auto vector = manager.GetDetectorQnVector(name, "latest", "latest");
      if ((!vector)) continue;
      QnCorrectionsQnVector element;
      bin = *vector;
    }
    nbinsrunning += detector->size();
  }
}

void SaveToTree(TTree &tree, std::map<int, std::unique_ptr<Qn::DataContainerQn>> &map) {
  for (auto &pair : map) {
    tree.Branch(Configuration::DetectorNames[pair.first], pair.second.get());
  }
}

void SaveToTree(TTree &tree, std::map<int, std::unique_ptr<Qn::DataContainerDataVector>> &map) {
  for (auto &pair : map) {
    tree.Branch(Configuration::DetectorNames[pair.first], pair.second.get());
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
