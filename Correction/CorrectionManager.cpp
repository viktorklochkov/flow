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

namespace Qn {

void CorrectionManager::InitializeCorrections() {
  // Connects the correction histogram list
  correction_input_file_ = std::make_unique<TFile>(correction_input_file_name_.data(), "READ");
  if (correction_input_file_ && !correction_input_file_->IsZombie()) {
    auto input = dynamic_cast<TList *>(correction_input_file_->FindObjectAny(kCorrectionListName));
    correction_input_.reset(input);
    correction_input_->SetOwner(kTRUE);
  }
  // Prepares the correctionsteps
  detectors_.CreateSupportQVectors();
  correction_output = std::make_unique<TList>();
  correction_output->SetName(kCorrectionListName);
  correction_output->SetOwner(kTRUE);
}

void CorrectionManager::SetCurrentRunName(const std::string &name) {
  runs_.SetCurrentRun(name);
  if (!runs_.empty()) {
    auto current_run = new TList();
    current_run->SetName(runs_.GetCurrent().data());
    current_run->SetOwner(true);
    correction_output->Add(current_run);
    detectors_.CreateCorrectionHistograms(current_run);
  }
  if (correction_input_) {
    auto current_run = (TList *) correction_input_->FindObject(runs_.GetCurrent().data());
    if (current_run) {
      detectors_.AttachCorrectionInput(current_run);
    }
  }
  detectors_.IncludeQnVectors();
}

void CorrectionManager::AttachQAHistograms() {
  correction_qa_histos_ = std::make_unique<TList>();
  correction_qa_histos_->SetName("QA_histograms");
  correction_qa_histos_->SetOwner(true);
  detectors_.AttachQAHistograms(correction_qa_histos_.get(), fill_qa_histos_, fill_validation_qa_histos_, &variable_manager_);
  auto event_qa_list = new TList();
  event_qa_list->SetName("event_QA");
  event_cuts_.AddToList(event_qa_list);
  for (auto &histo : event_histograms_) {
    histo->AddToList(event_qa_list);
  }
  correction_qa_histos_->Add(event_qa_list);
}

void CorrectionManager::InitializeOnNode() {
  variable_manager_.InitializeOnNode();
  for (const auto &axis : correction_axes_callback_) {
    correction_axes_.Add(axis(&variable_manager_));
  }
  // Creates histograms
  for (const auto &callback : event_histograms_callback_) {
    event_histograms_.push_back(callback(&variable_manager_));
  }
  detectors_.InitializeOnNode(this);
  event_cuts_.CreateCutReport("Event Cut Report");
  InitializeCorrections();
  AttachQAHistograms();
  if (fill_output_tree_ && out_tree_) {
    detectors_.SetOutputTree(out_tree_);
    variable_manager_.SetOutputTree(out_tree_);
  }
}

bool CorrectionManager::ProcessEvent() {
  event_passed_cuts_ = event_cuts_.CheckCuts(0);
  if (event_passed_cuts_) {
    event_cuts_.FillReport();
    variable_manager_.UpdateOutVariables();
    for (auto &histo : event_histograms_) {
      histo->Fill();
    }
  }
  return event_passed_cuts_;
}

void CorrectionManager::ProcessCorrections() {
  if (event_passed_cuts_) {
    detectors_.ProcessCorrections();
    detectors_.FillReport();
    if (fill_output_tree_) out_tree_->Fill();
  }
}

void CorrectionManager::Reset() {
  event_passed_cuts_ = false;
  detectors_.ResetDetectors();
}

void CorrectionManager::Finalize() {
  auto calibration_list = (TList *) correction_output->FindObject(kCorrectionListName);
  if (calibration_list) {
    correction_output->Add(calibration_list->Clone("all"));
  }
}

/**
 * Helper function to create the QA histograms
 * @param name name of the histogram
 * @param axes axes used for the histogram
 * @param weightname name of the weight
 * @return unique pointer to the histogram
 */
CorrectionManager::HistogramCallBack CorrectionManager::Create1DHisto(const std::string &name,
                                                                      const AxisD axis,
                                                                      const std::string &weight) {
  auto callback = [&name, axis, weight](InputVariableManager *var) {
    auto hist_name = name + "_" + axis.Name() + "_" + weight;
    auto axisname = std::string(";") + axis.Name();
    auto size = static_cast<const int>(axis.size());
    try { var->FindVariable(axis.Name()); }
    catch (std::out_of_range &) {
      std::cout << "QAHistogram " << name << ": Variable " << axis.Name()
                << " not found. Creating new channel variable."
                << std::endl;
      var->CreateChannelVariable(axis.Name(), axis.size());
    }
    float upper_edge = axis.GetLastBinEdge();
    float lower_edge = axis.GetFirstBinEdge();
    std::array<InputVariable, 2>
        arr = {{var->FindVariable(axis.Name()), var->FindVariable(weight)}};
    return std::make_unique<QAHisto1DPtr>(arr,
                                          new TH1F(hist_name.data(), axisname.data(), size, lower_edge, upper_edge));
  };
  return callback;
}

/**
 * Helper function to create the QA histograms
 * @param name name of the histogram
 * @param axes axes used for the histogram
 * @param weightname name of the weight
 * @return unique pointer to the histogram
 */
CorrectionManager::HistogramCallBack CorrectionManager::Create2DHisto(const std::string &name,
                                                                      const std::vector<AxisD> axes,
                                                                      const std::string &weight) {
  auto callback = [&name, axes, weight](InputVariableManager *var) {
    auto hist_name = name + "_" + axes[0].Name() + "_" + axes[1].Name() + "_" + weight;
    auto axisname = std::string(";") + axes[0].Name() + std::string(";") + axes[1].Name();
    auto size_x = static_cast<const int>(axes[0].size());
    auto size_y = static_cast<const int>(axes[1].size());
    for (const auto &axis : axes) {
      try { var->FindVariable(axis.Name()); }
      catch (std::out_of_range &) {
        std::cout << "QAHistogram " << name << ": Variable " << axis.Name()
                  << " not found. Creating new channel variable." << std::endl;
        var->CreateChannelVariable(axis.Name(), axis.size());
      }
    }
    auto upper_edge_x = axes[0].GetLastBinEdge();
    auto lower_edge_x = axes[0].GetFirstBinEdge();
    auto upper_edge_y = axes[1].GetLastBinEdge();
    auto lower_edge_y = axes[1].GetFirstBinEdge();
    std::array<InputVariable, 3>
        arr = {{var->FindVariable(axes[0].Name()), var->FindVariable(axes[1].Name()),
                var->FindVariable(weight)}};
    auto histo = new TH2F(hist_name.data(), axisname.data(),
                          size_x, lower_edge_x, upper_edge_x,
                          size_y, lower_edge_y, upper_edge_y);
    return std::make_unique<QAHisto2DPtr>(arr, histo);
  };
  return callback;
}

/**
 * Helper function to create the QA histograms
 * @param name name of the histogram
 * @param axes axes used for the histogram
 * @param weightname name of the weight
 * @return unique pointer to the histogram
 */
CorrectionManager::HistogramCallBack CorrectionManager::Create2DHistoArray(const std::string &name,
                                                                           const std::vector<AxisD> axes,
                                                                           const std::string &weight,
                                                                           const AxisD &histaxis) {
  auto callback = [&name, histaxis, axes, weight](InputVariableManager *var) {
    auto hist_name = name + "_" + axes[0].Name() + "_" + axes[1].Name() + "_" + weight;
    auto axisname = std::string(";") + axes[0].Name() + std::string(";") + axes[1].Name();
    auto size_x = static_cast<const int>(axes[0].size());
    auto size_y = static_cast<const int>(axes[1].size());
    for (const auto &axis : axes) {
      try { var->FindVariable(axis.Name()); }
      catch (std::out_of_range &) {
        std::cout << "QAHistogram " << name << ": Variable " << axis.Name()
                  << " not found. Creating new channel variable." << std::endl;
        var->CreateChannelVariable(axis.Name(), axis.size());
      }
    }
    auto upper_edge_x = axes[0].GetLastBinEdge();
    auto lower_edge_x = axes[0].GetFirstBinEdge();
    auto upper_edge_y = axes[1].GetLastBinEdge();
    auto lower_edge_y = axes[1].GetFirstBinEdge();
    std::array<InputVariable, 3>
        arr = {{var->FindVariable(axes[0].Name()), var->FindVariable(axes[1].Name()),
                var->FindVariable(weight)}};
    auto histo = new TH2F(hist_name.data(),
                          axisname.data(),
                          size_x,
                          lower_edge_x,
                          upper_edge_x,
                          size_y,
                          lower_edge_y,
                          upper_edge_y);
    auto haxis = std::make_unique<Qn::AxisD>(histaxis);
    auto haxisvar = var->FindVariable(histaxis.Name());
    return std::make_unique<QAHisto2DPtr>(arr, histo, std::move(haxis), haxisvar);
  };
  return callback;
}

}