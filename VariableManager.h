//
// Created by Lukas Kreis on 16.01.18.
//

#ifndef FLOW_VARIABLEMANAGER_H
#define FLOW_VARIABLEMANAGER_H

#include <string>
#include <map>
class VariableManager {

 public:
  void AddVariable(const std::string &name, const int number) {
    name_enum_map_.insert(std::make_pair(name,number));
    enum_name_map_.insert(std::make_pair(number,name));
  }

  int FindNum(const std::string &name) const {return name_enum_map_.at(name);}
  std::string FindName(const int number) const {return enum_name_map_.at(number);}

 private:
  std::map<std::string, int> name_enum_map_;
  std::map<int, std::string> enum_name_map_;
};

#endif //FLOW_VARIABLEMANAGER_H
