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
#include "ROOT/RStringView.hxx"

#include <utility>
#include "Detector.h"
#include "VariableManager.h"
#include "VariableCutBase.h"
#include "CorrectionProfile3DCorrelations.h"
#include "CorrectionProfileCorrelationComponents.h"
#include "SubEventChannels.h"
#include "SubEvent.h"
#include "SubEventTracks.h"
#include "Recentering.h"
#include "TwistAndRescale.h"

#include "GainEqualization.h"
#include "Alignment.h"
#include "CorrectionCalculator.h"
#include "DataContainer.h"
#include "RunList.h"
#include "DetectorList.h"

namespace Qn {
class CorrectionManager {
 public:

  using MapDetectors = std::map<std::string, std::unique_ptr<Detector>>;

  CorrectionManager() :
      event_cuts_(new Cuts()) {
    variable_manager_.CreateVariableOnes();
  }

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
  void AddCorrectionAxis(const Qn::AxisD &axis) { correction_axes_.Add(variable_manager_.FindVariable(axis.Name()), axis); }

  /**
   * Adds a event variable, which will be saved to the output tree.
   * @param name Name corresponds to a variable defined in the variable manager.
   */
  void AddEventVariable(const std::string &name) {
    variable_manager_.RegisterOutputF(name);
  }

  /**
   * Adds a event variable, which will be saved to the output tree.
   * Remember to add them as a variable first.
   * @param name Name corresponds to a variable defined in the variable manager.
   */
  void AddEventVariableInt(const std::string &name) {
    variable_manager_.RegisterOutputL(name);
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
                   const std::vector<Qn::AxisF> &axes,
                   int const(&harmo)[N]) {
    InputVariableD phi = variable_manager_.FindVariable(phi_name);
    InputVariableD weight = variable_manager_.FindVariable(weight_name);
    std::vector<InputVariableD> vars;
    vars.reserve(axes.size());
    for (const auto &axis : axes) {
      vars.push_back(variable_manager_.FindVariable(axis.Name()));
    }
    std::bitset<Qn::QVector::kmaxharmonics> harmonics;
    for (int i = 0; i < N; ++i) {
      harmonics.set(harmo[N] - 1);
    }
    auto det = std::make_unique<Detector>(std::move(name), type, axes, phi, weight, vars, harmonics);
    detectors_.AddDetector(std::move(det));
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
//  template<std::size_t N, typename FUNCTION>
//  void AddCut(const std::string &name, const char *const (&names)[N], FUNCTION lambda) {
//    InputVariable arr[N];
//    int i = 0;
//    for (auto &n : names) {
//      arr[i] = variable_manager_.FindVariable(n);
//      ++i;
//    }
//    auto cut = MakeUniqueNDimCut(arr, lambda);
//    if (detectors_track_.find(name)!=detectors_track_.end()) {
//      detectors_track_.at(name).AddCut(std::move(cut));
//    } else if (detectors_channel_.find(name)!=detectors_channel_.end()) {
//      detectors_channel_.at(name).AddCut(std::move(cut));
//    } else {
//      std::cout << "Detector" + name + "not found. Cut not Added." << std::endl;
//    }
//  }

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
    InputVariableD arr[N];
    int i = 0;
    for (auto &name : name_arr) {
      arr[i] = variable_manager_.FindVariable(name);
      ++i;
    }
    event_cuts_->AddCut(MakeUniqueNDimCut(arr, func));
  }

  /**
   * @brief Adds a one dimensional event histogram
   * @param axes axis of the histogram. Name corresponds to the axis.
   * @param weightname Name of the weights used when filling. Standard is "Ones" (1).
   */
  void AddEventHisto1D(const Qn::AxisF &axes, const std::string &weightname = "Ones");

  /**
   * @brief Adds a two n event histogram
   * @param axes axes of the histogram. Name corresponds to the axes.
   * @param weightname Name of the weights used when filling. Standard is "Ones" (1).
   */
  void AddEventHisto2D(const std::vector<Qn::AxisF> &axes, const std::string &weightname = "Ones");

  void AddEventHisto2D(const std::vector<Qn::AxisF> &axes,
                       const Qn::AxisF &axis,
                       const std::string &weightname = "Ones");

  /**
  * @brief Adds a one dimensional histogram to a detector.
  * @param Name name of the detector
  * @param axes axis of the histogram. Name corresponds to the axis.
  * @param weightname Name of the weights used when filling. Standard is "Ones" (1).
  */
  void AddHisto1D(const std::string &name, const Qn::AxisF &axis, const std::string &weightname = "Ones");

  /**
  * Adds a two dimensional histogram to a detector.
  * @param Name name of the detector
  * @param axes axis of the histogram. Name corresponds to the axis.
  * @param weightname Name of the weights used when filling. Standard is "Ones" (1).
  */
  void AddHisto2D(const std::string &name, const std::vector<Qn::AxisF> &axes, const std::string &weightname = "Ones");

