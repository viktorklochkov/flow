//
// Created by Lukas Kreis on 13.11.17.
//
#include <utility>
#include <TCanvas.h>
#include <TFile.h>
#include <TLegend.h>
#include <ReducedEvent/AliReducedVarManager.h>

#include "SimpleTask.h"
#include "Base/DataContainerHelper.h"

SimpleTask::SimpleTask(std::string filelist, std::string treename) :
    in_tree_(this->MakeChain(std::move(filelist), treename)),
    reader_(new TTreeReader(in_tree_.get())) {}

void SimpleTask::Configure(Qn::CorrelationManager &a) {
  auto scalar = [](const std::vector<Qn::QVector> &a) -> double {
    return a[0].x(2)*a[1].x(2) + a[0].y(2)*a[1].y(2);
  };
  auto XY = [](const std::vector<Qn::QVector> &a) {
    return a[0].x(1)*a[1].y(1);
  };
  auto YX = [](const std::vector<Qn::QVector> &a) {
    return a[0].y(1)*a[1].x(1);
  };
  auto XX = [](const std::vector<Qn::QVector> &a) {
    return a[0].x(1)*a[1].x(1);
  };
  auto YY = [](const std::vector<Qn::QVector> &a) {
    return a[0].y(1)*a[1].y(1);
  };
  auto X2X1 = [](const std::vector<Qn::QVector> &a) {
    return a[0].x(2)*a[1].x(1);
  };
  auto Y2Y1 = [](const std::vector<Qn::QVector> &a) {
    return a[0].y(2)*a[1].y(1);
  };
  auto Y2XY = [](const std::vector<Qn::QVector> &a) {
    return a[0].y(2)*a[1].x(1)* a[2].y(1);
  };
  auto Y2YX = [](const std::vector<Qn::QVector> &a) {
    return a[0].y(2)*a[1].y(1)* a[2].x(1);
  };
  auto X2XX = [](const std::vector<Qn::QVector> &a) {
    return a[0].x(2)*a[1].x(1)* a[2].x(1);
  };
  auto X2YY = [](const std::vector<Qn::QVector> &a) {
    return a[0].x(2)*a[1].y(1)* a[2].y(1);
  };
  auto Rebin = [](const Qn::DataContainerQVector &a) {
    auto result = a.Rebin({"Eta", 2, -0.8, 0.8}, [](Qn::QVector &a, Qn::QVector &b) { return  a + b; });
    return result;
  };

  a.AddDataContainer("TPC");
  a.AddDataContainer("TPC_reference");
  a.AddDataContainer("VZEROA_reference");
  a.AddDataContainer("VZEROC_reference");
  a.AddDataContainer("ZDCA_reference");
  a.AddDataContainer("ZDCC_reference");
  a.AddProjection("TPC", "TPCPt", "Pt");
  a.AddProjection("TPC", "TPCEta", "Eta");
  a.AddEventVariable({"CentralityVZERO", {0., 5., 10., 20., 30., 40., 50., 60., 70.}});
//  a.AddFunction("TPC", Rebin);
  a.AddCorrelation("TPCETAZDCAXX","TPCEta, ZDCA_reference", XX);
  a.AddCorrelation("TPCETAZDCAYY","TPCEta, ZDCA_reference", YY);
  a.AddCorrelation("TPCETAZDCAXY","TPCEta, ZDCA_reference", XY);
  a.AddCorrelation("TPCETAZDCAYX","TPCEta, ZDCA_reference", YX);
  a.AddCorrelation("TPCETAZDCCXX","TPCEta, ZDCC_reference", XX);
  a.AddCorrelation("TPCETAZDCCYY","TPCEta, ZDCC_reference", YY);
  a.AddCorrelation("TPCETAZDCCXY","TPCEta, ZDCC_reference", XY);
  a.AddCorrelation("TPCETAZDCCYX","TPCEta, ZDCC_reference", YX);
  a.AddCorrelation("TPCPTVA", "TPCPt, VZEROA_reference", scalar);
  a.AddCorrelation("TPCPTVC", "TPCPt, VZEROC_reference", scalar);
  a.AddCorrelation("TPCETAVA", "TPCEta, VZEROA_reference", scalar);
  a.AddCorrelation("TPCETAVC", "TPCEta, VZEROC_reference", scalar);
  a.AddCorrelation("TPCVA", "TPC_reference, VZEROA_reference", scalar);
  a.AddCorrelation("TPCVC", "TPC_reference, VZEROC_reference", scalar);
  a.AddCorrelation("VAVC", "VZEROA_reference, VZEROC_reference", scalar);
  a.AddCorrelation("TPCZDCAX2X1", "TPC_reference, ZDCA_reference", X2X1);
  a.AddCorrelation("TPCZDCAY2Y1", "TPC_reference, ZDCA_reference", Y2Y1);
  a.AddCorrelation("TPCZDCCX2X1", "TPC_reference, ZDCC_reference", X2X1);
  a.AddCorrelation("TPCZDCCY2Y1", "TPC_reference, ZDCC_reference", Y2Y1);
  a.AddCorrelation("TPCZDCAZDCCX2YY", "TPC_reference, ZDCA_reference, ZDCC_reference", X2YY);
  a.AddCorrelation("TPCZDCAZDCCX2XX", "TPC_reference, ZDCA_reference, ZDCC_reference", X2XX);
  a.AddCorrelation("TPCZDCAZDCCY2YX", "TPC_reference, ZDCA_reference, ZDCC_reference", Y2YX);
  a.AddCorrelation("TPCZDCAZDCCY2XY", "TPC_reference, ZDCA_reference, ZDCC_reference", Y2XY);
  a.AddCorrelation("TPCZDCAXX", "TPC_reference, ZDCA_reference", XX);
  a.AddCorrelation("TPCZDCAYY", "TPC_reference, ZDCA_reference", YY);
  a.AddCorrelation("TPCZDCCXX", "TPC_reference, ZDCC_reference", XX);
  a.AddCorrelation("TPCZDCCYY", "TPC_reference, ZDCC_reference", YY);
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

