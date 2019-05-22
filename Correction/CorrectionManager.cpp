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

#include "CorrectionManager.h"
#include "TList.h"

void Qn::CorrectionManager::SetCorrectionSteps(const std::string &name,
                                               std::function<void(DetectorConfiguration *config)> config) {

  if (detectors_track_.find(name)!=detectors_track_.end()) {
    detectors_track_.at(name)->SetConfig(std::move(config));
  } else if (detectors_channel_.find(name)!=detectors_channel_.end()) {
    detectors_channel_.at(name)->SetConfig(std::move(config));
  } else {
    throw std::out_of_range(
        name + " was not found in the list of detectors. It needs to be created before it can be configured.");
  }
}

void Qn::CorrectionManager::AddHisto1D(const std::string &name,
                                       const Qn::Axis &axis,
                                       const std::string &weightname) {
  auto histo = Create1DHisto(name, axis, weightname);
  if (detectors_track_.find(name)!=detectors_track_.end()) {
    detectors_track_.at(name)->AddHistogram(std::move(histo));
  } else if (detectors_channel_.find(name)!=detectors_channel_.end()) {
    detectors_channel_.at(name)->AddHistogram(std::move(histo));
  } else {
    throw std::out_of_range(
        name + " was not found in the list of detectors. It needs to be created before it can be configured.");
  }
}

void Qn::CorrectionManager::AddHisto2D(const std::string &name,
                                       const std::vector<Qn::Axis> &axes,
                                       const std::string &weightname) {
  auto histo = Create2DHisto(name, axes, weightname);
  if (detectors_track_.find(name)!=detectors_track_.end()) {
    detectors_track_.at(name)->AddHistogram(std::move(histo));
  } else if (detectors_channel_.find(name)!=detectors_channel_.end()) {
    detectors_channel_.at(name)->AddHistogram(std::move(histo));
  } else {
    throw std::out_of_range(
        name + " was not found in the list of detectors. It needs to be created before it can be configured.");
  }
}

void Qn::CorrectionManager::AddEventHisto2D(const std::vector<Qn::Axis> &axes,
                                            const Qn::Axis &axis,
                                            const std::string &weightname) {
  event_histograms_.push_back(Create2DHisto("Ev", axes, weightname, axis));
}

void Qn::CorrectionManager::AddEventHisto2D(const std::vector<Qn::Axis> &axes, const std::string &weightname) {
  event_histograms_.push_back(Create2DHisto("Ev", axes, weightname));
}

void Qn::CorrectionManager::AddEventHisto1D(const Qn::Axis &axes, const std::string &weightname) {
  event_histograms_.push_back(Create1DHisto("Ev", axes, weightname));
}

void Qn::CorrectionManager::Initialize(TFile *in_calibration_file_) {
  if (out_tree_) {
    for (auto &pair : detectors_track_) {
      out_tree_->Branch(pair.first.data(), pair.second->GetQnDataContainer().get());
    }
    for (auto &pair : detectors_channel_) {
      out_tree_->Branch(pair.first.data(), pair.second->GetQnDataContainer().get());
    }
    var_manager_->SetOutputToTree(out_tree_);
  }
  CalculateCorrectionAxis();
  CreateDetectors();
  for (auto &det : detectors_track_) {
    det.second->InitializeCutReports();
  }
  for (auto &det : detectors_channel_) {
    det.second->InitializeCutReports();
  }
  event_cuts_->CreateCutReport("Event", 1);
  qnc_calculator_.SetCalibrationHistogramsList(in_calibration_file_);
  qnc_calculator_.SetShouldFillQAHistograms();
  qnc_calculator_.SetShouldFillOutputHistograms();
  qnc_calculator_.InitializeQnCorrectionsFramework();
  unsigned nbinsrunning = 0;
  for (auto &pair : detectors_track_) {
    auto &detector = pair.second->GetDataContainer();
    int ibin = 0;
    for (auto &bin : *detector) {
      auto detectorid = nbinsrunning + ibin;
      ++ibin;
      auto datavector = qnc_calculator_.FindDetector(detectorid)->GetInputDataBank(0);
      bin.array = datavector;
    }
    pair.second->FillReport();
    nbinsrunning += detector->size();
  }
  for (auto &pair : detectors_channel_) {
    auto &detector = pair.second->GetDataContainer();
    int ibin = 0;
    for (auto &bin : *detector) {
      auto detectorid = nbinsrunning + ibin;
      ++ibin;
      auto datavector = qnc_calculator_.FindDetector(detectorid)->GetInputDataBank(0);
      bin.array = datavector;
    }
    pair.second->FillReport();
    nbinsrunning += detector->size();
  }
}

