//
// Created by Lukas Kreis on 13.11.17.
//

#include <TCanvas.h>
#include <TFile.h>
#include "SimpleTask.h"
#include "statistics.h"
#include "Resolution.h"
SimpleTask::SimpleTask(std::string filelist, std::string treename) :
    in_tree_(this->MakeChain(filelist, treename)),
    reader_(in_tree_.get()) {}

void SimpleTask::Initialize() {
  std::vector<Qn::Axis> noaxes;
  eventaxes_.emplace_back("CentralityVZERO", 9, 0, 100, 1);

  auto tpcr = *values_.at("TPC_reference");
  auto vc = *values_.at("VZEROC_reference");
  auto va = *values_.at("VZEROA_reference");
  Qn::Correlation TpcVc({tpcr, vc}, eventaxes_);
  correlations_.insert({"tpcvc", TpcVc});

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

  auto mult = [](std::vector<float> a, std::vector<float> b) {
    auto mean_a = std::get<0>(Qn::Stats::Mean(a));
    auto mean_b = std::get<0>(Qn::Stats::Mean(b));
    std::vector<float> vec = {mean_a * mean_b};
    return vec;
  };

  auto divide = [](std::vector<float> a, std::vector<float> b) {
    auto mean_a = std::get<0>(Qn::Stats::Mean(a));
    auto mean_b = std::get<0>(Qn::Stats::Mean(b));
    std::vector<float> vec = {mean_a / mean_b};
    return vec;
  };

//  Qn::DataContainerVF prod = tpcva.Add(tpcvc, mult);
//  Qn::DataContainerVF resolution = prod.Add(vavc, divide);

  auto square = [](std::vector<float> a) {

    std::cout << abs(a.at(0)) << " " << sqrt(abs(a.at(0))) << std::endl;
    std::vector<float> vec = {static_cast<float>(TMath::Sign(1, a.at(0)) * sqrt(abs(a.at(0))))};
    return vec;
  };

//  auto squared = resolution.Map(square);

  auto *testgraph = new TGraph(9);
  int ibin = 0;
  for (auto &bin : tpcvc) {
    float y = bin.at(0);
    float xhi = tpcvc.GetAxes().front().GetUpperBinEdge(ibin);
    float xlo = tpcvc.GetAxes().front().GetLowerBinEdge(ibin);
    float x = xlo + ((xhi - xlo) / 2);
    testgraph->SetPoint(ibin, x, y);
    ibin++;
  }
//  auto *testgraph2 = new TGraph(9);
//  int ibin2 = 0;
//  for (auto &bin : tpcvct) {
//    float y = bin.at(0);
//    float xhi = tpcvct.GetAxes().front().GetUpperBinEdge(ibin2);
//    float xlo = tpcvct.GetAxes().front().GetLowerBinEdge(ibin2);
//    float x = xlo + ((xhi - xlo) / 2);
//    testgraph2->SetPoint(ibin2,x,y);
//    ibin2++;
//  }
//
  auto *c1 = new TCanvas("c1", "c1", 800, 600);
  c1->cd(1);
  testgraph->SetMarkerStyle(kCircle);
  testgraph->SetMarkerStyle(kOpenSquare);
  testgraph->Draw("AP");
//  testgraph2->Draw("P");
  c1->SaveAs("test.pdf");

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
