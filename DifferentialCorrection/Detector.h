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

namespace Qn {
enum class DetectorType {
  Track,
  Channel
};
class Detector {
 public:
  Detector(const DetectorType type,
           const std::vector<Qn::Axis> &axes,
           const Variable phi,
           const Variable weight,
           const std::vector<Variable> &vars) :
      nchannels_(weight.length()),
      type_(type),
      phi_(phi), weight_(weight),
      vars_(vars),
      datavector_(new Qn::DataContainerDataVector()),
      qvector_(new Qn::DataContainerQVector()) {
    coordinates_.resize(vars.size());
    datavector_->AddAxes(axes);
    qvector_->AddAxes(axes);
  }

  explicit Detector(const DetectorType type) :
      type_(type), datavector_(new Qn::DataContainerDataVector()),
      qvector_(new Qn::DataContainerQVector()) {}

  void ClearData() {
    datavector_->ClearData();
    qvector_->ClearData();
  }

  QnCorrectionsDetector *GenerateDetector(const std::string &detname,
                                          int globalid,
                                          int binid,
                                          QnCorrectionsEventClassVariablesSet *set) {
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
                                                                      QnCorrectionsEventClassVariablesSet *set) {
    QnCorrectionsDetectorConfigurationBase *configuration = nullptr;
    if (type_==DetectorType::Channel) {
      configuration =
          new QnCorrectionsDetectorConfigurationChannels(name.data(), set, nchannels_, nharmonics_);
    }
    if (type_==DetectorType::Track)
      configuration = new QnCorrectionsDetectorConfigurationTracks(name.data(), set, nharmonics_);
    return configuration;
  }

  std::unique_ptr<DataContainerDataVector> &GetDataContainer() { return datavector_; }
  std::unique_ptr<DataContainerQVector> &GetQnDataContainer() { return qvector_; }
  void SetConfig(std::function<void(QnCorrectionsDetectorConfigurationBase *config)> conf) { configuration_ = conf; }

  template<typename FUNCTION>
  void AddCut(Variable var, FUNCTION lambda) {
    if (var.length()==weight_.length()) {
      cuts_.emplace_back(var, lambda);
    }
    if (var.length()==1) {
      cuts_int_.emplace_back(var, lambda);
    }
  }

  void FillData() {
    long i = 0;
    int icut = 1;
    int icut_ch = 1;
    if (cut_report_) cut_report_->Fill(0);
    if (!std::all_of(cuts_int_.begin(), cuts_int_.end(), [&icut, this](VariableCut cut) {
      auto check = cut.Check();
      if (check && cut_report_) cut_report_->Fill(icut);
      icut++;
      return check;
    }))
      return;
    for (auto &histo : histograms_) {
      histo->Fill();
    }
    for (auto weight : weight_) {
      if (cut_report_channels_) cut_report_channels_->Fill(0., i);
      icut_ch = 1;
      if (!std::all_of(cuts_.begin(), cuts_.end(), [&icut_ch, this, i](VariableCut cut) {
        auto check = cut.Check(i);
        if (check && cut_report_channels_) cut_report_channels_->Fill(icut_ch, i);
        icut_ch++;
        return check;
      }))
        continue;
      if (vars_.size()!=0) {
        long icoord = 0;
        for (const auto var : vars_) {
          coordinates_.at(icoord) = *(var.begin() + i);
          ++icoord;
        }
        datavector_->CallOnElement(datavector_->GetLinearIndex(coordinates_),
                                   [&](std::vector<DataVector> &vector) {
                                     vector.emplace_back(*(phi_.begin() + i),
                                                         weight);
                                   });
      } else {
        datavector_->CallOnElement(0, [&](std::vector<DataVector> &vector) {
          vector.emplace_back(*(phi_.begin() + i),
                              weight);
        });
      }
      ++i;
    }
  }

  const DetectorType Type() const { return type_; }

  void Initialize(const std::string &name, const VariableManager &man) {
    // cuts int
    const char *charname = (name + std::string("_cut_report")).data();
    int size = cuts_int_.size();
    if (size!=0) {
      cut_report_ = std::make_shared<TH1I>(charname, charname, size + 1, 0, size + 1);
      cut_report_->GetXaxis()->SetBinLabel(1, "all");
      for (int i = 2; i < cuts_int_.size() + 2; ++i) {
        cut_report_->GetXaxis()->SetBinLabel(i, man.FindName(cuts_int_.at(i - 2).GetVariable()).data());
      }
    }
    // cuts channels
    int size_channels = cuts_.size();
    if (size_channels!=0) {
      cut_report_channels_ =
          std::make_shared<TH2I>(charname,
                                 charname,
                                 size_channels + 1,
                                 0,
                                 size_channels + 1,
                                 nchannels_,
                                 0,
                                 nchannels_);
      cut_report_channels_->GetXaxis()->SetBinLabel(1, "all");
      for (int i = 2; i < cuts_.size() + 2; ++i) {
        cut_report_channels_->GetXaxis()->SetBinLabel(i, man.FindName(cuts_.at(i - 2).GetVariable()).data());
      }
    }
  }

  void SaveReport() {
    if (cut_report_) cut_report_->Write(cut_report_->GetName(), TObject::kSingleKey);
    if (cut_report_channels_) cut_report_channels_->Write(cut_report_channels_->GetName(), TObject::kSingleKey);
    for (auto &histo : histograms_) {
      histo->Write(histo->Name());
    }
  }

  template <typename HISTO, unsigned long N>
  void AddHistogram(std::array<Variable,N> vars, const HISTO &histo) {
    histograms_.push_back(std::make_unique<QAHisto<HISTO,N,Variable>>(vars,histo));
  }

 private:
  int nchannels_ = 0;
  int nharmonics_ = 4;
  DetectorType type_;
  Variable phi_;
  Variable weight_;
  std::vector<Variable> vars_;
  std::vector<float> coordinates_;
  std::vector<VariableCut> cuts_;
  std::vector<VariableCut> cuts_int_;
  std::shared_ptr<TH1I> cut_report_ = nullptr;
  std::shared_ptr<TH2I> cut_report_channels_ = nullptr;
  std::vector<std::unique_ptr<QAHistoBase>> histograms_;
  std::unique_ptr<DataContainerDataVector> datavector_;
  std::unique_ptr<DataContainerQVector> qvector_;
  std::function<void(QnCorrectionsDetectorConfigurationBase *config)> configuration_;
};
}

#endif //FLOW_DETECTOR_H
