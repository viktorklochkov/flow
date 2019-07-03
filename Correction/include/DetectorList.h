#include "Detector.h"
namespace Qn {
class DetectorList {
 public:
  void AddDetector(std::unique_ptr<Detector> det) {
    auto find = [&det](const std::unique_ptr<Detector> &a) { return det->GetName()==a->GetName(); };
    if (det->GetType()==Qn::DetectorType::CHANNEL) {
      if (std::find_if(det_ch_.begin(), det_ch_.end(), find)==det_ch_.end()) {
        det_ch_.push_back(std::move(det));
      }
    } else if (det->GetType()==Qn::DetectorType::TRACK) {
      if (std::find_if(det_trk_.begin(), det_trk_.end(), find)==det_trk_.end()) {
        det_trk_.push_back(std::move(det));
      }
    }

  }

  Detector &FindDetector(std::string name) {
    auto find = [&name](const std::unique_ptr<Detector> &a) { return a->GetName()==name; };
    auto det_ch = std::find_if(det_ch_.begin(), det_ch_.end(), find);
    auto det_trk = std::find_if(det_trk_.begin(), det_trk_.end(), find);
    if (det_ch!=det_ch_.end()) {
      return **det_ch;
    } else if (det_trk!=det_trk_.end()) {
      return **det_trk;
    } else {
      throw std::runtime_error("Detector" + name + "not found.");
    }
  }

  std::vector<std::unique_ptr<Detector>>::iterator beginCh() { return det_ch_.begin(); }
  std::vector<std::unique_ptr<Detector>>::iterator endCh() { return det_ch_.end(); }
  std::vector<std::unique_ptr<Detector>>::iterator beginTrk() { return det_trk_.begin(); }
  std::vector<std::unique_ptr<Detector>>::iterator endTrk() { return det_trk_.end(); }

  void FillTracking() {
    for (auto &dp : det_trk_) {
      dp->FillData();
    }
  }

  void FillChannel() {
    for (auto &dp : det_ch_) {
      dp->FillData();
    }
  }

  void ProcessCorrections() {
    for (auto &d : det_ch_) {
      d->ProcessCorrections();
    }
    for (auto &d : det_trk_) {
      d->ProcessCorrections();
    }
  }

  void CreateSupportStructures() {
    for (auto &d : det_ch_) {
      d->CreateSupportDataStructures();
    }
    for (auto &d : det_trk_) {
      d->CreateSupportDataStructures();
    }
  }

  void AttachSupportHistograms(TList *list) {
    for (auto &d : det_ch_) {
      d->AttachSupportHistograms(list);
    }
    for (auto &d : det_trk_) {
      d->AttachSupportHistograms(list);
    }
  }

  void AttachCorrectionInputs(TList *list) {
    for (auto &d : det_ch_) {
      d->AttachCorrectionInputs(list);
    }
    for (auto &d : det_trk_) {
      d->AttachCorrectionInputs(list);
    }
    for (auto &d : det_ch_) {
      d->AfterInputsAttachActions();
    }
    for (auto &d : det_trk_) {
      d->AfterInputsAttachActions();
    }
  }

  void AddQAHistograms(TList *list) {
    for (auto &det: det_ch_) {
      auto detlist = det->GetReportList();
      detlist->SetName(det->GetName().data());
      list->Add(detlist);
      det->AttachNveQAHistograms(list);
      det->AttachQAHistograms(list);
    }
    for (auto &det: det_trk_) {
      auto detlist = det->GetReportList();
      detlist->SetName(det->GetName().data());
      list->Add(detlist);
      det->AttachNveQAHistograms(list);
      det->AttachQAHistograms(list);
    }
  }

  void CreateSubEvents(const EventClassVariablesSet &set) {
    for (auto &d : det_ch_) {
      d->ConfigureSubEvents(set);
    }
    for (auto &d : det_trk_) {
      d->ConfigureSubEvents(set);
    }
  }

  void ResetDetectors() {
    for (auto &d : det_ch_) {
      d->ClearData();
    }
    for (auto &d : det_trk_) {
      d->ClearData();
    }
  }

  void IncludeQnVectors() {
    for (auto &d : det_ch_) {
      d->IncludeQnVectors();
    }
    for (auto &d : det_trk_) {
      d->IncludeQnVectors();
    }
  }

 private:
  std::vector<std::unique_ptr<Detector>> det_trk_; ///< map of tracking detectors
  std::vector<std::unique_ptr<Detector>> det_ch_; ///< map of channel detectors
};
}