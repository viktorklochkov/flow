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
    out_tree_(nullptr),
    out_tree_raw(nullptr),
    manager(),
    write_tree_(true) {
  out_file_->cd();
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
  while (i < 100) {
    Process();
    ++i;
  }
  std::cout << i << std::endl;
  Finalize();
}

void TestTask::Initialize() {
  using Axes = std::vector<Qn::Axis>;
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

  manager.AddVariable("Centrality", VarManager::Values::kCentrality);

  manager.AddDetector("DET1", DetectorType::Channel,4);
  manager.SetCorrectionSteps("DET1", configure);

  manager.AddCorrectionAxis({"Centrality", 5, 0, 100});

  manager.SetEventVariable("Centrality");

  manager.SaveQVectorsToTree(*out_tree_);
  manager.SaveEventVariablesToTree(*out_tree_);

  manager.Initialize(in_calibration_file_);
}

void TestTask::Process() {
  manager.Reset();
  Differential::Interface::DataFiller filler;

  manager.FillDataToFramework(filler);

  VarManager::FillEventInfo(manager.GetVariableContainer());
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

}
