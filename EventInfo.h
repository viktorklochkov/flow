//
// Created by Lukas Kreis on 05.07.17.
//

#ifndef FLOW_EVENTINFO_H
#define FLOW_EVENTINFO_H

#include <string>
#include <map>

namespace Qn {
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
    return map_.find(key)->second;
  }
/**
 * Sets a key value pair for an event parameter.
 * @param key identifier of event parameter
 * @param a value of event parameter
 */
  void SetVariable(std::string key, T a) {
    map_.emplace(key, a);
  }

  void Clear() {
    map_.clear();
  }

 private:
  std::map<std::string, T> map_; ///< stores key value pair for event parameters
};

typedef EventInfo<int> EventInfoI; ///< Typedef for integer parameters
typedef EventInfo<float> EventInfoF; ///< Typedef for floating point parameters

}
#endif //FLOW_QNEVENTINFO_H