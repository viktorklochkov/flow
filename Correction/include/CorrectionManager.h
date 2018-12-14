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
#include <utility>
#include "Detector.h"
#include "VariableManager.h"
#include "EventInfo.h"
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
#include "EventInfo.h"

#include "ROOT/RMakeUnique.hxx"
#include "ROOT/RIntegerSequence.hxx"

namespace Qn {
class CorrectionManager {
 public:

  using MapDetectors = std::map<std::string, std::unique_ptr<DetectorBase>>;

  CorrectionManager()
      : event_cuts_(new Cuts()), event_variables_(new Qn::EventInfoF()), var_manager_(new VariableManager()) {
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
  void AddCorrectionAxis(const Qn::Axis &axis) { qncorrections_axis_.push_back(axis); }

  /**
   * Adds a event variable, which will be saved to the output tree.
   * @param name Name corresponds to a variable defined in the variable manager.
   */
  void SetEventVariable(const std::string &name) { event_variables_->AddVariable(name); }

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
    std::unique_ptr<Detector<N>> det(new Detector<N>(type, axes, phi, weight, vars, harmo));
    auto pair = std::make_pair<std::string, std::unique_ptr<DetectorBase>>(std::move(name), std::move(det));
    if (type==DetectorType::CHANNEL) detectors_channel.emplace(std::move(pair));
    if (type==DetectorType::TRACK) detectors_track.emplace(std::move(pair));
  }

/**
 * Adds a detector to the correction framework.
 * Enables first second and third harmonic by default.
 * @param name Name of the detector
 * @param type Type of the detector: Channel or Track
 * @param phi_name Name of the variable which saves the angular information e.g. phi angle of a channel or particle
 * @param weight_name Name of the variable which saves a weight. "Ones" is used for a weight of 1.
 * @param axes Axes used for differential corrections. Names correspond to the name of variables.
 */
  void AddDetector(const std::string &name,
                   DetectorType type,
                   const std::string &phi_name,
                   const std::string &weight_name  = "Ones",
                   const std::vector<Qn::Axis> &axes = {}) {
  AddDetector(name,type,phi_name,weight_name,axes,{1,2,3});
}

  /**
   * Adds a cut to a detector.
   * @tparam FUNCTION
   * @param name
   * @param var_name
   * @param lambda
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
    if (detectors_track.find(name) != detectors_track.end()) {
    detectors_track.at(name)->AddCut(std::move(cut));
    } else if (detectors_channel.find(name) != detectors_channel.end()) {
      detectors_channel.at(name)->AddCut(std::move(cut));
    } else {
      std::cout << "Detector" + name + "not found. Cut not Added." << std::endl;
    }
  }
  /**
   * Adds a cut based on event variables. Only events which pass the cuts are used for the corrections.
   * Template parameters are automatically deduced.
   * @tparam N dimensionality of the cut
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
   * Adds a one dimesional event histogram
   * @param axes axis of the histogram. Name corresponds to the axis.
   * @param weightname Name of the weights used when filling. Standard is "Ones" (1).
   */
  void AddEventHisto1D(std::vector<Qn::Axis> axes, const std::string &weightname = "Ones");
  /**
   * Adds a two dimesional event histogram
   * @param axes axes of the histogram. Name corresponds to the axes.
   * @param weightname Name of the weights used when filling. Standard is "Ones" (1).
   */
  void AddEventHisto2D(std::vector<Qn::Axis> axes, const std::string &weightname = "Ones");

  /**
  * Adds a one dimesional histogram to a detector.
  * @param Name name of the detector
  * @param axes axis of the histogram. Name corresponds to the axis.
  * @param weightname Name of the weights used when filling. Standard is "Ones" (1).
  */
  void AddHisto1D(const std::string &name, std::vector<Qn::Axis> axes, const std::string &weightname = "Ones");
  /**
  * Adds a two dimesional histogram to a detector.
  * @param Name name of the detector
  * @param axes axis of the histogram. Name corresponds to the axis.
  * @param weightname Name of the weights used when filling. Standard is "Ones" (1).
  */
  void AddHisto2D(const std::string &name, std::vector<Qn::Axis> axes, const std::string &weightname = "Ones");

  /**
   * Adds Correctionsteps to one detector.
   * @param name Name of the detector.
   * @param config function configuring the correction framework.
   * C-callable of signature void(QnCorrectionsDetectorConfigurationBase *config).
   */
  void SetCorrectionSteps(const std::string &name,
                          std::function<void(DetectorConfiguration *config)> config);

  void SetTree(TTree *tree) { out_tree_ = tree; }

  void SaveOutput(std::shared_ptr<TFile> qa_histograms_file, std::shared_ptr<TFile> tree_file) {
    SaveHistograms(qa_histograms_file);
    SaveTree(tree_file);
  }

  void Initialize(std::shared_ptr<TFile> &in_calibration_file_);

  void Initialize(TFile *in_calibration_file_);


  void ProcessEvent();

  void ProcessQnVectors();

  void Finalize();

  void Reset();

  void FillChannelDetectors() {
    if (event_passed_cuts_) {
      for (auto &dp : detectors_channel) {
        dp.second->FillData();
      }
    }
  }

  void FillTrackingDetectors() {
    if (event_passed_cuts_) {
      for (auto &dp : detectors_track) {
        dp.second->FillData();
      }
    }
  }

  double* GetVariableContainer() {return var_manager_->GetVariableContainer();}

  TList* GetCalibrationList() {return qncorrections_manager_.GetOutputHistogramsList();}

  TList* GetCalibrationQAList() {return qncorrections_manager_.GetQAHistogramsList();}

  void FillDetectorQAToList(TList*);

  void SetProcessName(std::string name) {qncorrections_manager_.SetCurrentProcessListName(name.data());}

 private:

  std::pair<std::array<Variable, 2>, TH1F> Create1DHisto(const std::string &name,
                                                         std::vector<Qn::Axis> axes,
                                                         const std::string &weightname = "Ones");

  std::pair<std::array<Variable, 3>, TH2F> Create2DHisto(const std::string &name,
                                                         std::vector<Qn::Axis> axes,
                                                         const std::string &weightname = "Ones");

  void CreateDetectors();

  void GetQnFromFramework(const std::string &step);

  void CalculateCorrectionAxis();

  void SaveHistograms(std::shared_ptr<TFile> file);

  void SaveTree(const std::shared_ptr<TFile> &file);

  EventClassVariablesSet *qncorrections_varset_ = nullptr;
  std::unique_ptr<Cuts> event_cuts_;
  std::unique_ptr<Qn::EventInfoF> event_variables_;
  std::vector<Qn::Axis> qncorrections_axis_;
  CorrectionCalculator qncorrections_manager_;
  std::shared_ptr<VariableManager> var_manager_;
  std::map<std::string, std::unique_ptr<DetectorBase>> detectors_track;
  std::map<std::string, std::unique_ptr<DetectorBase>> detectors_channel;
  std::vector<std::unique_ptr<Qn::QAHistoBase>> event_histograms_;
  TTree *out_tree_ = nullptr;
  bool event_passed_cuts_ = false;
};
}

#endif //FLOW_CORRECTIONMANAGER_H
