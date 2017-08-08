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
class DetectorGenerator {
 public:
//  DetectorGenerator() = default;
//  ~DetectorGenerator() = default;
  QnCorrectionsDetector *GenerateDetector(int id, DetectorType type, int nchannels);
  inline void SetEventVariables(QnCorrectionsEventClassVariablesSet *set) { event_variables_ = set; }
  inline void SetHarmonics(std::array<int, 16> harmonics) {
    int i = 0;
    for (auto harmonic : harmonics) {
      harmonics_[i] = harmonic;
      ++i;
    }
  }
 private:
  int n_harmonics_ = 4;
  int harmonics_[4] = {1,1,1,1};
  QnCorrectionsEventClassVariablesSet *event_variables_ = nullptr;
  QnCorrectionsDetectorConfigurationBase *CreateDetectorConfiguration(DetectorType type, std::string name, int nchannels);
};
}

#endif //FLOW_DETECTORGENERATOR_H
