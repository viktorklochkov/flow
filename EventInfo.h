//
// Created by Lukas Kreis on 05.07.17.
//

#ifndef FLOW_EVENTINFO_H
#define FLOW_EVENTINFO_H

#include <string>
#include <iostream>
#include <map>
#include <TTree.h>
#include <TLeaf.h>
#include "Rtypes.h"

namespace Qn {
/**
 * Stores an event information value and flag to check if value is valid.
 * @tparam T type of event information
 */
template<typename T>
class EventInfoValue  {
  template<typename TT>
  friend class EventInfo;
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
/**
 * Gets value and valididty of event parameter.
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
 * Attaches input tree to event information object for reading.
 * @param tree input tree which contains the  event information in branches
 */
  void AttachToTree(TTree &tree) {
    for (auto &element : map_) {
      element.second.isgood_ = true;
      auto branch = tree.GetBranch(element.first.data());
      if (branch) {
        tree.SetBranchAddress(element.first.data(), &element.second.GetAddress());
      } else throw std::invalid_argument("EventInfo was not saved to tree");
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

typedef EventInfo<int> EventInfoI; ///< Typedef for integer parameters
typedef EventInfo<float> EventInfoF; ///< Typedef for floating point parameters

}
#endif //FLOW_QNEVENTINFO_H