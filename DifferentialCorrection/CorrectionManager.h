//
// Created by Lukas Kreis on 16.01.18.
//

#ifndef FLOW_CORRECTIONMANAGER_H
#define FLOW_CORRECTIONMANAGER_H

#include <string>
#include <map>
#include <utility>
#include <QnCorrections/QnCorrectionsManager.h>
#include "Detector.h"
#include "VariableManager.h"
#include "Base/Axis.h"
#include "EventInfo.h"
#include "DifferentialCorrection/Interface/DataFiller.h"
#include "VariableCutBase.h"
namespace Qn {
class CorrectionManager {
 public:

  using MapDetectors = std::map<std::string, Detector>;

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
   */
  void AddDetector(const std::string &name, DetectorType type, const std::string &phi_name,
                   const std::string &weight_name = "Ones", const std::vector<Qn::Axis> &axes = {});

  /**
   * Adds a cut to a detector.
   * @tparam FUNCTION
   * @param name
   * @param var_name
   * @param lambda
   */
  template<std::size_t N, typename FUNCTION>
  void AddCut(const std::string &name, const char* const (&names)[N], FUNCTION lambda) {
    Variable arr[N];
    int i = 0;
    for (auto &n : names) {
      arr[i] = var_manager_->FindVariable(n);
      ++i;
    }
    auto cut = MakeUniqueNDimCut(arr, lambda);
    try { detectors_track.at(name).AddCut(std::move(cut)); }
    catch (std::out_of_range &) {
      try { detectors_channel.at(name).AddCut(std::move(cut)); }
      catch (std::out_of_range &) {
        throw std::out_of_range(
            name + " was not found in the list of detectors. It needs to be created before a cut can be added.");
      }
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
  void AddEventCut(const char* const (&name_arr)[N], FUNCTION &&func) {
    Variable arr[N];
    int i = 0;
    for (auto &name : name_arr) {
      arr[i] = var_manager_->FindVariable(name);
      ++i;
    }
    auto cut = MakeUniqueNDimCut(arr, func);
    event_cuts_->AddCut(std::move(cut));
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
                          std::function<void(QnCorrectionsDetectorConfigurationBase *config)> config);

  void SetTree(std::unique_ptr<TTree> tree) { out_tree_ = std::move(tree); }

  void SaveHistograms(std::shared_ptr<TFile> file);

  void SaveTree(const std::shared_ptr<TFile> &file) {
    file->cd();
    out_tree_->Write();
    out_tree_.release();
  }

  void Initialize(std::shared_ptr<TFile> &in_calibration_file_);

  void Process(Qn::DataFiller filler);

  void Finalize();

  void Reset();

 private:

  void FillEventData(Qn::DataFiller filler);

  void FillData(Qn::DataFiller filler);

  void SetQVectorsToTree();

  void SetEventVariablesToTree();

  std::pair<std::array<Variable, 2>, TH1F> Create1DHisto(const std::string &name,
                                                         std::vector<Qn::Axis> axes,
                                                         const std::string &weightname);

  std::pair<std::array<Variable, 3>, TH2F> Create2DHisto(const std::string &name,
                                                         std::vector<Qn::Axis> axes,
                                                         const std::string &weightname);

  void CreateDetectors();

  void FillDataToFramework();

  void GetQnFromFramework(const std::string &step);

  void CalculateCorrectionAxis();
/**
 * Get Normalization from Qn::CorrectionsQnVector framework
 * @param method normalization method.
 * @return corresponding correlation.
 */
  inline Qn::QVector::Normalization GetNormalization(QnCorrectionsQnVector::QnVectorNormalizationMethod method) {
    if (method==QnCorrectionsQnVector::QnVectorNormalizationMethod::QVNORM_noCalibration)
      return Qn::QVector::Normalization::NOCALIB;
    if (method==QnCorrectionsQnVector::QnVectorNormalizationMethod::QVNORM_QoverM)
      return Qn::QVector::Normalization::QOVERM;
    if (method==QnCorrectionsQnVector::QnVectorNormalizationMethod::QVNORM_QoverSqrtM)
      return Qn::QVector::Normalization::QOVERSQRTM;
    if (method==QnCorrectionsQnVector::QnVectorNormalizationMethod::QVNORM_QoverQlength)
      return Qn::QVector::Normalization::QOVERNORMQ;
    return Qn::QVector::Normalization::NOCALIB;
  }

  QnCorrectionsEventClassVariablesSet *qncorrections_varset_ = nullptr;
  std::unique_ptr<Cuts> event_cuts_;
  std::unique_ptr<Qn::EventInfoF> event_variables_;
  std::vector<Qn::Axis> qncorrections_axis_;
  QnCorrectionsManager qncorrections_manager_;
  std::shared_ptr<VariableManager> var_manager_;
  std::map<std::string, Detector> detectors_track;
  std::map<std::string, Detector> detectors_channel;
  std::vector<std::unique_ptr<Qn::QAHistoBase>> event_histograms_;
  std::unique_ptr<TTree> out_tree_ = nullptr;
};
}

#endif //FLOW_CORRECTIONMANAGER_H
