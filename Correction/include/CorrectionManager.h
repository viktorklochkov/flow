#include <utility>

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
#include "InputVariableManager.h"
#include "Cuts.h"
#include "CorrectionProfile3DCorrelations.h"
#include "CorrectionProfileCorrelationComponents.h"
#include "SubEventChannels.h"
#include "SubEvent.h"
#include "SubEventTracks.h"
#include "Recentering.h"
#include "TwistAndRescale.h"

#include "GainEqualization.h"
#include "Alignment.h"
#include "DataContainer.h"
#include "RunList.h"
#include "DetectorList.h"

namespace Qn {
class CorrectionManager {
  using ConfigurationFunction = std::function<void(SubEvent *config)>;
  using CutCallBack =  std::function<std::unique_ptr<CutBase>(Qn::InputVariableManager *)>;
  using HistogramCallBack =  std::function<std::unique_ptr<QAHistoBase>(Qn::InputVariableManager *)>;
  using CorrectionAxisCallBack = std::function<CorrectionAxis(Qn::InputVariableManager *)>;
 public:
  /**
   * Add a variable to the variable manager
   * @param name Name of the variable
   * @param id Id of the variable inside the array used to pass the data into the framework.
   * @param length Lenght of the variable inside the array e.g. number of channels.
   */
  void AddVariable(const std::string &name, const int id, const int length) {
    variable_manager_.CreateVariable(name, id, length);
  }

  /**
   * Adds a axis used for correction.
   * @param axis Axis used for correction. The name of the axis corresponds to the name of a variable.
   */
  void AddCorrectionAxis(Qn::AxisD axis) {
    auto callback = [axis](InputVariableManager *var) {
      return CorrectionAxis(var->FindVariable(axis.Name()), axis);
    };
    correction_axes_callback_.emplace_back(callback);
  }

  /**
   * Adds a event variable, which will be saved to the output tree.
   * @param name Name corresponds to a variable defined in the variable manager.
   */
  void AddEventVariable(const std::string &name) { variable_manager_.RegisterOutputF(name); }

  /**
   * Adds a event variable, which will be saved to the output tree.
   * Remember to add them as a variable first.
   * @param name Name corresponds to a variable defined in the variable manager.
   */
  void AddEventVariableInt(const std::string &name) { variable_manager_.RegisterOutputL(name); }

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
                   const std::vector<Qn::AxisD> &axes,
                   int const(&harmo)[N],
                   QVector::Normalization norm = QVector::Normalization::M) {
    InputVariable phi = variable_manager_.FindVariable(phi_name);
    InputVariable weight = variable_manager_.FindVariable(weight_name);
    std::vector<InputVariable> vars;
    vars.reserve(axes.size());
    for (const auto &axis : axes) {
      vars.push_back(variable_manager_.FindVariable(axis.Name()));
    }
    std::bitset<Qn::QVector::kmaxharmonics> harmonics;
    for (std::size_t i = 0; i < N; ++i) {
      harmonics.set(harmo[i] - 1);
    }
    auto det = std::make_unique<Detector>(std::move(name),
                                          type,
                                          axes,
                                          phi_name,
                                          weight_name,
                                          vars,
                                          harmonics,
                                          norm,
                                          &variable_manager_);
    detectors_.AddDetector(std::move(det));
  }

  template<std::size_t N, typename FUNCTION>
  void AddCutOnDetector(const std::string &detector_name,
                        const char *const (&variable_names)[N],
                        FUNCTION cut_function,
                        const std::string &cut_description) {
    detectors_.AddCutCallBack(detector_name, CreateCutCallBack(variable_names, cut_function, cut_description));
  }

  template<std::size_t N, typename FUNCTION>
  void AddEventCut(const char *const (&variable_names)[N],
                   FUNCTION &&cut_function,
                   const std::string &cut_description) {
    event_cuts_.AddCutCallBack(CreateCutCallBack(variable_names, cut_function, cut_description));
  }