void Qn::CorrectionManager::ProcessEvent() {
  if (event_cuts_->CheckCuts(0)) event_passed_cuts_ = true;
  if (event_passed_cuts_) {
    event_cuts_->FillReport();
    var_manager_->UpdateOutVariables();
  }
}

void Qn::CorrectionManager::ProcessQnVectors() {
  if (event_passed_cuts_) {
    for (auto &histo : event_histograms_) {
      histo->Fill();
    }
    var_manager_->FillToQnCorrections(qnc_calculator_.GetDataPointer());
    qnc_calculator_.ProcessEvent();
    for (auto &pair : detectors_track_) {
      pair.second->GetCorrectedQVectors();
      pair.second->FillReport();
    }
    for (auto &pair : detectors_channel_) {
      pair.second->GetCorrectedQVectors();
      pair.second->FillReport();
    }
    if (out_tree_) out_tree_->Fill();
  }
}

void Qn::CorrectionManager::Reset() {
  qnc_calculator_.ClearEvent();
  event_passed_cuts_ = false;
  for (auto &det : detectors_channel_) {
    det.second->ClearData();
  }
  for (auto &det : detectors_track_) {
    det.second->ClearData();
  }
}

void Qn::CorrectionManager::Finalize() { qnc_calculator_.FinalizeQnCorrectionsFramework(); }

TList *Qn::CorrectionManager::GetEventAndDetectorQAList() {
  qa_list_ = new TList();
  qa_list_->SetOwner(kTRUE);
  qa_list_->SetName("QA");
  for (auto &det: detectors_channel_) {
    auto detlist = det.second->GetReportList();
    detlist->SetName(det.first.data());
    qa_list_->Add(detlist);
  }
  for (auto &det: detectors_track_) {
    auto detlist = det.second->GetReportList();
    detlist->SetName(det.first.data());
    qa_list_->Add(detlist);
  }
  auto evlist = new TList();
  evlist->SetName("Event Histograms");
  event_cuts_->AddToList(evlist);
  for (auto &histo : event_histograms_) {
    histo->AddToList(evlist);
  }
  qa_list_->Add(evlist);
  return qa_list_;
}

void Qn::CorrectionManager::CalculateCorrectionAxis() {
  qnc_varset_ = std::make_unique<EventClassVariablesSet>(correction_axes_.size());
  for (const auto &axis : correction_axes_) {
    double axisbins[kMaxCorrectionArrayLength];
    auto nbins = axis.size();
    for (unsigned int ibin = 0; ibin < nbins + 1; ++ibin) {
      axisbins[ibin] = *(axis.begin() + ibin);
    }
    std::unique_ptr<EventClassVariable>
        variable(new EventClassVariable(var_manager_->FindNum(axis.Name()), axis.Name().data(), nbins, axisbins));
    qnc_evvars_.push_back(std::move(variable));
    qnc_varset_->Add(qnc_evvars_.back().get());
  }
}

void Qn::CorrectionManager::CreateDetectors() {
  int nbinsrunning = 0;
  for (auto &pair : detectors_track_) {
    auto &detector = pair.second;
    for (unsigned int ibin = 0; ibin < detector->GetDataContainer()->size(); ++ibin) {
      auto globalid = nbinsrunning + ibin;
      auto frameworkdetector = detector->GenerateDetector(globalid, ibin, qnc_varset_.get());
      qnc_calculator_.AddDetector(frameworkdetector);
    }
    nbinsrunning += detector->GetDataContainer()->size();
  }
  for (auto &pair : detectors_channel_) {
    auto &detector = pair.second;
    for (unsigned int ibin = 0; ibin < detector->GetDataContainer()->size(); ++ibin) {
      auto globalid = nbinsrunning + ibin;
      auto frameworkdetector = detector->GenerateDetector(globalid, ibin, qnc_varset_.get());
      qnc_calculator_.AddDetector(frameworkdetector);
    }
    nbinsrunning += detector->GetDataContainer()->size();
  }
}

/**
 * Helper function to create the QA histograms
 * @param name name of the histogram
 * @param axes axes used for the histogram
 * @param weightname name of the weight
 * @return unique pointer to the histogram
 */
