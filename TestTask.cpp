//
// Created by Lukas Kreis on 29.06.17.
//

#include <iostream>
#include <QnCorrections/QnCorrectionsLog.h>
#include <random>
#include <THnSparse.h>
#include "TestTask.h"

namespace Qn {

TestTask::TestTask(std::string filelist, std::string incalib, std::string treename) :
    out_file_(new TFile("output.root", "RECREATE")),
    in_calibration_file_(new TFile(incalib.data(), "READ")),
    out_calibration_file_(new TFile("qn.root", "RECREATE")),
    in_tree_(this->MakeChain(filelist, treename)),
    out_tree_(nullptr),
    out_tree_raw(nullptr),
    histograms_(nullptr),
    tree_reader_(in_tree_.get()),
    event_(tree_reader_, "Event"),
    manager(),
    write_tree_(true) {
  out_file_->cd();
  histograms_.reset(new TList());
  std::unique_ptr<TTree> treeraw(new TTree("datatree", "datatree"));
  std::unique_ptr<TTree> tree(new TTree("tree", "tree"));
  out_tree_ = std::move(tree);
  out_tree_raw = std::move(treeraw);
}

void TestTask::Run() {
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

void TestTask::Initialize() {
  using Axes = std::vector<Qn::Axis>;
  SetVariables({VAR::Variables::kVtxZ, VAR::Variables::kPt, VAR::Variables::kEta, VAR::Variables::kP,
                               VAR::Variables::kPhi, VAR::Variables::kTPCncls, VAR::Variables::kDcaXY,
                               VAR::Variables::kDcaZ,
                               VAR::Variables::kTPCsignal, VAR::Variables::kTPCchi2, VAR::Variables::kCharge});

//  Axis ptaxis("Pt", {0.2, 0.4, 0.6, 1.0, 2.0, 5.0,10.0}, VAR::kPt);
//  Axis etaaxis("Eta", 6, -0.8, 0.8, VAR::kEta);
//  Axis ptaxis("Pt", {0.2,0.4,0.6,0.8,1.,1.25,1.5,1.75,2.0,2.5,3,3.5,4.,5.}, VAR::kPt);
  Axis ptaxis("Pt", {0.2, 5, 10.});
  Axis etaaxis("Eta", 2, -0.8, 0.8);
  Axes tpcaxes = {ptaxis, etaaxis};

  auto configure = [](QnCorrectionsDetectorConfigurationBase *config) {
    config->SetQVectorNormalizationMethod(QnCorrectionsQnVector::QVNORM_QoverM);
    config->AddCorrectionOnQnVector(new QnCorrectionsQnVectorRecentering());
    auto rescale = new QnCorrectionsQnVectorTwistAndRescale();
    rescale->SetApplyTwist(true);
    rescale->SetApplyRescale(false);
    rescale->SetTwistAndRescaleMethod(QnCorrectionsQnVectorTwistAndRescale::TWRESCALE_doubleHarmonic);
    config->AddCorrectionOnQnVector(rescale);
  };

  manager.AddVariable("CentralityVZERO", VAR::Variables::kCentVZERO);
  manager.AddVariable("Pt", VAR::Variables::kPt);
  manager.AddVariable("Eta", VAR::Variables::kEta);
  manager.AddVariable("VtxZ", VAR::Variables::kVtxZ);

  manager.AddDetector("TPC", DetectorType::Track, tpcaxes);
  manager.SetCorrectionSteps("TPC", configure);

  manager.AddCorrectionAxis({"CentralityVZERO", 100, 0, 100});
  manager.AddCorrectionAxis({"VtxZ",
                             {-10, -9.25, -8.5, -7.75, -7.0, -6., -5., -4., -3., -2., -1., 0., 1., 2., 3., 4., 5., 6.,
                              7., 7.75, 8.5, 9.25, 10.}});

  manager.SetEventVariable("CentralityVZERO");

  manager.SaveQVectorsToTree(*out_tree_);
  manager.SaveEventVariablesToTree(*out_tree_);

  manager.Initialize(in_calibration_file_);
}

void TestTask::Process() {
  manager.Reset();
  auto event = event_.Get();
  Qn::Differential::Interface::DataFiller filler(event);
  if (event->IsA()!=AliReducedEventInfo::Class()) return;
  if (event->NTracks()==0) return;
  if (event_->Vertex(2) < -10 || event->Vertex(2) > 10) return;
  if (event_->CentralityVZERO() <= 0 || event->CentralityVZERO() >= 100) return;

  manager.FillDataToFramework(filler);

  VAR::FillEventInfo(event, manager.GetVariableContainer());
  manager.Process();
  out_tree_->Fill();
  out_tree_raw->Fill();
}

void TestTask::Finalize() {
  manager.Finalize();
  out_calibration_file_->cd();
  manager.SaveCorrectionHistograms();
  out_file_->cd();
  if (write_tree_) {
    out_file_->Write();
    std::cout << "Output file written." << std::endl;
  }
}

std::unique_ptr<TChain> TestTask::MakeChain(std::string filename, std::string treename) {
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
