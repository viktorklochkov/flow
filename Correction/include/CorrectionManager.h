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

#ifndef FLOW_CORRECTIONMANAGER_H
#define FLOW_CORRECTIONMANAGER_H

#include <string>
#include <map>

#include "ROOT/RMakeUnique.hxx"
#include "ROOT/RIntegerSequence.hxx"

#include <utility>
#include "Detector.h"
#include "VariableManager.h"
#include "VariableCutBase.h"
#include "CorrectionProfile3DCorrelations.h"
#include "CorrectionProfileCorrelationComponents.h"
#include "DetectorConfigurationChannels.h"
#include "DetectorConfiguration.h"
#include "DetectorConfigurationTracks.h"
#include "Recentering.h"
#include "TwistAndRescale.h"
#include "CutSetBit.h"
#include "CutWithin.h"
#include "GainEqualization.h"
#include "Alignment.h"
#include "CorrectionCalculator.h"
#include "DataContainer.h"

namespace Qn {
class CorrectionManager {
 public:

  using MapDetectors = std::map<std::string, std::unique_ptr<DetectorBase>>;

  CorrectionManager()
      : event_cuts_(new Cuts()),
        var_manager_(new VariableManager()) {
    var_manager_->CreateVariableOnes();
  }

  /**
   * Add a variable to the variable manager
   * @param name Name of the variable
   * @param id Id of the variable inside the array used to pass the data into the framework.
   * @param length Lenght of the variable inside the array e.g. number of channels.
   */
  void AddVariable(const std::string &name, const int id, const int length) {
    var_manager_->CreateVariable(name, id, length);
  }

  /**
   * Adds a axis used for correction.
   * @param axis Axis used for correction. The name of the axis corresponds to the name of a variable.
   */
  void AddCorrectionAxis(const Qn::Axis &axis) { correction_axes_.push_back(axis); }

  /**
   * Adds a event variable, which will be saved to the output tree.
   * @param name Name corresponds to a variable defined in the variable manager.
   */
  void AddEventVariable(const std::string &name) {
    var_manager_->RegisterOutputF(name);
  }

  /**
   * Adds a event variable, which will be saved to the output tree.
   * Remember to add them as a variable first.
   * @param name Name corresponds to a variable defined in the variable manager.
   */
  void AddRunEventId(const std::string &run, const std::string &event) {
    var_manager_->RegisterOutputL(run);
    var_manager_->RegisterOutputL(event);
  }

  /**
   * Adds a detector to the correction framework.
   * @param name Name of the detector
   * @param type Type of the detector: Channel or Track
   * @param phi_name Name of the variable which saves the angular information e.g. phi angle of a channel or particle
   * @param weight_name Name of the variable which saves a weight. "Ones" is used for a weight of 1.
   * @param axes Axes used for differential corrections. Names correspond to the name of variables.
   * @param harmo activated harmonics in a ordered initializer list e.g. `{1, 2, 4}`.
   */
  template<std::size_t N>
  void AddDetector(std::string name,
                   Qn::DetectorType type,
                   const std::string &phi_name,
                   const std::string &weight_name,
                   const std::vector<Qn::Axis> &axes,
                   int const(&harmo)[N]) {
    Variable phi = var_manager_->FindVariable(phi_name);
    Variable weight = var_manager_->FindVariable(weight_name);
    std::vector<Variable> vars;
    vars.reserve(axes.size());
    for (const auto &axis : axes) {
      vars.push_back(var_manager_->FindVariable(axis.Name()));
    }
    std::unique_ptr<Detector<N>> det(new Detector<N>(name, type, axes, phi, weight, vars, harmo));
    if (type==DetectorType::CHANNEL) {
      detectors_channel_.emplace(name, std::move(det));
    }
    if (type==DetectorType::TRACK) {
      detectors_track_.emplace(name, std::move(det));
    }
  }

