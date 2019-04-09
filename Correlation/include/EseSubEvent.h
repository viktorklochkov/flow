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

class EseHandler;

struct SubEventPrototype {

  SubEventPrototype(std::string iname, std::vector<std::string> iinput, Correlation::function_type ilambda, TH1F ihisto)
      :
      name(std::move(iname)),
      input(std::move(iinput)),
      lambda(std::move(ilambda)),
      histo(std::move(ihisto)),
      weights(input.size(), Qn::kRef) {}

  std::string name;
  std::vector<std::string> input;
  Correlation::function_type lambda;
  TH1F histo;
  std::vector<Qn::Weight> weights;
};

class EseSubEvent {

 public:

  static constexpr int kNBins = 10;

  enum class State : int {
    unini = 0,
    collect = 1,
    calib = 2,
    percent = 3
  };

  EseSubEvent(EseHandler *handler, const std::string &name, const std::vector<std::string> &input,
              Correlation::function_t lambda, const TH1F &histo) :
      name_(name),
      handler_(handler),
      proto_(name, input, lambda, histo) {}

  void ConnectInput(TFile *input_treefile, TFile *calib);

  void ConnectOutput(TTree *tree, TFile *calib);

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
      if (!out_calib_->GetListOfKeys()->Contains("calibrations")) {
        out_calib_->mkdir("calibrations");
      }
      out_calib_->cd("calibrations");
      result_->GetCalibration().Write(name_.data(), TObject::kSingleKey);
    }
  }

  std::string Report() {
    std::string report;
    report += name_ + " ";
    switch (state_) {
      case State::unini :report += "is uninitialized!";
        break;
      case State::calib :report += "is calibrating percentiles.";
        break;
      case State::percent :report += "is applying ESE to correlations.";
        break;
      case State::collect :report += "is collecting the distributions.";
        break;
    }
    return report;
  }

  State GetState() const { return state_; }

 private:
  std::string name_;
  Qn::EseHandler *handler_ = nullptr;
  SubEventPrototype proto_;
  float out_value_ = NAN;
  TFile *out_calib_ = nullptr;
  State state_ = State::unini;
  std::unique_ptr<Qn::DataContainerEventShape> calib_ = nullptr;
  std::unique_ptr<EventShapeResult> result_ = nullptr;

};

}

#endif //FLOW_ESESUBEVENT_H
