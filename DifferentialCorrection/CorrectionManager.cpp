//
// Created by Lukas Kreis on 16.01.18.
//

#include "CorrectionManager.h"

void Qn::CorrectionManager::SetCorrectionSteps(const std::string &name,
                                               std::function<void(QnCorrectionsDetectorConfigurationBase *config)> config) {
  try { detectors_track.at(name).SetConfig(std::move(config)); }
  catch (std::out_of_range &) {
    try { detectors_channel.at(name).SetConfig(std::move(config)); }
    catch (std::out_of_range &) {
      throw std::out_of_range(
          name + " was not found in the list of detectors. It needs to be created before it can be configured.");
    }
  }
}
void Qn::CorrectionManager::CreateDetectors() {
  int nbinsrunning = 0;
  for (auto &pair : detectors_track) {
    auto &detector = pair.second;
    for (unsigned int ibin = 0; ibin < detector.GetDataContainer()->size(); ++ibin) {
      auto globalid = nbinsrunning + ibin;
      auto frameworkdetector = detector.GenerateDetector(pair.first, globalid, ibin, qncorrections_varset_);
      qncorrections_manager_.AddDetector(frameworkdetector);
    }
    nbinsrunning += detector.GetDataContainer()->size();
  }
  for (auto &pair : detectors_channel) {
    auto &detector = pair.second;
    for (unsigned int ibin = 0; ibin < detector.GetDataContainer()->size(); ++ibin) {
      auto globalid = nbinsrunning + ibin;
      auto frameworkdetector = detector.GenerateDetector(pair.first, globalid, ibin, qncorrections_varset_);
      qncorrections_manager_.AddDetector(frameworkdetector);
    }
    nbinsrunning += detector.GetDataContainer()->size();
  }
}
void Qn::CorrectionManager::FillDataToFramework() {
  int nbinsrunning = 0;
  for (auto &pair : detectors_channel) {
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
  for (auto &pair : detectors_track) {
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
void Qn::CorrectionManager::GetQnFromFramework(const std::string &step) {
  int nbinsrunning = 0;
  for (auto &pair : detectors_track) {
    auto &detector = pair.second.GetQnDataContainer();
    auto ibin = 0;
    for (auto &bin : *detector) {
      std::string name;
      if (detector->IsIntegrated()) {
        name = pair.first;
      } else {
        name = (pair.first + std::to_string(ibin));
      }
      ++ibin;
      auto vector = qncorrections_manager_.GetDetectorQnVector(name.data(), step.c_str(), step.c_str());
      if (!vector) continue;
      auto method =
          qncorrections_manager_.FindDetector(name.data())->FindDetectorConfiguration(name.data())->GetQVectorNormalizationMethod();
      QVector temp(GetNormalization(method), *vector);
      bin = temp;
    }
    nbinsrunning += detector->size();
  }
  for (auto &pair : detectors_channel) {
    auto &detector = pair.second.GetQnDataContainer();
    auto ibin = 0;
    for (auto &bin : *detector) {
      std::string name;
      if (detector->IsIntegrated()) {
        name = pair.first;
      } else {
        name = (pair.first + std::to_string(ibin));
      }
      ++ibin;
      auto vector = qncorrections_manager_.GetDetectorQnVector(name.data(), step.c_str(), step.c_str());
      if (!vector) continue;
      auto method =
          qncorrections_manager_.FindDetector(name.data())->FindDetectorConfiguration(name.data())->GetQVectorNormalizationMethod();
      QVector temp(GetNormalization(method), *vector);
      bin = temp;
    }
    nbinsrunning += detector->size();
  }
}
void Qn::CorrectionManager::SaveQVectorsToTree(TTree &tree) {
  for (auto &pair : detectors_track) {
    tree.Branch(pair.first.data(), pair.second.GetQnDataContainer().get());
  }
  for (auto &pair : detectors_channel) {
    tree.Branch(pair.first.data(), pair.second.GetQnDataContainer().get());
  }
}

void Qn::CorrectionManager::SaveEventVariablesToTree(TTree &tree) {
  event_variables_->SetToTree(tree);
}

void Qn::CorrectionManager::SaveCorrectionHistograms(std::shared_ptr<TFile> file) {
  file->cd();
  qncorrections_manager_.GetOutputHistogramsList()->Write(qncorrections_manager_.GetOutputHistogramsList()->GetName(),
                                                          TObject::kSingleKey);
  qncorrections_manager_.GetQAHistogramsList()->Write(qncorrections_manager_.GetQAHistogramsList()->GetName(),
                                                      TObject::kSingleKey);
  TDirectory *dir = file->mkdir("Cut_Reports");
  dir->cd();
  for (auto &det: detectors_channel) {
    det.second.SaveReport();
  }
  for (auto &det: detectors_track) {
    det.second.SaveReport();
  }
}
void Qn::CorrectionManager::Initialize(std::shared_ptr<TFile> &in_calibration_file_) {
  CalculateCorrectionAxis();
  CreateDetectors();
  for (auto &det : detectors_channel) {
    det.second.Initialize(det.first, *var_manager_);
  }
  for (auto &det : detectors_track) {
    det.second.Initialize(det.first, *var_manager_);
  }
  qncorrections_manager_.SetCalibrationHistogramsList(in_calibration_file_.get());
  qncorrections_manager_.SetShouldFillQAHistograms();
  qncorrections_manager_.SetShouldFillOutputHistograms();
  qncorrections_manager_.InitializeQnCorrectionsFramework();
  qncorrections_manager_.SetCurrentProcessListName("correction");
}
void Qn::CorrectionManager::Process(Qn::DataFiller filler) {
  FillData(filler);
  for (auto &event_var : *event_variables_) {
    event_var.second.SetValue(*(var_manager_->FindVariable(event_var.first).begin()));
  }
  FillDataToFramework();
  qncorrections_manager_.ProcessEvent();
  GetQnFromFramework("latest");
}
void Qn::CorrectionManager::Reset() {
  qncorrections_manager_.ClearEvent();
  for (auto &det : detectors_channel) {
    det.second.ClearData();
  }
  for (auto &det : detectors_track) {
    det.second.ClearData();
  }
  event_variables_->Reset();
}
void Qn::CorrectionManager::Finalize() { qncorrections_manager_.FinalizeQnCorrectionsFramework(); }

void Qn::CorrectionManager::CalculateCorrectionAxis() {
  qncorrections_varset_ = new QnCorrectionsEventClassVariablesSet(qncorrections_axis_.size());
  for (const auto &axis : qncorrections_axis_) {
    double axisbins[kMaxCorrectionArrayLength];
    auto nbins = axis.size();
    for (unsigned int ibin = 0; ibin < nbins + 1; ++ibin) {
      axisbins[ibin] = *(axis.begin() + ibin);
    }
    auto variable =
        new QnCorrectionsEventClassVariable(var_manager_->FindNum(axis.Name()), axis.Name().data(), nbins, axisbins);
    qncorrections_varset_->Add(variable);
  }
}
