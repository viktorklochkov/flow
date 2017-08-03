//
// Created by Lukas Kreis on 03.08.17.
//

#include <QnCorrections/QnCorrectionsDetectorConfigurationTracks.h>
#include <QnCorrections/QnCorrectionsDetectorConfigurationChannels.h>
#include "DetectorGenerator.h"
namespace Qn {

QnCorrectionsDetector *DetectorGenerator::GenerateDetector(DetectorType type) {
  auto detector = new QnCorrectionsDetector(name.data(), (int) id);
  auto configuration = CreateDetectorConfiguration(type);
  detector->AddDetectorConfiguration(configuration);
  return detector;
}

QnCorrectionsDetectorConfigurationBase *DetectorGenerator::CreateDetectorConfiguration(DetectorType type) {
  QnCorrectionsDetectorConfigurationBase *configuration = nullptr;
  if (type == DetectorType::Channel) configuration = new QnCorrectionsDetectorConfigurationTracks();
  if (type == DetectorType::Track) configuration = new QnCorrectionsDetectorConfigurationChannels();
  return configuration;
}

}
