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
  eventaxes_.emplace_back("CentralityVZERO", 8, 0, 100, 1);

  auto tpc = *values_.at("TPC_reference");
  auto vc = *values_.at("VZEROC_reference");
  auto va = *values_.at("VZEROA_reference");
  Qn::Axis etaaxis("Eta", 6, -0.8, 0.8, VAR::kEta);
  Qn::Axis rebinetaaxis("Eta", 2, -0.8, 0.8, VAR::kEta);
  auto addqvec = [](Qn::QVector a, Qn::QVector b) {return a + b;};
//  auto tpceta = tpc.Projection({etaaxis},addqvec);
//  tpceta = tpceta.Rebin(addqvec,{rebinetaaxis});
  Qn::Correlation TpcVc({tpc, va}, eventaxes_);
  Qn::Correlation TpcVa({tpc, va}, eventaxes_);
  Qn::Correlation VcVa({vc, va}, eventaxes_);
  correlations_.insert({"tpcvc", TpcVc});
  correlations_.insert({"tpcva", TpcVa});
  correlations_.insert({"vcva", VcVa});

  ab = new TProfile("ab","ab",5,0,100);
  ac = new TProfile("ac","ac",5,0,100);
  bc = new TProfile("bc","bc",5,0,100);
}

void SimpleTask::Run() {
  AddEventVariable("CentralityVZERO");
  AddDataContainer("TPC_reference");
  AddDataContainer("TPC");
  AddDataContainer("VZEROC_reference");
  AddDataContainer("FMDC_reference");
  AddDataContainer("FMDA_reference");
  AddDataContainer("VZEROA_reference");
  int events = 1;
  reader_.SetEntry(0);
  Initialize();
  Process();
  while (reader_.Next()) {
    events++;
    Process();
  }
  std::cout << events << std::endl;

  auto tpcvc = correlations_.at("tpcvc").GetCorrelation();
  auto tpcva = correlations_.at("tpcva").GetCorrelation();
  auto vcva = correlations_.at("vcva").GetCorrelation();


  auto multiply = [](Qn::Statistics a, Qn::Statistics b) {
    return a * b;
  };

  auto divide = [](Qn::Statistics a, Qn::Statistics b) {
    return a / b;
  };

  auto sqrt = [](Qn::Statistics a) {
    return a.Sqrt();
  };

  auto pac = ac->ProjectionX();
  auto pab = ab->ProjectionX();
  auto pbc = bc->ProjectionX();

  auto ra = (TH1D *) pab->Clone("test2");
  ra->Multiply(pac);
  ra->Divide(pbc);
//  SqrtHist(*ra);


  auto step1 = tpcvc.Apply(tpcva,multiply);
  auto step2 = tpcvc.Apply(tpcva,multiply).Apply(vcva,divide);//.Map(sqrt);
  auto resolution = tpcvc.Apply(tpcva,multiply).Apply(vcva,divide).Map(sqrt);


  auto gstep1 = Qn::DataToProfileGraph(step1);
  auto gstep2 = Qn::DataToProfileGraph(step2);
  auto gresolution = Qn::DataToProfileGraph(resolution);
  auto gtpcvc = Qn::DataToProfileGraph(tpcvc);
  auto gtpcva = Qn::DataToProfileGraph(tpcva);
  auto gvcva = Qn::DataToProfileGraph(vcva);

//
  auto *c1 = new TCanvas("c1", "c1", 1200, 600);
  c1->Divide(4);
  c1->cd(1);
//  ra->SetMaximum(1);
//  ra->SetMinimum(0);
  gresolution.Draw("AP");
  c1->cd(2);
//  ac->Draw();
  ra->Draw();
  ra->SetMinimum(0);
  ra->SetMaximum(1);
  gstep2.Draw("P");
  c1->cd(3);
//  ab->Draw();
  gstep1.Draw("AP");
  c1->cd(4);
//  bc->Draw();
  pbc->Draw();
  gvcva.Draw("P");
//  c1->SaveAs("test.root");
  c1->SaveAs("test.pdf");

}

void SimpleTask::Process() {
  std::vector<Qn::Axis> noaxes;

  auto tpc = *values_.at("TPC_reference");
  auto vc = *values_.at("VZEROC_reference");
  auto va = *values_.at("VZEROA_reference");
  Qn::Axis etaaxis("Eta", 6, -0.8, 0.8, VAR::kEta);
  Qn::Axis rebinetaaxis("Eta", 2, -0.8, 0.8, VAR::kEta);
  auto addqvec = [](Qn::QVector a, Qn::QVector b) {return a + b;};
  auto tpceta = tpc.Projection({etaaxis},addqvec);
  tpceta = tpceta.Rebin(addqvec,{rebinetaaxis});

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

  auto correlatetest = [psi_n, cos_n](Qn::QVector a, Qn::QVector b) {
    return cos_n(2, psi_n(a, 2), psi_n(b, 2));
  };
//
//  std::cout << *eventvalues_.at("CentralityVZERO") << " : " << tpc.GetElement(0).n() << "\n ";

  ab->Fill(*eventvalues_.at("CentralityVZERO"),correlatetest(tpc.GetElement(0),va.GetElement(0)));
  ac->Fill(*eventvalues_.at("CentralityVZERO"),correlatetest(tpc.GetElement(0),vc.GetElement(0)));
  bc->Fill(*eventvalues_.at("CentralityVZERO"),correlatetest(va.GetElement(0),vc.GetElement(0)));
//
//
  correlations_.at("tpcva").Fill({tpc, va}, eventbin, correlate);
  correlations_.at("tpcvc").Fill({tpc, vc}, eventbin, correlate);
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
