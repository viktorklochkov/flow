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

#include "EventShapeResult.h"

void Qn::EventShapeResult::Configure() {
  const auto &current_event = correlation_current_event_->GetResult();
  auto histo = dynamic_cast<TH1F*>(event_shape_result_->At(0).GetHist()->Clone("temp"));
  try {
    event_shape_result_->AddAxes(current_event.GetAxes());
  } catch (std::logic_error &e) {
    std::string errormsg = "ese correlation trying to add axes, but they already exist.";
    throw std::logic_error(errormsg);
  }
  unsigned long ibin = 0;
  for (auto &bin : *event_shape_result_) {
    std::string name = event_shape_result_->GetBinDescription(ibin);
    bin.SetHisto(histo, name);
    ibin++;
  }
  state_ = State::Collecting;
  delete histo;
}

void Qn::EventShapeResult::FillCalibrationHistogram() {
  const auto &current_event = correlation_current_event_->GetResult();
  unsigned int ibin = 0;
  for (auto &bin : *event_shape_result_) {
    bin.Fill(current_event.At(ibin));
    ++ibin;
  }
}
