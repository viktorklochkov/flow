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
//      if (!vector) continue;
      auto method =
          qncorrections_manager_.FindDetector(name.data())->FindDetectorConfiguration(name.data())->GetQVectorNormalizationMethod();
      QVector temp(GetNormalization(method), vector);
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
//      if (!vector) continue;
      auto method =
          qncorrections_manager_.FindDetector(name.data())->FindDetectorConfiguration(name.data())->GetQVectorNormalizationMethod();
      QVector temp(GetNormalization(method), vector);
      bin = temp;
    }
    nbinsrunning += detector->size();
  }
}
void Qn::CorrectionManager::SetQVectorsToTree() {
  for (auto &pair : detectors_track) {
    out_tree_->Branch(pair.first.data(), pair.second.GetQnDataContainer().get());
  }
  for (auto &pair : detectors_channel) {
    out_tree_->Branch(pair.first.data(), pair.second.GetQnDataContainer().get());
  }
}

void Qn::CorrectionManager::SetEventVariablesToTree() {
  event_variables_->SetToTree(*out_tree_);
}

void Qn::CorrectionManager::SaveHistograms(std::shared_ptr<TFile> file) {
  file->cd();
  qncorrections_manager_.GetOutputHistogramsList()->Write(qncorrections_manager_.GetOutputHistogramsList()->GetName(),
                                                          TObject::kSingleKey);
  qncorrections_manager_.GetQAHistogramsList()->Write(qncorrections_manager_.GetQAHistogramsList()->GetName(),
                                                      TObject::kSingleKey);
  TDirectory *dir = file->mkdir("DetectorQA");
  dir->cd();
  for (auto &det: detectors_channel) {
    auto detdir = dir->mkdir(det.first.data());
    detdir->cd();
    det.second.SaveReport();
  }
  for (auto &det: detectors_track) {
    auto detdir = dir->mkdir(det.first.data());
    detdir->cd();
    det.second.SaveReport();
  }
  TDirectory *evdir = file->mkdir("EventQA");
  evdir->cd();
  event_cuts_->Write("");
  for (auto &histo : event_histograms_) {
    histo->Write(histo->Name());
  }
}
void Qn::CorrectionManager::Initialize(std::shared_ptr<TFile> &in_calibration_file_) {
  SetQVectorsToTree();
  SetEventVariablesToTree();
  CalculateCorrectionAxis();
  CreateDetectors();
  for (auto &det : detectors_track) {
    det.second.Initialize(det.first, *var_manager_);
  }
  for (auto &det : detectors_channel) {
    det.second.Initialize(det.first, *var_manager_);
  }
  event_cuts_->CreateCutReport("Event",1);
  qncorrections_manager_.SetCalibrationHistogramsList(in_calibration_file_.get());
  qncorrections_manager_.SetShouldFillQAHistograms();
  qncorrections_manager_.SetShouldFillOutputHistograms();
  qncorrections_manager_.InitializeQnCorrectionsFramework();
  qncorrections_manager_.SetCurrentProcessListName("correction");
}

