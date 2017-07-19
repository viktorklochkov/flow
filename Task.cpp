//
// Created by Lukas Kreis on 29.06.17.
//

#include <iostream>
#include "Task.h"
#include "DataContainer.h"
#include "CorrectionsInterface.h"

namespace Qn {

Task::Task(std::array<std::shared_ptr<TFile>, 4> files) :
    in_file_(files[0]),
    out_file_(files[1]),
    in_calibration_file_(files[2]),
    out_calibration_file_(files[2]),
    in_tree_(std::move(static_cast<TTree*>(in_file_->Get("DstTree")))),
    out_tree_(new TTree("qn_tree","qn_tree")),
    tree_reader_(in_tree_.get()),
    event_(tree_reader_, "Event"),
    qn_data_(new Qn::DataContainerQn("data")),
    qn_eventinfo_f_(new Qn::EventInfoF()),
    qn_eventinfo_i_(new Qn::EventInfoI()),
    qn_manager_()
{
}

void Task::Run() {
  Initialize();
  while (tree_reader_.Next()) {
    Process();
  }
  Finalize();
}

void Task::Initialize() {
  AddTPC(qn_manager_);
//  in_tree_->SetImplicitMT(true);
//  qn_manager_.SetCalibrationHistogramsList(in_calibration_file_.get());
  qn_manager_.SetShouldFillQAHistograms();
  qn_manager_.SetShouldFillOutputHistograms();
  qn_manager_.SetCurrentProcessListName("process");
  qn_manager_.InitializeQnCorrectionsFramework();
  Qn::CorrectionsInterface::SetVariables({VAR::kP, VAR::kPt, VAR::kPhi, VAR::kEta, VAR::kRap,VAR::kCentVZERO, VAR::kVtxZ});
//  out_tree_->Branch("TPC",qn_data_.get());
//  out_tree_->Branch("EventInfoF",qn_eventinfo_f_.get());
//  out_tree_->Branch("EventInfoI",qn_eventinfo_i_.get());
//  qn_data_->AddAxis("name",{1.0,2.0,3.0});
}
void Task::Process() {
//  qn_eventinfo_f_->SetVariable("ev1",1.2);
  qn_manager_.ClearEvent();
  auto event = event_.Get();
  if (event->IsA()!=AliReducedEventInfo::Class()) return;
  if (event->NTracks() == 0) return;
//  std::cout << event->NTracks() << std::endl;
  Qn::CorrectionsInterface::FillData(qn_manager_, *event);
  qn_manager_.ProcessEvent();
//  std::cout << qn_data_->GetAxis("name").Name() << std::endl;
  out_tree_->Fill();
//  qn_data_->ClearData();
}
void Task::Finalize() {
  qn_manager_.FinalizeQnCorrectionsFramework();
  out_file_->cd();
  qn_manager_.GetOutputHistogramsList()->Write(qn_manager_.GetOutputHistogramsList()->GetName(),TObject::kSingleKey);
  out_tree_->Write();
}
}