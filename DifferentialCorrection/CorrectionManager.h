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

  void AddChannelVariable(const std::string &name, const int size) {
    var_manager_->CreateChannelVariable(name, size);
  }

  void AddCorrectionAxis(const Qn::Axis &variable) { qncorrections_axis_.push_back(variable); }

  void SetEventVariable(const std::string &name) { event_variables_->AddVariable(name); }

  void AddDetector(const std::string &name,
                   DetectorType type,
                   const std::string &phi_name,
                   const std::string &weight_name = "Ones",
                   const std::vector<Qn::Axis> &axes = {}) {
    Variable phi = var_manager_->FindVariable(phi_name);
    Variable weight = var_manager_->FindVariable(weight_name);
    std::vector<Variable> vars;
    for (const auto &axis : axes) {
      vars.push_back(var_manager_->FindVariable(axis.Name()));
    }
    Detector det(type, axes, phi, weight, vars);
    if (type==DetectorType::Channel) detectors_channel.emplace(std::make_pair(name, std::move(det)));
//    if (type == DetectorType::Track) detectors_track.emplace(std::move(name), std::move(det));
  }
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

  void AddHisto1D(const std::string &name, std::vector<Qn::Axis> axes, const std::string &weightname) {
    std::string spacer("_");
    auto hist_name = (name + spacer + axes[0].Name() + spacer + weightname);
    const int size = axes[0].size();
    Variable var;
    try { var = var_manager_->FindVariable(axes[0].Name()); }
    catch (std::out_of_range &) {
      std::cout << "QAHistogram "<< name << ": Variable " << axes[0].Name() << " not found. Creating new channel variable." << std::endl;
      var_manager_->CreateChannelVariable(axes[0].Name(), size);
    }
    float upper_edge = axes[0].GetUpperBinEdge(size - 1);
    float lower_edge = axes[0].GetLowerBinEdge(0);
    TH1F histo(hist_name.data(), (std::string(";")+axes[0].Name()).data(), size, lower_edge, upper_edge);
    std::array<Variable, 2> arr = {{var_manager_->FindVariable(axes[0].Name()), var_manager_->FindVariable(weightname)}};
    try { detectors_track.at(name).AddHistogram(arr, histo); }
    catch (std::out_of_range &) {
      try { detectors_channel.at(name).AddHistogram(arr, histo); }
      catch (std::out_of_range &) {
        throw std::out_of_range(
            name + " was not found in the list of detectors. It needs to be created before a histogram can be added.");
      }
    }
  }

  void AddHisto1D(const std::string &name, const std::string &varname, const std::string &weightname, TH1F histo) {
    std::array<Variable, 2> arr = {{var_manager_->FindVariable(varname), var_manager_->FindVariable(weightname)}};
    try { detectors_track.at(name).AddHistogram(arr, histo); }
    catch (std::out_of_range &) {
      try { detectors_channel.at(name).AddHistogram(arr, histo); }
      catch (std::out_of_range &) {
        throw std::out_of_range(
            name + " was not found in the list of detectors. It needs to be created before a histogram can be added.");
      }
    }
  }

  void SetCorrectionSteps(const std::string &name,
                          std::function<void(QnCorrectionsDetectorConfigurationBase *config)> config);

  void SaveQVectorsToTree(TTree &tree);

  void SaveEventVariablesToTree(TTree &tree);

  void FillData(Qn::DataFiller filler) {
    filler.Fill(detectors_channel, detectors_track, var_manager_);
    var_manager_->FillToQnCorrections(qncorrections_manager_.GetDataPointer());
  }

  void SaveCorrectionHistograms(std::shared_ptr<TFile> file);

  void Initialize(std::shared_ptr<TFile> &in_calibration_file_);

  void Process(Qn::DataFiller filler);

  void Finalize();

  void Reset();

 private:
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
};
}

#endif //FLOW_CORRECTIONMANAGER_H
