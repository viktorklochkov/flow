#include "Detector.h"
namespace Qn {
class DetectorList {
 public:
  void AddDetector(std::unique_ptr<Detector> det) {
    auto find = [&det](const std::unique_ptr<Detector> &a) { return det->GetName()==a->GetName(); };
    if (det->GetType()==Qn::DetectorType::CHANNEL) {
      if (std::find_if(channel_detectors.begin(), channel_detectors.end(), find)==channel_detectors.end()) {
        channel_detectors.push_back(std::move(det));
      }
    } else if (det->GetType()==Qn::DetectorType::TRACK) {
      if (std::find_if(tracking_detectors_.begin(), tracking_detectors_.end(), find)==tracking_detectors_.end()) {
        tracking_detectors_.push_back(std::move(det));
      }
    }
  }

  void AddCutCallBack(const std::string &name, std::function<std::unique_ptr<CutBase>(Qn::InputVariableManager *man)> cut) {
    auto &det = FindDetector(name);
    det.AddCutCallBack(std::move(cut));
  }

  Detector &FindDetector(std::string name) {
    auto find = [&name](const std::unique_ptr<Detector> &a) { return a->GetName()==name; };
    auto det_ch = std::find_if(channel_detectors.begin(), channel_detectors.end(), find);
    auto det_trk = std::find_if(tracking_detectors_.begin(), tracking_detectors_.end(), find);
    if (det_ch!=channel_detectors.end()) {
      return **det_ch;
    } else if (det_trk!=tracking_detectors_.end()) {
      return **det_trk;
    } else {
      throw std::runtime_error("Detector" + name + "not found.");
    }
  }

  void SetOutputTree(TTree *output_tree) {
    if (output_tree) {
      for (auto &detector : tracking_detectors_) {
        detector->AttachToTree(output_tree);
      }
      for (auto &detector : channel_detectors) {
        detector->AttachToTree(output_tree);
      }
    }
  }

  void InitializeOnNode(CorrectionManager *manager) {
    for (auto &detector : channel_detectors) {
      detector->InitializeOnNode(manager);
    }
    for (auto &detector : tracking_detectors_) {
      detector->InitializeOnNode(manager);
    }
  }

  void FillTracking() {
    for (auto &dp : tracking_detectors_) {
      dp->FillData();
    }
  }

  void FillChannel() {
    for (auto &dp : channel_detectors) {
      dp->FillData();
    }
  }

  void ProcessCorrections() {
    for (auto &d : channel_detectors) {
      d->ProcessCorrections();
    }
    for (auto &d : tracking_detectors_) {
      d->ProcessCorrections();
    }
  }

  void CreateSupportQVectors() {
    for (auto &d : channel_detectors) {
      d->CreateSupportQVectors();
    }
    for (auto &d : tracking_detectors_) {
      d->CreateSupportQVectors();
    }
  }

  void FillReport() {
    for (auto &d : channel_detectors) {
      d->FillReport();
    }
    for (auto &d : tracking_detectors_) {
      d->FillReport();
    }
  }

  void CreateCorrectionHistograms(TList *list) {
    for (auto &d : channel_detectors) {
      d->CreateCorrectionHistograms(list);
    }
    for (auto &d : tracking_detectors_) {
      d->CreateCorrectionHistograms(list);
    }
  }

  void AttachCorrectionInput(TList *list) {
    for (auto &d : channel_detectors) {
      d->AttachCorrectionInputs(list);
    }
    for (auto &d : tracking_detectors_) {
      d->AttachCorrectionInputs(list);
    }
    for (auto &d : channel_detectors) {
      d->AfterInputAttachAction();
    }
    for (auto &d : tracking_detectors_) {
      d->AfterInputAttachAction();
    }
  }

  void AttachQAHistograms(TList *list, bool fill_qa, bool fill_validation, InputVariableManager *var) {
    for (auto &detector: channel_detectors) {
      auto detector_list = detector->CreateQAHistogramList(fill_qa, fill_validation, var);
      list->Add(detector_list);
    }
    for (auto &detector: tracking_detectors_) {
      auto detector_list = detector->CreateQAHistogramList(fill_qa, fill_validation, var);
      list->Add(detector_list);
    }
  }

  void ResetDetectors() {
    for (auto &d : channel_detectors) {
      d->ClearData();
    }
    for (auto &d : tracking_detectors_) {
      d->ClearData();
    }
  }

  void IncludeQnVectors() {
    for (auto &d : channel_detectors) {
      d->IncludeQnVectors();
    }
    for (auto &d : tracking_detectors_) {
      d->IncludeQnVectors();
    }
  }

 private:
  std::vector<std::unique_ptr<Detector>> tracking_detectors_; ///< map of tracking detectors
  std::vector<std::unique_ptr<Detector>> channel_detectors; ///< map of channel detectors
};
}