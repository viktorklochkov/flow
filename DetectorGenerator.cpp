//
// Created by Lukas Kreis on 03.08.17.
//

#include <QnCorrections/QnCorrectionsDetectorConfigurationTracks.h>
#include <QnCorrections/QnCorrectionsDetectorConfigurationChannels.h>
#include <iostream>
#include "DetectorGenerator.h"
namespace Qn {

QnCorrectionsDetector *DetectorGenerator::GenerateDetector(int globalid, int detid, int binid, DetectorTuple &tuple) {
  auto type = std::get<0>(tuple);
  auto config  = std::get<3>(tuple);
  auto nchannels = std::get<2>(tuple);
  auto& datacontainer = std::get<1>(tuple);
  auto detectorname = std::string(Configuration::DetectorNames[detid]);
  auto binname = datacontainer->GetBinDescription(binid);
  auto name  = (detectorname + std::to_string(binid)).c_str();
  std::cout << name << std::endl;
  auto detector = new QnCorrectionsDetector(name, globalid);
  auto configuration = CreateDetectorConfiguration(type, name, nchannels);
  (*config)(configuration);
  detector->AddDetectorConfiguration(configuration);
  return detector;
}
QnCorrectionsDetectorConfigurationBase *DetectorGenerator::CreateDetectorConfiguration(Configuration::DetectorType type,
                                                                                       std::string name,
                                                                                       int nchannels = 0) {
  QnCorrectionsDetectorConfigurationBase *configuration = nullptr;
  if (type == Configuration::DetectorType::Channel) {
    configuration = new QnCorrectionsDetectorConfigurationChannels(name.data(), event_variables_, nchannels,
                                                                   n_harmonics_);
    ((QnCorrectionsDetectorConfigurationChannels *) configuration)->SetChannelsScheme(nullptr, nullptr, nullptr);
  }
  if (type == Configuration::DetectorType::Track)
    configuration = new QnCorrectionsDetectorConfigurationTracks(name.data(), event_variables_,
                                                                 n_harmonics_);
  return configuration;
}

}
