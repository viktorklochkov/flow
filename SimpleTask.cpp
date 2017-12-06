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
  eventaxes_.emplace_back("CentralityVZERO", std::vector<float>{0.,5.,10.,20.,30.,40.,50.,60.,70.,80.}, 1);

  auto tpc = *values_.at("TPC_reference");
  auto tpcetapt = *values_.at("TPC");
  auto vc = *values_.at("VZEROC_reference");
  auto va = *values_.at("VZEROA_reference");
  auto fc = *values_.at("FMDC_reference");
  auto fa = *values_.at("FMDA_reference");
  auto zc = *values_.at("ZDCC_reference");
  auto za = *values_.at("ZDCA_reference");

  correlations_.insert({"tpcetaptvc", {{tpcetapt, vc}, eventaxes_}});
  correlations_.insert({"tpcetaptva", {{tpcetapt, vc}, eventaxes_}});
  correlations_.insert({"tpcvc", {{tpc, vc}, eventaxes_}});
  correlations_.insert({"tpcva", {{tpc, vc}, eventaxes_}});
  correlations_.insert({"vavc", {{va, vc}, eventaxes_}});
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
  std::cout << "number of events: " << events << std::endl;

  auto tpcvc = correlations_.at("tpcvc").GetCorrelation();
  auto tpcva = correlations_.at("tpcvc").GetCorrelation();
  auto tpcetaptvc = correlations_.at("tpcetaptvc").GetCorrelation();
  auto tpcetaptva = correlations_.at("tpcetaptvc").GetCorrelation();
  auto vavc = correlations_.at("vavc").GetCorrelation();

  auto multiply = [](Qn::Statistics a, Qn::Statistics b) {
    return a * b;
  };

  auto divide = [](Qn::Statistics a, Qn::Statistics b) {
    return a / b;
  };

  auto sqrt = [](Qn::Statistics a) {
    return a.Sqrt();
  };

  auto rvatpcvc = tpcva.Apply(vavc,multiply).Apply(tpcvc,divide).Map(sqrt);
  auto rvatpcptetavc = tpcetaptva.Apply(vavc,multiply).Apply(tpcetaptvc,divide).Map(sqrt);
  auto v2tpcva = tpcva.Apply(rvatpcvc,divide);

//  auto tpcptva = tpcetaptva.Projection({{"0Pt",{0.2,2,10},819},eventaxes_[0]},[](Qn::Statistics a, Qn::Statistics b){return a + b;});

  auto v2tpcetava = tpcetaptva.Apply(rvatpcptetavc,divide);
//
  auto v2tpcvaeta1 = v2tpcetava.Select({"0Eta",{-0.8,0},23});//.Projection({eventaxes_[0]},[](Qn::Statistics a, Qn::Statistics b){return a +b;});
  auto v2tpcvaeta2 = v2tpcetava.Select({"0Eta",{0,0.8},23});//.Projection({eventaxes_[0]},[](Qn::Statistics a, Qn::Statistics b){return a+b;});

  auto *c1 = new TCanvas("c1", "c1", 1800, 600);
//  c1->cd();
   auto gv2tpcva= Qn::DataToProfileGraph(v2tpcva);
  gv2tpcva->SetMaximum(0.15);
  gv2tpcva->SetMinimum(0);
  gv2tpcva->Draw();
  auto v2eta1 = Qn::DataToProfileGraph(v2tpcvaeta1);
  v2eta1->Draw();
  v2eta1->SetLineColor(kBlue);
  auto v2eta2 = Qn::DataToProfileGraph(v2tpcvaeta2);
  v2eta2->Draw();
  v2eta2->SetLineColor(kRed);
  Qn::DataToProfileGraph(rvatpcvc)->Draw();
  c1->SaveAs("test.root");
  c1->SaveAs("test.pdf");


}

void SimpleTask::Process() {
  std::vector<Qn::Axis> noaxes;

  auto tpc = *values_.at("TPC_reference");
  auto tpcetapt = *values_.at("TPC");
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

  auto scalar = [](std::vector<Qn::QVector> &a) {
    return a[0].x(2) * a[1].x(2) + a[0].y(2) * a[1].y(2);
  };

  correlations_.at("tpcetaptva").Fill({tpcetapt, va}, eventbin, scalar);
  correlations_.at("tpcetaptvc").Fill({tpcetapt, vc}, eventbin, scalar);
  correlations_.at("tpcva").Fill({tpc, va}, eventbin, scalar);
  correlations_.at("tpcvc").Fill({tpc, vc}, eventbin, scalar);
  correlations_.at("vavc").Fill({va, vc}, eventbin, scalar);

}

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
