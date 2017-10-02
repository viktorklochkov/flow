//
// Created by Lukas Kreis on 22.08.17.
//

#ifndef FLOW_DETECTORMAP_H
#define FLOW_DETECTORMAP_H
#include "DataContainer.h"
#include "DetectorConfig.h"
#include <map>
#include <tuple>
namespace Qn {

namespace Internal {
/**
 * Typedef for DetectorMap for storing the configuration of the detectors.
 */
using DetectorMap = std::map<int, std::tuple<Configuration::DetectorType, std::unique_ptr<DataContainerDataVector>, int, Configuration::DetectorConfig*>>;


void AddDetectorToMap(DetectorMap &map,
                      Configuration::DetectorId id,
                      Configuration::DetectorConfig *config,
                      Configuration::DetectorType type,
                      std::vector<Qn::Axis> axes,
                      int channels = 0);

void AddDetectorToMap(DetectorMap &map,
                      Configuration::DetectorId id,
                      Configuration::DetectorConfig *config,
                      Configuration::DetectorType type,
                      int channels = 0);
}
}

#endif //FLOW_DETECTORMAP_H