  /**
   * @brief Adds a one dimensional event histogram
   * @param axes axis of the histogram. Name corresponds to the axis.
   * @param weight Name of the weights used when filling. Standard is "Ones" (1).
   */
  void AddEventHisto1D(const Qn::AxisD &axes, const std::string &weight = "Ones") {
    event_histograms_callback_.push_back(Create1DHisto("Event", axes, weight));
  }

  /**
   * @brief Adds a two n event histogram
   * @param axes axes of the histogram. Name corresponds to the axes.
   * @param weight Name of the weights used when filling. Standard is "Ones" (1).
   */
  void AddEventHisto2D(const std::vector<Qn::AxisD> &axes, const std::string &weight = "Ones") {
    event_histograms_callback_.push_back(Create2DHisto("Event", axes, weight));
  }

  void AddEventHisto2DArray(const std::vector<Qn::AxisD> &axes,
                            const Qn::AxisD &axis,
                            const std::string &weight = "Ones") {
    event_histograms_callback_.push_back(Create2DHistoArray("Event", axes, weight, axis));
  }

  /**
  * @brief Adds a one dimensional histogram to a detector.
  * @param Name name of the detector
  * @param axes axis of the histogram. Name corresponds to the axis.
  * @param weight Name of the weights used when filling. Standard is "Ones" (1).
  */
  void AddHisto1D(const std::string &detector, const Qn::AxisD &axis, const std::string &weight = "Ones") {
    detectors_.FindDetector(detector).AddHistogramCallBack(Create1DHisto(detector, axis, weight));
  }

  /**
  * Adds a two dimensional histogram to a detector.
  * @param Name name of the detector
  * @param axes axis of the histogram. Name corresponds to the axis.
  * @param weight Name of the weights used when filling. Standard is "Ones" (1).
  */
  void AddHisto2D(const std::string &detector,
                  const std::vector<Qn::AxisD> &axes,
                  const std::string &weight = "Ones") {
    detectors_.FindDetector(detector).AddHistogramCallBack(Create2DHisto(detector, axes, weight));
  }
  /**
   * @brief Adds correction steps to a detector.
   * @param name Name of the detector.
   * @param config function configuring the correction framework.
   * C-callable of signature void(DetectorConfiguration *config) config.
   */
  void SetCorrectionSteps(const std::string &name, ConfigurationFunction config) {
    detectors_.FindDetector(name).SetConfig(std::move(config));
  }
  void SetOutputQVectors(const std::string &name, const std::vector<Qn::QVector::CorrectionStep> &steps) {
    auto &detector = detectors_.FindDetector(name);
    for (auto &step : steps) {
      detector.SetOutputQVector(step);
    }
  }
  void SetFillOutputTree(bool tree) { fill_output_tree_ = tree; }
  void SetFillCalibrationQA(bool calibration) { fill_qa_histos_ = calibration; }
  void SetFillValidationQA(bool validation) { fill_validation_qa_histos_ = validation; }
  void SetCurrentRunName(const std::string &name);
  void SetCalibrationInputFileName(const std::string &file_name) { correction_input_file_name_ = file_name; }
  void SetCalibrationInputFile(TFile *file) { correction_input_file_.reset(file); }

  /**
   * @brief Set output tree.
   * Lifetime of the tree is managed by the user.
   * @param tree non-owning pointer to the tree.
   */
  void ConnectOutputTree(TTree *tree) { if (fill_output_tree_) out_tree_ = tree; }

  /**
   * @brief Initializes the correction framework
   * @param in_calibration_file_ non-owning pointer to the calibration file.
   * Lifetime of the file has to be managed by the user.
   */
  void InitializeOnNode();

  bool ProcessEvent();

  double *GetVariableContainer() { return variable_manager_.GetVariableContainer(); }

