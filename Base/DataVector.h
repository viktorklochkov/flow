//
// Created by Lukas Kreis on 03.08.17.
//

#ifndef FLOW_DATAVECTOR_H
#define FLOW_DATAVECTOR_H

#include <math.h>

#include "Rtypes.h"

namespace Qn {
/**
 * simple struct containing information of the raw data for the use in the DataContainer.
 */
struct DataVector {
  /**
   * Default constructor
   */
  DataVector() = default;
/**
 * Constructor with default weight 0.
 * @param phi azimuthal angle
 */
  explicit DataVector(float phi) : phi(phi), weight(1.0) {}
  /**
   * Constructor
   * @param phi azimuthal angle
   * @param weight
   */
  DataVector(float phi, float weight) : phi(phi), weight(weight) {}
  virtual ~DataVector() = default;
  float phi{NAN}; ///< Azimuthal angle of signal
  float weight{NAN}; ///< weight of signal

  /// \cond CLASSIMP
 ClassDef(DataVector, 2);
  /// \endcond
};

}

#endif //FLOW_DATAVECTOR_H
