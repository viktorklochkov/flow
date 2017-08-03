//
// Created by Lukas Kreis on 03.08.17.
//

#ifndef FLOW_DETECTORGENERATOR_H
#define FLOW_DETECTORGENERATOR_H

#include <QnCorrections/QnCorrectionsDetector.h>

namespace Qn {
class DetectorGenerator {
  enum class DetectorType {
    Track,
    Channel
  };
 public:
  DetectorGenerator() = default;
  ~DetectorGenerator() = default;
  QnCorrectionsDetector *GenerateDetector(DetectorType type);
 private:
  QnCorrectionsDetectorConfigurationBase *CreateDetectorConfiguration(DetectorType type);
};
}

#endif //FLOW_DETECTORGENERATOR_H
