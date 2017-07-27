//
// Created by Lukas Kreis on 29.06.17.
//

#include <iostream>
#include "Task.h"

namespace Qn {

Task::Task(std::string filelist, std::string incalib) :
    out_calibration_file_(new TFile("qn.root", "RECREATE")),
    in_calibration_file_(TFile::Open(incalib.data())),
    in_tree_(std::move(this->MakeChain(filelist))),
    tree_reader_(in_tree_.get()),
    event_(tree_reader_, "Event"),
    out_file_(new TFile("output.root", "RECREATE")),
    out_tree_(nullptr),
    qn_data_(new Qn::DataContainerQn()),
    qn_eventinfo_f_(new Qn::EventInfoF()),
    qn_eventinfo_i_(new Qn::EventInfoI()),
    qn_manager_() {
  out_file_->cd();
  std::unique_ptr<TTree> tree(new TTree("tree","tree"));
  out_tree_ = std::move(tree);
}

//Task::Task(std::array<std::shared_ptr<TFile>, 4> files) :
//    in_file_(files[0]),
//    out_file_(files[1]),
//    out_calibration_file_(files[2]),
//    in_calibration_file_(files[3]),
//    in_tree_(std::move(static_cast<TTree *>(in_file_->Get("DstTree")))),
//    out_tree_(new TTree("qn_tree", "qn_tree")),
//    tree_reader_(in_tree_.get()),
//    event_(tree_reader_, "Event"),
//    qn_data_(new Qn::DataContainerQn("data")),
//    qn_eventinfo_f_(new Qn::EventInfoF()),
//    qn_eventinfo_i_(new Qn::EventInfoI()),
//    qn_manager_() {
//}

void Task::Run() {
  Initialize();
  while (tree_reader_.Next()) {
    Process();
  }
  Finalize();
}

void Task::Initialize() {
  qn_data_->AddAxis("Pt", {0.0, 1.0, 3.0});
  qn_data_->AddAxis("Eta", {-0.9, 0.5, 0.0, 0.5, 0.9});
  Qn::SetVariables({VAR::kP, VAR::kPt, VAR::kPhi, VAR::kEta, VAR::kRap, VAR::kCentVZERO,
                                          VAR::kVtxZ});
  Qn::ConfigureBins(qn_manager_, qn_data_, Qn::DetectorId::TPC);
  in_tree_->SetImplicitMT(true);
  if (in_calibration_file_) qn_manager_.SetCalibrationHistogramsList(in_calibration_file_.get());
  qn_manager_.SetShouldFillQAHistograms();
  qn_manager_.SetShouldFillOutputHistograms();
  qn_manager_.SetCurrentProcessListName("process");
  qn_manager_.InitializeQnCorrectionsFramework();

  out_tree_->Branch("TPC", qn_data_.get());
  out_tree_->Branch("EventInfoF", qn_eventinfo_f_.get());
  out_tree_->Branch("EventInfoI", qn_eventinfo_i_.get());
}

void Task::Process() {
  qn_manager_.ClearEvent();
  qn_data_->ClearData();
  qn_eventinfo_f_->Clear();
  if (event_->IsA() != AliReducedEventInfo::Class()) return;
  if (event_->NTracks() == 0) return;
  qn_eventinfo_f_->SetVariable("CentVZERO", event_->CentralityVZERO());
//  std::cout << qn_eventinfo_f_->GetVariable("CentVZERO") << std::endl;
  Qn::FillData(qn_manager_, *event_);
  qn_manager_.ProcessEvent();
  Qn::FillTree(qn_manager_, qn_data_, DetectorId::TPC);
  out_tree_->Fill();
}

void Task::Finalize() {
  qn_manager_.FinalizeQnCorrectionsFramework();
  out_calibration_file_->cd();
  qn_manager_.GetOutputHistogramsList()->Write(qn_manager_.GetOutputHistogramsList()->GetName(), TObject::kSingleKey);
  qn_manager_.GetQAHistogramsList()->Write(qn_manager_.GetQAHistogramsList()->GetName(), TObject::kSingleKey);
  out_file_->cd();
  out_file_->Write();
}

std::unique_ptr<TChain> Task::MakeChain(std::string filename) {
  std::unique_ptr<TChain> chain(new TChain("DstTree"));
  std::ifstream in;
  in.open(filename);
  std::string line;
  //loop over file
  std::cout << "Adding files to chain:" << std::endl;
  while ((in >> line).good()) {
    if (!line.empty()) {
      chain->AddFile(line.data());
      std::cout << line << std::endl;
    }
  }
  return chain;
}
}