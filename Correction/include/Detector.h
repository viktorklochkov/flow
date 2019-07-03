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

#include "ROOT/RMakeUnique.hxx"

#include "EventClassVariablesSet.h"
#include "SubEventChannels.h"
#include "SubEventTracks.h"

#include "VariableManager.h"
#include "DataContainer.h"
#include "QVector.h"
#include "QAHistogram.h"
#include "VariableCutBase.h"

namespace Qn {
class DetectorList;
/**
 * Enumerator class used to determine the type of detector
 */
enum class DetectorType {
  TRACK,
  CHANNEL
};

/**
 * Detector class
 * Template parameter automatically deduced.
 * @tparam N number of harmonics activated. Harmonic max is 8.
 */
class Detector {
  using DCSUBEVENTS = Qn::DataContainer<std::unique_ptr<SubEvent>, AxisF>;
 public:
  Detector(std::string name, const DetectorType type, std::vector<AxisF> axes, const InputVariableD phi,
           const InputVariableD weight, const std::vector<InputVariableD> &vars, std::bitset<Qn::QVector::kmaxharmonics> harmonics)
      :
      name_(name),
      nchannels_(phi.length()),
      harmonics_bits_(harmonics),
      type_(type),
      phi_(phi),
      weight_(weight),
      vars_(vars),
      cuts_(new Qn::Cuts),
      int_cuts_(new Qn::Cuts) {
    coordinates_.resize(vars.size());
    subevents_.AddAxes(axes);
  }

  /**
   * @brief Clears data before filling new event.
   */
  void ClearData() {
    for (auto &ev : subevents_) {
      ev->Clear();
    }
  }

  void ConfigureSubEvents(const EventClassVariablesSet &set) {
    if (!configuration_) {
      throw (std::runtime_error("No Qn correction configuration found for " + name_));
    }
    int ibin = 0;
    for (auto &bin : subevents_) {
      std::string name = name_;
      if (!subevents_.IsIntegrated()) name += std::to_string(ibin);
      if (type_==DetectorType::CHANNEL) {
        bin = std::make_unique<SubEventChannels>(ibin, &set, nchannels_, harmonics_bits_);
      } else if (type_==DetectorType::TRACK) {
        bin = std::make_unique<SubEventTracks>(ibin, &set, harmonics_bits_);
      }
      configuration_(bin.get());
      ++ibin;
    }
  }
  /**
   * @brief Add the configuration function to the detector.
   * @param conf function which configures the detector. needs to have the specified form.
   */
  void SetConfig(std::function<void(SubEvent *config)> conf) {
    configuration_ = std::move(conf);
  }

  DetectorType GetType() const { return type_; }

  std::string GetName() const { return name_; }

  std::string GetBinName(unsigned int ibin) const { return name_ + subevents_.GetBinDescription(ibin); }

  /**
   * @brief Adds a cut to the detector
   * @param cut unique pointer to the cut. It is moved into the function and cannot be reused!
   */
  void AddCut(std::unique_ptr<VariableCutBase> cut) {
    if (cut->GetVariableLength()==1) {
      int_cuts_->AddCut(std::move(cut));
    } else {
      cuts_->AddCut(std::move(cut));
    }
  }

  /**
   * @brief Fills the data into the data vectors, histograms and cut reports after the cuts have been checked.
   */
  void FillData() {
    long i = 0;
    if (!int_cuts_->CheckCuts(0)) return;
    for (auto &histo : histograms_) {
      histo->Fill();
    }
    for (const auto phi : phi_) {
      if (!cuts_->CheckCuts(i)) {
        ++i;
        continue;
      }
      if (vars_.empty()) {
        subevents_.At(0)->AddDataVector(phi, *(weight_.begin() + i), i);
      } else {
        long icoord = 0;
        for (const auto &var : vars_) {
          coordinates_.at(icoord) = *(var.begin() + i);
          ++icoord;
        }
        const auto ibin = subevents_.FindBin(coordinates_);
        if (ibin > -1) subevents_.At(ibin)->AddDataVector(phi, *(weight_.begin() + i), i);
      }
      ++i;
    }
  }

