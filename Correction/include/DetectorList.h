#include "Detector.h"
namespace Qn {
class DetectorList {
 public:
  void AddDetector(std::string name,
                   Qn::DetectorType type,
                   InputVariable phi,
                   InputVariable weight,
                   const std::vector<Qn::AxisD> &axes,
                   std::bitset<Qn::QVector::kmaxharmonics> harmonics,
                   QVector::Normalization norm) {
    auto find = [&name](const Detector &a) { return name==a.GetName(); };
    if (type==Qn::DetectorType::CHANNEL) {
      if (std::find_if(channel_detectors_.begin(), channel_detectors_.end(), find)==channel_detectors_.end()) {
        channel_detectors_.emplace_back(name, type, axes, phi, weight, harmonics, norm);

      }
    } else if (type==Qn::DetectorType::TRACK) {
      if (std::find_if(tracking_detectors_.begin(), tracking_detectors_.end(), find)==tracking_detectors_.end()) {
        tracking_detectors_.emplace_back(name, type, axes, phi, weight, harmonics, norm);
      }
    }
  }

  void AddCutCallBack(const std::string &name,
                      std::function<std::unique_ptr<CutBase>(Qn::InputVariableManager *man)> cut) {
    auto &det = FindDetector(name);
    det.AddCutCallBack(std::move(cut));
  }

  Detector &FindDetector(const std::string name) {
    auto find = [&name](const Detector &a) { return a.GetName()==name; };
    auto det_ch = std::find_if(channel_detectors_.begin(), channel_detectors_.end(), find);
    auto det_trk = std::find_if(tracking_detectors_.begin(), tracking_detectors_.end(), find);
    if (det_ch!=channel_detectors_.end()) {
      return *det_ch;
    } else if (det_trk!=tracking_detectors_.end()) {
      return *det_trk;
    } else {
      throw std::runtime_error("Detector" + name + "not found.");
    }
  }

  void SetOutputTree(TTree *output_tree) {
    if (output_tree) {
      for (auto &detector : tracking_detectors_) {
        detector.AttachToTree(output_tree);
      }
      for (auto &detector : channel_detectors_) {
        detector.AttachToTree(output_tree);
      }
    }
  }

  void InitializeOnNode(CorrectionManager *manager) {
    for (auto &detector : channel_detectors_) {
      all_detectors_.push_back(&detector);
      detector.InitializeOnNode(manager);
    }
    for (auto &detector : tracking_detectors_) {
      all_detectors_.push_back(&detector);
      detector.InitializeOnNode(manager);
    }
  }

  void FillTracking() {
    for (auto &dp : tracking_detectors_) {
      dp.FillData();
    }
  }

  void FillChannel() {
    for (auto &dp : channel_detectors_) {
      dp.FillData();
    }
  }

  void ProcessCorrections() {
    for (auto &d : all_detectors_) {
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
        remaining_iterations_global = total_iterations - performed_iterations;
      }
      if (total_iterations_global < total_iterations) total_iterations_global = total_iterations;
    }
    return {total_iterations_global - remaining_iterations_global + 1, total_iterations_global + 1};
  }

  std::vector<Detector> tracking_detectors_; ///< vector of tracking detectors
  std::vector<Detector> channel_detectors_; ///< vector of channel detectors
  std::vector<Detector *> all_detectors_; ///< storing pointers to all detectors
};
}