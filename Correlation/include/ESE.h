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

#ifndef FLOW_ESE_H
#define FLOW_ESE_H

#include "EventShapeResult.h"
#include "TFile.h"
#include "TTree.h"

namespace Qn {

class CorrelationManager;

class ESE {
//
//  using size_type =  CorrelationESE::size_type;
  using inputs_type = std::unique_ptr<std::map<std::string, DataContainerQVector *>>;
//
  enum class Status {
    Collecting,
    Calibrating
  };
//
 public:

  explicit ESE(CorrelationManager *ptr) :
      main_manager_(ptr) {}
//
  void SetCalibrationFile(TFile *file) {
    calibration_file_ = file;
  }
//
  void AddESE(const std::string &name, const std::vector<std::string> &input,
              Correlation::function_type &&lambda, const TH1F &histo) {}
//
//  void CheckInputQVectors();
//

  void ReadCalibrationFile() {
    auto calibration_list = dynamic_cast<TDirectory*>(calibration_file_->Get("calibrations"));
    for (const auto &entry : *calibration_list->GetListOfKeys()) {
        auto name = entry->GetName();
        auto eventshape = dynamic_cast<DataContainerEventShape*>(calibration_list->Get(name));
        auto unievent = std::make_unique<DataContainerEventShape>(*eventshape);
        correlations_file_.emplace(name, std::move(unievent));
//      auto name = static_cast<TNamed*>(entry)->GetName();
//      auto eventshape = dynamic_cast<DataContainerEventShape *>(entry);
//      correlations_from_file.emplace(name, std::make_unique<DataContainerEventShape>(*eventshape));
    }
  }

  void Configure() {
    for (auto &pair : es_results) {
      pair.second->Configure();
    }
  }

  void Calibrate() {
    for (auto &pair : es_results) {
      auto &correlation = pair.second;
      correlation->FitSplines();
    }
  }

//
//  void SetBranches() {
//    for (auto &pair :correlations_) {
//      if (pair.second.GetState()==EventShape::State::ReadyForCalculation) {}
//    }
//  }
//
//  void SaveCalibrationToFile() {
//    calibration_file_->cd();
//    for (auto &pair  : correlations_) {
//      auto correlation = pair.second;
//      auto calibrationobject = correlation.GetCalibration();
//      calibrationobject.Write(pair.first.data());
//    }
//  }
//
//  void FillCorrelations(const std::vector<unsigned long> &eventindices) {
//    if (status_==Status::Collecting || status_==Status::Calibrating) {
//      for (auto &pair : correlations_) {
//        pair.second.Fill(eventindices, 0);
//      }
//    }
//    if (status_==Status::Calibrating)  {
//      for (auto &pair : correlations_) {
//        pair.second.GetPercentile(eventindices);
//      }
//    }
//  }
//
//
//
 private:
  Status status_;
  CorrelationManager *main_manager_;
  TFile *calibration_file_ = nullptr;
  TTree *output_tree_ = nullptr;
  std::map<std::string, std::unique_ptr<Qn::EventShapeResult>> es_results;
  std::map<std::string, std::unique_ptr<Qn::DataContainerEventShape >> correlations_file_;
};
}

#endif