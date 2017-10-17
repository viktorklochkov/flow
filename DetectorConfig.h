//
// Created by Lukas Kreis on 24.08.17.
//

#ifndef FLOW_DETECTORCONFIG_H
#define FLOW_DETECTORCONFIG_H
#include <QnCorrections/QnCorrectionsProfile3DCorrelations.h>
#include <QnCorrections/QnCorrectionsProfileCorrelationComponents.h>
#include <QnCorrections/QnCorrectionsDetectorConfigurationTracks.h>
#include <QnCorrections/QnCorrectionsDetectorConfigurationChannels.h>
#include <QnCorrections/QnCorrectionsQnVectorRecentering.h>
#include <QnCorrections/QnCorrectionsQnVectorAlignment.h>
#include <QnCorrections/QnCorrectionsQnVectorTwistAndRescale.h>
#include <QnCorrections/QnCorrectionsDetector.h>
#include <map>

namespace Qn {
namespace Configuration {
enum class DetectorId : int {
  TPC,
  TPC_reference,
  VZEROA_reference,
  VZEROC_reference,
  VZEROA,
  VZEROC,
  FMDA_reference,
  FMDC_reference,
  ZDCA_reference,
  ZDCC_reference,
};
static std::map<int, const char *> DetectorNames = {{(int) DetectorId::TPC, "TPC"},
                                                    {(int) DetectorId::TPC_reference, "TPC_reference"},
                                                    {(int) DetectorId::VZEROA_reference, "VZEROA_reference"},
                                                    {(int) DetectorId::VZEROC_reference, "VZEROC_reference"},
                                                    {(int) DetectorId::VZEROA, "VZEROA"},
                                                    {(int) DetectorId::VZEROC, "VZEROC"},
                                                    {(int) DetectorId::FMDA_reference, "FMDA_reference"},
                                                    {(int) DetectorId::FMDC_reference, "FMDC_reference"},
                                                    {(int) DetectorId::ZDCA_reference, "ZDCA_reference"},
                                                    {(int) DetectorId::ZDCC_reference, "ZDCC_reference"}};

enum class DetectorType {
  Track,
  Channel
};

class DetectorConfig {
 public:
  DetectorConfig() = default;
  virtual ~DetectorConfig() = default;
  virtual void operator()(QnCorrectionsDetectorConfigurationBase *config) {};
};

class TPC : public DetectorConfig {
 public:
  void operator()(QnCorrectionsDetectorConfigurationBase *config) override {
    config->SetQVectorNormalizationMethod(QnCorrectionsQnVector::QVNORM_QoverM);
    config->AddCorrectionOnQnVector(new QnCorrectionsQnVectorRecentering());
    auto rescale = new QnCorrectionsQnVectorTwistAndRescale();
    rescale->SetApplyTwist(true);
    rescale->SetApplyRescale(false);
    rescale->SetTwistAndRescaleMethod(QnCorrectionsQnVectorTwistAndRescale::TWRESCALE_doubleHarmonic);
    config->AddCorrectionOnQnVector(rescale);
  }
};

class VZEROA : public DetectorConfig {
 public:
  void operator()(QnCorrectionsDetectorConfigurationBase *config) override {
    config->SetQVectorNormalizationMethod(QnCorrectionsQnVector::QVNORM_QoverM);
    config->AddCorrectionOnQnVector(new QnCorrectionsQnVectorRecentering());
    auto alignment = new QnCorrectionsQnVectorAlignment();
    alignment->SetReferenceConfigurationForAlignment("TPC_reference0");
    alignment->SetHarmonicNumberForAlignment(2);
    config->AddCorrectionOnQnVector(alignment);
    auto rescale = new QnCorrectionsQnVectorTwistAndRescale();
    rescale->SetApplyTwist(true);
    rescale->SetApplyRescale(true);
    rescale->SetTwistAndRescaleMethod(QnCorrectionsQnVectorTwistAndRescale::TWRESCALE_correlations);
    rescale->SetReferenceConfigurationsForTwistAndRescale("TPC_reference0", "VZEROC_reference0");
    config->AddCorrectionOnQnVector(rescale);
  }
};

class VZEROC : public DetectorConfig {
 public:
  void operator()(QnCorrectionsDetectorConfigurationBase *config) override {
    config->SetQVectorNormalizationMethod(QnCorrectionsQnVector::QVNORM_QoverM);
    config->AddCorrectionOnQnVector(new QnCorrectionsQnVectorRecentering());
    auto alignment = new QnCorrectionsQnVectorAlignment();
    alignment->SetReferenceConfigurationForAlignment("TPC_reference0");
    alignment->SetHarmonicNumberForAlignment(2);
    config->AddCorrectionOnQnVector(alignment);
    auto rescale = new QnCorrectionsQnVectorTwistAndRescale();
    rescale->SetApplyTwist(true);
    rescale->SetApplyRescale(true);
    rescale->SetTwistAndRescaleMethod(QnCorrectionsQnVectorTwistAndRescale::TWRESCALE_correlations);
    rescale->SetReferenceConfigurationsForTwistAndRescale("TPC_reference0", "VZEROA_reference0");
    config->AddCorrectionOnQnVector(rescale);
  }
};

class VZEROA_reference : public DetectorConfig {
 public:
  void operator()(QnCorrectionsDetectorConfigurationBase *config) override {
    config->SetQVectorNormalizationMethod(QnCorrectionsQnVector::QVNORM_QoverM);
    config->AddCorrectionOnQnVector(new QnCorrectionsQnVectorRecentering());
    auto alignment = new QnCorrectionsQnVectorAlignment();
    alignment->SetReferenceConfigurationForAlignment("TPC_reference0");
    alignment->SetHarmonicNumberForAlignment(2);
    config->AddCorrectionOnQnVector(alignment);
    auto rescale = new QnCorrectionsQnVectorTwistAndRescale();
    rescale->SetApplyTwist(true);
    rescale->SetApplyRescale(true);
    rescale->SetTwistAndRescaleMethod(QnCorrectionsQnVectorTwistAndRescale::TWRESCALE_correlations);
    rescale->SetReferenceConfigurationsForTwistAndRescale("TPC_reference0", "VZEROC_reference0");
    config->AddCorrectionOnQnVector(rescale);
  }
};

class VZEROC_reference : public DetectorConfig {
 public:
  void operator()(QnCorrectionsDetectorConfigurationBase *config) override {
    config->SetQVectorNormalizationMethod(QnCorrectionsQnVector::QVNORM_QoverM);
    config->AddCorrectionOnQnVector(new QnCorrectionsQnVectorRecentering());
    auto alignment = new QnCorrectionsQnVectorAlignment();
    alignment->SetReferenceConfigurationForAlignment("TPC_reference0");
    alignment->SetHarmonicNumberForAlignment(2);
    config->AddCorrectionOnQnVector(alignment);
    auto rescale = new QnCorrectionsQnVectorTwistAndRescale();
    rescale->SetApplyTwist(true);
    rescale->SetApplyRescale(true);
    rescale->SetTwistAndRescaleMethod(QnCorrectionsQnVectorTwistAndRescale::TWRESCALE_correlations);
    rescale->SetReferenceConfigurationsForTwistAndRescale("TPC_reference0", "VZEROA_reference0");
    config->AddCorrectionOnQnVector(rescale);
  }
};

class FMDA_reference : public DetectorConfig {
 public:
  void operator()(QnCorrectionsDetectorConfigurationBase *config) override {
//    config->SetQVectorNormalizationMethod(QnCorrectionsQnVector::QVNORM_QoverM);
    config->AddCorrectionOnQnVector(new QnCorrectionsQnVectorRecentering());
    auto align = new QnCorrectionsQnVectorAlignment();
    align->SetHarmonicNumberForAlignment(2);
    align->SetReferenceConfigurationForAlignment("TPC_reference0");
    config->AddCorrectionOnQnVector(align);
    auto rescale = new QnCorrectionsQnVectorTwistAndRescale();
    rescale->SetApplyTwist(kTRUE);
    rescale->SetApplyRescale(kTRUE);
    rescale->SetTwistAndRescaleMethod(QnCorrectionsQnVectorTwistAndRescale::TWRESCALE_correlations);
    rescale->SetReferenceConfigurationsForTwistAndRescale("TPC_reference0", "FMDC_reference0");
    config->AddCorrectionOnQnVector(rescale);
  }
};

class FMDC_reference : public DetectorConfig {
 public:
  void operator()(QnCorrectionsDetectorConfigurationBase *config) override {
    config->SetQVectorNormalizationMethod(QnCorrectionsQnVector::QVNORM_QoverM);
    config->AddCorrectionOnQnVector(new QnCorrectionsQnVectorRecentering());
    auto align = new QnCorrectionsQnVectorAlignment();
    align->SetHarmonicNumberForAlignment(2);
    align->SetReferenceConfigurationForAlignment("TPC_reference0");
    config->AddCorrectionOnQnVector(align);
    auto rescale = new QnCorrectionsQnVectorTwistAndRescale();
    rescale->SetApplyTwist(kTRUE);
    rescale->SetApplyRescale(kTRUE);
    rescale->SetTwistAndRescaleMethod(QnCorrectionsQnVectorTwistAndRescale::TWRESCALE_correlations);
    rescale->SetReferenceConfigurationsForTwistAndRescale("TPC_reference0", "FMDA_reference0");
    config->AddCorrectionOnQnVector(rescale);
  }
};

class ZDCA_reference : public DetectorConfig {
 public:
  void operator()(QnCorrectionsDetectorConfigurationBase *config) override {
    config->SetQVectorNormalizationMethod(QnCorrectionsQnVector::QVNORM_QoverM);
    config->AddCorrectionOnQnVector(new QnCorrectionsQnVectorRecentering());
  }
};

class ZDCC_reference : public DetectorConfig {
 public:
  void operator()(QnCorrectionsDetectorConfigurationBase *config) override {
    config->SetQVectorNormalizationMethod(QnCorrectionsQnVector::QVNORM_QoverM);
    config->AddCorrectionOnQnVector(new QnCorrectionsQnVectorRecentering());
  }
};

}
}

#endif //FLOW_DETECTORCONFIG_H
