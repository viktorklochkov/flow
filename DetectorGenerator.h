//
// Created by Lukas Kreis on 03.08.17.
//

#ifndef FLOW_DETECTORGENERATOR_H
#define FLOW_DETECTORGENERATOR_H

#include <QnCorrections/QnCorrectionsDetector.h>
#include <array>

namespace Qn {
enum class DetectorType {
  Track,
  Channel
};
/**
 * @brief Generates detectors
 * #TODO Add way to configure corrections to be applied
 */
class DetectorGenerator {
 public:
//  DetectorGenerator() = default;
//  ~DetectorGenerator() = default
/**
 * Generate detector
 * @param id identification of detector / binnumber
 * @param type type of detector
 * @param nchannels number of channels / only used for channel detector
 * @return configured detector
 */
  QnCorrectionsDetector *GenerateDetector(int id, DetectorType type, int nchannels);
  /**
   * Set event variables used for correction before the generation of detectors
   * @param set event variables used for the correction
   */
  inline void SetEventVariables(QnCorrectionsEventClassVariablesSet *set) { event_variables_ = set; }
 private:
  int n_harmonics_ = 10;
  QnCorrectionsEventClassVariablesSet *event_variables_ = nullptr;
  /**
   * Creates detector configuration for the correction framework
   * @param type type of detector
   * @param name name of detector
   * @param nchannels number of channels / only needed for channel detectors
   * @return configured detector
   */
  QnCorrectionsDetectorConfigurationBase *CreateDetectorConfiguration(DetectorType type, std::string name, int nchannels);
};
}

#endif //FLOW_DETECTORGENERATOR_H
