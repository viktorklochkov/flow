#ifndef QN_DETECTOR_H
#define QN_DETECTOR_H

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

#include <utility>
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
 public:
  Detector(std::string name,
           DetectorType type,
           std::vector<AxisD> axes,
           InputVariable phi,
           InputVariable weight,
           InputVariable radial_offset,
           std::bitset<Qn::QVector::kmaxharmonics> harmonics,
           QVector::Normalization norm)
      :
      phi_(std::move(phi)),
      weight_(std::move(weight)),
      radial_offset_(std::move(radial_offset)),
      name_(std::move(name)),
      type_(type),
      nchannels_(phi_.size()),
      harmonics_bits_(harmonics),
      q_vector_normalization_method_(norm) {
    sub_events_.AddAxes(axes);
  }

  Detector(Detector &&detector) = default;
  Detector &operator=(Detector &&detector) = default;

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
   * @param cut unique pointer to the cut.
   */
  void AddCut(CorrectionCut::CallBack callback, bool is_channel_wise) {
    if (is_channel_wise) {
      cuts_.AddCut(callback);
    } else {
      int_cuts_.AddCut(callback);
    }
  }

  template<typename... Args>
  void AddHistogram(Args &&... args) {
    histograms_.Add(std::forward<Args>(args)...);
  }

  void SetOutputQVector(QVector::CorrectionStep step) {
    output_tree_q_vectors_.emplace_back(step);
  }

  void SetConfig(std::function<void(SubEvent *)> config) {
    correction_configuration_ = std::move(config);
  }

  void Initialize(CorrectionManager *manager);

  /**
   * @brief Fills the data into the data vectors, histograms and cut reports after the cuts have been checked.
   */
  void FillData() {
    if (!int_cuts_.CheckCuts(0)) return;
    histograms_.Fill();
    for (unsigned int i = 0; i < phi_.size(); ++i) {
      if (!cuts_.CheckCuts(i)) continue;
      /// Integrated case (detector only has one bin)
      if (input_variables_.empty()) {
        sub_events_.At(0)->AddDataVector(i, phi_[i], weight_[i], radial_offset_[i]);
        /// differential case (detector has more than one bin)
      } else {
        for (unsigned int icoord = 0; icoord < input_variables_.size(); ++icoord) {
          coordinates_.at(icoord) = input_variables_[icoord][i];
        }
        const auto ibin = sub_events_.FindBin(coordinates_);
        if (ibin > -1) sub_events_.At(ibin)->AddDataVector(i, phi_[i], weight_[i], radial_offset_[i]);
      }
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
        try {
          (*pair_step_qvector.second)[i] = *sub_events_[i]->GetQVector(pair_step_qvector.first);
        } catch (std::out_of_range &) {
          throw std::out_of_range(name_ + " bin " + std::to_string(i) + " correctionstep: " +
              kCorrectionStepNamesArray[pair_step_qvector.first] + "not found.");
        }
      }
    }
  }

  void IncludeQnVectors() {
    for (auto &ev : sub_events_) { ev->IncludeQnVectors(); }
    // Adds DataContainerQVector for each of the active correction steps.
    auto correction_steps = sub_events_[0]->GetCorrectionSteps();
    for (auto correction_step : correction_steps) {
      if (sub_events_.IsIntegrated()) {
        q_vectors_.emplace(correction_step, std::make_unique<DataContainerQVector>());
      } else {
        q_vectors_.emplace(correction_step, std::make_unique<DataContainerQVector>(sub_events_.GetAxes()));
      }
    }
  }

  void AttachToTree(TTree *tree) {
    for (const auto &qvec : q_vectors_) {
      auto is_output_variable = std::find(output_tree_q_vectors_.begin(), output_tree_q_vectors_.end(), qvec.first);
      if (is_output_variable!=output_tree_q_vectors_.end()) {
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
  SubEvent *GetSubEvent(unsigned int ibin) { return sub_events_.At(ibin).get(); }
  TList *CreateQAHistogramList(bool fill_qa, bool fill_validation);

 private:
  InputVariable phi_; /// variable holding the azimuthal angle
  InputVariable weight_; /// variable holding the weight which is used for the calculation of the Q vector.
  InputVariable radial_offset_; /// variable holding the radial offset
  std::string name_; /// name of  the detector
  DetectorType type_; /// type of detector
  int nchannels_ = 0; /// number of channels in case of channel detector
  std::bitset<Qn::QVector::kmaxharmonics> harmonics_bits_; /// bitset of all activated harmonics
  Qn::QVector::Normalization q_vector_normalization_method_ = Qn::QVector::Normalization::NONE;
  std::vector<InputVariable> input_variables_; /// variables used for the binning of the Q vector.
  std::vector<float> coordinates_;  ///  vector holding the temporary coordinates of one track or channel.
  std::function<void(SubEvent *)>
      correction_configuration_; /// configures the correction steps applied in the sub event.
  std::map<QVector::CorrectionStep, std::unique_ptr<DataContainerQVector>> q_vectors_;
  std::vector<QVector::CorrectionStep> output_tree_q_vectors_;
  CorrectionCuts cuts_; /// per channel selection  cuts
  CorrectionCuts int_cuts_; /// integrated selection cuts
  QAHistograms histograms_; /// QA histograms of the detector
  Qn::DataContainer<std::unique_ptr<SubEvent>, AxisD> sub_events_;
  Qn::DetectorList *detectors_ = nullptr;
};

}

#endif //FLOW_DETECTOR_H
