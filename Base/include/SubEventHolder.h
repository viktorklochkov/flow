#include "SubEvent.h"

namespace Qn {
class SubEventHolder {
 public:
  SubEventHolder() = default;
  virtual ~SubEventHolder() = default;
  std::shared_ptr<SubEvent> operator->() {return ptr;}
  std::shared_ptr<SubEvent> ptr = nullptr; //!<! non persistent
  /// \cond CLASSIMP
  ClassDef(SubEventHolder, 1);
  /// \endcond
};
}