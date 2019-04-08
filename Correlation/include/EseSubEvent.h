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

#ifndef FLOW_ESESUBEVENT_H
#define FLOW_ESESUBEVENT_H

#include <string>
#include "TTreeReader.h"
#include "TFile.h"

#include "DataContainer.h"
#include "Correlation.h"
#include "EventShapeResult.h"

namespace Qn {

class CorrelationManager;

struct SubEventPrototype {
  std::string name;
  std::vector<std::string> input;
  Correlation::function_type lambda;
  TH1F histo;
};

class EseSubEvent {

 public:

  enum class State : int {
    unini = 0,
    collect = 1,
    calib = 2,
    percent = 3
  };

  EseSubEvent(CorrelationManager *man, const std::string &name, const std::vector<std::string> &input,
              Correlation::function_p lambda, const TH1F &histo) :
      manager_(man),
      name_(name) {
    proto_.name = name_;
    proto_.input = input;
    proto_.lambda = lambda;
    proto_.histo = histo;
  }

  void ConnectInput(TFile *input_treefile, TFile *calib);

  void ConnectOutput(TTree *tree, std::shared_ptr<TFile> *calib) {
    if (tree) {
      tree->Branch(name_.data(), &out_value_);
    } else {
      if (state_==State::calib) state_ = State::unini;
    }
    out_calib_ = calib;
    if (state_==State::collect && !out_calib_) { state_ = State::unini; }
  }


  void Configure();

  void AddCorrelation();

  void Do(const std::vector<unsigned long> &eventindices) {
    if (state_==State::collect) {
      result_->FillCalibrationHistogram();
    }
    if (state_==State::calib) {
      out_value_ = (float) result_->GetPercentile(eventindices);
    }
  }

  void Finalize() {
    if (state_==State::collect) {
      result_->FitSplines();
      if (!(*out_calib_)->GetListOfKeys()->Contains("calibrations")) {
        (*out_calib_)->mkdir("calibrations");
      }
      (*out_calib_)->cd("calibrations");
      result_->GetCalibration().Write(name_.data(), TObject::kSingleKey);
    }
  }

  std::string Report() {
    std::string report;
    report += name_ + " ";
    switch (state_) {
      case State::unini :
        report += "is uninitialized!";
        break;
      case State::calib :
        report += "is calibrating percentiles.";
        break;
      case State::percent :
        report += "is applying ESE to correlations.";
        break;
      case State::collect :
        report += "is collecting the distributions.";
        break;
    }
    return report;
  }

  State GetState() const { return state_; }

 private:
  SubEventPrototype proto_;
  std::shared_ptr<TFile> *out_calib_ = nullptr;
  float out_value_ = NAN;
  Qn::CorrelationManager *manager_ = nullptr;
  std::string name_;
  State state_ = State::unini;
  std::shared_ptr<Qn::DataContainerEventShape> calib_ = nullptr;
  std::shared_ptr<EventShapeResult> result_ = nullptr;

};

}

#endif //FLOW_ESESUBEVENT_H
