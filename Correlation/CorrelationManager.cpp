//
// Created by Lukas Kreis on 15.12.17.
//

#include "CorrelationManager.h"
#include <TFile.h>
#include <memory>
namespace Qn {

void CorrelationManager::AddDataContainer(const std::string &name) {
  TTreeReaderValue<Qn::DataContainerQVector>
      value(*reader_, name.data());
  tree_values_.emplace(name, value);
  DataContainerQVector a;
  qvectors_.emplace(name, a);
}

void CorrelationManager::AddProjection(const std::string &input_name,
                                       const std::string &output_name,
                                       const std::string axis_names) {
  auto toproject = qvectors_.at(input_name);
  DataContainerQVector projection;
  std::vector<std::string> name_vector;
  tokenize(axis_names, name_vector, ", ", true);
  projections_.emplace(output_name, std::make_tuple(input_name, name_vector));
  qvectors_.emplace(output_name, projection);
}

void CorrelationManager::AddEventVariable(const Qn::Axis &eventaxis) {
  TTreeReaderValue<float> value(*reader_, eventaxis.Name().data());
  tree_event_values_.emplace(eventaxis.Name(), value);
  event_values_.emplace_back(-999);
  eventbin_.emplace_back(-1);
  event_axes_.push_back(eventaxis);
}

void CorrelationManager::AddCorrelation(std::string name,
                                        const std::string &input_names,
                                        CorrelationManager::FUNCTION &&lambda,
                                        int nsamples,
                                        Sampler::Method method) {
  std::vector<std::string> containernamelist;
  tokenize(input_names, containernamelist, ", ", true);
  Qn::Correlator correlator(containernamelist, lambda);
  correlator.ConfigureSampler(method, nsamples);
  correlations_.emplace(name, std::move(correlator));
}

void CorrelationManager::AddCorrelation(std::string name,
                                        const std::string &containernames,
                                        CorrelationManager::FUNCTION &&lambda) {
  std::vector<std::string> containernamelist;
  tokenize(containernames, containernamelist, ", ", true);
  Qn::Correlator correlator(containernamelist, lambda);
  correlator.ConfigureSampler(Qn::Sampler::Method::NONE, 0);
  correlations_.emplace(name, std::move(correlator));
}

void CorrelationManager::AddESE(const std::string &name, int harmonic, float qmax) {
  use_ese_ = true;
  auto Mag = [harmonic](const std::vector<Qn::QVector> &a) {
    return 1./TMath::Sqrt(a[0].sumweights())*a[0].DeNormal().mag(harmonic);
  };
  Qn::Correlator correlator({name}, Mag, TH1F("q", ";q;%;", 100, 0., qmax));
  correlator.ConfigureSampler(Sampler::Method::NONE, 0);
  ese_correlations_.emplace("ESE_" + name, std::move(correlator));
}

void CorrelationManager::Initialize() {
  // read ese file to determine if already calibrated
  if (use_ese_) {
    ese_file_ = std::make_unique<TFile>(ese_file_name_.data(), "READ");
    fill_ese_ = true;
    if (ese_file_->IsOpen()) {
      event_shape_.reset((Qn::DataContainerESE *) (ese_file_->Get("ESE"))->Clone("ESE"));
      fill_ese_ = false;
    }
    if (event_shape_) eventbin_.push_back(-1);
  }

  // initialize values to be able to build the correlations.
  for (auto &value : tree_values_) {
    qvectors_.at(value.first) = *value.second.Get();
  }
  int i = 0;
  for (auto &value : tree_event_values_) {
    event_values_.at(i) = *value.second.Get();
    i++;
  }
  MakeProjections();
  BuildESECorrelation();
  BuildCorrelations();
}

void CorrelationManager::Process() {
  UpdateEvent();
  if (CheckEvent()) {
    if (use_ese_) {
      if (event_shape_) FillESE(ese_correlations_);
      if (IsESE()) {
        CheckESEEvent();
      }
    }
    FillCorrelations(correlations_);
  }
}

void CorrelationManager::Finalize() {
  if (use_ese_) {
    FitEventShape();
    SaveEventShape(ese_file_name_);
  }
  SaveToFile(correlation_file_name_);
}

void CorrelationManager::SaveToFile(std::string name) {
  auto outputfile = TFile::Open(name.data(), "RECREATE");
  for (const auto &correlation : correlations_) {
    correlation.second.GetResult().Write(correlation.first.data());
  }
  outputfile->Close();
}

}