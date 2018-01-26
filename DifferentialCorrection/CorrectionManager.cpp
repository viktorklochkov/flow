//
// Created by Lukas Kreis on 16.01.18.
//

#include "CorrectionManager.h"
void Qn::CorrectionManager::AddHist2D(const std::string &x, int xbins, float xlo, float xhi,
                                      const std::string &y, int ybins, float ylo, float yhi) {
  std::vector<float> x_edges;
  for (int i = 0; i < xbins + 1; ++i) {
    float bin_width = (xhi - xlo)/(float) xbins;
    x_edges.push_back(xlo + i*bin_width);
  }
  std::vector<float> y_edges;
  for (int i = 0; i < ybins + 1; ++i) {
    float bin_width = (yhi - ylo)/(float) ybins;
    y_edges.push_back(ylo + i*bin_width);
  }
  histogram_manager_.AddHist2D(x, x_edges, y, y_edges);
}
void Qn::CorrectionManager::AddDetector(const std::string &name, Qn::DetectorType type,
                                        const std::vector<Qn::Axis> &axes, int nchannels) {
  std::vector<int> enums;
  enums.reserve(axes.size());
  for (const auto &axis : axes) {
    enums.push_back(var_manager_->FindNum(axis.Name()));
  }
  Detector detector(type, axes, enums, nchannels);
  detectors_.insert(std::make_pair(name, std::move(detector)));
}
void Qn::CorrectionManager::AddDetector(const std::string &name, Qn::DetectorType type, int nchannels) {
  std::vector<int> enums;
  Detector detector(type, nchannels);
  detectors_.insert(std::make_pair(name, std::move(detector)));
}
void Qn::CorrectionManager::SetCorrectionSteps(const std::string &name,
                                               std::function<void(QnCorrectionsDetectorConfigurationBase *config)> config) {
  try { detectors_.at(name).SetConfig(std::move(config)); }
  catch (std::out_of_range &) {
    throw std::out_of_range(
        name + " was not found in the list of detectors. It needs to be created before it can be configured.");
  }
}
void Qn::CorrectionManager::CreateDetectors() {
  int nbinsrunning = 0;
  for (auto &pair : detectors_) {
    auto &detector = pair.second;
    for (int ibin = 0; ibin < detector.GetDataContainer()->size(); ++ibin) {
      auto globalid = nbinsrunning + ibin;
      auto frameworkdetector = detector.GenerateDetector(pair.first, globalid, ibin, qncorrections_varset_);
      qncorrections_manager_.AddDetector(frameworkdetector);
    }
    nbinsrunning += detector.GetDataContainer()->size();
  }
}
void Qn::CorrectionManager::FillDataToFramework() {
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
void Qn::CorrectionManager::GetQnFromFramework(const std::string &step) {
  int nbinsrunning = 0;
  for (auto &pair : detectors_) {
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
  for (auto &pair : detectors_) {
    tree.Branch(pair.first.data(), pair.second.GetQnDataContainer().get());
  }
}
void Qn::CorrectionManager::SaveQaHistograms() {
  histogram_manager_.GetList()->Write("QAHistograms", TObject::kSingleKey);
}
void Qn::CorrectionManager::SaveEventVariablesToTree(TTree &tree) {
  event_variables_->SetToTree(tree);
}
void Qn::CorrectionManager::FillDataToFramework(Qn::Differential::Interface::DataFiller filler) {
  filler.Fill(detectors_);
}
void Qn::CorrectionManager::SaveCorrectionHistograms() {
  qncorrections_manager_.GetOutputHistogramsList()->Write(qncorrections_manager_.GetOutputHistogramsList()->GetName(),
                                                          TObject::kSingleKey);
  qncorrections_manager_.GetQAHistogramsList()->Write(qncorrections_manager_.GetQAHistogramsList()->GetName(),
                                                      TObject::kSingleKey);
}
void Qn::CorrectionManager::Initialize(std::shared_ptr<TFile> &in_calibration_file_) {
  CalculateCorrectionAxis();
  CreateDetectors();
  histogram_manager_.CreateHistograms();
  qncorrections_manager_.SetCalibrationHistogramsList(in_calibration_file_.get());
  qncorrections_manager_.SetShouldFillQAHistograms();
  qncorrections_manager_.SetShouldFillOutputHistograms();
  qncorrections_manager_.InitializeQnCorrectionsFramework();
  qncorrections_manager_.SetCurrentProcessListName("correction");
}
void Qn::CorrectionManager::Process() {
  FillDataToFramework();
  for (auto &event_var : *event_variables_) {
    event_var.second.SetValue(qncorrections_manager_.GetDataContainer()[var_manager_->FindNum(event_var.first)]);
  }
  histogram_manager_.FillHist1D(qncorrections_manager_.GetDataContainer());
  qncorrections_manager_.ProcessEvent();
  GetQnFromFramework("latest");
}
void Qn::CorrectionManager::Reset() {
  qncorrections_manager_.ClearEvent();
  for (auto &det : detectors_) {
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
    for (int ibin = 0; ibin < nbins + 1; ++ibin) {
      axisbins[ibin] = *(axis.begin() + ibin);
    }
    auto variable =
        new QnCorrectionsEventClassVariable(var_manager_->FindNum(axis.Name()), axis.Name().data(), nbins, axisbins);
    qncorrections_varset_->Add(variable);
  }
}
void Qn::CorrectionManager::AddHist1D(const std::string &x, int nbins, float xlo, float xhi) {
  std::vector<float> bin_edges;
  for (int i = 0; i < nbins + 1; ++i) {
    float bin_width = (xhi - xlo)/(float) nbins;
    bin_edges.push_back(xlo + i*bin_width);
  }
  histogram_manager_.AddHist1D(x, bin_edges);
}
void Qn::CorrectionManager::AddHist1D(const std::string &x, const std::vector<float> &bin_edges) {
  histogram_manager_.AddHist1D(x, bin_edges);
}
void Qn::CorrectionManager::AddHist2D(const std::string &x,
                                      const std::vector<float> &x_edges,
                                      const std::string &y,
                                      const std::vector<float> &y_edges) {
  histogram_manager_.AddHist2D(x, x_edges, y, y_edges);
}
