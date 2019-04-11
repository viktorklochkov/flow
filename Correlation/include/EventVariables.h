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

#include "TTreeReaderValue.h"

#include "Axis.h"

namespace Qn {

class CorrelationManager;

class EventAxisInterface {
 public:
  virtual unsigned long GetBin() = 0;
  virtual bool IsValid() = 0;
  virtual ~EventAxisInterface() = default;
};

template<typename T>
class EventAxis : public EventAxisInterface {
 public:
  EventAxis(Qn::Axis axis, TTreeReaderValue<T> value) :
      axis_(axis),
      value_(std::move(value)) {}
  unsigned long GetBin() override {
    return axis_.FindBin(*value_);
  }
  const Qn::Axis &GetAxis() const { return axis_; }
  bool IsValid() override { return !isnan(*value_.Get()); }
 private:
  Qn::Axis axis_;
  TTreeReaderValue<T> value_;
};

class EventVariables {

 public:

  enum class Type {
    Integer,
    Float
  };

  explicit EventVariables(Qn::CorrelationManager *manager) : manager_(manager) {}

  void RegisterEventAxis(Axis eventaxis, Type type);

//  bool CheckEvent() {
//    u_long ie = 0;
//    for (const auto &axis : event_axes_) {
//      long bin = -1;
//      if (axis->IsValid()) {
//        bin = ae.FindBin(values_[ie]);
//      }
//      if (bin!=-1) {
//        bin_[ie] = (unsigned long) bin;
//      } else {
//        return false;
//      }
//      ie++;
//    }
//    return true;
//  }

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
    for (auto &treeval : tree_values_F) {
      values_[i] = *treeval.Get();
      i++;
    }
  }

  const std::vector<Qn::Axis> &GetAxes() const { return axes_; }

  const std::vector<unsigned long> GetBin() const { return bin_; }

 private:
  Qn::CorrelationManager *manager_;
  std::vector<std::unique_ptr<Qn::EventAxisInterface>> event_axes_;

  std::vector<TTreeReaderValue<Float_t >> tree_values_F;
  std::vector<TTreeReaderValue<Long64_t >> tree_values_L;
  std::vector<float> values_;
  std::vector<unsigned long> bin_;
  std::vector<Qn::Axis> axes_;

};
}

#endif //FLOW_EVENTVARIABLES_H
