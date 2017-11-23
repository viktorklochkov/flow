//
// Created by Lukas Kreis on 13.11.17.
//

#include <TCanvas.h>
#include <TFile.h>
#include "SimpleTask.h"
#include "Stats.h"
#include "Resolution.h"
#include "DataContainerHelper.h"
SimpleTask::SimpleTask(std::string filelist, std::string treename) :
    in_tree_(this->MakeChain(filelist, treename)),
    reader_(in_tree_.get()) {}

void SimpleTask::Initialize() {
  std::vector<Qn::Axis> noaxes;
  eventaxes_.emplace_back("CentralityVZERO", 5, 0, 100, 1);

  auto tpcr = *values_.at("TPC_reference");
  auto vc = *values_.at("VZEROC_reference");
  auto va = *values_.at("VZEROA_reference");
  Qn::Correlation TpcVc({tpcr, vc}, eventaxes_);
  Qn::Correlation TpcVa({tpcr, va}, eventaxes_);
  Qn::Correlation VcVa({vc, va}, eventaxes_);
  correlations_.insert({"tpcvc", TpcVc});
  correlations_.insert({"tpcva", TpcVa});
  correlations_.insert({"vcva", VcVa});

}

void SimpleTask::Run() {
  AddEventVariable("CentralityVZERO");
  AddDataContainer("TPC_reference");
  AddDataContainer("VZEROC_reference");
  AddDataContainer("VZEROA_reference");

  reader_.SetEntry(0);
  Initialize();
  Process();
  while (reader_.Next()) {
    Process();
  }

  auto tpcvc = correlations_.at("tpcvc").GetCorrelation();
  auto tpcva = correlations_.at("tpcva").GetCorrelation();
  auto vcva = correlations_.at("vcva").GetCorrelation();


  auto multiply = [] (Qn::Statistics a, Qn::Statistics b) {
    return a * b;
  };

  auto divide = [] (Qn::Statistics a, Qn::Statistics b) {
    return a / b;
  };

  auto sqrt = [] (Qn::Statistics a) {
    return a.Sqrt();
  };

  auto resolution = tpcvc.Apply(tpcva,multiply).Apply(vcva,divide).Map(sqrt);

  auto gresolution = Qn::DataToProfileGraph(resolution);
//  auto gtpcva = Qn::DataToProfileGraph(tpcva);
//  gtpcva.SetLineColor(kBlue);
//  auto gvcva = Qn::DataToProfileGraph(vcva);
//  gvcva.SetLineColor(kRed);

//
  auto *c1 = new TCanvas("c1", "c1", 800, 600);
  c1->cd(1);
  gresolution.Draw("AP");
//  gtpcvc.Draw("AP");
//  gtpcva.Draw("P");
//  gvcva.Draw("AP");
  c1->SaveAs("test.root");

}

void SimpleTask::Process() {
  std::vector<Qn::Axis> noaxes;

  auto tpcr = *values_.at("TPC_reference");
  auto vc = *values_.at("VZEROC_reference");
  auto va = *values_.at("VZEROA_reference");

  std::vector<float> eventparameters;
  eventparameters.push_back(*eventvalues_.at("CentralityVZERO"));
  auto eventbin = Qn::CalculateEventBin(eventaxes_, eventparameters);

  auto psi_n = [](Qn::QVector a, int n) {
    return 1 / (float) n * atan2(a.y(n), a.x(n));
  };

  auto cos_n = [](int n, float a, float b) {
    return cos((float) n * (a - b));
  };

  auto correlate = [psi_n, cos_n](std::vector<Qn::QVector> a) {
    return cos_n(2, psi_n(a.at(0), 2), psi_n(a.at(1), 2));
  };

  correlations_.at("tpcvc").Fill({tpcr, vc}, eventbin, correlate);
  correlations_.at("tpcva").Fill({tpcr, va}, eventbin, correlate);
  correlations_.at("vcva").Fill({va, vc}, eventbin, correlate);

}

std::unique_ptr<TChain> SimpleTask::MakeChain(std::string filename, std::string treename) {
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
void SimpleTask::AddDataContainer(std::string name) {
  TTreeReaderValue<Qn::DataContainerQVector>
      value(reader_, name.data());
  auto pair = std::make_pair(name, value);
  values_.insert(std::make_pair(name, value));
}
void SimpleTask::AddEventVariable(std::string name) {
  TTreeReaderValue<float> value(reader_, name.data());
  eventvalues_.insert(std::make_pair(name, value));
}
