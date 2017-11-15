//
// Created by Lukas Kreis on 13.11.17.
//

#include "SimpleTask.h"
#include "Correlation.h"
SimpleTask::SimpleTask(std::string filelist, std::string treename) :
    in_tree_(this->MakeChain(filelist, treename)),
    reader_(in_tree_.get()) {}

void SimpleTask::Initialize() {
  std::vector<Qn::Axis> noaxes;
  eventaxes_.emplace_back("CentralityVZERO", 100, 0, 100, 1);
//  correlations_.insert(std::make_pair("tpcxy", Qn::CreateCorrelation(*values_.at("TPC_reference").Get(),*values_.at("TPC_reference").Get(), noaxes, noaxes, std::plus(), eventaxes_)));
  auto test = Qn::CreateCorrelation(*values_.at("TPC_reference").Get(),
                                    *values_.at("TPC_reference").Get(),
                                    noaxes,
                                    noaxes,
                                    [](const Qn::QVector &a, const Qn::QVector &b) { return a + b; },
                                    eventaxes_);

}

void SimpleTask::Run() {
  reader_.SetEntry(0);
  Initialize();
  Process();
  while (reader_.Next()) {
    Process();
  }
}

void SimpleTask::Process() {
  std::vector<float> eventparameters;
  eventparameters.push_back(*eventvalues_.at("CentralityVZERO"));
  Qn::CalculateEventBin(eventaxes_, eventparameters);
  auto correlate = [](Qn::QVector a, Qn::QVector b){

  };
  for (auto &corr : correlations_) {
    Qn::FillCorrelation(std::get<0>(corr.second),std::get<1>(corr.second),std::get<2>(corr.second),correlate, eventparameters);
//    corr.second
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
  TTreeReaderValue<Qn::DataContainerQVector> value(reader_, name.data());
  auto pair = std::make_pair(name, value);
  values_.insert(pair);
}
void SimpleTask::AddEventVariable(std::string name) {
  TTreeReaderValue<float> value(reader_, name.data());
  eventvalues_.insert(std::make_pair(name, value));
}
