//
// Created by Lukas Kreis on 29.06.17.
//

#include <iostream>
#include <QnCorrections/QnCorrectionsLog.h>
#include <random>
#include "Task.h"
#include "DataInterface.h"

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
    qn_eventinfo_f_(new Qn::EventInfoF()),
    qn_manager_(),
    rnd(0, 2 * M_PI),
    eng(42) {
  out_file_->cd();
  std::unique_ptr<TTree> tree(new TTree("tree", "tree"));
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

//  std::vector<VAR::Variables> vars = {VAR::Variables::kPt, VAR::Variables::kEta,VAR::Variables::kP, VAR::Variables::kPhi};
  Qn::Interface::SetVariables({VAR::Variables::kPt, VAR::Variables::kEta,VAR::Variables::kP, VAR::Variables::kPhi});
//  std::cout << AliReducedVarManager::GetUsedVar(AliReducedVarManager::Variables::kPhi) << std::endl;
  std::unique_ptr<Qn::DataContainerDataVector> data(new DataContainerDataVector());
  Axis ptaxis("Pt", 1, 0, 3, VAR::kPt);
  Axis etaaxis("Eta", 1, -0.8, 0.8, VAR::kEta);
  data->AddAxis(ptaxis);
  data->AddAxis(etaaxis);

  std::unique_ptr<Qn::DataContainerDataVector> data1(new DataContainerDataVector());
  data1->AddAxis(ptaxis);
  data1->AddAxis(etaaxis);

  raw_data_.insert(std::pair<int, std::unique_ptr<Qn::DataContainerDataVector>>(0, std::move(data)));
  raw_data_.insert(std::pair<int, std::unique_ptr<Qn::DataContainerDataVector>>(1, std::move(data1)));

  std::unique_ptr<Qn::DataContainerQn> qndata(new DataContainerQn);
  qndata->AddAxis(ptaxis);
  qndata->AddAxis(etaaxis);
  std::unique_ptr<Qn::DataContainerQn> tpc(new DataContainerQn);

  qn_data_.insert(std::pair<int, std::unique_ptr<Qn::DataContainerQn>>(0, std::move(qndata)));
  qn_data_.insert(std::pair<int, std::unique_ptr<Qn::DataContainerQn>>(1, std::move(tpc)));

  auto eventset = new QnCorrectionsEventClassVariablesSet(1);
  double centbins[][2] = {{0.0, 2}, {100.0, 100}};
  eventset->Add(new QnCorrectionsEventClassVariable(1, "Centrality", centbins));

  Qn::Internal::AddDetectorToFramework(qn_manager_, Qn::DetectorType::Track, raw_data_, *eventset);
  Qn::Internal::SaveToTree(*out_tree_, qn_data_);
//  Qn::Internal::SaveToTree(*out_tree_, raw_data_);
  qn_eventinfo_f_->AddVariable("Centrality");
  qn_eventinfo_f_->AddVariable("VtxZ");
  Qn::Internal::SaveToTree(*out_tree_, qn_eventinfo_f_);
  in_tree_->SetImplicitMT(true);
  qn_manager_.SetCalibrationHistogramsList(in_calibration_file_.get());
  qn_manager_.SetShouldFillQAHistograms();
  qn_manager_.SetShouldFillOutputHistograms();
  qn_manager_.InitializeQnCorrectionsFramework();
  qn_manager_.SetCurrentProcessListName("test");
}

void Task::Process() {

  qn_manager_.ClearEvent();
  qn_data_[0]->ClearData();
  raw_data_[0]->ClearData();
  qn_eventinfo_f_->Reset();

  if (event_->IsA() != AliReducedEventInfo::Class()) return;
  if (event_->NTracks() == 0) return;
  qn_eventinfo_f_->SetVariable("Centrality", event_->CentralityVZERO());
  qn_eventinfo_f_->SetVariable("VtxZ", event_->Vertex(2));
  if (event_->Vertex(2) < -10 || event_->Vertex(2) > 10) return;
  if (event_->CentralityVZERO() <= 0 || event_->CentralityVZERO() >= 100) return;
  Qn::Interface::FillTpc(raw_data_[0], *event_);

  Qn::Internal::FillDataToFramework(qn_manager_, raw_data_);
  qn_manager_.ProcessEvent();
  Qn::Internal::GetQnFromFramework(qn_manager_, qn_data_);
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