  /**
   * @brief Adds correction steps to a detector.
   * @param name Name of the detector.
   * @param config function configuring the correction framework.
   * C-callable of signature void(DetectorConfiguration *config) config.
   */
  void SetCorrectionSteps(const std::string &name, std::function<void(SubEvent *config)> config);

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
  void Initialize();

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
   * @brief Get the variable container to be able to fill the variables to the framework.
   * @return pointer to the variable container
   */
  double *GetVariableContainer() { return variable_manager_.GetVariableContainer(); }

  /**
   * @brief Get the list containing the calibration histograms.
   * @return A pointer of the list to which the calibration histograms will be saved.
   */
  TList *GetCalibrationList() { return output_calibration_runs_.get(); }

  /**
   * @brief Get the list containing the calibration QA histograms.
   * @return A pointer of the list to which the calibration QA histograms will be saved.
   */
  TList *GetCalibrationQAList() { return calib_qa_histos_.get(); }

  /**
   * @brief Get the list containing the event and detector QA histograms.
   * @return A pointer of the list to which the event and detector QA histograms will be saved.
   */
  TList *GetEventAndDetectorQAList();

  /**
   * @brief Sets the name of the current correction period (e.g. run number).
   * @param name Name of the current correction period
   */
  void SetRunName(const std::string& name) {
    runs_.SetCurrentRun(name);
  }

  void SetCalibrationInputFile(const std::string &file_name) {
    calibration_input_file_name_ = file_name;
  }

 private:

  void AttachCalibrationInputs() {
    calibration_input_file_ = std::make_unique<TFile>(calibration_input_file_name_.data(), "READ");
    if (calibration_input_file_ && !calibration_input_file_->IsZombie()) {
      input_calibration_runs_ =
          std::make_unique<TList>(dynamic_cast<TList *>(calibration_input_file_->FindObject(kCalibListName))->Clone());
      input_calibration_runs_->SetOwner(kTRUE);
    }
  }

  void InitializeQnCorrectionsFramework() {
    detectors_.CreateSupportStructures();
    output_calibration_runs_ = std::make_unique<TList>();
    output_calibration_runs_->SetName(kCalibListName);
    output_calibration_runs_->SetOwner(kTRUE);


//    if (!runs_.empty()) {
//      auto current_run = new TList();
//      current_run->SetName(runs_.GetCurrent().data());
//      current_run->SetOwner(true);
//      output_calibration_runs_->Add(current_run);
//      detectors_.AttachSupportHistograms(current_run);
//    }
//    if (input_calibration_runs_) {
//      auto current_run = (TList *) input_calibration_runs_->FindObject(runs_.GetCurrent().data());
//      if (current_run) {
//        detectors_.AttachCorrectionInputs(current_run);
//      } else {
//        current_run = new TList();
//        current_run->SetName((const char *) fProcessListName);
//        current_run->SetOwner(kTRUE);
//        /* we add it but probably temporarily */
//        fSupportHistogramsList->Add(current_run);
//      }
//    }

    if (fill_qa_histos_) {
    }

    if (fill_validation_qa_histos_) {
    }
    detectors_.IncludeQnVectors();
  }

//  void InitializeQnCorrectionsFramework() {
//    /* the data bank */
//    fDataContainer = new double[nMaxNoOfDataVariables];
//    for (auto &det : fSubEvents) {
//      det->CreateSupportDataStructures();
//    }
//    /* build the support histograms list */
//    fSupportHistogramsList = new TList();
//    fSupportHistogramsList->SetName(szCalibrationHistogramsKeyName);
//    fSupportHistogramsList->SetOwner(kTRUE);
//    /* build the support histograms lists for the list of concurrent processes */
//    /* the QA histograms are no longer rooted on a per process basis */
//    if (fProcessesNames && fProcessesNames->GetEntries()!=0) {
//      for (Int_t i = 0; i < fProcessesNames->GetEntries(); i++) {
//        /* the support histgrams list */
//        auto newList = new TList();
//        newList->SetName(((TObjString *) fProcessesNames->At(i))->GetName());
//        newList->SetOwner(kTRUE);
//        fSupportHistogramsList->Add(newList);
//        /* leave the selected process list name for a latter time */
//        if (!fProcessListName.EqualTo(fProcessesNames->At(i)->GetName())) {
//          /* build the support histograms list associated to the process */
//          for (auto &det : fSubEvents) {
//            det->CreateSupportHistograms(newList);
//          }
//        }
//      }
//    }
//    /* build the support histograms list associated to this process */
//    /* and pass it to the detectors for support histograms creation */
//    if (fProcessListName.Length()!=0) {
//      /* let's see first whether we have the current process name within the processes names list */
//      TList *processList;
//      if (fProcessesNames && fProcessesNames->GetEntries()!=0
//          && fSupportHistogramsList->FindObject(fProcessListName)) {
//        processList = (TList *) fSupportHistogramsList->FindObject(fProcessListName);
//      } else {
//        processList = new TList();
//        processList->SetName((const char *) fProcessListName);
//        processList->SetOwner(kTRUE);
//        /* we add it but probably temporarily */
//        fSupportHistogramsList->Add(processList);
//      }
//      /* now transfer the order to the defined detectors */
//      /* so, we always create the histograms to use the last ones */
//      Bool_t retvalue = kTRUE;
//      for (auto &det : fSubEvents) {
//        retvalue = retvalue && det->CreateSupportHistograms(processList);
//        if (!retvalue)
//          break;
//      }
//      if (!retvalue) {
//        std::clog << "Failed to build the necessary support histograms." << std::endl;
//      }
//    } else {
//      std::clog << "process label is missing." << std::endl;
//    }
//    /* now get the process list on the calibration histograms list if any */
//    /* and pass it to the detectors for input calibration histograms attachment, */
//    if (fCalibrationHistogramsList) {
//      auto processList = (TList *) fCalibrationHistogramsList->FindObject((const char *) fProcessListName);
//      if (processList) {
//
//        /* now transfer the order to the defined detectors */
//        for (auto &det : fSubEvents) {
//          det->AttachCorrectionInputs(processList);
//        }
//        for (auto &det : fSubEvents) {
//          det->AfterInputsAttachActions();
//        }
//      }
//    }
//    /* now build the QA histograms list if needed */
//    /* QA histograms are no longer stored on a per run basis */
//    if (GetShouldFillQAHistograms()) {
//      fQAHistogramsList = new TList();
//      fQAHistogramsList->SetName(szCalibrationQAHistogramsKeyName);
//      fQAHistogramsList->SetOwner(kTRUE);
//      if (GetShouldFillNveQAHistograms()) {
//        fNveQAHistogramsList = new TList();
//        fNveQAHistogramsList->SetName(szCalibrationNveQAHistogramsKeyName);
//        fNveQAHistogramsList->SetOwner(kTRUE);
//      }
//    }
//    /* pass the list to the detectors for QA histograms creation */
//    /* the QA histograms list if needed */
//    if (GetShouldFillQAHistograms()) {
//      /* pass it to the detectors for QA histograms creation */
//      for (auto &det : fSubEvents) {
//        det->CreateQAHistograms(fQAHistogramsList);
//      }
//
//    }
//    /* the non validated QA histograms list if needed */
//    if (GetShouldFillNveQAHistograms()) {
//      /* pass it to the detectors for non validated entries QA histograms creation */
//      for (auto &det : fSubEvents) {
//        det->CreateNveQAHistograms(fNveQAHistogramsList);
//      }
//    }
//    /* build the Qn vectors list */
//    fQnVectorList = new TList();
//    /* the list does not own the Qn vectors */
//    fQnVectorList->SetOwner(kFALSE);
//    /* pass it to the detectors for Qn vector creation and attachment */
//
//  }



