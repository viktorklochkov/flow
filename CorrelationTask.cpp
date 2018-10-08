//
// Created by Lukas Kreis on 13.11.17.
//
#include <utility>
#include <TCanvas.h>
#include <TFile.h>
#include <TLegend.h>
#include <ReducedEvent/AliReducedVarManager.h>
#include <TTreeReaderValue.h>

#include "CorrelationTask.h"
#include "Base/DataContainerHelper.h"

CorrelationTask::CorrelationTask(std::string filelist, std::string treename) :
    in_tree_(this->MakeChain(std::move(filelist), treename)),
    reader_(new TTreeReader(in_tree_.get())) {}

void CorrelationTask::Configure(Qn::CorrelationManager &a) {
  // Correlation functions
  auto scalar = [](const std::vector<Qn::QVector> &a) {
    return a[0].x(2)*a[1].x(2) + a[0].y(2)*a[1].y(2);
  };
  auto spNoNormal = [](const std::vector<Qn::QVector> &a) {
    auto Q0 = a[0].DeNormal();
    auto Q1 = a[1].DeNormal();
    return Q0.x(2)*Q1.x(2) + Q0.y(2)*Q1.y(2);
  };
  auto spNormal = [](const std::vector<Qn::QVector> &a) {
    auto Q0 = a[0];
    auto Q1 = a[1].DeNormal();
    return Q0.x(2)*Q1.x(2) + Q0.y(2)*Q1.y(2);
  };

  auto num = [](const std::vector<Qn::QVector> &a) {
    auto u = a[0];
    auto Q = a[1].DeNormal();
    return u.x(2)*Q.x(2) + u.y(2)*Q.y(2);
  };

//  auto den = [](const std::vector<Qn::QVector> &a) {
//    auto Q = a[0].Normal(Qn::QVector::Normalization::QOVERNORMQ);
//    auto Q = a[0].DeNormal();
//    return u.x(2)*Q.x(2) + u.y(2)*Q.y(2);
//  };

  // Set file names
  a.SetESECalibrationFile("ese.root");
  a.SetOutputFile("corr.root");

  // ESE Configuration
  a.AddESE("ZDCA", 1, 200);

  // Add Detectors for Correlation
  a.AddQVectors("TPC, TPC_R, V0A, V0C, T0A, T0C, ZDCA, ZDCC, FMDA, FMDC");
//  a.AddQVectors("TPC_R, V0A");

  // Add Eventvariables for correlation
  // Use Qn::Axis
  a.AddEventVariable({"CentralityV0M", {0., 5., 10., 15.,20.,25.,30.,35.,40.,45.,50.}});
//  a.AddEventVariable({"VTXZ", 10,-10,10});

//  a.AddProjection("TPC_R","TPC_RR","");
//  a.AddProjection("TPC","TPC_PT","TPCPt");
  // Add Correlation
//    a.AddCorrelation("TPC_R_TPC_R","TPC_R,TPC_R",spNoNormal,0,Qn::Sampler::Method::NONE);
//    a.AddCorrelation("TPC_TPC","TPC,TPC_R",spNormal,0,Qn::Sampler::Method::NONE);

//    a.AddCorrelation("TPCPT_V0A", "TPC, V0A", scalar, 0, Qn::Sampler::Method::NONE);
//    a.AddCorrelation("TPCPT_V0C", "TPC, V0C", scalar, 0, Qn::Sampler::Method::NONE);
////
//    a.AddCorrelation("TPC_V0A", "TPC_R, V0A", scalar, 0, Qn::Sampler::Method::NONE);
//    a.AddCorrelation("TPC_V0C", "TPC_R, V0C", scalar, 0, Qn::Sampler::Method::NONE);
//    a.AddCorrelation("V0A_V0C", "V0A, V0C", scalar, 0, Qn::Sampler::Method::NONE);
//
//    a.AddCorrelation("TPC_T0A", "TPC_R, T0A", scalar, 10, Qn::Sampler::Method::BOOTSTRAP);
//    a.AddCorrelation("TPC_T0C", "TPC_R, T0C", scalar, 10, Qn::Sampler::Method::BOOTSTRAP);
//    a.AddCorrelation("T0A_T0C", "T0A, T0C", scalar, 10, Qn::Sampler::Method::BOOTSTRAP);
//
//    a.AddCorrelation("ZDCAC_XX","ZDCA, ZDCC",[](const std::vector<Qn::QVector> &a){return -a[0].x(1)*a[1].x(1);});
//    a.AddCorrelation("ZDCAC_YY","ZDCA, ZDCC",[](const std::vector<Qn::QVector> &a){return a[0].y(1)*a[1].y(1);});
//    a.AddCorrelation("ZDCAC_YX","ZDCA, ZDCC",[](const std::vector<Qn::QVector> &a){return a[0].y(1)*a[1].x(1);});
//    a.AddCorrelation("ZDCAC_XY","ZDCA, ZDCC",[](const std::vector<Qn::QVector> &a){return a[0].x(1)*a[1].y(1);});
}

void CorrelationTask::Run() {
  int nevents = in_tree_->GetEntries();
  Qn::CorrelationManager a(reader_, nevents);
  in_tree_->LoadTree(0); // prevents weird TTree errors
  Configure(a);
  int events = 1;
  reader_->SetEntry(0); //  first entry needed to access configuration of DataContainers in the input file
  a.Initialize();
  in_tree_->LoadTree(0); // prevents weird TTree errors
  std::cout << "Processing..." <<std::endl;
  int eventsteps = nevents / 100;
  while (reader_->Next()) {
    events++;
    a.Process();
    if ( events % eventsteps == 0) {
      float progress = events/(float) nevents;
      ProgressBar(progress);
    }
  }
  a.Finalize();
  std::cout << "number of analyzed events: " << events << std::endl;
}

std::unique_ptr<TChain> CorrelationTask::MakeChain(std::string filename, std::string treename) {
  std::unique_ptr<TChain> chain(new TChain(treename.data()));
  std::ifstream in;
  in.open(filename);
  std::string line;
  std::cout << "files in TChain:" << "\n";
  while ((in >> line).good()) {
    if (!line.empty()) {
      chain->AddFile(line.data());
      std::cout << line << std::endl;
    }
  }
  return chain;
}

void CorrelationTask::ProgressBar(float progress) {
    int barWidth = 70;
    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
      if (i < pos) std::cout << "=";
      else if (i == pos) std::cout << ">";
      else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
 }