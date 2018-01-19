//
// Author: Ionut-Cristian Arsene, 2015/04/05
// email: iarsene@cern.ch
//
// Variable manager
//

#ifndef VARMANAGER_H
#define VARMANAGER_H

namespace Qn {

class VarManager {

 public:

  enum Values {
    kSignal = 0,
    kCentrality = kSignal + 4,
    kNMax = kCentrality + 1
  };

  static void FillEventInfo(float *values) {
    values[kSignal] = -1.0;
    values[kSignal] = 0.0;
    values[kSignal] = 1.0;
    values[kSignal] = 0.0;
    values[kCentrality] = 50.0;

  }
};
}

#endif
