// Flow Vector Correction Framework
//
// Copyright (C) 2018  Lukas Kreis, Ilya Selyuzhenkov
// Contact: l.kreis@gsi.de; ilya.selyuzhenkov@gmail.com
// For a full list of contributors please see docs/Credits
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef FLOW_DETECTOR_H
#define FLOW_DETECTOR_H

#include <memory>
#include <utility>

#include "CorrectionDetector.h"
#include "EventClassVariablesSet.h"
#include "DetectorConfigurationChannels.h"
#include "DetectorConfigurationTracks.h"

#include "VariableManager.h"
#include "DataContainer.h"
#include "QVector.h"
#include "DataVector.h"
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

  virtual CorrectionDetector *GenerateDetector(const std::string &detname, int globalid, int binid,
                                                  EventClassVariablesSet *set) = 0;
  virtual DetectorConfiguration *CreateDetectorConfiguration(const std::string &name,
                                                                              EventClassVariablesSet *set) = 0;
  virtual void SetConfig(std::function<void(DetectorConfiguration *config)> conf) = 0;
  virtual void AddCut(std::unique_ptr<VariableCutBase> cut) = 0;
  virtual void AddHistogram(std::unique_ptr<QAHistoBase> base) = 0;
  virtual void Initialize(const std::string &name) = 0;
  virtual void FillReport() = 0;
  virtual void FillData() = 0;
  virtual void ClearData() = 0;
  virtual void SaveReport() = 0;
  virtual TList* GetReportList() = 0;
  virtual void ReserveDataVectors(int number) = 0;
  virtual std::bitset<8> GetHarmonics() const = 0;

};

template <std::size_t N>
class Detector : public DetectorBase {
 public:
  Detector(const DetectorType type,
           std::vector<Qn::Axis> axes,
           const Variable phi,
           const Variable weight,
           const std::vector<Variable> &vars,
           int const(&harmo)[N]) :
      nchannels_(phi.length()),
      harmonics_(new int[N]),
      type_(type),
      phi_(phi),
      weight_(weight),
      vars_(vars),
      cuts_(new Qn::Cuts),
      int_cuts_(new Qn::Cuts),
      datavector_(new Qn::DataContainerDataVector()),
      qvector_(new Qn::DataContainerQVector()) {
    coordinates_.resize(vars.size());
    datavector_->AddAxes(axes);
    qvector_->AddAxes(axes);
    for (unsigned int i = 0; i < N; ++i) {
      harmonics_[i] = harmo[i];
      harmonics_bits_.set(harmo[i]);
    }
  }

  std::bitset<8> GetHarmonics() const override {
    return harmonics_bits_;
  }

  void ClearData() override {
    datavector_->ClearData();
    qvector_->ClearData();
  }

  CorrectionDetector *GenerateDetector(const std::string &detname,
                                          int globalid,
                                          int binid,
                                          EventClassVariablesSet *set) override {
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
    auto detector = new CorrectionDetector(name.data(), globalid);
    auto configuration = CreateDetectorConfiguration(name, set);
    configuration_(configuration);
    detector->AddDetectorConfiguration(configuration);
    return detector;
  }

  DetectorConfiguration *CreateDetectorConfiguration(const std::string &name,
                                                                      EventClassVariablesSet *set) override {
    DetectorConfiguration *configuration = nullptr;
    if (type_==DetectorType::CHANNEL) {
      configuration =
          new DetectorConfigurationChannels(name.data(), set, nchannels_, nharmonics_, harmonics_.get());
    }
    if (type_==DetectorType::TRACK)
      configuration = new DetectorConfigurationTracks(name.data(), set, nharmonics_, harmonics_.get());
    return configuration;
  }
  std::unique_ptr<DataContainerDataVector> &GetDataContainer() override { return datavector_; }
  std::unique_ptr<DataContainerQVector> &GetQnDataContainer() override { return qvector_; }
  void SetConfig(std::function<void(DetectorConfiguration *config)> conf) override {
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
    for (const auto &phi : phi_) {
      if (!cuts_->CheckCuts(i)) {
        ++i;
        continue;
      }
      if (vars_.empty()) {
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
          datavector_->CallOnElement(coordinates_,
                                     [&](std::vector<DataVector> &vector) {
                                       vector.emplace_back(phi, *(weight_.begin() + i));
                                     });
        }
        catch (std::exception &) {}
      }
      ++i;
    }
  }

  DetectorType Type() const { return type_; }

  void Initialize(const std::string &name) override {
    int_cuts_->CreateCutReport(name, 1);
    cuts_->CreateCutReport(name, static_cast<size_t>(phi_.length()));
  }

  void SaveReport() override {
    int_cuts_->Write("");
    cuts_->Write("Channel");
    for (auto &histo : histograms_) {
      histo->Write(histo->Name());
    }
  }

  TList* GetReportList() override {
    auto list = new TList();
    int_cuts_->AddToList(list);
    cuts_->AddToList(list);
    for (auto &histo : histograms_) {
      histo->AddToList(list);
    }
    return list;
  }

  void FillReport() override {
    int_cuts_->FillReport();
    cuts_->FillReport();
  }

  void AddHistogram(std::unique_ptr<QAHistoBase> histo) override {
    histograms_.push_back(std::move(histo));
  }

  void ReserveDataVectors(int number) override {
    for (auto &bin : *datavector_) {
      bin.reserve(number);
    }
  }

 private:
  int nchannels_ = 0;
  int nharmonics_ = N;
  std::bitset<8> harmonics_bits_;
  std::unique_ptr<int[]> harmonics_;
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
  std::function<void(DetectorConfiguration *config)> configuration_;
};
}

#endif //FLOW_DETECTOR_H
