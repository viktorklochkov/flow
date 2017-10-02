//
// Created by Lukas Kreis on 03.08.17.
//

#ifndef FLOW_DETECTORGENERATOR_H
#define FLOW_DETECTORGENERATOR_H

#include <QnCorrections/QnCorrectionsDetector.h>
#include <array>
#include "DetectorConfig.h"
#include "DetectorMap.h"

namespace Qn {

/**
 * @brief Generates detectors
 * #TODO Add way to configure corrections to be applied
 */
class DetectorGenerator {
  using DetectorTuple = std::tuple<Configuration::DetectorType,
                                   std::unique_ptr<DataContainerDataVector>,
                                   int,
                                   Configuration::DetectorConfig *>;
 public:
//  DetectorGenerator() = default;
//  ~DetectorGenerator() = default
/**
 * Generate detector
 * @param id identification of detector / binnumber
 * @param type type of detector
 * @param nchannels number of channels / only used for channel detector
 * @param config DetectorConfig function object for configuration of correction steps
 * @return configured detector
 */
  QnCorrectionsDetector *GenerateDetector(int globalid, int detid, int binid, DetectorTuple &tuple);
  /**
   * Set event variables used for correction before the generation of detectors
   * @param set event variables used for the correction
   */
  inline void SetEventVariables(QnCorrectionsEventClassVariablesSet *set) { event_variables_ = set; }
 private:
  int n_harmonics_ = 4;
  QnCorrectionsEventClassVariablesSet *event_variables_ = nullptr;
  /**
   * Creates detector configuration for the correction framework
   * @param type type of detector
   * @param name name of detector
   * @param nchannels number of channels / only needed for channel detectors
   * @return configured detector
   */
  QnCorrectionsDetectorConfigurationBase *CreateDetectorConfiguration(Configuration::DetectorType type,
                                                                      std::string name,
                                                                      int nchannels);
};
}

#endif //FLOW_DETECTORGENERATOR_H
