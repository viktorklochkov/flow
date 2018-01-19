//
// Created by Lukas Kreis on 13.11.17.
//
#include <utility>
#include <TCanvas.h>
#include <TFile.h>
#include <TLegend.h>

#include "SimpleTask.h"
#include "Base/DataContainerHelper.h"

SimpleTask::SimpleTask(std::string filelist, std::string treename) :
    in_tree_(this->MakeChain(std::move(filelist), treename)),
    reader_(new TTreeReader(in_tree_.get())) {}

void SimpleTask::Configure(Qn::CorrelationManager &a) {

  auto XX = [](const std::vector<Qn::QVector> &a) {
    return 1.0 + a[0].x(1)*a[1].x(1);
  };


  a.AddDataContainer("DET1");
  a.AddEventVariable({"Centrality", {0., 5., 10., 20., 30., 40., 50., 60., 70.}});
//  a.AddFunction("TPC", Rebin);
  a.AddCorrelation("TEST","DET1, DET1", XX);
}

void SimpleTask::Run() {
  Qn::CorrelationManager a(reader_);
  Configure(a);
  int events = 1;
  reader_->SetEntry(0);
  a.Initialize();
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
//  auto multiply = [](Qn::Profile a, Qn::Profile b) {
//    return a*b;
//  };
//
//  auto divide = [](Qn::Profile a, Qn::Profile b) {
//    return a/b;
//  };
//
//  auto sqrt = [](Qn::Profile a) {
//    return a.Sqrt();
//  };
//
//  auto add = [](Qn::Profile a, Qn::Profile b) {
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

