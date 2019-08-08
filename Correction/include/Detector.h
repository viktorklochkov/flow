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

#include "CorrectionAxisSet.h"
#include "SubEventChannels.h"
#include "SubEventTracks.h"

#include "InputVariableManager.h"
#include "DataContainer.h"
#include "QVector.h"
#include "QAHistogram.h"
#include "CorrectionCuts.h"

namespace Qn {
class DetectorList;
class CorrectionManager;
/**
 * Enumerator class used to determine the type of detector
 */
enum class DetectorType {
  TRACK,
  CHANNEL
};

class Detector {
  using CutCallBack = std::function<std::unique_ptr<CutBase>(Qn::InputVariableManager *)>;
  using HistogramCallBack = std::function<std::unique_ptr<QAHistoBase>(Qn::InputVariableManager *)>;
 public:
  Detector(std::string name,
           DetectorType type,
           std::vector<AxisD> axes,
           std::string phi_name_,
           std::string weight_name_,
           std::bitset<Qn::QVector::kmaxharmonics> harmonics,
           QVector::Normalization norm,
           Qn::InputVariableManager *var)
      :
      phi_name_(phi_name_),
      weight_name_(weight_name_),
      name_(name),
      nchannels_(var->FindVariable(phi_name_).GetSize()),
      harmonics_bits_(harmonics),
      q_vector_normalization_method_(norm),
      type_(type) {
    sub_events_.AddAxes(axes);
  }

  /**
   * @brief Clears data before filling new event.
   */
  void ClearData() {
    for (auto &bin : sub_events_) {
      bin->Clear();
    }
  }

  /**
   * @brief Adds a cut to the detector
   * @param cut unique pointer to the cut. It is moved into the function and cannot be reused!
   */
  void AddCutCallBack(CutCallBack callback) {
    cuts_callback_.push_back(std::move(callback));
  }

  void CreateCuts(Qn::InputVariableManager *variable_manager) {
    for (auto &proto : cuts_callback_) {
      auto cut = proto(variable_manager);
      if (cut->IsChannelWise()) {
        cuts_.AddCut(std::move(cut));
      } else {
        int_cuts_.AddCut(std::move(cut));
      }
    }
  }

  /**
   * @brief Adds a QA histogram to the detector.
   * @param histo pointer to the histogram.
   */
  void AddHistogramCallBack(HistogramCallBack histo) {
    histograms_callback.push_back(histo);
  }

  void SetOutputQVector(QVector::CorrectionStep step) {
    output_tree_q_vectors_.emplace_back(step);
  }

  void SetConfig(std::function<void(SubEvent *)> config) {
    configuration_ = std::move(config);
  }

  void InitializeOnNode(CorrectionManager *manager);

  void CreateHistograms(InputVariableManager *var) {
    for (auto &callback : histograms_callback) {
      histograms_.push_back(callback(var));
    }
  }

  /**
   * @brief Fills the data into the data vectors, histograms and cut reports after the cuts have been checked.
   */
  void FillData() {
    long i = 0;
    if (!int_cuts_.CheckCuts(0)) return;
    for (auto &histo : histograms_) {
      histo->Fill();
    }
    for (const auto phi : phi_) {
      if (!cuts_.CheckCuts(i)) {
        ++i;
        continue;
      }
      if (vars_.empty()) {
        sub_events_.At(0)->AddDataVector(phi, *(weight_.begin() + i), i);
      } else {
        long icoord = 0;
        for (const auto &var : vars_) {
          coordinates_.at(icoord) = *(var.begin() + i);
          ++icoord;
        }
        const auto ibin = sub_events_.FindBin(coordinates_);
        if (ibin > -1) sub_events_.At(ibin)->AddDataVector(phi, *(weight_.begin() + i), i);
      }
      ++i;
    }
  }

  /**
   * Fill the cuts reports to the
   */
  void FillReport() {
    int_cuts_.FillReport();
    cuts_.FillReport();
  }

  void CreateSupportQVectors() { for (auto &ev : sub_events_) { ev->CreateSupportQVectors(); }}

  void AttachCorrectionInputs(TList *list) { for (auto &ev : sub_events_) { ev->AttachCorrectionInput(list); }}

