//
// Created by Lukas Kreis on 29.06.17.
//

#include <iostream>
#include "QnTask.h"
#include "DataContainer.h"

QnTask::QnTask(std::array<std::unique_ptr<TFile>, 4> files) :
    in_file_(std::move(files[0])),
    out_file_(std::move(files[1])),
    in_calibration_file_(std::move(files[2])),
    out_calibration_file_(std::move(files[2])),
    in_tree_(std::move(static_cast<TTree*>(in_file_->Get("DstTree")))),
    out_tree_(new TTree("qn_tree","qn_tree")),
    tree_reader_(in_tree_.get()),
    event_(tree_reader_, "Event"),
    qn_data_(new Qn::DataContainerQn("data")),
    qn_eventinfo_(new Qn::EventInfoF()),
    qn_manager_()
{
}

void QnTask::Run() {
  Initialize();
  std::cout << "Processing" << std::endl;
  std::cout << "----------" << std::endl;
  while (tree_reader_.Next()) {
    Process();
  }
  Finalize();
}

void QnTask::Initialize() {
  std::cout << "Initializing" << std::endl;
  std::cout << "------------" << std::endl;
//  in_tree_->SetImplicitMT(true);
  qn_manager_.SetShouldFillQAHistograms();
  qn_manager_.SetShouldFillOutputHistograms();
  qn_manager_.SetCurrentProcessListName("process");
  qn_manager_.InitializeQnCorrectionsFramework();
  out_tree_->Branch("TPC",qn_data_.get());
  out_tree_->Branch("EventInfoF",qn_eventinfo_.get());
  qn_data_->AddAxis("name",{1.0,2.0,3.0});
}
void QnTask::Process() {
  qn_eventinfo_->SetVariable("ev1",1.2);
  qn_manager_.ClearEvent();
  auto event =event_.Get();
  std::cout << event->NTracks() << std::endl;
  qn_manager_.ProcessEvent();
  std::cout << qn_data_->GetAxis("name").Name() << std::endl;
  out_tree_->Fill();
  qn_data_->ClearData();
}
void QnTask::Finalize() {
  std::cout<< "Finalizing" << std::endl;
  std::cout << "---------" << std::endl;
  qn_manager_.FinalizeQnCorrectionsFramework();
  out_file_->cd();
  out_tree_->Write();
}
