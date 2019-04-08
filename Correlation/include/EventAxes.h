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

#ifndef FLOW_EVENTVARIABLES_H
#define FLOW_EVENTVARIABLES_H

#include "Axis.h"
#include "TTreeReaderValue.h"

namespace Qn {

class CorrelationManager;

class EventAxes {

 public:
  EventAxes(Qn::CorrelationManager *manager) : manager_(manager) {}

  void AddEventVariable(const Axis &eventaxis);

  bool CheckEvent() {
    u_long ie = 0;
    for (const auto &ae : axes_) {
      long bin = -1;
      if (!isnan(values_[ie])) {
        bin = ae.FindBin(values_[ie]);
      }
      if (bin!=-1) {
        bin_[ie] = (unsigned long) bin;
      } else {
        return false;
      }
      ie++;
    }
    return true;
  }

  void UpdateEvent() {
    unsigned long i = 0;
    for (auto &treeval : tree_values_) {
      values_[i] = *treeval.Get();
      i++;
    }
  }

  const std::vector<Qn::Axis> &GetAxes() const {return axes_;}

  const std::vector<unsigned long> GetBin() const {return bin_;}

 private:
  Qn::CorrelationManager *manager_;
  std::vector<TTreeReaderValue<float>> tree_values_;
  std::vector<float> values_;
  std::vector<unsigned long> bin_;
  std::vector<Qn::Axis> axes_;

};
}

#endif //FLOW_EVENTVARIABLES_H
