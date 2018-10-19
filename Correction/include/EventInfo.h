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

#ifndef FLOW_EVENTINFO_H
#define FLOW_EVENTINFO_H

#include <string>
#include <map>

#include "TTree.h"
#include "Rtypes.h"

namespace Qn {
/**
 * Stores an event information value and flag to check if value is valid.
 * @tparam T type of event information
 */
template<typename T>
class EventInfoValue {
  template<typename TT>
  friend
  class EventInfo;
 public:
  /**
   * Checks if stored value is valid.
   * @return returns true is value is valid.
   */
  bool IsValid() const { return isgood_; }
  T Get() const { return value_; }
  /**
   * Sets its value and the validity to true.
   * @param value value to parameter.
   */
  void SetValue(T value) {
    value_ = value;
    isgood_ = true;
  }
  /**
   * Gets Address of contained value.
   * @return Address of contained value.
   */
  T &GetAddress() { return value_; }
 private:
  bool isgood_ = false;
  T value_;

};

/**
 * @brief Container for event information
 * @tparam T type of information e.g. floating point or integer
 */
template<typename T>
class EventInfo {
 private:

 public:

  typedef typename std::map<std::string, EventInfoValue<T>>::iterator iterator;
  iterator begin() { return map_.begin(); } ///< iterator for external use
  iterator end() { return map_.end(); } ///< iterator for external use
/**
 * Gets value and validity of event parameter.
 * @param key identifier of event parameter
 * @return pair of value of event parameter and its validity.
 */
  std::pair<T, bool> GetVariable(std::string key) {
    auto element = map_.find(key)->second;
    return std::make_pair<T, bool>(element.Get(), element.IsValid());
  }
/**
 * Adds a key value pair for an event parameter.
 * @param key identifier of event parameter
 * @param a value of event parameter
 */
  void AddVariable(std::string key) {
    EventInfoValue<T> a;
    map_.emplace(key, a);
  }
  /**
 * Sets a key value pair for an event parameter.
 * @param key identifier of event parameter
 * @param a value of event parameter
 */
  void SetVariable(std::string key, T a) {
    map_[key].SetValue(a);
  }
/**
 * Creates Branches in tree for saving the event information.
 * @param tree output tree to contain the event information
 */
  void SetToTree(TTree &tree) {
    for (auto &element : map_) {
      tree.Branch(element.first.data(), &element.second.GetAddress());
    }
  }
/**
 * Clears information store in the event information.
 */
  void Clear() {
    map_.clear();
  }
  /**
   * resets validity of all object to false. Needs to be called before filling new event.
   */
  void Reset() {
    for (auto element : map_) {
      element.second.isgood_ = false;
    }
  }

 private:
  std::map<std::string, EventInfoValue<T>> map_; ///< stores key value pair for event parameters

};

typedef EventInfo<float> EventInfoF; ///< Typedef for floating point parameters

}
#endif //FLOW_QNEVENTINFO_H