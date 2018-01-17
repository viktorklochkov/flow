//
// Created by Lukas Kreis on 03.08.17.
//

#ifndef FLOW_DATAVECTOR_H
#define FLOW_DATAVECTOR_H

#include "Rtypes.h"
//#include "Base/DataContainer.h"
#include <math.h>

namespace Qn {
/**
 * simple struct containing information of the raw data for the use in the DataContainer.
 */
struct DataVector {
  DataVector() : phi(NAN), weight(NAN) {}
  DataVector(float phi) : phi(phi), weight(1.0) {}
  DataVector(float phi, float weight) : phi(phi), weight(weight) {}
  virtual ~DataVector() = default;
  float phi; ///< Azimuthal angle of signal
  float weight; ///< weight of signal

  void test() {
//    Qn::Internal::DetectorMap map;
//    int a = (int) Qn::Interface::DetectorId::TPC;
  }
  /// \cond CLASSIMP
 ClassDef(DataVector, 2);
  /// \endcond
};



}

#endif //FLOW_DATAVECTOR_H
