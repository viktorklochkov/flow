//
// Created by Lukas Kreis on 29.06.17.
//

#include <iostream>
#include <QnCorrections/QnCorrectionsLog.h>
#include "Task.h"

namespace Qn {

Task::Task(std::string filelist, std::string incalib) :
    write_tree_(true),
    out_calibration_file_(new TFile("qn.root", "RECREATE")),
    in_calibration_file_(new TFile(incalib.data(), "READ")),
    in_tree_(std::move(this->MakeChain(filelist))),
    tree_reader_(in_tree_.get()),
    event_(tree_reader_, "Event"),
    out_file_(new TFile("output.root", "RECREATE")),
    out_tree_(nullptr),
    qn_data_(new Qn::DataContainerTest()),
    qn_eventinfo_f_(new Qn::EventInfoF()),
    qn_eventinfo_i_(new Qn::EventInfoI()),
    qn_manager_() {
  out_file_->cd();
  std::unique_ptr<TTree> tree(new TTree("tree","tree"));
  out_tree_ = std::move(tree);
}

void Task::Run() {
  Initialize();
  QnCorrectionsSetTracingLevel(kError);
  std::cout << "Processing..." << std::endl;
  while (tree_reader_.Next()) {
    Process();
  }
  Finalize();
}

void Task::Initialize() {
  qn_data_->AddAxis("Pt", {0.0, 3.0});
  qn_data_->AddAxis("Eta", {-0.9, 0.9});
  Qn::SetVariables({VAR::kP, VAR::kPt, VAR::kPhi, VAR::kEta, VAR::kRap, VAR::kCentVZERO,
                                          VAR::kVtxZ});
  Qn::ConfigureBins(qn_manager_, *qn_data_, Qn::DetectorId::TPC, "TPC", {VAR::kPt, VAR::kEta});
  in_tree_->SetImplicitMT(true);
  qn_manager_.SetCalibrationHistogramsList(in_calibration_file_.get());
  qn_manager_.SetShouldFillQAHistograms();
  qn_manager_.SetShouldFillOutputHistograms();
  qn_manager_.InitializeQnCorrectionsFramework();
  qn_manager_.SetCurrentProcessListName("test");
  qn_eventinfo_f_->AddVariable("Centrality");
  qn_eventinfo_f_->AddVariable("VtxZ");
  qn_eventinfo_f_->SetOutputTree(*out_tree_);
//  out_tree_->Branch("test",&centrality,"F");
  out_tree_->Branch("TPC", qn_data_.get());
//  out_tree_->Branch("EventInfoF", qn_eventinfo_f_.get());
//  out_tree_->Branch("EventInfoI", qn_eventinfo_i_.get());
}

void Task::Process() {
  qn_manager_.ClearEvent();
  qn_data_->ClearData();
//  qn_eventinfo_f_->Clear();
//  qn_eventinfo_i_->Clear();
  if (event_->IsA() != AliReducedEventInfo::Class()) return;
  if (event_->NTracks() == 0) return;
  qn_eventinfo_f_->SetVariable("Centrality", event_->CentralityVZERO());
  qn_eventinfo_f_->SetVariable("VtxZ", event_->Vertex(2));
  if (event_->Vertex(2) < -10 || event_->Vertex(2) > 10) return;
  if (event_->CentralityVZERO() <= 0 || event_->CentralityVZERO() >= 100) return;
  Qn::FillData(qn_manager_, *event_);
  qn_manager_.ProcessEvent();
  Qn::FillTree(qn_manager_, *qn_data_, DetectorId::TPC);
  out_tree_->Fill();
}

void Task::Finalize() {
  qn_manager_.FinalizeQnCorrectionsFramework();
  out_calibration_file_->cd();
  qn_manager_.GetOutputHistogramsList()->Write(qn_manager_.GetOutputHistogramsList()->GetName(), TObject::kSingleKey);
  qn_manager_.GetQAHistogramsList()->Write(qn_manager_.GetQAHistogramsList()->GetName(), TObject::kSingleKey);
  out_file_->cd();
  if (write_tree_) {
    out_file_->Write();
    std::cout << "Output file written." << std::endl;
  }
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