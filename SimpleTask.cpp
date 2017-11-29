//
// Created by Lukas Kreis on 13.11.17.
//

#include <TCanvas.h>
#include <TFile.h>
#include <ReducedEvent/AliReducedVarManager.h>
#include "SimpleTask.h"
#include "DataContainerHelper.h"
SimpleTask::SimpleTask(std::string filelist, std::string treename) :
    in_tree_(this->MakeChain(filelist, treename)),
    reader_(in_tree_.get()) {}

void SimpleTask::Initialize() {
  std::vector<Qn::Axis> noaxes;
  eventaxes_.emplace_back("CentralityVZERO", 8, 0, 80, 1);

  auto tpc = *values_.at("TPC_reference");
  auto vc = *values_.at("VZEROC_reference");
  auto va = *values_.at("VZEROA_reference");
  auto fc = *values_.at("FMDC_reference");
  auto fa = *values_.at("FMDA_reference");
  auto zc = *values_.at("ZDCC_reference");
  auto za = *values_.at("ZDCA_reference");

  correlations_.insert({"rtpcvc", {{tpc, vc}, eventaxes_}});
  correlations_.insert({"rtpcva", {{tpc, va}, eventaxes_}});
  correlations_.insert({"rvcva", {{vc, va}, eventaxes_}});
  correlations_.insert({"rtpcfc", {{tpc, fc}, eventaxes_}});
  correlations_.insert({"rtpcfa", {{tpc, fa}, eventaxes_}});
  correlations_.insert({"rfcfa", {{fc, fa}, eventaxes_}});
  correlations_.insert({"rtpczc", {{tpc, zc}, eventaxes_}});
  correlations_.insert({"rtpcza", {{tpc, za}, eventaxes_}});
  correlations_.insert({"rzcza", {{zc, za}, eventaxes_}});
}

void SimpleTask::Run() {
  AddEventVariable("CentralityVZERO");
  AddDataContainer("TPC_reference");
  AddDataContainer("TPC");
  AddDataContainer("VZEROA_reference");
  AddDataContainer("VZEROC_reference");
  AddDataContainer("FMDA_reference");
  AddDataContainer("FMDC_reference");
  AddDataContainer("ZDCA_reference");
  AddDataContainer("ZDCC_reference");

  int events = 1;
  reader_.SetEntry(0);
  Initialize();
  Process();
  while (reader_.Next()) {
    events++;
    Process();
  }
  std::cout << events << std::endl;

  auto rtpcvc = correlations_.at("rtpcvc").GetCorrelation();
  auto rtpcva = correlations_.at("rtpcva").GetCorrelation();
  auto rvcva = correlations_.at("rvcva").GetCorrelation();
  auto rtpcfc = correlations_.at("rtpcfc").GetCorrelation();
  auto rtpcfa = correlations_.at("rtpcfa").GetCorrelation();
  auto rfcfa = correlations_.at("rfcfa").GetCorrelation();
  auto rtpczc = correlations_.at("rtpczc").GetCorrelation();
  auto rtpcza = correlations_.at("rtpcza").GetCorrelation();
  auto rzcza = correlations_.at("rzcza").GetCorrelation();

  auto multiply = [](Qn::Statistics a, Qn::Statistics b) {
    return a * b;
  };

  auto divide = [](Qn::Statistics a, Qn::Statistics b) {
    return a / b;
  };

  auto sqrt = [](Qn::Statistics a) {
    return a.Sqrt();
  };


  auto rtpcvcva = rtpcvc.Apply(rtpcva,multiply).Apply(rvcva,divide).Map(sqrt);
  auto rtpcfcfa = rtpcfc.Apply(rtpcfa,multiply).Apply(rfcfa,divide).Map(sqrt);
  auto rtpczcza = rtpczc.Apply(rtpcza,multiply).Apply(rzcza,divide).Map(sqrt);


  auto grtpcvcva = Qn::DataToProfileGraph(rtpcvcva);
  auto grtpcfcfa = Qn::DataToProfileGraph(rtpcfcfa);
  auto grtpczcza = Qn::DataToProfileGraph(rtpczcza);

//
  auto *c1 = new TCanvas("c1", "c1", 1200, 600);
  c1->cd(1);
  grtpcfcfa.Draw("AP");
  grtpcvcva.Draw("P");
  grtpczcza.Draw("P");

  c1->SaveAs("test.pdf");

}

void SimpleTask::Process() {
  std::vector<Qn::Axis> noaxes;

  auto tpc = *values_.at("TPC_reference");
  auto vc = *values_.at("VZEROC_reference");
  auto va = *values_.at("VZEROA_reference");
  auto fc = *values_.at("FMDC_reference");
  auto fa = *values_.at("FMDA_reference");
  auto zc = *values_.at("ZDCC_reference");
  auto za = *values_.at("ZDCA_reference");


  std::vector<float> eventparameters;
  eventparameters.push_back(*eventvalues_.at("CentralityVZERO"));
  auto eventbin = Qn::CalculateEventBin(eventaxes_, eventparameters);
  for (auto bin : eventbin) {
    if (bin == -1) return;
  }

  auto psi_n = [](Qn::QVector a, int n) {
    return 1 / (float) n * TMath::ATan2(a.y(n), a.x(n));
  };

  auto cos_n = [](int n, float a, float b) {
    return TMath::Cos((float) n * (a - b));
  };

  auto correlate = [psi_n, cos_n](std::vector<Qn::QVector> a) {
    return cos_n(2, psi_n(a.at(0), 2), psi_n(a.at(1), 2));
  };

  correlations_.at("rtpcva").Fill({tpc, va}, eventbin, correlate);
  correlations_.at("rtpcvc").Fill({tpc, vc}, eventbin, correlate);
  correlations_.at("rvcva").Fill({va, vc}, eventbin, correlate);
  correlations_.at("rtpcfa").Fill({tpc, fa}, eventbin, correlate);
  correlations_.at("rtpcfc").Fill({tpc, fc}, eventbin, correlate);
  correlations_.at("rfcfa").Fill({fa, fc}, eventbin, correlate);
  correlations_.at("rtpcza").Fill({tpc, za}, eventbin, correlate);
  correlations_.at("rtpczc").Fill({tpc, zc}, eventbin, correlate);
  correlations_.at("rzcza").Fill({za, zc}, eventbin, correlate);
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
