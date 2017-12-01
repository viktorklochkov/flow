//
// Created by Lukas Kreis on 20.10.17.
//

#ifndef FLOW_RESOLUTION_H
#define FLOW_RESOLUTION_H

#include <TTreeReader.h>
#include <TList.h>
#include "DataContainer.h"
#include "TH1D.h"
#include "TProfile.h"
namespace Qn {

namespace Resolution {

inline float PsiN(Qn::QVector a, int n) {
  return 1 / (float) n * atan2(a.y(n), a.x(n));
}

}
}

#endif //FLOW_RESOLUTION_H
