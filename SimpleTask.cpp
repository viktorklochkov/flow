//
// Created by Lukas Kreis on 13.11.17.
//
#include <utility>
#include <TCanvas.h>
#include <TFile.h>
#include <TLegend.h>
#include <ReducedEvent/AliReducedVarManager.h>

#include "SimpleTask.h"
#include "DataContainerHelper.h"
#include "CorrelationManager.h"

SimpleTask::SimpleTask(std::string filelist, std::string treename) :
    in_tree_(this->MakeChain(std::move(filelist), treename)),
    reader_(new TTreeReader(in_tree_.get())) {}

void SimpleTask::Configure(Qn::CorrelationManager &a) {
  auto scalar = [](std::vector<Qn::QVector> &a) -> double {
    return a[0].x(2)*a[1].x(2) + a[0].y(2)*a[1].y(2);
  };
  auto XY = [](std::vector<Qn::QVector> &a) {
    return a[0].x(1)*a[1].y(1);
  };
  auto YX = [](std::vector<Qn::QVector> &a) {
    return a[0].y(1)*a[1].x(1);
  };
  auto XX = [](std::vector<Qn::QVector> &a) {
    return a[0].x(1)*a[1].x(1);
  };
  auto YY = [](std::vector<Qn::QVector> &a) {
    return a[0].y(1)*a[1].y(1);
  };
  a.AddDataContainer("TPC");
  a.AddDataContainer("TPC_reference");
  a.AddDataContainer("VZEROA_reference");
  a.AddDataContainer("ZDCA_reference");
  a.AddDataContainer("ZDCC_reference");
  a.AddProjection("TPC", "TPCPt", "Pt, Eta");
  a.AddEventVariable({"CentralityVZERO", {0., 5., 10., 20., 30., 40., 50., 60., 70., 80.}, 1});
  a.AddCorrelation("tpcvascalar", "TPC_reference, VZEROA_reference", scalar);
  a.AddCorrelation("tpczdcac", "TPC_reference, TPC_reference, TPC_reference", XX);
  a.AddCorrelation("ZDCAZDCXY", "ZDCA_reference, ZDCC_reference", XY);
  a.AddCorrelation("ZDCAZDCYX", "ZDCA_reference, ZDCC_reference", YX);
  a.AddCorrelation("ZDCAZDCYY", "ZDCA_reference, ZDCC_reference", XX);
  a.AddCorrelation("ZDCAZDCXX", "ZDCA_reference, ZDCC_reference", YY);
}

void SimpleTask::Run() {
  Qn::CorrelationManager a(reader_);
  Configure(a);
  int events = 1;
  reader_->SetEntry(0);
  a.Initialize();
  a.UpdateEvent();
  while (reader_->Next()) {
    events++;
    a.UpdateEvent();
    if (!a.CheckEvent()) continue;
    a.FillCorrelations();
  }
  a.SaveToFile("corr.root");
  std::cout << "number of events: " << events << std::endl;
}

//
//
//  auto multiply = [](Qn::Statistics a, Qn::Statistics b) {
//    return a*b;
//  };
//
//  auto divide = [](Qn::Statistics a, Qn::Statistics b) {
//    return a/b;
//  };
//
//  auto sqrt = [](Qn::Statistics a) {
//    return a.Sqrt();
//  };
//
//  auto add = [](Qn::Statistics a, Qn::Statistics b) {
//    return a + b;
//  };
//
//  auto rvatpcvc = tpcva.Apply(vavc, multiply).Apply(tpcvc, divide).Map(sqrt);
//  auto v2tpcva = tpcva.Apply(rvatpcvc, divide);


std::unique_ptr<TChain> SimpleTask::MakeChain(std::string filename, std::string treename) {
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

