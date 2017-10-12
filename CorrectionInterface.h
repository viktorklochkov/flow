//
// Created by Lukas Kreis on 03.08.17.
//
#ifndef FLOW_CORRECTIONINTERFACE_H
#define FLOW_CORRECTIONINTERFACE_H

#include <QnCorrections/QnCorrectionsManager.h>
#include "DataContainer.h"
#include "DetectorGenerator.h"
#include "EventInfo.h"
#include "DataInterface.h"

namespace Qn {
/**
 * Namespace used for internal calculations
 */
namespace Internal {
/**
 * Add Detector for differential \f$Q_n\f$ vectors
 * @param[in,out] map Map to which detector configuration should be added.
 * @param id Identification of detector
 * @param type type of detector: channel or tracking. determines how framework handles correction
 * @param axes vector of axes, which determine binning of \f$Q_n\f$ vectors
 * @param config function object to configure correctionsteps
 * @param channels number of channels in case of channel detector ( not used for tracking detectors)
 */
//void AddDetectorToMap(DetectorMap &map, Qn::Interface::DetectorId id, DetectorType type, std::vector<Qn::Axis> axes,
//                      Configuration::DetectorConfig config, int channels = 0);

/**
 * Adds a detector for integrated \f$Q_n\f$ vectors
 * @param[in,out] map Map to which detector configuration should be added.
 * @param id Identification of detector.
 * @param type type of detector: channel or tracking. This determines how framwork handles correction
 * @param config function object to configure correctionsteps
 * @param channels number of channels in case of channel detector ( not used for tracking detectors)
 */
//void AddDetectorToMap(DetectorMap &map, Qn::Interface::DetectorId id, DetectorType type, Configuration::DetectorConfig config, int channels = 0);

inline void ClearDataInMap(DetectorMap &map) {
  for (auto &pair : map) {
    std::get<1>(pair.second)->ClearData();
  }
}

inline void ClearDataInMap(std::map<int, std::unique_ptr<Qn::DataContainerQn>> &map) {
  for (const auto &pair : map) {
    pair.second->ClearData();
  }
}

/**
 * Creates DataContainerQn containing \f$Q_n\f$ Vectors from a DataContainerDataVector.
 * @param map Detector map containing raw data \f$(\phi,w)\f$ with the specified binning.
 * @return DataContainer with the same configuration as the raw data container.
 */
std::map<int, std::unique_ptr<Qn::DataContainerQn>> MakeQnDataContainer(const DetectorMap &map);

/**
 * Fills data to framework for the correction
 * @param[in,out] manager Corrections manager used for the correction of the \f$Q_n\f$ vectors
 * @param map DetectorMap containing the raw data vectors \f$(\phi,w)\f$.
 */
void FillDataToFramework(QnCorrectionsManager &manager, DetectorMap &map);

/**
 * Creates Detectors corresponding to the detectors saved in the DetectorMap.
 * @param[in,out] manager Corrections manager used for the correction of the \f$Q_n\f$ vectors.
 * @param map DetectorMap containing the raw data vectors \f$(\phi,w)\f$.
 * @param set Event variable set to be used for the correction of the detectors.
 */
void AddDetectorsToFramework(QnCorrectionsManager &manager,
                             DetectorMap &map,
                             QnCorrectionsEventClassVariablesSet &set);

/**
 * Gets corrected \f$Q_n\f$ vectors from the QnCorrectionsFramework.
 * @param[in,out] manager Corrections manager used for the correction of the \f$Q_n\f$ vectors.
 * @param[out] map Data container in which the \f$Q_n\f$ vectors are inserted into.
 * @param step correction step of the requested \f$Q_n\f$ vector.
 */
void GetQnFromFramework(QnCorrectionsManager &manager, std::map<int, std::unique_ptr<Qn::DataContainerQn>> &map, std::string step = "latest");

void SaveToTree(TTree &tree, std::map<int, std::unique_ptr<Qn::DataContainerQn>> &map);

void SaveToTree(TTree &tree, std::map<int, std::unique_ptr<Qn::DataContainerDataVector>> &map);

void SaveToTree(TTree &tree, std::unique_ptr<Qn::EventInfoF> &eventinfo);
}
}

#endif //FLOW_CORRECTIONINTERFACE_H
