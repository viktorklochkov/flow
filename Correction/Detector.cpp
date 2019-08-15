
#include "Detector.h"
#include "CorrectionManager.h"

namespace Qn {

void Detector::Initialize(CorrectionManager *manager) {
  auto var = manager->GetVariableManager();
  detectors_ = manager->GetDetectors();
  var->InitVariable(phi_);
  var->InitVariable(weight_);
  var->InitVariable(radial_offset_);
  //Configure sub events
  if (!correction_configuration_) {
    throw (std::runtime_error("No Qn correction configuration found for " + name_));
  }
  int ibin = 0;
  for (auto &event : sub_events_) {
    if (type_==DetectorType::CHANNEL) {
      event = std::make_unique<SubEventChannels>(ibin, manager->GetCorrectionAxes(), nchannels_, harmonics_bits_);
    } else if (type_==DetectorType::TRACK) {
      event = std::make_unique<SubEventTracks>(ibin, manager->GetCorrectionAxes(), harmonics_bits_);
    }
    event->SetDetector(this);
    correction_configuration_(event.get());
    ++ibin;
  }
  if (!sub_events_.IsIntegrated()) {
    for (const auto &axis : sub_events_.GetAxes()) {
      input_variables_.push_back(var->FindVariable(axis.Name()));
    }
    coordinates_.resize(input_variables_.size());
  }
  // Initialize the cuts
  cuts_.Initialize(var);
  int_cuts_.Initialize(var);
  histograms_.Initialize(var);
}

TList *Detector::CreateQAHistogramList(bool fill_qa, bool fill_validation) {
  auto list = new TList();
  list->SetName(name_.data());
  // Add detector qa
  auto detector_qa = new TList();
  detector_qa->SetOwner(true);
  detector_qa->SetName("detector_QA");
  int_cuts_.CreateCutReport(name_+":", 1);
  cuts_.CreateCutReport(name_+":", phi_.size());
  int_cuts_.AddToList(detector_qa);
  cuts_.AddToList(detector_qa);
  histograms_.AddToList(detector_qa);
  list->Add(detector_qa);
  // Add qvector qa
  if (fill_qa) {
    auto qvector_qa = new TList();
    qvector_qa->SetName("q_vector_QA");
    qvector_qa->SetOwner(true);
    for (auto &ev : sub_events_) ev->AttachQAHistograms(qvector_qa);
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