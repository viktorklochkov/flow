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

#include "EventAxes.h"
#include "CorrelationManager.h"

void Qn::EventAxes::AddEventVariable(const Qn::Axis &eventaxis) {
  TTreeReaderValue<float> value(*(manager_->reader_), eventaxis.Name().data());
  tree_values_.push_back(value);
  values_.emplace_back(-999);
  bin_.emplace_back(-1);
  axes_.push_back(eventaxis);
}