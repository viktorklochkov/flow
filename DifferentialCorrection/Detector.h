//
// Created by Lukas Kreis on 16.01.18.
//
#ifndef FLOW_DETECTOR_H
#define FLOW_DETECTOR_H
#include <memory>
#include <utility>
#include <QnCorrections/QnCorrectionsDetector.h>
#include <QnCorrections/QnCorrectionsEventClassVariablesSet.h>
#include <QnCorrections/QnCorrectionsDetectorConfigurationChannels.h>
#include <QnCorrections/QnCorrectionsDetectorConfigurationTracks.h>
#include <TH2I.h>
#include "DifferentialCorrection/VariableManager.h"
#include "Base/DataContainer.h"
#include "Base/QVector.h"
#include "Base/DataVector.h"
#include "QAHistogram.h"
#include "VariableCutBase.h"

namespace Qn {
enum class DetectorType {
  TRACK,
  CHANNEL
};

class DetectorBase {
 public:
  virtual ~DetectorBase() = default;

  virtual std::unique_ptr<DataContainerDataVector> &GetDataContainer() = 0;
  virtual std::unique_ptr<DataContainerQVector> &GetQnDataContainer() = 0;

  virtual QnCorrectionsDetector *GenerateDetector(const std::string &detname, int globalid, int binid,
                                                  QnCorrectionsEventClassVariablesSet *set) = 0;
  virtual QnCorrectionsDetectorConfigurationBase *CreateDetectorConfiguration(const std::string &name,
                                                                              QnCorrectionsEventClassVariablesSet *set) = 0;
  virtual void SetConfig(std::function<void(QnCorrectionsDetectorConfigurationBase *config)> conf) = 0;
  virtual void AddCut(std::unique_ptr<VariableCutBase> cut) = 0;
  virtual void AddHistogram(std::unique_ptr<QAHistoBase> base) = 0;
  virtual void Initialize(const std::string &name, const VariableManager &man) = 0;
  virtual void FillData() = 0;
  virtual void ClearData() = 0;
  virtual void SaveReport() = 0;

};

class Detector : DetectorBase {
 public:
  Detector(const DetectorType type,
           const std::vector<Qn::Axis> &axes,
           const Variable phi,
           const Variable weight,
           const std::vector<Variable> &vars) :
      nchannels_(phi.length()),
      type_(type),
      phi_(phi), weight_(weight),
      vars_(vars),
      cuts_(new Qn::Cuts),
      int_cuts_(new Qn::Cuts),
      datavector_(new Qn::DataContainerDataVector()),
      qvector_(new Qn::DataContainerQVector()) {
    coordinates_.resize(vars.size());
    datavector_->AddAxes(axes);
    qvector_->AddAxes(axes);
  }

  explicit Detector(const DetectorType type) :
      type_(type), datavector_(new Qn::DataContainerDataVector()),
      qvector_(new Qn::DataContainerQVector()) {}

  void ClearData() override {
    datavector_->ClearData();
    qvector_->ClearData();
  }

  QnCorrectionsDetector *GenerateDetector(const std::string &detname,
                                          int globalid,
                                          int binid,
                                          QnCorrectionsEventClassVariablesSet *set) override {
    if (!configuration_) {
      throw (std::runtime_error("No Qn correction configuration found for " + detname));
    }
    std::string name;
    if (datavector_->IsIntegrated()) {
      name = detname;
    } else {
      auto binname = datavector_->GetBinDescription(binid);
      name = detname + std::to_string(binid);
    }
    auto detector = new QnCorrectionsDetector(name.data(), globalid);
    auto configuration = CreateDetectorConfiguration(name, set);
    configuration_(configuration);
    detector->AddDetectorConfiguration(configuration);
    return detector;
  }

  QnCorrectionsDetectorConfigurationBase *CreateDetectorConfiguration(const std::string &name,
                                                                      QnCorrectionsEventClassVariablesSet *set) override {
    QnCorrectionsDetectorConfigurationBase *configuration = nullptr;
    if (type_==DetectorType::CHANNEL) {
      configuration =
          new QnCorrectionsDetectorConfigurationChannels(name.data(), set, nchannels_, nharmonics_);
    }
    if (type_==DetectorType::TRACK)
      configuration = new QnCorrectionsDetectorConfigurationTracks(name.data(), set, nharmonics_);
    return configuration;
  }

  std::unique_ptr<DataContainerDataVector> &GetDataContainer() override { return datavector_; }
  std::unique_ptr<DataContainerQVector> &GetQnDataContainer() override { return qvector_; }
  void SetConfig(std::function<void(QnCorrectionsDetectorConfigurationBase *config)> conf) override {
    configuration_ = conf;
  }

  void AddCut(std::unique_ptr<VariableCutBase> cut) override {
    if (cut->GetVariableLength()==1) {
      int_cuts_->AddCut(std::move(cut));
    } else {
      cuts_->AddCut(std::move(cut));
    }
  }

  void FillData() override {
    long i = 0;
    if (!int_cuts_->CheckCuts(0)) return;
    for (auto &histo : histograms_) {
      histo->Fill();
    }
    for (auto phi : phi_) {
      if (!cuts_->CheckCuts(i)) {
        ++i;
        continue;
      }
      if (vars_.empty()) {
//        if (i == 0 && type_ == DetectorType::TRACK) {std::cout << phi << std::endl;}
        datavector_->CallOnElement(0, [&](std::vector<DataVector> &vector) {
          vector.emplace_back(phi, *(weight_.begin() + i));
        });
      } else {
        long icoord = 0;
        for (const auto &var : vars_) {
          coordinates_.at(icoord) = *(var.begin() + i);
          ++icoord;
        }
        try {
          datavector_->CallOnElement(datavector_->GetLinearIndex(coordinates_),
                                     [&](std::vector<DataVector> &vector) {
                                       vector.emplace_back(phi, *(weight_.begin() + i));
                                     });
        }
        catch (std::exception &) {}
      }
      ++i;
    }
  }

  const DetectorType Type() const { return type_; }

  void Initialize(const std::string &name, const VariableManager &man) override {
    int_cuts_->CreateCutReport(name, 1);
    cuts_->CreateCutReport(name, phi_.length());
  }

  void SaveReport() override {
    int_cuts_->Write("");
    cuts_->Write("Channel");
    for (auto &histo : histograms_) {
      histo->Write(histo->Name());
    }
  }

  void FillReport() {
    int_cuts_->FillReport();
    cuts_->FillReport();
  }

  void AddHistogram(std::unique_ptr<QAHistoBase> histo) override {
    histograms_.push_back(std::move(histo));
  }

 private:
  int nchannels_ = 0;
  int nharmonics_ = 4;
  DetectorType type_;
  Variable phi_;
  Variable weight_;
  std::vector<Variable> vars_;
  std::vector<float> coordinates_;
  std::unique_ptr<Cuts> cuts_;
  std::unique_ptr<Cuts> int_cuts_;
  std::vector<std::unique_ptr<QAHistoBase>> histograms_;
  std::unique_ptr<DataContainerDataVector> datavector_;
  std::unique_ptr<DataContainerQVector> qvector_;
  std::function<void(QnCorrectionsDetectorConfigurationBase *config)> configuration_;
};
}

#endif //FLOW_DETECTOR_H
