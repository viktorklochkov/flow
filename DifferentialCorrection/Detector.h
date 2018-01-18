//
// Created by Lukas Kreis on 16.01.18.
//

#ifndef FLOW_DETECTOR_H
#define FLOW_DETECTOR_H
#include <utility>
#include <QnCorrections/QnCorrectionsDetector.h>
#include <QnCorrections/QnCorrectionsEventClassVariablesSet.h>
#include <QnCorrections/QnCorrectionsDetectorConfigurationTracks.h>

#include "DetectorConfig.h"
#include "Base/DataContainer.h"
#include "Base/QVector.h"
#include "Base/DataVector.h"

namespace Qn {
enum class DetectorType {
  Track,
  Channel
};
class Detector {
 public:
  Detector(DetectorType type, const std::vector<Qn::Axis> &axes, std::vector<int> enums) :
      type_(type),
      enums_(std::move(enums)),
      datavector_(new Qn::DataContainerDataVector()),
      qvector_(new Qn::DataContainerQVector()) {
    datavector_->AddAxes(axes);
    qvector_->AddAxes(axes);
  }

  Detector(DetectorType type, const std::vector<Qn::Axis> &axes, std::vector<int> enums, int nchannels) :
      nchannels_(nchannels),
      type_(type),
      enums_(std::move(enums)),
      datavector_(new Qn::DataContainerDataVector()),
      qvector_(new Qn::DataContainerQVector()) {
    datavector_->AddAxes(axes);
    qvector_->AddAxes(axes);
  }

  Detector(DetectorType type, int nchannels) :
      nchannels_(nchannels),
      type_(type),
      datavector_(new Qn::DataContainerDataVector()),
      qvector_(new Qn::DataContainerQVector()) {
  }

  explicit Detector(DetectorType type) :
      type_(type), datavector_(new Qn::DataContainerDataVector()),
      qvector_(new Qn::DataContainerQVector()) {
  }

  void ClearData() {
    datavector_->ClearData();
    qvector_->ClearData();
  }

  QnCorrectionsDetector *GenerateDetector(const std::string &detname,
                                          int globalid,
                                          int binid,
                                          QnCorrectionsEventClassVariablesSet *set) {
    auto binname = datavector_->GetBinDescription(binid);
    auto name = (detname + std::to_string(binid)).c_str();
    std::cout << name << std::endl;
    auto detector = new QnCorrectionsDetector(name, globalid);
    auto configuration = CreateDetectorConfiguration(name, set);
    configuration_(configuration);
    detector->AddDetectorConfiguration(configuration);
    return detector;
  }

  QnCorrectionsDetectorConfigurationBase *CreateDetectorConfiguration(const std::string &name,
                                                                      QnCorrectionsEventClassVariablesSet *set) {
    QnCorrectionsDetectorConfigurationBase *configuration = nullptr;
    if (type_==DetectorType::Channel) {
      configuration =
          new QnCorrectionsDetectorConfigurationChannels(name.data(), set, nchannels_, nharmonics_);
      ((QnCorrectionsDetectorConfigurationChannels *) configuration)->SetChannelsScheme(nullptr, nullptr, nullptr);
    }
    if (type_==DetectorType::Track)
      configuration = new QnCorrectionsDetectorConfigurationTracks(name.data(), set, nharmonics_);
    return configuration;
  }

  std::unique_ptr<DataContainerDataVector> &GetDataContainer() { return datavector_; }
  std::unique_ptr<DataContainerQVector> &GetQnDataContainer() { return qvector_; }
  DetectorType GetType() const { return type_; }
  int GetNChannels() const { return nchannels_; }
  std::function<void(QnCorrectionsDetectorConfigurationBase *config)> GetConfig() { return configuration_; }
  void SetConfig(std::function<void(QnCorrectionsDetectorConfigurationBase *config)> conf) { configuration_ = conf; }
  std::vector<int> GetEnums() const { return enums_; }

 private:

  int nchannels_ = 0;
  int nharmonics_ = 4;
  DetectorType type_;
  std::vector<int> enums_;
  std::unique_ptr<DataContainerDataVector> datavector_;
  std::unique_ptr<DataContainerQVector> qvector_;
  std::function<void(QnCorrectionsDetectorConfigurationBase *config)> configuration_;

};
}

#endif //FLOW_DETECTOR_H