  /**
   * @brief Adds a cut to a detector.
   * Template parameters are automatically deduced.
   * @tparam N number of variables used in the cut
   * @tparam FUNCTION type of function
   * @param name name of the detector
   * @param names array of names of the cut variables
   * @param lambda function used to evaluate the cut condition
   */
  template<std::size_t N, typename FUNCTION>
  void AddCut(const std::string &name, const char *const (&names)[N], FUNCTION lambda) {
    Variable arr[N];
    int i = 0;
    for (auto &n : names) {
      arr[i] = var_manager_->FindVariable(n);
      ++i;
    }
    auto cut = MakeUniqueNDimCut(arr, lambda);
    if (detectors_track_.find(name)!=detectors_track_.end()) {
      detectors_track_.at(name)->AddCut(std::move(cut));
    } else if (detectors_channel_.find(name)!=detectors_channel_.end()) {
      detectors_channel_.at(name)->AddCut(std::move(cut));
    } else {
      std::cout << "Detector" + name + "not found. Cut not Added." << std::endl;
    }
  }

  /**
   * @brief Adds a cut based on event variables.
   * Only events which pass the cuts are used for the corrections.
   * Template parameters are automatically deduced.
   * @tparam N number of variables used in the cut.
   * @tparam FUNCTION type of function
   * @param name_arr Array of variable names used for the cuts.
   * @param func C-callable describing the cut of signature bool(double &...).
   *             The number of double& corresponds to the number of variables
   */
  template<std::size_t N, typename FUNCTION>
  void AddEventCut(const char *const (&name_arr)[N], FUNCTION &&func) {
    Variable arr[N];
    int i = 0;
    for (auto &name : name_arr) {
      arr[i] = var_manager_->FindVariable(name);
      ++i;
    }
    event_cuts_->AddCut(MakeUniqueNDimCut(arr, func));
  }

  /**
   * @brief Adds a one dimensional event histogram
   * @param axes axis of the histogram. Name corresponds to the axis.
   * @param weightname Name of the weights used when filling. Standard is "Ones" (1).
   */
  void AddEventHisto1D(const Qn::Axis &axes, const std::string &weightname = "Ones");

  /**
   * @brief Adds a two n event histogram
   * @param axes axes of the histogram. Name corresponds to the axes.
   * @param weightname Name of the weights used when filling. Standard is "Ones" (1).
   */
  void AddEventHisto2D(const std::vector<Qn::Axis> &axes, const std::string &weightname = "Ones");

  void AddEventHisto2D(const std::vector<Qn::Axis> &axes, const Qn::Axis &axis, const std::string &weightname = "Ones");

  /**
  * @brief Adds a one dimensional histogram to a detector.
  * @param Name name of the detector
  * @param axes axis of the histogram. Name corresponds to the axis.
  * @param weightname Name of the weights used when filling. Standard is "Ones" (1).
  */
  void AddHisto1D(const std::string &name, const Qn::Axis &axis, const std::string &weightname = "Ones");

  /**
  * Adds a two dimensional histogram to a detector.
  * @param Name name of the detector
  * @param axes axis of the histogram. Name corresponds to the axis.
  * @param weightname Name of the weights used when filling. Standard is "Ones" (1).
  */
  void AddHisto2D(const std::string &name, const std::vector<Qn::Axis> &axes, const std::string &weightname = "Ones");

  /**
   * @brief Adds correction steps to a detector.
   * @param name Name of the detector.
   * @param config function configuring the correction framework.
   * C-callable of signature void(DetectorConfiguration *config) config.
   */
  void SetCorrectionSteps(const std::string &name, std::function<void(DetectorConfiguration *config)> config);

  /**
   * @brief Set output tree.
   * Lifetime of the tree is managed by the user.
   * @param tree non-owning pointer to the tree.
   */
  void SetTree(TTree *tree) { out_tree_ = tree; }

  /**
   * @brief Initializes the correction framework
   * @param in_calibration_file_ non-owning pointer to the calibration file.
   * Lifetime of the file has to be managed by the user.
   */
  void Initialize(TFile *in_calibration_file_);

  /**
   * @brief Process the event variables
   */
  void ProcessEvent();

  /**
   * @brief Process the Qn vector corrections
   */
  void ProcessQnVectors();

  /**
   * @brief Finalizes the correction framework. To be called after all events are processed.
   */
  void Finalize();

  /**
   * @brief Resets the correction framework. To be called before a new event is processed.
   */
  void Reset();

