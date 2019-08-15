
#ifndef QN_INPUTVARIABLE
#define QN_INPUTVARIABLE

namespace Qn {
/**
 * Variable
 * Constructor is private so it is only created in the variable manager
 * with the correct pointer to the values container.
 */
class InputVariable {
 private:
  InputVariable(const unsigned int id, const unsigned int length, std::string name) :
      id_(id),
      size_(length),
      name_(std::move(name)) {}
  InputVariable(const unsigned int id, const unsigned int length) : id_(id), size_(length) {}
  unsigned int id_{0}; /// position in the values container
  unsigned int size_{0}; /// length in the  values container
  double *values_container_ = nullptr; /// pointer to the values container
  std::string name_; /// name of the variable
  friend class InputVariableManager;
  friend class CorrectionCuts;
 public:
  InputVariable() = default;
  inline double operator[](int i) const noexcept { return values_container_[id_ + i];}
  inline double *begin() noexcept { return &values_container_[id_]; } /// implements begin for iteration
  inline double *end() noexcept { return &values_container_[id_ + size_]; } /// implements end for iteration
  const double *begin() const noexcept { return &values_container_[id_]; }  /// implements begin for iteration
  inline double *end() const noexcept { return &values_container_[id_ + size_]; }  /// implements end for iteration
  inline double *at(int i) noexcept { return &values_container_[id_ + i]; }
  const inline double *Get() const noexcept { return &values_container_[id_]; }
  inline unsigned int size() const noexcept { return size_; }
  inline int GetID() const noexcept { return id_; }
  std::string GetName() const { return name_; }
};
}

#endif