  inline void FillTrackingDetectors() { if (event_passed_cuts_) detectors_.FillTracking(); }
  inline void FillChannelDetectors() { if (event_passed_cuts_) detectors_.FillChannel(); }

  void ProcessCorrections();
  /**
   * @brief Resets the correction framework. To be called before a new event is processed.
   */
  void Reset();

  /**
 * @brief Finalizes the correction framework. To be called after all events are processed.
 */
  void Finalize();

  /**
   * @brief Get the list containing the calibration histograms.
   * @return A pointer of the list to which the calibration histograms will be saved.
   */
  TList *GetCorrectionList() { return correction_output.get(); }

  /**
   * @brief Get the list containing the calibration QA histograms.
   * @return A pointer of the list to which the calibration QA histograms will be saved.
   */
  TList *GetCorrectionQAList() { return correction_qa_histos_.get(); }

 private:
  template<std::size_t N, typename FUNCTION>
  CutCallBack CreateCutCallBack(const char *const (&names)[N], FUNCTION lambda, const std::string &cut_description) {
    auto callback = [&names, lambda, cut_description](Qn::InputVariableManager *var) {
      InputVariable arr[N];
      int i = 0;
      for (auto &name : names) {
        arr[i] = var->FindVariable(name);
        ++i;
      }
      return MakeUniqueNDimCut(arr, lambda, cut_description, arr[0].GetSize() > 1);
    };
    return callback;
  };

  void InitializeCorrections();

  void AttachQAHistograms();

  HistogramCallBack Create1DHisto(const std::string &name, Qn::AxisD axis,
                                  const std::string &weight);

  HistogramCallBack Create2DHisto(const std::string &name, std::vector<Qn::AxisD> axes,
                                  const std::string &weight);

  HistogramCallBack Create2DHistoArray(const std::string &name, std::vector<Qn::AxisD> axes,
                                       const std::string &weight, const Qn::AxisD &histogram_axis);

  InputVariableManager *GetVariableManager() { return &variable_manager_; }
  DetectorList *GetDetectors() { return &detectors_; }
  CorrectionAxisSet *GetCorrectionAxes() { return &correction_axes_; }

 private:
  friend class Detector;

  static constexpr int kMaxCorrectionArrayLength = 1000;
  static constexpr auto kCorrectionListName = "CorrectionHistograms";

  RunList runs_;
  DetectorList detectors_;
  InputVariableManager variable_manager_; ///< manager of the variables

  std::string correction_input_file_name_; ///< name of the calibration input file

  std::unique_ptr<TList> correction_input_;   //!<! the list of the input calibration histograms
  std::unique_ptr<TList> correction_output;  //!<! the list of the support histograms
  std::unique_ptr<TList> correction_qa_histos_;          //!<! the list of QA histograms
  std::unique_ptr<TFile> correction_input_file_;               //!<! input calibration file
//  TList *validation_histos_ = nullptr;     //!<! the list of not validated entries QA histograms
  bool fill_qa_histos_ = true;
  bool fill_validation_qa_histos_ = true;
  bool fill_output_tree_ = false;
  std::vector<CorrectionAxisCallBack> correction_axes_callback_;
  CorrectionAxisSet correction_axes_; /// CorrectionCalculator correction axes
  CorrectionCuts event_cuts_; ///< Pointer to the event cuts
  TList *det_qa_histos_ = nullptr; //!<! List holding the Detector QA histograms. Lifetime is managed by the user.
//  CorrectionCalculator qnc_calculator_; ///< calculator of the corrections
  std::vector<HistogramCallBack> event_histograms_callback_; ///< event QA histograms
  std::vector<std::unique_ptr<Qn::QAHistoBase>> event_histograms_; ///< event QA histograms
  TTree *out_tree_ = nullptr;  //!<! Tree of Qn Vectors and event variables. Lifetime is managed by the user.
  bool event_passed_cuts_ = false; ///< variable holding status if an event passed the cuts.
};
}

#endif //FLOW_CORRECTIONMANAGER_H
