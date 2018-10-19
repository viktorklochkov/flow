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

#ifndef FLOW_VARIABLEMANAGER_H
#define FLOW_VARIABLEMANAGER_H

#include <string>
#include <map>
#include <utility>
#include <cmath>

namespace Qn {

class Variable {
 private:
  Variable(const int id, const int length) : id_(id), length_(length), name_("") {}
  Variable(const int id, const int length, std::string name) : id_(id), length_(length), name_(std::move(name)) {}
  int id_{};
  int length_{};
  double *var_container = nullptr;
  std::string name_;
  friend class VariableManager;
  friend struct std::less<Qn::Variable>;
  friend class Cuts;
 public:
  Variable() = default;
  inline double *at(int i) noexcept { return &var_container[id_ + i]; }
  inline double *begin() noexcept { return &var_container[id_]; }
  inline double *end() noexcept { return &var_container[id_ + length_]; }
  inline double *begin() const noexcept { return &var_container[id_]; }
  inline double *end() const noexcept { return &var_container[id_ + length_]; }
  inline int length() const noexcept { return length_; }
  std::string Name() const { return name_; }
};
}

namespace std {
template<>
struct less<Qn::Variable> {
  bool operator()(const Qn::Variable &lhs, const Qn::Variable &rhs) const {
    return lhs.id_ < rhs.id_;
  }
};
}

namespace Qn {

class VariableManager {
 public:

  VariableManager() {
    for (int i=0; i< maxsize; ++i) {
      var_container[i] = NAN;
    }
  }

  void CreateVariable(std::string name, const int id, const int length) {
    Variable var(id, length, name);
    var.var_container = var_container;
    name_var_map_.emplace(name, var);
    var_name_map_.emplace(var, name);
  }
  void CreateVariableOnes() {
    Variable var(0, 1000, "Ones");
    for (int i = 0; i < 1000; ++i) { var_ones[i] = 1.0; }
    var.var_container = var_ones;
    name_var_map_.emplace("Ones", var);
    var_name_map_.emplace(var, "Ones");
  }

  void CreateChannelVariable(std::string name, const int size) {
    Variable var(0, size, name);
    auto *arr = new double[size];
    for (int i = 0; i < size; ++i) { arr[i] = i; }
    var.var_container = arr;
    name_var_map_.emplace(name, var);
    var_name_map_.emplace(var, name);
  }

  void AddVariable(const std::string &name, Variable var) {
    name_var_map_.insert(std::make_pair(name, var));
    var_name_map_.insert(std::make_pair(var, name));
  }
  Variable FindVariable(const std::string &name) const { return name_var_map_.at(name); }
  std::string FindName(const Variable &var) const { return var_name_map_.at(var); }
  int FindNum(const std::string &name) const { return name_var_map_.at(name).id_; }
  void SetVariables(double *vars) { var_container = vars; }
  double *GetVariableContainer() { return var_container; }
  void CreateVariableContainer(const int size) { var_container = new double[size]; }
  void FillToQnCorrections(double **var) {
    *var = var_container;
  }

 private:
  static constexpr int maxsize = 11000;
  double *var_container = new double[maxsize](); // non-owning pointer to variables
  double *var_ones = new double[1000];
  std::map<std::string, Variable> name_var_map_;
  std::map<Variable, std::string> var_name_map_;

};
}

#endif //FLOW_VARIABLEMANAGER_H