  /**
   * Initializes the histograms used for the cut report.
   * @param name name of the detector.
   */
  void InitializeCutReports() {
    int_cuts_->CreateCutReport(name_, 1);
    cuts_->CreateCutReport(name_, static_cast<size_t>(phi_.length()));
  }

  void IncludeQnVectors() {
    for (auto &ev : subevents_) {
      ev->IncludeQnVectors();
    }
  }

  void AttachSupportHistograms(TList *list) {
    for (auto &ev : subevents_) {
      ev->AttachSupportHistograms(list);
    }
  }

  /**
   * @brief Returns TList of QA and cut report histograms.
   * @return list of histograms. Lifetime managed by the user.
   */
  TList *GetReportList() {
    auto list = new TList();
    int_cuts_->AddToList(list);
    cuts_->AddToList(list);
    for (auto &histo : histograms_) {
      histo->AddToList(list);
    }
    return list;
  }

  /**
   * Fill the cuts reports to the
   */
  void FillReport() {
    int_cuts_->FillReport();
    cuts_->FillReport();
  }

  /**
   * @brief Adds a QA histogram to the detector.
   * @param histo pointer to the histogram.
   */
  void AddHistogram(std::unique_ptr<QAHistoBase> histo) {
    histograms_.push_back(std::move(histo));
  }

  void CreateSupportDataStructures() {
    for (auto &ev : subevents_) {
      ev->CreateSupportDataStructures();
    }
  }

  void AttachCorrectionInputs(TList *list) {
    for (auto &ev : subevents_) {
      ev->AttachCorrectionInputs(list);
    }
  }

  void AfterInputsAttachActions() {
    for (auto &ev : subevents_) {
      ev->AfterInputsAttachActions();
    }
  }

  void AttachQAHistograms(TList *list) {
    for (auto &ev : subevents_) {
      ev->AttachQAHistograms(list);
    }
  }

  void AttachNveQAHistograms(TList *list) {
    for (auto &ev : subevents_) {
      ev->AttachNveQAHistograms(list);
    }
  }

  bool IsIntegrated() const { return subevents_.IsIntegrated(); }

  void ProcessCorrections() {
    for (auto &detector : subevents_) {
      detector->ProcessCorrections(variable_manager_->GetVariableContainer());
    }
    for (auto &detector : subevents_) {
      detector->ProcessDataCollection(variable_manager_->GetVariableContainer());
    }
  }

  SubEvent *GetSubEvent(unsigned int ibin) {
    return subevents_.At(ibin).get();
  }

  void ActivateHarmonic(unsigned int i) {
    harmonics_bits_.set(i - 1);
    for (auto &ev : subevents_) {
      ev->ActivateHarmonic(i);
    }
  }

  DetectorList *GetDetectors() const { return detectors_; }

 private:
  Qn::DetectorList *detectors_ = nullptr;
  Qn::VariableManager *variable_manager_ = nullptr;
  const std::string name_; /// name of  the detector
  int nchannels_ = 0; /// number of channels in case of channel detector
  std::bitset<Qn::QVector::kmaxharmonics> harmonics_bits_; /// bitset of all activated harmonics
  DCSUBEVENTS subevents_;
  const DetectorType type_; /// type of the detector: channel or tracking detector
  const InputVariableD phi_; /// variable holding the azimuthal angle
  const InputVariableD weight_; /// variable holding the weight which is used for the calculation of the Q vector.
  const std::vector<InputVariableD> vars_; /// variables used for the binning of the Q vector.
  std::vector<float> coordinates_;  ///  vector holding the temporary coordinates of one track or channel.
  std::unique_ptr<Cuts> cuts_; /// per channel selection  cuts
  std::unique_ptr<Cuts> int_cuts_; /// integrated selection cuts
  std::vector<std::unique_ptr<QAHistoBase>> histograms_; /// QA histograms of the detector
  /// Container holding the Q vectors of the current event.
  std::function<void(SubEvent *config)> configuration_; /// correction configuration function
};

}

#endif //FLOW_DETECTOR_H