  static constexpr int kMaxCorrectionArrayLength = 1000;
  static constexpr auto kCalibListName = "CalibrationHistograms";
  static constexpr auto kQACalibListName = "CalibrationQAHistograms";

  std::unique_ptr<Qn::QAHisto1DPtr> Create1DHisto(const std::string &name, const Qn::AxisF axis,
                                                  const std::string &weightname);

  std::unique_ptr<Qn::QAHisto2DPtr> Create2DHisto(const std::string &name, const std::vector<Qn::AxisF> axes,
                                                  const std::string &weightname);

  std::unique_ptr<Qn::QAHisto2DPtr> Create2DHisto(const std::string &name, const std::vector<Qn::AxisF> axes,
                                                  const std::string &weightname,
                                                  const Qn::AxisF &histaxis);

  void CreateDetectors();

  RunList runs_;
  DetectorList detectors_;
  VariableManager variable_manager_; ///< manager of the variables

  std::string calibration_input_file_name_; ///< name of the calibration input file

  std::unique_ptr<TList> input_calibration_runs_;   //!<! the list of the input calibration histograms
  std::unique_ptr<TList> output_calibration_runs_;  //!<! the list of the support histograms
  std::unique_ptr<TList> calib_qa_histos_;          //!<! the list of QA histograms
  std::unique_ptr<TFile> calibration_input_file_;               //!<! input calibration file
//  TList *validation_histos_ = nullptr;     //!<! the list of not validated entries QA histograms
  bool fill_qa_histos_ = true;
  bool fill_validation_qa_histos_ = true;
  EventClassVariablesSet correction_axes_; /// CorrectionCalculator correction axes

  std::unique_ptr<Cuts> event_cuts_; ///< Pointer to the event cuts
  TList *det_qa_histos_ = nullptr; //!<! List holding the Detector QA histograms. Lifetime is managed by the user.
//  CorrectionCalculator qnc_calculator_; ///< calculator of the corrections
  std::vector<std::unique_ptr<Qn::QAHistoBase>> event_histograms_; ///< event QA histograms
  TTree *out_tree_ = nullptr;  //!<! Tree of Qn Vectors and event variables. Lifetime is managed by the user.
  bool event_passed_cuts_ = false; ///< variable holding status if an event passed the cuts.
};
}

#endif //FLOW_CORRECTIONMANAGER_H
