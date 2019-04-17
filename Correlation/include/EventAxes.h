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

#ifndef FLOW_EVENTAXES_H
#define FLOW_EVENTAXES_H

#include "TTreeReaderValue.h"

#include "Axis.h"

namespace Qn {

class CorrelationManager;

/**
 * @class EventAxisInterface
 * Base class of the Event Axes
 */
class EventAxisInterface {
 public:
  virtual unsigned long GetBin() = 0;
  virtual bool IsValid() = 0;
  virtual ~EventAxisInterface() = default;
  virtual const Qn::Axis &GetAxis() const = 0;
};

/**
 * @class EventAxis
 * Template class for event axes of different underlying data type.
 * @tparam T Data type of the variable in the input TTree.
 */
template<typename T>
class EventAxis : public EventAxisInterface {
 public:
  /**
   * @brief Constructor
   * @param axis Binning of the EventAxis.
   * @param value Associated value in the input tree
   */
  EventAxis(const Qn::Axis &axis, TTreeReaderValue<T> value) :
      axis_(axis),
      value_(std::move(value)) {}

  /**
   * @brief Get the current event bin.
   * Information of the variable is read directly from the tree.
   * @return bin to which the current value belongs to.
   */
  unsigned long GetBin() override { return axis_.FindBin(*value_.Get()); }

  /**
   * @brief Get the underlying Qn::Axis
   * @return Returns the underlying Qn::Axis
   */
  const Qn::Axis &GetAxis() const override { return axis_; }

  /**
   * Checks if the current entry read from the tree is a valid number (no NAN in case of float)
   * @return Returns true if the current entry is a valid number.
   */
  bool IsValid() override { return !isnan(*value_.Get()); }

 private:
  Qn::Axis axis_; /// Underlying axies determining the binning and the name
  TTreeReaderValue<T> value_; /// value of the currently read entry from the TTree
};

/**
 * @class EventAxes
 * A collection of EventAxis with methods to facilitate the binning of event variables.
 */
class EventAxes {

 public:

  /**
   * Enumerator to specify the underlying data type in the input tree.
   */
  enum class Type {
    Integer,
    Float
  };

  explicit EventAxes(Qn::CorrelationManager *manager) : manager_(manager) {}

  /**
   * @brief Registers a new axes with a specific datatype to be read from the tree and used for binning.
   * @param axis Axis specifing the name and the size of the binning for the correlations.
   * @param type Data type in which the information was saved to the tree.
   */
  void RegisterEventAxis(Axis axis, Type type);

  /**
   * @brief Check if current event is inside the event axes.
   * @return Returns true if the event is inside the event axes.
   */
  bool CheckEvent() {
    u_long ie = 0;
    for (const auto &axis : event_axes_) {
      long bin = -1;
      if (axis->IsValid()) {
        bin = axis->GetBin();
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

  /**
   * @brief Returns a vector of the event axes.
   * @return vector of axes
   */
  const std::vector<Qn::Axis> GetAxes() const {
    std::vector<Qn::Axis> axes;
    for (auto &axis : event_axes_) {
      axes.push_back(axis->GetAxis());
    }
    return axes;
  }

  /**
   * @brief Returns the current event bin.
   * @return multidimensional bin
   */
  const std::vector<unsigned long> &GetBin() const {
    return bin_;
  }

 private:
  Qn::CorrelationManager *manager_; /// non-owning pointer to the correlation manager
  std::vector<std::unique_ptr<Qn::EventAxisInterface>> event_axes_; /// vector of event axes
  std::vector<unsigned long> bin_; /// bin of the current event.
};
}

#endif //FLOW_EVENTAXES_H
