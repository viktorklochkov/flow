//
// Created by Lukas Kreis on 29.06.17.
//

#include <iostream>
#include <QnCorrections/QnCorrectionsLog.h>
#include <random>
#include <THnSparse.h>
#include "Task.h"
#include "CorrectionInterface.h"

namespace Qn {

Task::Task(std::string filelist, std::string incalib, std::string treename) :
    out_file_(new TFile("output.root", "RECREATE")),
    in_calibration_file_(new TFile(incalib.data(), "READ")),
    out_calibration_file_(new TFile("qn.root", "RECREATE")),
    in_tree_(this->MakeChain(filelist, treename)),
    out_tree_(nullptr),
    out_tree_raw(nullptr),
    histograms_(nullptr),
    tree_reader_(in_tree_.get()),
    event_(tree_reader_, "Event"),
    qn_eventinfo_f_(new Qn::EventInfoF()),
    qn_manager_(),
    eng(42),
    rnd(0, 2 * M_PI),
    write_tree_(true) {
  out_file_->cd();
  histograms_.reset(new TList());
  std::unique_ptr<TTree> treeraw(new TTree("datatree", "datatree"));
  std::unique_ptr<TTree> tree(new TTree("tree", "tree"));
  out_tree_ = std::move(tree);
  out_tree_raw = std::move(treeraw);
}

void Task::Run() {
  Initialize();
  QnCorrectionsSetTracingLevel(kError);
  std::cout << "Processing..." << std::endl;
  int i = 0;
  while (tree_reader_.Next()) {
    Process();
    ++i;
  }
  std::cout << i << std::endl;
  Finalize();
}

void Task::Initialize() {
  using Axes = std::vector<Qn::Axis>;
  Qn::Interface::SetVariables({VAR::Variables::kVtxZ, VAR::Variables::kPt, VAR::Variables::kEta, VAR::Variables::kP,
                               VAR::Variables::kPhi, VAR::Variables::kTPCncls, VAR::Variables::kDcaXY, VAR::Variables::kDcaZ,
                              VAR::Variables::kTPCsignal, VAR::Variables::kTPCchi2, VAR::Variables::kCharge});

  Axis ptaxis("Pt", {0.2, 0.4, 0.6, 1.0, 2.0, 5.0}, VAR::kPt);
  Axis etaaxis("Eta", 6, -0.8, 0.8, VAR::kEta);
  Axis vzerorings("EtaRings", {-3.7, -3.2, -2.7, -2.2, -1.7, 2.8, 3.4, 3.9, 4.5, 5.1}, VAR::kEta);
  Axis vzeroringsA("EtaRings", {2.8, 3.4, 3.9, 4.5, 5.1}, VAR::kEta);
  Axis vzeroringsC("EtaRings", {-3.7, -3.2, -2.7, -2.2, -1.7}, VAR::kEta);

  Axes tpcaxes = {ptaxis, etaaxis};
  Axes vzeroaxes = {vzerorings};
  Axes vzeroaxesA = {vzeroringsA};
  Axes vzeroaxesC = {vzeroringsC};

  Qn::Internal::AddDetectorToMap(raw_data_,
                                 Configuration::DetectorId::TPC_reference,
                                 new Configuration::TPC(),
                                 Configuration::DetectorType::Track
  );
  Qn::Internal::AddDetectorToMap(raw_data_,
                                 Configuration::DetectorId::TPC,
                                 new Configuration::TPC(),
                                 Configuration::DetectorType::Track,
                                 tpcaxes
  );
  Qn::Internal::AddDetectorToMap(raw_data_,
                                 Configuration::DetectorId::VZEROA,
                                 new Configuration::VZEROA(),
                                 Configuration::DetectorType::Channel,
                                 vzeroaxesA,
                                 64);
  Qn::Internal::AddDetectorToMap(raw_data_,
                                 Configuration::DetectorId::VZEROC,
                                 new Configuration::VZEROC(),
                                 Configuration::DetectorType::Channel,
                                 vzeroaxesC,
                                 64);
  Qn::Internal::AddDetectorToMap(raw_data_,
                                 Configuration::DetectorId::VZEROA_reference,
                                 new Configuration::VZEROA_reference(),
                                 Configuration::DetectorType::Channel,
                                 64);
  Qn::Internal::AddDetectorToMap(raw_data_,
                                 Configuration::DetectorId::VZEROC_reference,
                                 new Configuration::VZEROC_reference(),
                                 Configuration::DetectorType::Channel,
                                 64);
  Qn::Internal::AddDetectorToMap(raw_data_,
                                 Configuration::DetectorId::FMDC_reference,
                                 new Configuration::FMDC_reference(),
                                 Configuration::DetectorType::Channel,
                                 2000);
  Qn::Internal::AddDetectorToMap(raw_data_,
                                 Configuration::DetectorId::FMDA_reference,
                                 new Configuration::FMDA_reference(),
                                 Configuration::DetectorType::Channel,
                                 2000);
  Qn::Internal::AddDetectorToMap(raw_data_,
                                 Configuration::DetectorId::ZDCA_reference,
                                 new Configuration::ZDCA_reference(),
                                 Configuration::DetectorType::Channel,
                                 5);
  Qn::Internal::AddDetectorToMap(raw_data_,
                                 Configuration::DetectorId::ZDCC_reference,
                                 new Configuration::ZDCC_reference(),
                                 Configuration::DetectorType::Channel,
                                 5);

  qn_data_ = Qn::Internal::MakeQnDataContainer(raw_data_);

  auto eventset = new QnCorrectionsEventClassVariablesSet(2);
  double centbins[][2] = {{0.0, 2}, {100.0, 20}};
  double vtxbins[][2] = {{-10.0, 4}, {-7.0, 1}, {7.0, 8}, {10.0, 1}};
  eventset->Add(new QnCorrectionsEventClassVariable(VAR::kCentVZERO, "Centrality", centbins));
  eventset->Add(new QnCorrectionsEventClassVariable(VAR::kVtxZ, "z vertex", vtxbins));

  const int ndim = 8;
  int bins[ndim] = {40, 40, 40, 40, 40, 50, 3, 20};
  double minbin[ndim] = {0, -0.9, 0, 0, -4, 0, -1, 0};
  double maxbin[ndim] = {8, 0.9, 2 * TMath::Pi(), 2.5, 4, 1500, 2, 5};
  auto *h_track_params =
      new THnSparseF("trackqa", "tracks;pT;eta;phi;dcaxy;dcaz;dEdx;charge;chi2/ndf", ndim, bins, minbin, maxbin);

  histograms_->Add(h_track_params);

  Qn::Internal::AddDetectorsToFramework(qn_manager_, raw_data_, *eventset);
  Qn::Internal::SaveToTree(*out_tree_, qn_data_);
  qn_eventinfo_f_->AddVariable("CentralityVZERO");
  qn_eventinfo_f_->AddVariable("CentralityZEMvsZDC");
  qn_eventinfo_f_->AddVariable("CentralityTPC");
  qn_eventinfo_f_->AddVariable("CentralitySPD");
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
  Qn::Internal::ClearDataInMap(qn_data_);
  Qn::Internal::ClearDataInMap(raw_data_);
  qn_eventinfo_f_->Reset();

  if (event_->IsA() != AliReducedEventInfo::Class()) return;
  if (event_->NTracks() == 0) return;
  qn_eventinfo_f_->SetVariable("CentralityVZERO", event_->CentralityVZERO());
  qn_eventinfo_f_->SetVariable("CentralityZEMvsZDC", event_->CentralityZEMvsZDC());
  qn_eventinfo_f_->SetVariable("CentralityTPC", event_->CentralityTPC());
  qn_eventinfo_f_->SetVariable("CentralitySPD", event_->CentralitySPD());
  qn_eventinfo_f_->SetVariable("VtxZ", event_->Vertex(2));
  if (event_->Vertex(2) < -10 || event_->Vertex(2) > 10) return;
  if (event_->CentralityVZERO() <= 0 || event_->CentralityVZERO() >= 100) return;
  Qn::Interface::FillDetectors(raw_data_, *event_, *histograms_);
  VAR::FillEventInfo(&*event_, qn_manager_.GetDataContainer());
  Qn::Internal::FillDataToFramework(qn_manager_, raw_data_);
  qn_manager_.ProcessEvent();
  Qn::Internal::GetQnFromFramework(qn_manager_, qn_data_);
  out_tree_->Fill();
  out_tree_raw->Fill();
}

void Task::Finalize() {
  qn_manager_.FinalizeQnCorrectionsFramework();
  out_calibration_file_->cd();
  qn_manager_.GetOutputHistogramsList()->Write(qn_manager_.GetOutputHistogramsList()->GetName(), TObject::kSingleKey);
  qn_manager_.GetQAHistogramsList()->Write(qn_manager_.GetQAHistogramsList()->GetName(), TObject::kSingleKey);
  out_file_->cd();
  histograms_->Write("histograms",TObject::kSingleKey);
  if (write_tree_) {
    out_file_->Write();
    std::cout << "Output file written." << std::endl;
  }
}

std::unique_ptr<TChain> Task::MakeChain(std::string filename, std::string treename) {
  std::unique_ptr<TChain> chain(new TChain(treename.data()));
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