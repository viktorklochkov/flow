#include "Detector.h"
namespace Qn {
class DetectorList {
 public:
  void AddDetector(std::unique_ptr<Detector> det) {
    auto find = [&det](const std::unique_ptr<Detector> &a) { return det->GetName()==a->GetName(); };
    if (det->GetType()==Qn::DetectorType::CHANNEL) {
      if (std::find_if(channel_detectors_.begin(), channel_detectors_.end(), find)==channel_detectors_.end()) {
        channel_detectors_.push_back(std::move(det));
        all_detectors_.push_back(channel_detectors_.back().get());
      }
    } else if (det->GetType()==Qn::DetectorType::TRACK) {
      if (std::find_if(tracking_detectors_.begin(), tracking_detectors_.end(), find)==tracking_detectors_.end()) {
        tracking_detectors_.push_back(std::move(det));
        all_detectors_.push_back(tracking_detectors_.back().get());
      }
    }
  }

  void AddCutCallBack(const std::string &name,
                      std::function<std::unique_ptr<CutBase>(Qn::InputVariableManager *man)> cut) {
    auto &det = FindDetector(name);
    det.AddCutCallBack(std::move(cut));
  }

  Detector &FindDetector(std::string name) {
    auto find = [&name](const std::unique_ptr<Detector> &a) { return a->GetName()==name; };
    auto det_ch = std::find_if(channel_detectors_.begin(), channel_detectors_.end(), find);
    auto det_trk = std::find_if(tracking_detectors_.begin(), tracking_detectors_.end(), find);
    if (det_ch!=channel_detectors_.end()) {
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
      for (auto &detector : channel_detectors_) {
        detector->AttachToTree(output_tree);
      }
    }
  }

  void InitializeOnNode(CorrectionManager *manager) {
    for (auto &detector : channel_detectors_) {
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
    for (auto &dp : channel_detectors_) {
      dp->FillData();
    }
  }

  void ProcessCorrections() {
    for (auto &d : channel_detectors_) {
      d->ProcessCorrections();
    }
    for (auto &d : tracking_detectors_) {
      d->ProcessCorrections();
    }
  }

  void CreateSupportQVectors() {
    for (auto &d : all_detectors_) {
      d->CreateSupportQVectors();
    }
  }

  void FillReport() {
    for (auto &d : all_detectors_) {
      d->FillReport();
    }
  }

  void CreateCorrectionHistograms(TList *list) {
    for (auto &d : all_detectors_) {
      d->CreateCorrectionHistograms(list);
    }
  }

  void AttachCorrectionInput(TList *list) {
    for (auto &d : all_detectors_) {
      d->AttachCorrectionInputs(list);
    }
    for (auto &d : all_detectors_) {
      d->AfterInputAttachAction();
    }
  }

  void AttachQAHistograms(TList *list, bool fill_qa, bool fill_validation, InputVariableManager *var) {
    for (auto &detector: all_detectors_) {
      auto detector_list = detector->CreateQAHistogramList(fill_qa, fill_validation, var);
      list->Add(detector_list);
    }
  }

  void ResetDetectors() {
    for (auto &d : all_detectors_) {
      d->ClearData();
    }
  }

  void IncludeQnVectors() {
    for (auto &d : all_detectors_) {
      d->IncludeQnVectors();
    }
  }

  void CreateReport() {
    auto iteration = CalculateProgress(all_detectors_);
    std::cout << "iteration " << iteration.first << " of " << iteration.second << std::endl;
    for (auto &d : all_detectors_) {
      auto corrections = d->GetSubEvent(0)->ReportOnCorrections();
      std::cout << d->GetName() << std::endl;
      for (const auto &step : corrections) {
        std::cout << step.first << " : ";
        if (step.second.first) {
          std::cout << "collecting ";
        }
        if (step.second.second) {
          std::cout << "applying";
        }
        if (!step.second.first && !step.second.second) {
          std::cout << "waiting";
        }
        std::cout << std::endl;
      }
    }
  }

 private:

  std::pair<int, int> CalculateProgress(const std::vector<Detector *> &detectors) {
    int remaining_iterations_global = 0;
    int total_iterations_global = 0;
    for (const auto &d : detectors) {
      auto corrections = d->GetSubEvent(0)->ReportOnCorrections();
      int performed_iterations = 0;
      int total_iterations = corrections.size();
      for (auto &c : corrections) { if (c.second.second) performed_iterations++; }
      if (total_iterations - performed_iterations > remaining_iterations_global) {
        remaining_iterations_global = performed_iterations;
      }
      if (total_iterations_global < total_iterations) total_iterations_global = total_iterations;
    }
    return {total_iterations_global - remaining_iterations_global + 1, total_iterations_global + 1};
  }

  std::vector<std::unique_ptr<Detector>> tracking_detectors_; ///< map of tracking detectors
  std::vector<std::unique_ptr<Detector>> channel_detectors_; ///< map of channel detectors
  std::vector<Detector *> all_detectors_; ///<
};
}