void Qn::CorrectionManager::Process(Qn::DataFiller filler) {
  FillEventData(filler);
  if(!event_cuts_->CheckCuts(0)) return;
  event_cuts_->FillReport();
  for (auto &event_var : *event_variables_) {
    event_var.second.SetValue(*(var_manager_->FindVariable(event_var.first).begin()));
  }
  for (auto & histo : event_histograms_) {
    histo->Fill();
  }
  FillData(filler);
  FillDataToFramework();
  qncorrections_manager_.ProcessEvent();
  GetQnFromFramework("latest");
  out_tree_->Fill();
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
  int kMaxCorrectionArrayLength = 200;
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
void Qn::CorrectionManager::AddDetector(const std::string &name,
                                        Qn::DetectorType type,
                                        const std::string &phi_name,
                                        const std::string &weight_name,
                                        const std::vector<Qn::Axis> &axes) {
  Variable phi = var_manager_->FindVariable(phi_name);
  Variable weight = var_manager_->FindVariable(weight_name);
  std::vector<Variable> vars;
  for (const auto &axis : axes) {
    vars.push_back(var_manager_->FindVariable(axis.Name()));
  }
  Detector det(type, axes, phi, weight, vars);
  if (type==DetectorType::Channel) detectors_channel.emplace(std::make_pair(name, std::move(det)));
  if (type==DetectorType::Track) detectors_track.emplace(std::make_pair(name,std::move(det)));
}

void Qn::CorrectionManager::AddHisto1D(const std::string &name,
                                       std::vector<Qn::Axis> axes,
                                       const std::string &weightname) {
  auto pair = Create1DHisto(name, axes, weightname);
  auto histo = std::make_unique<QAHisto1D>(pair.first, pair.second);
  try { detectors_track.at(name).AddHistogram(std::move(histo)); }
  catch (std::out_of_range &) {
    try { detectors_channel.at(name).AddHistogram(std::move(histo)); }
    catch (std::out_of_range &) {
      throw std::out_of_range(
          name + " was not found in the list of detectors. It needs to be created before a histogram can be added.");
    }
  }
}

void Qn::CorrectionManager::AddHisto2D(const std::string &name,
                                       std::vector<Qn::Axis> axes,
                                       const std::string &weightname) {
  auto pair = Create2DHisto(name, axes, weightname);
  auto histo = std::make_unique<QAHisto2D>(pair.first, pair.second);
  try { detectors_track.at(name).AddHistogram(std::move(histo)); }
  catch (std::out_of_range &) {
    try { detectors_channel.at(name).AddHistogram(std::move(histo)); }
    catch (std::out_of_range &) {
      throw std::out_of_range(
          name + " was not found in the list of detectors. It needs to be created before a histogram can be added.");
    }
  }
}

void Qn::CorrectionManager::FillEventData(Qn::DataFiller filler) {
  filler.FillEventInfo(var_manager_);
}

void Qn::CorrectionManager::FillData(Qn::DataFiller filler) {
  filler.FillDetectors(detectors_channel, detectors_track, var_manager_);
  var_manager_->FillToQnCorrections(qncorrections_manager_.GetDataPointer());
  for (auto &dp : detectors_channel) {dp.second.FillReport();}
  for (auto &dp : detectors_track) {dp.second.FillReport();}
}

std::pair<std::array<Qn::Variable,2>,TH1F> Qn::CorrectionManager::Create1DHisto(const std::string &name,
                                          std::vector<Qn::Axis> axes,
                                          const std::string &weightname) {
  std::string spacer("_");
  auto hist_name = (name + spacer + axes[0].Name() + spacer + weightname);
  const int size = axes[0].size();
  try { var_manager_->FindVariable(axes[0].Name()); }
  catch (std::out_of_range &) {
    std::cout << "QAHistogram " << name << ": Variable " << axes[0].Name()
              << " not found. Creating new channel variable." << std::endl;
    var_manager_->CreateChannelVariable(axes[0].Name(), size);
  }
  float upper_edge = axes[0].GetUpperBinEdge(size - 1);
  float lower_edge = axes[0].GetLowerBinEdge(0);
  TH1F histo(hist_name.data(), (std::string(";") + axes[0].Name()).data(), size, lower_edge, upper_edge);
  std::array<Variable, 2>
      arr = {{var_manager_->FindVariable(axes[0].Name()), var_manager_->FindVariable(weightname)}};
  return std::make_pair(arr, histo);
}
std::pair<std::array<Qn::Variable,3>,TH2F> Qn::CorrectionManager::Create2DHisto(const std::string &name,
                                          std::vector<Qn::Axis> axes,
                                          const std::string &weightname) {
  std::string spacer("_");
  auto hist_name = (name + spacer + axes[0].Name() + spacer + axes[1].Name() + spacer + weightname);
  const int size_x = axes[0].size();
  const int size_y = axes[1].size();
  try { var_manager_->FindVariable(axes[0].Name()); }
  catch (std::out_of_range &) {
    std::cout << "QAHistogram " << name << ": Variable " << axes[0].Name()
              << " not found. Creating new channel variable." << std::endl;
    var_manager_->CreateChannelVariable(axes[0].Name(), size_x);
  }
  try { var_manager_->FindVariable(axes[1].Name()); }
  catch (std::out_of_range &) {
    std::cout << "QAHistogram " << name << ": Variable " << axes[1].Name()
              << " not found. Creating new channel variable." << std::endl;
    var_manager_->CreateChannelVariable(axes[1].Name(), size_y);
  }
  float upper_edge_x = axes[0].GetUpperBinEdge(size_x - 1);
  float lower_edge_x = axes[0].GetLowerBinEdge(0);
  float upper_edge_y = axes[1].GetUpperBinEdge(size_y - 1);
  float lower_edge_y = axes[1].GetLowerBinEdge(0);
  TH2F histo(hist_name.data(), (std::string(";") + axes[0].Name() + std::string(";") + axes[1].Name()).data(),
             size_x, lower_edge_x, upper_edge_x, size_y, lower_edge_y, upper_edge_y);
  std::array<Variable, 3>
      arr = {{var_manager_->FindVariable(axes[0].Name()), var_manager_->FindVariable(axes[1].Name()),
              var_manager_->FindVariable(weightname)}};
  return std::make_pair(arr, histo);
}
void Qn::CorrectionManager::AddEventHisto2D(std::vector<Qn::Axis> axes, const std::string &weightname) {
  std::string name("Ev");
  auto pair = Create2DHisto(name, std::move(axes), weightname);
  event_histograms_.push_back(std::make_unique<QAHisto2D>(pair.first, pair.second));
}
void Qn::CorrectionManager::AddEventHisto1D(std::vector<Qn::Axis> axes, const std::string &weightname) {
  std::string name("Ev");
  auto pair = Create1DHisto(name, std::move(axes), weightname);
  event_histograms_.push_back(std::make_unique<QAHisto1D>(pair.first, pair.second));
}
