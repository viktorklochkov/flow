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

#include <TTreeReader.h>
#include "TFile.h"
#include "TTree.h"

#include "EseSubEvent.h"

namespace Qn {

class CorrelationManager;

class EseHandler {

 public:

  explicit EseHandler(CorrelationManager *man) : manager_(man) {}

  void ConnectInput(std::string percentiles_name, TFile *calibration) {
    if (calibration) {
      if (!calibration->IsZombie()) {
        input_file_ = calibration;
      }
    }
    input_treefile_name_ = percentiles_name;
    input_treefile_ = std::make_shared<TFile>(input_treefile_name_.data(),"READ");
  }

  void ConnectOutput(std::string tree_filename,
                     std::string calib_filename,
                     std::shared_ptr<TFile> *percentile_file_,
                     std::shared_ptr<TFile> *collection) {
    output_file_ = collection;
    output_treefile_ = percentile_file_;
    output_file_name_ = calib_filename;
    output_treefile_name_ = tree_filename;
  }

  void AddESE(const std::string &name, const std::vector<std::string> &input,
              Correlation::function_p lambda, const TH1F &histo);

  void Connect() {
    for (auto &event : subevents_) {
      event.ConnectInput(input_treefile_.get(), input_file_);
      if (event.GetState() > furthest_state_) {
        furthest_state_ = event.GetState();
      }
    }
    if (furthest_state_==EseSubEvent::State::collect) {
      (*output_file_) = std::make_shared<TFile>(output_file_name_.data(), "NEW");
    }
    if (furthest_state_==EseSubEvent::State::calib) {
      *output_treefile_ = std::make_shared<TFile>(output_treefile_name_.data(), "NEW");
      (*output_treefile_)->cd();
      output_tree_ = new TTree("ESE", "ESE");
    }
    for (auto &event : subevents_) {
      event.ConnectOutput(output_tree_, output_file_);
    }
  }

  void Initialize() {
    for (auto &event : subevents_) {
      event.AddCorrelation();
    }
  }

  void Configure() {
    for (auto &event : subevents_) {
      event.Configure();
    }
  }

  void Process(const std::vector<unsigned long> &eventindices) {
    EseSubEvent::State iscalib = EseSubEvent::State::unini;
    for (auto &event : subevents_) {
      event.Do(eventindices);
      if (event.GetState()==EseSubEvent::State::calib) iscalib = event.GetState();
    }
    if (iscalib==EseSubEvent::State::calib) output_tree_->Fill();
  }

  void Finalize() {
    for (auto &event : subevents_) {
      event.Finalize();
    }
    if (furthest_state_==EseSubEvent::State::calib) {
      if (output_treefile_ && output_tree_) {
        (*output_treefile_)->cd();
        output_tree_->Write();
      }
    }
  }

  std::string Report() {
    std::string report;
    if (input_file_) {
      report += std::string("Calibration input file: ") + input_file_->GetName() + "\n";
    }
    if ((*output_file_)) {
      report += std::string("Calibration output file: ") + (*output_file_)->GetName() + "\n";
    }
    if (input_treefile_) {
      report += std::string("Percentiles input tree name: ") + input_treefile_->GetName() + "\n";
    }
    if (output_tree_) {
      report += std::string("Percentiles output tree name: ") + output_tree_->GetName() + "\n";
    }
    for (auto &event : subevents_) {
      report += event.Report() + "\n";
    }
    return report;
  }

 private:
  EseSubEvent::State furthest_state_ = EseSubEvent::State::unini;
  CorrelationManager *manager_;
  std::string input_treefile_name_;
  std::shared_ptr<TFile> input_treefile_;
  TTree *output_tree_ = nullptr;
  TFile *input_file_ = nullptr;
  std::shared_ptr<TFile> *output_file_ = nullptr;
  std::shared_ptr<TFile> *output_treefile_ = nullptr;
  std::string output_file_name_;
  std::string output_treefile_name_;
  std::vector<Qn::EseSubEvent> subevents_;

};
}

#endif //FLOW_ESEHANDLER_H
