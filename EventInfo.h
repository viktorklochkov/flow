//
// Created by Lukas Kreis on 05.07.17.
//

#ifndef FLOW_EVENTINFO_H
#define FLOW_EVENTINFO_H

#include <string>
#include <map>

namespace Qn {

template<typename T>
class EventInfo {
 public:
  EventInfo() = default;
  ~EventInfo() = default;

  T GetVariable(std::string key) {
    return map_.find(key)->second;
  }

  void SetVariable(std::string key, T a) {
    map_.emplace(key, a);
  }

 private:
  std::map<std::string, T> map_;
};

typedef EventInfo<int> EventInfoI;
typedef EventInfo<float> EventInfoF;

}
#endif //FLOW_QNEVENTINFO_H