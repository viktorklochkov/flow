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
#include "HistogramManager.h"
namespace Qn {
class CorrectionManager {
 public:

  CorrectionManager()
      : event_variables_(new Qn::EventInfoF()), var_manager_(new VariableManager()), histogram_manager_(var_manager_) {}

  void AddVariable(const std::string &name, const int number) { var_manager_->AddVariable(name, number); }

  void AddCorrectionAxis(const Qn::Axis &variable) { qncorrections_axis_.push_back(variable); }

  void SetEventVariable(const std::string &name) { event_variables_->AddVariable(name); }

  void AddHist1D(const std::string &x, const std::vector<float> &bin_edges);

  void AddHist1D(const std::string &x, int nbins, float xlo, float xhi);

  void AddHist2D(const std::string &x, int xbins, float xlo, float xhi,
                 const std::string &y, int ybins, float ylo, float yhi);

  void AddHist2D(const std::string &x, const std::vector<float> &x_edges,
                 const std::string &y, const std::vector<float> &y_edges);

  void SetEventCut() {};

  void AddDetector(const std::string &name, DetectorType type,
                   const std::vector<Qn::Axis> &axes, int nchannels = 0);

  void AddDetector(const std::string &name, DetectorType type, int nchannels = 0);

  void SetCorrectionSteps(const std::string &name,
                          std::function<void(QnCorrectionsDetectorConfigurationBase *config)> config);

  void SaveQVectorsToTree(TTree &tree);

  void SaveQaHistograms();

  void SaveEventVariablesToTree(TTree &tree);

  void FillDataToFramework(Qn::Differential::Interface::DataFiller filler);

  void SaveCorrectionHistograms();

  float *GetVariableContainer() { return qncorrections_manager_.GetDataContainer(); }

  void Initialize(std::shared_ptr<TFile> &in_calibration_file_);

  void Process();

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

  QnCorrectionsEventClassVariablesSet *qncorrections_varset_;
  std::unique_ptr<Qn::EventInfoF> event_variables_;
  std::vector<Qn::Axis> qncorrections_axis_;
  QnCorrectionsManager qncorrections_manager_;
  std::shared_ptr<VariableManager> var_manager_;
  std::map<std::string, Detector> detectors_;
  Qn::HistogramManager histogram_manager_;
};
}

#endif //FLOW_CORRECTIONMANAGER_H
