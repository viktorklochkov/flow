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

#include "EseSubEvent.h"
#include "EseHandler.h"

#include "ROOT/RMakeUnique.hxx"

void Qn::EseSubEvent::AddCorrelation() {
  if (state_==State::collect) {
    result_ = std::make_unique<Qn::EventShapeResult>(handler_->RequestCorrelation(proto_), proto_.histo);
  }
  if (state_==State::calib) {
    result_ = std::make_unique<Qn::EventShapeResult>(handler_->RequestCorrelation(proto_));
    result_->SetInputData(std::move(calib_));
  }
}

void Qn::EseSubEvent::Configure() {
  if (state_==State::collect) { result_->Configure(); }
}

void Qn::EseSubEvent::ConnectInput(TFile *input_treefile, TFile *calib) {
  if (input_treefile) {
    auto tree = (TTree *) input_treefile->Get("ESE");
    if (tree) {
      auto branchlist = tree->GetListOfBranches();
      if (branchlist->Contains(name_.data())) {
        handler_->RequestEventAxis({name_, kNBins, 0., 1.});
        state_ = State::percent;
        return;
      }
    }
  }
  if (calib) {
    auto dir = (TDirectory *) calib->Get("calibrations");
    if (dir) {
      if (dir->GetListOfKeys()->Contains(name_.data())) {
        calib_ =
            std::make_unique<Qn::DataContainerEventShape>(*dynamic_cast<Qn::DataContainerEventShape *>(dir->Get(name_.data())));
        state_ = State::calib;
        return;
      }
    }
  }
  state_ = State::collect;
}

void Qn::EseSubEvent::ConnectOutput(TTree *tree, TFile *calib) {
  if (tree) {
    tree->Branch(name_.data(), &out_value_);
  } else {
    if (state_==State::calib) state_ = State::unini;
  }
  out_calib_ = calib;
  if (state_==State::collect && !out_calib_) { state_ = State::unini; }
}