std::unique_ptr<Qn::QAHisto1DPtr> Qn::CorrectionManager::Create1DHisto(const std::string &name,
                                                                       const Qn::Axis &axis,
                                                                       const std::string &weightname) {
  auto hist_name = name + "_" + axis.Name() + "_" + weightname;
  auto axisname = std::string(";") + axis.Name();
  auto size = static_cast<const int>(axis.size());
  try { var_manager_->FindVariable(axis.Name()); }
  catch (std::out_of_range &) {
    std::cout << "QAHistogram " << name << ": Variable " << axis.Name()
              << " not found. Creating new channel variable." << std::endl;
    var_manager_->CreateChannelVariable(axis.Name(), size);
  }
  float upper_edge = axis.GetLastBinEdge();
  float lower_edge = axis.GetFirstBinEdge();
  std::array<Variable, 2> arr = {{var_manager_->FindVariable(axis.Name()), var_manager_->FindVariable(weightname)}};
  return std::make_unique<QAHisto1DPtr>(arr, new TH1F(hist_name.data(), axisname.data(), size, lower_edge, upper_edge));
}

/**
 * Helper function to create the QA histograms
 * @param name name of the histogram
 * @param axes axes used for the histogram
 * @param weightname name of the weight
 * @return unique pointer to the histogram
 */
std::unique_ptr<Qn::QAHisto2DPtr> Qn::CorrectionManager::Create2DHisto(const std::string &name,
                                                                       const std::vector<Qn::Axis> &axes,
                                                                       const std::string &weightname) {
  auto hist_name = name + "_" + axes[0].Name() + "_" + axes[1].Name() + "_" + weightname;
  auto axisname = std::string(";") + axes[0].Name() + std::string(";") + axes[1].Name();
  auto size_x = static_cast<const int>(axes[0].size());
  auto size_y = static_cast<const int>(axes[1].size());
  for (const auto &axis : axes) {
    try { var_manager_->FindVariable(axis.Name()); }
    catch (std::out_of_range &) {
      std::cout << "QAHistogram " << name << ": Variable " << axis.Name()
                << " not found. Creating new channel variable." << std::endl;
      var_manager_->CreateChannelVariable(axis.Name(), axis.size());
    }
  }
  auto upper_edge_x = axes[0].GetLastBinEdge();
  auto lower_edge_x = axes[0].GetFirstBinEdge();
  auto upper_edge_y = axes[1].GetLastBinEdge();
  auto lower_edge_y = axes[1].GetFirstBinEdge();
  std::array<Variable, 3>
      arr = {{var_manager_->FindVariable(axes[0].Name()), var_manager_->FindVariable(axes[1].Name()),
              var_manager_->FindVariable(weightname)}};
  auto histo = new TH2F(hist_name.data(),
                        axisname.data(),
                        size_x,
                        lower_edge_x,
                        upper_edge_x,
                        size_y,
                        lower_edge_y,
                        upper_edge_y);
  return std::make_unique<QAHisto2DPtr>(arr, histo);
}

/**
 * Helper function to create the QA histograms
 * @param name name of the histogram
 * @param axes axes used for the histogram
 * @param weightname name of the weight
 * @return unique pointer to the histogram
 */
std::unique_ptr<Qn::QAHisto2DPtr> Qn::CorrectionManager::Create2DHisto(const std::string &name,
                                                                       const std::vector<Qn::Axis> &axes,
                                                                       const std::string &weightname,
                                                                       const Qn::Axis &histaxis) {
  auto hist_name = name + "_" + axes[0].Name() + "_" + axes[1].Name() + "_" + weightname;
  auto axisname = std::string(";") + axes[0].Name() + std::string(";") + axes[1].Name();
  auto size_x = static_cast<const int>(axes[0].size());
  auto size_y = static_cast<const int>(axes[1].size());
  for (const auto &axis : axes) {
    try { var_manager_->FindVariable(axis.Name()); }
    catch (std::out_of_range &) {
      std::cout << "QAHistogram " << name << ": Variable " << axis.Name()
                << " not found. Creating new channel variable." << std::endl;
      var_manager_->CreateChannelVariable(axis.Name(), axis.size());
    }
  }
  auto upper_edge_x = axes[0].GetLastBinEdge();
  auto lower_edge_x = axes[0].GetFirstBinEdge();
  auto upper_edge_y = axes[1].GetLastBinEdge();
  auto lower_edge_y = axes[1].GetFirstBinEdge();
  std::array<Variable, 3>
      arr = {{var_manager_->FindVariable(axes[0].Name()), var_manager_->FindVariable(axes[1].Name()),
              var_manager_->FindVariable(weightname)}};
  auto histo = new TH2F(hist_name.data(),
                        axisname.data(),
                        size_x,
                        lower_edge_x,
                        upper_edge_x,
                        size_y,
                        lower_edge_y,
                        upper_edge_y);
  auto haxis = std::make_unique<Qn::Axis>(histaxis);
  auto haxisvar = var_manager_->FindVariable(histaxis.Name());
  return std::make_unique<QAHisto2DPtr>(arr, histo, std::move(haxis), haxisvar);
}


