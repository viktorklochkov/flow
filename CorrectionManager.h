//
// Created by Lukas Kreis on 16.01.18.
//

#ifndef FLOW_CORRECTIONMANAGER_H
#define FLOW_CORRECTIONMANAGER_H

#include <string>
#include <map>
#include <utility>
#include <QnCorrections/QnCorrectionsManager.h>
#include "Detector.h"
#include "VariableManager.h"
#include "Axis.h"
#include "EventInfo.h"
#include "CorrectionInterface.h"
#include "DataFiller.h"
namespace Qn {
class CorrectionManager {
 public:
  void AddVariable(const std::string &name, const int number) { var_manager_.AddVariable(name, number); }

  void AddCorrectionAxis(const Qn::Axis &variable) { qncorrections_axis_.push_back(variable); }

  void SetEventVariable(const std::string &name) {
    event_variables_->AddVariable(name);
  }

  void SetEventCut() {};

  void AddDetector(const std::string &name,
                   Configuration::DetectorType type,
                   const std::vector<Qn::Axis> &axes = {{"integrated", 1, 0, 1, -1}},
                   int nchannels = 0) {
    std::vector<int> enums;
    enums.reserve(axes.size());
    for (const auto &axis : axes) {
      enums.push_back(var_manager_.FindNum(axis.Name()));
    }
    detectors_.insert(std::make_pair(name, (Detector){type, axes, enums, nchannels}));
  }

  void SetCorrectionSteps(const std::string &name, std::function<void(QnCorrectionsDetectorConfigurationBase *config)> config) {
    detectors_.at(name).SetConfig(std::move(config));
  }

  void CreateDetectors() {
    int nbinsrunning = 0;
    for (auto &pair : detectors_) {
      auto &detector = pair.second;
      for (int ibin = 0; ibin < detector.GetDataContainer()->size(); ++ibin) {
        auto globalid = nbinsrunning + ibin;
        auto frameworkdetector = detector.GenerateDetector(pair.first,globalid,ibin,qncorrections_varset_);
        qncorrections_manager_.AddDetector(frameworkdetector);
      }
      nbinsrunning += detector.GetDataContainer()->size();
    }
  }

  void FillDataToFramework() {
    int nbinsrunning = 0;
    for (auto &pair : detectors_) {
      auto &detector = pair.second.GetDataContainer();
      int ibin = 0;
      for (const std::vector<DataVector> &bin : *detector) {
        auto detectorid = nbinsrunning + ibin;
        ++ibin;
        int idata = 0;
        for (const auto &data : bin) {
          qncorrections_manager_.AddDataVector(detectorid, data.phi, data.weight, idata);
          ++idata;
        }
      }
      nbinsrunning += detector->size();
    }
  }

  void GetQnFromFramework(const std::string &step) {
    int nbinsrunning = 0;
    for (auto &pair : detectors_) {
      auto &detector = pair.second.GetQnDataContainer();
      auto ibin = 0;
      for (auto &bin : *detector) {
        auto name = (pair.first + std::to_string(ibin)).data();
        ++ibin;
        auto vector = qncorrections_manager_.GetDetectorQnVector(name, step.c_str(), step.c_str());
        if (!vector) continue;
        auto method =
            qncorrections_manager_.FindDetector(name)->FindDetectorConfiguration(name)->GetQVectorNormalizationMethod();
        QVector temp(Internal::GetNormalization(method), *vector);
        bin = temp;
      }
      nbinsrunning += detector->size();
    }
  }

  void SaveQVectorsToTree(TTree &tree) {
    for (auto &pair : detectors_) {
      tree.Branch(pair.first.data(), pair.second.GetQnDataContainer().get());
    }
  }

  void SaveEventVariablesToTree(TTree &tree) {
    event_variables_->SetToTree(tree);
  }

  void FillDataToFramework(Qn::Differential::Interface::DataFiller filler) {
    for (auto & detector : detectors_) {
      filler.Fill(detector.first, detector.second);
    }
  }

 private:
  int kMaxCorrectionArrayLength = 200;

  void CalculateCorrectionAxis() {
    qncorrections_varset_ = new QnCorrectionsEventClassVariablesSet(qncorrections_axis_.size());
    for (const auto &axis : qncorrections_axis_) {
      double axisbins[kMaxCorrectionArrayLength];
      auto nbins = axis.size();
      for (int ibin = 0; ibin < nbins; ++ibin) {
        axisbins[ibin] = *(axis.begin() + ibin);
      }
      auto variable = new QnCorrectionsEventClassVariable(var_manager_.FindNum(axis.Name()), axis.Name().data(), nbins, axisbins);
      qncorrections_varset_->Add(variable);
    }
  }

  QnCorrectionsEventClassVariablesSet *qncorrections_varset_;
  std::unique_ptr<Qn::EventInfoF> event_variables_;
  std::vector<Qn::Axis> qncorrections_axis_;
  QnCorrectionsManager qncorrections_manager_;
  VariableManager var_manager_;
  std::map<std::string, Detector> detectors_;
};
}

#endif //FLOW_CORRECTIONMANAGER_H
