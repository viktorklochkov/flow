//
// Created by Lukas Kreis on 03.08.17.
//

#ifndef FLOW_DATAVECTOR_H
#define FLOW_DATAVECTOR_H

#include "Rtypes.h"
#include <math.h>
namespace Qn {
struct DataVector {
  DataVector() : phi(NAN), weight(NAN) {}
  DataVector(float phi, float weight) : phi(phi), weight(weight) {}
  float phi;
  float weight;

  /// \cond CLASSIMP
 ClassDef(DataVector, 1);
  /// \endcond
};

}

#endif //FLOW_DATAVECTOR_H
