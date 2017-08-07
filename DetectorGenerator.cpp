//
// Created by Lukas Kreis on 03.08.17.
//

#include <QnCorrections/QnCorrectionsDetectorConfigurationTracks.h>
#include <QnCorrections/QnCorrectionsDetectorConfigurationChannels.h>
#include <QnCorrections/QnCorrectionsQnVectorRecentering.h>
#include "DetectorGenerator.h"
namespace Qn {

QnCorrectionsDetector *DetectorGenerator::GenerateDetector(int id, DetectorType type) {
  auto detector = new QnCorrectionsDetector(std::to_string(id).data(), id);
  auto configuration = CreateDetectorConfiguration(type, std::to_string(id));
  configuration->AddCorrectionOnQnVector(new QnCorrectionsQnVectorRecentering);
  detector->AddDetectorConfiguration(configuration);
  return detector;
}

QnCorrectionsDetectorConfigurationBase *DetectorGenerator::CreateDetectorConfiguration(DetectorType type, std::string name) {
  QnCorrectionsDetectorConfigurationBase *configuration = nullptr;
//  if (type == DetectorType::Channel) configuration = new QnCorrectionsDetectorConfigurationChannels(name.data(),eventvars,nchannels,nharmonics,harmonics);
  if (type == DetectorType::Track) configuration = new QnCorrectionsDetectorConfigurationTracks(name.data(),event_variables_,n_harmonics_,harmonics_);
  return configuration;
}

}
