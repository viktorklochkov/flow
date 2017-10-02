//
// Created by Lukas Kreis on 24.08.17.
//
#include "DetectorMap.h"
#include "DetectorGenerator.h"
namespace Qn {
namespace Internal {
void AddDetectorToMap(DetectorMap &map,
                      Configuration::DetectorId id,
                      Configuration::DetectorConfig *config,
                      Configuration::DetectorType type,
                      std::vector<Qn::Axis> axes,
                      int channels) {
  std::unique_ptr<Qn::DataContainerDataVector> data(new Qn::DataContainerDataVector());
  for (auto axis : axes) {
    data->AddAxis(axis);
  }
  std::tuple<Configuration::DetectorType,
             std::unique_ptr<Qn::DataContainerDataVector>,
             int,
             Configuration::DetectorConfig*> tuple(type, std::move(data), channels, config);
  map.insert(std::make_pair((int) id, std::move(tuple)));

}

void AddDetectorToMap(DetectorMap &map,
                      Configuration::DetectorId id,
                      Configuration::DetectorConfig *config,
                      Configuration::DetectorType type,
                      int channels) {
  std::unique_ptr<Qn::DataContainerDataVector> data(new Qn::DataContainerDataVector());
  Qn::Axis integrated("integrated", 1, 0, 1, -999);
  data->AddAxis(integrated);
  std::tuple<Configuration::DetectorType,
             std::unique_ptr<Qn::DataContainerDataVector>,
             int,
             Configuration::DetectorConfig*> tuple(type, std::move(data), channels, config);
  map.insert(std::make_pair((int) id, std::move(tuple)));
}
}
}