  /**
   * @brief Fill all channel detectors. To be called the channel variables have been filled to the variable container.
   */
  void FillChannelDetectors() {
    if (event_passed_cuts_) {
      for (auto &dp : detectors_channel_) {
        dp.second->FillData();
      }
    }
  }

  /**
   * @brief Fill all tracking detectors. To be called after the track variables of one particle track have been filled
   * to the detector.
   */
  void FillTrackingDetectors() {
    if (event_passed_cuts_) {
      for (auto &dp : detectors_track_) {
        dp.second->FillDataTracking();
      }
    }
  }

  /**
   * @brief Get the variable container to be able to fill the variables to the framework.
   * @return pointer to the variable container
   */
  double *GetVariableContainer() { return var_manager_->GetVariableContainer(); }

  /**
   * @brief Get the list containing the calibration histograms.
   * @return A pointer of the list to which the calibration histograms will be saved.
   */
  TList *GetCalibrationList() { return qnc_calculator_.GetOutputHistogramsList(); }

  /**
   * @brief Get the list containing the calibration QA histograms.
   * @return A pointer of the list to which the calibration QA histograms will be saved.
   */
  TList *GetCalibrationQAList() { return qnc_calculator_.GetQAHistogramsList(); }

  /**
   * @brief Get the list containing the event and detector QA histograms.
   * @return A pointer of the list to which the event and detector QA histograms will be saved.
   */
  TList *GetEventAndDetectorQAList();

  /**
   * @brief Sets the name of the current correction period (e.g. run number).
   * @param name Name of the current correction period
   */
  void SetProcessName(std::string name) {
    qnc_calculator_.SetCurrentProcessListName(name.data());
    ConnectCorrectionQVectors("latest");
  }

 private:

  static constexpr int kMaxCorrectionArrayLength = 1000;

  std::unique_ptr<Qn::QAHisto1DPtr> Create1DHisto(const std::string &name, const Qn::Axis &axis,
                                                  const std::string &weightname);

  std::unique_ptr<Qn::QAHisto2DPtr> Create2DHisto(const std::string &name, const std::vector<Qn::Axis> &axes,
                                                  const std::string &weightname);

  std::unique_ptr<Qn::QAHisto2DPtr> Create2DHisto(const std::string &name, const std::vector<Qn::Axis> &axes,
                                                  const std::string &weightname,
                                                  const Qn::Axis &histaxis);

  void CreateDetectors();

  void ConnectCorrectionQVectors(const std::string &step) {
    for (auto &pair : detectors_track_) {
      pair.second->SetUpCorrectionVectorPtrs(qnc_calculator_, step);
    }
    for (auto &pair : detectors_channel_) {
      pair.second->SetUpCorrectionVectorPtrs(qnc_calculator_, step);
    }
  }

  void CalculateCorrectionAxis();

  std::vector<std::unique_ptr<EventClassVariable>> qnc_evvars_; ///!<! List holding the correction axes
  std::unique_ptr<EventClassVariablesSet> qnc_varset_ = nullptr; ///!<! CorrectionCalculator correction axes
  std::unique_ptr<Cuts> event_cuts_; ///< Pointer to the event cuts
  TList *qa_list_ = nullptr; ///!<! List holding the Detector QA histograms. Lifetime has to be managed by the user.
  std::vector<Qn::Axis> correction_axes_; ///< vector of event axes used in the correctionstep
  CorrectionCalculator qnc_calculator_; ///< calculator of the corrections
  std::shared_ptr<VariableManager> var_manager_; ///< manager of the variables
  std::map<std::string, std::unique_ptr<DetectorBase>> detectors_track_; ///< map of tracking detectors
  std::map<std::string, std::unique_ptr<DetectorBase>> detectors_channel_; ///< map of channel detectors
  std::vector<std::unique_ptr<Qn::QAHistoBase>> event_histograms_; ///< event QA histograms
  TTree *out_tree_ = nullptr;  ///!<! Tree of Qn Vectors and event variables. Lifetime has to be managed by the user.
  bool event_passed_cuts_ = false; ///< variable holding status if an event passed the cuts.
};
}

#endif //FLOW_CORRECTIONMANAGER_H
