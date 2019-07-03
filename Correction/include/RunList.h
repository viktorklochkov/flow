#include <string>
#include <vector>
#include "ROOT/RStringView.hxx"
namespace Qn {
class RunList {
 public:
  explicit RunList(std::vector<std::string> list) : run_list_(std::move(list)) {}
  RunList() = default;
  std::vector<std::string>::const_iterator begin() const { return run_list_.begin(); }
  std::vector<std::string>::const_iterator end() const { return run_list_.end(); }
  void SetCurrentRun(std::string_view name) {
    current_run_name_ = name;
    if (std::find(run_list_.begin(), run_list_.end(), current_run_name_)!=run_list_.end()) {
      run_list_.emplace_back(current_run_name_);
    }
  }
  std::string GetCurrent() const {return current_run_name_;}
  bool empty() const {return run_list_.empty();}
 private:
  std::string current_run_name_;
  std::vector<std::string> run_list_;
};
}