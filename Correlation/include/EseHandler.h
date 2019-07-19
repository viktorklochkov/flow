// Flow Vector Correction Framework
//
// Copyright (C) 2019  Lukas Kreis Ilya Selyuzhenkov
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

#ifndef FLOW_ESEHANDLER_H
#define FLOW_ESEHANDLER_H

#include "TTreeReader.h"
#include "TFile.h"
#include "TTree.h"
#include "TSystem.h"

#include "EseSubEvent.h"

namespace Qn {

class CorrelationManager;

class EseHandler {

 public:

  explicit EseHandler(CorrelationManager *man) : manager_(man) {}

  void SetInput(std::string percentiles_name, std::string calib_filename);

  void SetOutput(const std::string &tree_filename, const std::string &calib_filename) {
    output_file_name_ = calib_filename;
    output_treefile_name_ = tree_filename;
  }

  void AddESE(const std::string &name, const std::vector<std::string> &input,
              Correlation::function_t lambda, const TH1F &histo);

  void Connect() {
    for (auto &event : subevents_) {
      event.ConnectInput(input_treefile_.get(), input_file_.get());
      if (event.GetState() > furthest_state_) {
        furthest_state_ = event.GetState();
      }
    }
    if (furthest_state_==EseSubEvent::State::collect) {
      output_file_ = std::make_shared<TFile>(output_file_name_.data(), "NEW");
    }
    if (furthest_state_==EseSubEvent::State::calib) {
      output_treefile_ = std::make_shared<TFile>(output_treefile_name_.data(), "NEW");
      output_treefile_->cd();
      output_tree_ = new TTree("ESE", "ESE");
    }
    for (auto &event : subevents_) {
      event.ConnectOutput(output_tree_, output_file_.get());
    }
    SetupEventMatching();
  }

  void Initialize() {
    for (auto &event : subevents_) {
      event.AddCorrelation();
    }
  }

  void Configure() {
    for (auto &event : subevents_) {
      event.Configure();
      if (event.GetState()==EseSubEvent::State::calib) iscalib_ = true;
    }
  }

  bool Process(const std::vector<unsigned long> &eventindices) {
    for (auto &event : subevents_) {
      event.Do(eventindices);
    }
    if (!subevents_.empty()) {
      return CheckMatching();
    }
    return true;
  }

  void Finalize() {
    for (auto &event : subevents_) {
      event.Finalize();
    }
    if (furthest_state_==EseSubEvent::State::calib) {
      if (output_treefile_ && output_tree_) {
        output_treefile_->cd();
        output_tree_->Write();
      }
    }
    if (output_file_) output_file_->Close();
    if (output_treefile_) output_treefile_->Close();
  }

  std::string Report() {
    std::string report;
    if (input_file_) {
      report += std::string("Calibration input file: ") + input_file_->GetName() + "\n";
    }
    if (output_file_) {
      report += std::string("Calibration output file: ") + output_file_->GetName() + "\n";
    }
    if (input_treefile_) {
      report += std::string("Percentiles input tree name: ") + input_treefile_->GetName() + "\n";
    }
    if (output_treefile_) {
      report += std::string("Percentiles output tree name: ") + output_tree_->GetName() + "\n";
    }
    for (auto &event : subevents_) {
      report += event.Report() + "\n";
    }
    return report;
  }

  Qn::Correlation *RequestCorrelation(const SubEventPrototype &prototype);

  void RequestEventAxis(const Qn::AxisD &axis);

  void FillTree() {
    if (iscalib_) {
      output_tree_->Fill();
      run_id_ = 0;
      event_id_ = 0;
      for (auto &event : subevents_) {
        event.Clear();
      }
    }
  }

  void SetRunEventId(const std::string &run, const std::string &event);

  void SetupEventMatching();

  bool CheckMatching() {
    if (run_id_friend_ && event_id_friend_) {
      auto friend_run = *run_id_friend_->Get();
      auto friend_event = *event_id_friend_->Get();
      return run_id_==friend_run && event_id_==friend_event;
    }
    return true;
  }

  void UpdateIDs() {
    if (run_id_input_ && event_id_input_) {
      run_id_ = *run_id_input_->Get();
      event_id_ = *event_id_input_->Get();
    }
  }

 private:
  bool iscalib_ = false;
  EseSubEvent::State furthest_state_ = EseSubEvent::State::unini;
  CorrelationManager *manager_;
  TTree *output_tree_ = nullptr;
  std::shared_ptr<TFile> input_file_ = nullptr;
  std::shared_ptr<TFile> input_treefile_ = nullptr;
  std::shared_ptr<TFile> output_file_ = nullptr;
  std::shared_ptr<TFile> output_treefile_ = nullptr;
  std::string output_file_name_;
  std::string output_treefile_name_;
  std::vector<Qn::EseSubEvent> subevents_;
  std::unique_ptr<TTreeReaderValue<Long64_t>> run_id_input_ = nullptr;
  std::unique_ptr<TTreeReaderValue<Long64_t>> event_id_input_ = nullptr;
  Long64_t run_id_;
  Long64_t event_id_;
  std::unique_ptr<TTreeReaderValue<Long64_t>> run_id_friend_ = nullptr;
  std::unique_ptr<TTreeReaderValue<Long64_t>> event_id_friend_ = nullptr;
};
}

#endif //FLOW_ESEHANDLER_H
