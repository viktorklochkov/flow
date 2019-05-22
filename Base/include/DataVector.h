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

#ifndef FLOW_DATAVECTOR_H
#define FLOW_DATAVECTOR_H

#include <math.h>
#include "Rtypes.h"
#include "TClonesArray.h"

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

class DataVectorHolder {
 public:
  DataVectorHolder() = default;
  virtual ~DataVectorHolder() = default;
  unsigned int nentries = 0;
  TClonesArray *array = nullptr; //!<! non persistent
  /// \cond CLASSIMP
 ClassDef(DataVectorHolder, 1);
  /// \endcond
};

}

#endif //FLOW_DATAVECTOR_H
