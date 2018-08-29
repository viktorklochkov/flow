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
  CorrectionManager()
      : event_variables_(new Qn::EventInfoF()), var_manager_(new VariableManager()) {
    var_manager_->CreateVariableOnes();
  }

  void AddVariable(const std::string &name, const int id, const int length) {
    var_manager_->CreateVariable(name, id, length);
  }

  void AddCorrectionAxis(const Qn::Axis &variable) { qncorrections_axis_.push_back(variable); }

  void SetEventVariable(const std::string &name) { event_variables_->AddVariable(name); }

  void AddDetector(const std::string &name, DetectorType type, const std::string &phi_name,
                   const std::string &weight_name = "Ones", const std::vector<Qn::Axis> &axes = {});

  template<typename FUNCTION>
  void AddCut(const std::string &name, const std::string &var_name, FUNCTION lambda) {
    auto variable = var_manager_->FindVariable(var_name);
    try { detectors_track.at(name).AddCut(variable, lambda); }
    catch (std::out_of_range &) {
      try { detectors_channel.at(name).AddCut(variable, lambda); }
      catch (std::out_of_range &) {
        throw std::out_of_range(
            name + " was not found in the list of detectors. It needs to be created before a cut can be added.");
      }
    }
  }
  template<std::size_t N, typename FUNCTION>
  auto AddEventCut(std::string const (&name_arr)[N], FUNCTION &&func) {
    Variable arr[N];
    int i = 0;
    for (auto &name : name_arr) {
      arr[i] = var_manager_->FindVariable(name);
      ++i;
    }
    event_cuts_.push_back(MakeUniqueNDimCut(arr, func));
  }

  void AddEventHisto1D(std::vector<Qn::Axis> axes, const std::string &weightname = "Ones") {
    std::string name("Ev");
    auto pair = Create1DHisto(name,axes,weightname);
    event_histograms_.push_back(std::make_unique<QAHisto1D>(pair.first,pair.second));
  }

  void AddEventHisto2D(std::vector<Qn::Axis> axes, const std::string &weightname = "Ones") {
    std::string name("Ev");
    auto pair = Create2DHisto(name,axes,weightname);
    event_histograms_.push_back(std::make_unique<QAHisto2D>(pair.first,pair.second));
  }

  void AddHisto1D(const std::string &name, std::vector<Qn::Axis> axes, const std::string &weightname);

  void AddHisto2D(const std::string &name, std::vector<Qn::Axis> axes, const std::string &weightname = "Ones");

  void AddHisto1D(const std::string &name, const std::string &varname, const std::string &weightname, TH1F histo);

  void SetCorrectionSteps(const std::string &name,
                          std::function<void(QnCorrectionsDetectorConfigurationBase *config)> config);

  void SaveQVectorsToTree(TTree &tree);

  void SaveEventVariablesToTree(TTree &tree);

  void FillData(Qn::DataFiller filler);

  void SaveCorrectionHistograms(std::shared_ptr<TFile> file);

  void Initialize(std::shared_ptr<TFile> &in_calibration_file_);

  void Process(Qn::DataFiller filler);

  void Finalize();

  void Reset();

 private:

  std::pair<std::array<Variable,2>,TH1F> Create1DHisto(const std::string &name,
                     std::vector<Qn::Axis> axes,
                     const std::string &weightname);

  std::pair<std::array<Variable,3>,TH2F> Create2DHisto(const std::string &name,
                     std::vector<Qn::Axis> axes,
                     const std::string &weightname);

  int kMaxCorrectionArrayLength = 200;

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
  std::unique_ptr<Qn::EventInfoF> event_variables_;
  std::vector<Qn::Axis> qncorrections_axis_;
  QnCorrectionsManager qncorrections_manager_;
  std::shared_ptr<VariableManager> var_manager_;
  std::map<std::string, Detector> detectors_track;
  std::map<std::string, Detector> detectors_channel;
  std::vector<std::unique_ptr<Qn::QAHistoBase>> event_histograms_;
  std::vector<std::unique_ptr<Qn::VariableCutBase>> event_cuts_;
};
}

#endif //FLOW_CORRECTIONMANAGER_H
