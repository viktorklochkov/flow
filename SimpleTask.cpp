//
// Created by Lukas Kreis on 13.11.17.
//

#include <TCanvas.h>
#include "SimpleTask.h"
#include "Correlation.h"
#include "statistics.h"
SimpleTask::SimpleTask(std::string filelist, std::string treename) :
    in_tree_(this->MakeChain(filelist, treename)),
    reader_(in_tree_.get()) {}

void SimpleTask::Initialize() {
  std::vector<Qn::Axis> noaxes;
  eventaxes_.emplace_back("CentralityVZERO", 100, 0, 100, 1);
  correlations_.insert(
      std::make_pair("tpcxy", std::make_tuple(Qn::CreateCorrelation(**values_.at("TPC_reference"),
                                                                    **values_.at("VZEROC_reference"),
                                                                    noaxes,
                                                                    noaxes,
                                                                    [](const Qn::QVector &a,
                                                                       const Qn::QVector &b) { return a + b; },
                                                                    eventaxes_),
                                              values_.at("TPC_reference"),
                                              values_.at("VZEROC_reference"))
      )
  );

}

void SimpleTask::Run() {
  AddEventVariable("CentralityVZERO");
  AddDataContainer("TPC_reference");
  AddDataContainer("VZEROC_reference");
  reader_.SetEntry(0);
  Initialize();
  Process();
  while (reader_.Next()) {
    Process();
  }
  Qn::DataContainerVF test2 = std::get<0>(correlations_.at("tpcxy"));
  auto *testgraph = new TGraphErrors(100);
  int ibin = 0;
  for (auto &bin : test2) {
    float mean = std::get<0>(Qn::Stats::Mean(bin));
    float error = Qn::Stats::Error(bin);
    float xhi = test2.GetAxes().front().GetUpperBinEdge(ibin);
    float xlo = test2.GetAxes().front().GetLowerBinEdge(ibin);
    float x = xlo + ((xhi - xlo) /2);
    testgraph->SetPoint(ibin, x, mean);
    testgraph->SetPointError(ibin,(xhi-xlo)/2,error);

    ibin++;
  }
  TCanvas *c1 = new TCanvas("c1","c1",800,600);
  c1->cd();
  testgraph->Draw("ALP");
  testgraph->SetMinimum(-0.3);
  c1->SaveAs("test.pdf");
}

void SimpleTask::Process() {
  std::vector<float> eventparameters;
  eventparameters.push_back(*eventvalues_.at("CentralityVZERO"));
  auto eventbin = Qn::CalculateEventBin(eventaxes_, eventparameters);
  auto correlate = [](Qn::QVector a, Qn::QVector b) {
    std::cout << a.x(1) * b.x(1) << std::endl;
    return a.x(1) * b.x(1);
  };
  for (auto &corr : correlations_) {
    Qn::FillCorrelation(std::get<0>(corr.second),
                        **std::get<1>(corr.second),
                        **std::get<2>(corr.second),
                        correlate,
                        eventbin);
  }
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
  std::shared_ptr<TTreeReaderValue<Qn::DataContainerQVector>> value(new TTreeReaderValue<Qn::DataContainerQVector>(reader_, name.data()));
  auto pair = std::make_pair(name, value);
  values_.insert(pair);
}
void SimpleTask::AddEventVariable(std::string name) {
  TTreeReaderValue<float> value(reader_, name.data());
  eventvalues_.insert(std::make_pair(name, value));
}
