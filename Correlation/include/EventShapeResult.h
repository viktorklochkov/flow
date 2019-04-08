#include <utility>

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

#ifndef FLOW_CORRELATIONESE_H
#define FLOW_CORRELATIONESE_H

#include <utility>

#include "StatsResult.h"

namespace Qn {

class EventShapeResult {
 public:

  enum class State {
    Uninitialized,
    Collecting,
    Calibrating,
    Reading
  };

  EventShapeResult(Qn::Correlation *ptr, TH1F histo) :
      correlation_current_event_(ptr),
      event_shape_result_(new Qn::DataContainerEventShape()) {
    for (auto &bin : *event_shape_result_) {
      bin.SetHisto(&histo, correlation_current_event_->GetName());
    }
  }

  explicit EventShapeResult(Qn::Correlation *ptr) :
      correlation_current_event_(ptr) {
  }

  void FitSplines() {
    for (auto &bin : *event_shape_result_) {
      bin.FitWithSpline();
    }
  }

  const Qn::DataContainerEventShape& GetCalibration() const { return *event_shape_result_; }

  void Configure();
  void FillCalibrationHistogram();
  double GetPercentile(const std::vector<unsigned long> &eventindices) {
    auto prod = correlation_current_event_->GetResult().At(eventindices);
    double percentile = NAN;
    if (prod.validity) percentile = event_shape_result_->At(eventindices).GetPercentile(prod.result);
    return percentile;
  }

  void SetInputData(std::shared_ptr<Qn::DataContainerEventShape> eventshape) {
    event_shape_result_ = std::move(eventshape);
    state_ = State::Calibrating;
  }

  State GetState() const { return state_; }
 private:
  State state_ = State::Uninitialized;
  Correlation *correlation_current_event_ = nullptr; ///< Pointer to the correlation result.
  std::shared_ptr<Qn::DataContainerEventShape> event_shape_result_ = nullptr;
};
}

#endif
