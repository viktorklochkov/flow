#include <utility>


#ifndef QN_INPUTVARIABLE
#define QN_INPUTVARIABLE

namespace Qn {
/**
 * Variable
 * Constructor is private so it is only created in the variable manager
 * with the correct pointer to the values container.
 */
 template <typename T>
class InputVariable {
 private:
  using iterator_type = typename std::vector<T>::iterator;
  using const_iterator_type = typename std::vector<T>::const_iterator;

  InputVariable(std::string name, const int id, const int length) : id_(id), name_(std::move(name)), values_(length) {}
  int id_{}; /// position in the values container
  std::vector<T> values_; ///< values of the variable
  std::string name_; /// name of the variable
  friend class VariableManager;
  friend struct std::less<Qn::InputVariable<T>>;
  friend class Cuts;
 public:
  InputVariable() = default;
  iterator_type begin() noexcept {return values_.begin();}
  iterator_type end() noexcept {return values_.end();}
  const_iterator_type begin() const noexcept {return values_.begin();}
  const_iterator_type end() const noexcept {return values_.end();}
  inline int length() const noexcept { return values_.size(); }
  inline int id() const noexcept { return id_; }
  std::string Name() const { return name_; }
};
 using InputVariableD = InputVariable<double>;
 using InputVariableL = InputVariable<long>;
}

namespace std {
/**
 * std::less specialization for the Qn::Variable
 */
template<typename T>
struct less<Qn::InputVariable<T>> {
  bool operator()(const Qn::InputVariable<T> &lhs, const Qn::InputVariable<T> &rhs) const {
    return lhs.id_ < rhs.id_;
  }
};
}

#endif