//
// Created by Lukas Kreis on 05.07.17.
//

#ifndef FLOW_EVENTINFO_H
#define FLOW_EVENTINFO_H

#include <string>
#include <iostream>
#include <map>
#include <TTree.h>
#include "Rtypes.h"

namespace Qn {

template<typename T>
class EventInfoValue {
 public:
  EventInfoValue() : isgood_(false), value_(0.0) {}
  bool IsValid() const { return isgood_; }
  T Get() const { return value_; }
  void Set(T value) {value_ = value;}
  T& GetAddress() {return value_;}
 private:
  bool isgood_;
  T value_;

  /// \cond CLASSIMP
 ClassDef(EventInfoValue, 1);
  /// \endcond

};

/**
 * @brief Container for event information
 * @tparam T type of information e.g. floating point or integer
 */
template<typename T>
class EventInfo {
 public:
  EventInfo() = default;
  ~EventInfo() = default;
/**
 * Gets value of event parameter.
 * @param key identifier of event parameter
 * @return value of event parameter
 */
  T GetVariable(std::string key) {
    return map_.find(key)->second->Get();
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
    map_[key].Set(a);
  }

  void SetOutputTree(TTree &tree) {
    for (auto &element : map_) {
        tree.Branch(element.first.data(), &element.second.GetAddress());
        std::cout << (element.second.Get()) << std::endl;
    }
  }

  void Clear() {
    map_.clear();
  }

 private:
  std::map<std::string, EventInfoValue<T>> map_; ///< stores key value pair for event parameters

  /// \cond CLASSIMP
 ClassDef(EventInfo, 1);
  /// \endcond
};

typedef EventInfo<int> EventInfoI; ///< Typedef for integer parameters
typedef EventInfo<float> EventInfoF; ///< Typedef for floating point parameters

}
#endif //FLOW_QNEVENTINFO_H