  void AfterInputAttachAction() { for (auto &ev : sub_events_) { ev->AfterInputAttachAction(); }}

  void CreateCorrectionHistograms(TList *list) { for (auto &ev : sub_events_) { ev->CreateCorrectionHistograms(list); }}

  bool IsIntegrated() const { return sub_events_.IsIntegrated(); }

  void ProcessCorrections() {
    for (auto &ev : sub_events_) { ev->ProcessCorrections(); }
    for (auto &ev : sub_events_) { ev->ProcessDataCollection(); }
    // passes the corrected Q-vectors to the output container.
    for (auto &pair_step_qvector : q_vectors_) {
      for (unsigned int i = 0; i < sub_events_.size(); ++i) {
        (*pair_step_qvector.second)[i] = *sub_events_[i]->GetQVector(pair_step_qvector.first);
      }
    }
  }

  void IncludeQnVectors() {
    for (auto &ev : sub_events_) { ev->IncludeQnVectors(); }
    // Adds DataContainerQVector for each of the active correction steps.
    auto correction_steps = sub_events_[0]->GetCorrectionSteps();
    for (auto correction_step : correction_steps) {
      q_vectors_.emplace(correction_step, std::make_unique<DataContainerQVector>(sub_events_.GetAxes()));
    }
  }

  void AttachToTree(TTree *tree) {
    for (const auto &qvec : q_vectors_) {
      auto is_output_variable = std::find(output_tree_q_vectors_.begin(),output_tree_q_vectors_.end(),qvec.first);
      if (is_output_variable != output_tree_q_vectors_.end()) {
        auto suffix = kCorrectionStepNamesArray[qvec.first];
        auto name = name_ + "_" + suffix;
        tree->Branch(name.data(), qvec.second.get());
      }
    }
  }

  void ActivateHarmonic(unsigned int i) {
    harmonics_bits_.set(i - 1);
    for (auto &ev : sub_events_) {
      ev->ActivateHarmonic(i);
    }
  }

  DetectorList *GetDetectors() const { return detectors_; }
  Qn::QVector::Normalization GetNormalizationMethod() const { return q_vector_normalization_method_; }
  std::string GetName() const { return name_; }
  std::string GetBinName(unsigned int id) const { return sub_events_.GetBinDescription(id); }
  DetectorType GetType() const { return type_; }
  SubEvent *GetSubEvent(unsigned int ibin) { return sub_events_.At(ibin).get(); }
  TList *CreateQAHistogramList(bool fill_qa, bool fill_validation, InputVariableManager *var);

 private:
  std::string phi_name_;
  std::string weight_name_;
  InputVariable phi_; /// variable holding the azimuthal angle
  InputVariable weight_; /// variable holding the weight which is used for the calculation of the Q vector.
  const std::string name_; /// name of  the detector
  int nchannels_ = 0; /// number of channels in case of channel detector
  std::bitset<Qn::QVector::kmaxharmonics> harmonics_bits_; /// bitset of all activated harmonics
  Qn::QVector::Normalization q_vector_normalization_method_ = Qn::QVector::Normalization::NONE;
  std::vector<InputVariable> vars_; /// variables used for the binning of the Q vector.
  std::vector<float> coordinates_;  ///  vector holding the temporary coordinates of one track or channel.
  std::function<void(SubEvent *)> configuration_;
  DetectorType type_;
  std::map<QVector::CorrectionStep, std::unique_ptr<DataContainerQVector>> q_vectors_;
  std::vector<QVector::CorrectionStep> output_tree_q_vectors_;
  CorrectionCuts cuts_; /// per channel selection  cuts
  CorrectionCuts int_cuts_; /// integrated selection cuts
  std::vector<std::unique_ptr<QAHistoBase>> histograms_; /// QA histograms of the detector
  Qn::DataContainer<std::unique_ptr<SubEvent>, AxisD> sub_events_;
  
  Qn::DetectorList *detectors_ = nullptr;

  std::vector<CutCallBack> cuts_callback_;
  std::vector<HistogramCallBack> histograms_callback; /// QA histograms of the detector


};

}

#endif //FLOW_DETECTOR_H
