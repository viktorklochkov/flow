
#include "Detector.h"
#include "CorrectionManager.h"

namespace Qn {

void Detector::InitializeOnNode(CorrectionManager *manager) {
  auto var = manager->GetVariableManager();
  auto set = manager->GetCorrectionAxes();
  detectors_ = manager->GetDetectors();
  phi_ = var->FindVariable(phi_.GetName());
  weight_ = var->FindVariable(weight_.GetName());
  //Configure sub events
  if (!configuration_) {
    throw (std::runtime_error("No Qn correction configuration found for " + name_));
  }
  int ibin = 0;
  for (auto &ev : sub_events_) {
    std::string name = name_;
    if (!sub_events_.IsIntegrated()) name += std::to_string(ibin);
    if (type_==DetectorType::CHANNEL) {
      ev = std::make_unique<SubEventChannels>(ibin, set, nchannels_, harmonics_bits_);
    } else if (type_==DetectorType::TRACK) {
      ev = std::make_unique<SubEventTracks>(ibin, set, harmonics_bits_);
    }
    ev->SetDetector(this);
    configuration_(ev.get());
    ++ibin;
  }
  if (!sub_events_.IsIntegrated()) {
    for (const auto &axis : sub_events_.GetAxes()) {
      vars_.push_back(var->FindVariable(axis.Name()));
    }
    coordinates_.resize(vars_.size());
  }
  // Initialize the cuts
  CreateCuts(var);
  int_cuts_.CreateCutReport(name_, 1);
  cuts_.CreateCutReport(name_, phi_.GetSize());
}

TList *Detector::CreateQAHistogramList(bool fill_qa, bool fill_validation, InputVariableManager *var) {
  auto list = new TList();
  list->SetName(name_.data());
  // Add cut reports
  int_cuts_.AddToList(list);
  cuts_.AddToList(list);
  // Add detector qa
  auto detector_qa = new TList();
  detector_qa->SetOwner(true);
  detector_qa->SetName("detector_QA");
  CreateHistograms(var);
  for (auto &histo : histograms_) {
    histo->AddToList(detector_qa);
  }
  list->Add(detector_qa);
  // Add qvector qa
  if (fill_qa) {
    auto qvector_qa = new TList();
    qvector_qa->SetName("q_vector_QA");
    qvector_qa->SetOwner(true);
    for (auto &ev : sub_events_) { ev->AttachQAHistograms(qvector_qa); }
    list->Add(qvector_qa);
  }
  // Add qvector bin validation qa
  if (fill_validation) {
    auto valid_qa = new TList();
    valid_qa->SetName("bin_validation_QA");
    valid_qa->SetOwner(true);
    for (auto &ev : sub_events_) { ev->AttachNveQAHistograms(valid_qa); }
    list->Add(valid_qa);
  }
  return list;
}

}