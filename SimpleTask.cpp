//
// Created by Lukas Kreis on 13.11.17.
//

#include <TCanvas.h>
#include <TFile.h>
#include "SimpleTask.h"
#include "Correlation.h"
#include "statistics.h"
#include "Resolution.h"
SimpleTask::SimpleTask(std::string filelist, std::string treename) :
    in_tree_(this->MakeChain(filelist, treename)),
    reader_(in_tree_.get()) {}

void SimpleTask::Initialize() {
  std::vector<Qn::Axis> noaxes;
  eventaxes_.emplace_back("CentralityVZERO", 9, 0, 100, 1);
  correlations_.insert(
      std::make_pair("TPCVZEROC", std::make_tuple(Qn::CreateCorrelation(**values_.at("TPC_reference"),
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
  correlations_.insert(
      std::make_pair("TPCVZEROCT", std::make_tuple(Qn::CreateCorrelation(**values_.at("TPC"),
                                                                        **values_.at("VZEROC_reference"),
                                                                        noaxes,
                                                                        noaxes,
                                                                        [](const Qn::QVector &a,
                                                                           const Qn::QVector &b) { return a + b; },
                                                                        eventaxes_),
                                                  values_.at("TPC"),
                                                  values_.at("VZEROC_reference"))
      )
  );
//  correlations_.insert(
//      std::make_pair("TPCVZEROA", std::make_tuple(Qn::CreateCorrelation(**values_.at("TPC_reference"),
//                                                                        **values_.at("VZEROA_reference"),
//                                                                        noaxes,
//                                                                        noaxes,
//                                                                        [](const Qn::QVector &a,
//                                                                           const Qn::QVector &b) { return a + b; },
//                                                                        eventaxes_),
//                                                  values_.at("TPC_reference"),
//                                                  values_.at("VZEROA_reference"))
//      )
//  );
//  correlations_.insert(
//      std::make_pair("VZEROAVZEROC", std::make_tuple(Qn::CreateCorrelation(**values_.at("VZEROA_reference"),
//                                                                           **values_.at("VZEROC_reference"),
//                                                                           noaxes,
//                                                                           noaxes,
//                                                                           [](const Qn::QVector &a,
//                                                                              const Qn::QVector &b) { return a + b; },
//                                                                           eventaxes_),
//                                                     values_.at("VZEROA_reference"),
//                                                     values_.at("VZEROC_reference"))
//      )
//  );

}

void SimpleTask::Run() {
  AddEventVariable("CentralityVZERO");
  AddDataContainer("TPC_reference");
  AddDataContainer("TPC");
  AddDataContainer("VZEROC_reference");
  AddDataContainer("VZEROA_reference");
  reader_.SetEntry(0);
  Initialize();
  Process();
  while (reader_.Next()) {
    Process();
  }
//  Qn::DataContainerVF tpcva = std::get<0>(correlations_.at("TPCVZEROA"));
  Qn::DataContainerVF tpcvct = std::get<0>(correlations_.at("TPCVZEROCT"));
  Qn::DataContainerVF tpcvc = std::get<0>(correlations_.at("TPCVZEROC"));
//  Qn::DataContainerVF vavc = std::get<0>(correlations_.at("VZEROAVZEROC"));

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

    std::cout << abs(a.at(0))<< " " << sqrt(abs(a.at(0))) << std::endl;
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
    testgraph->SetPoint(ibin,x,y);
    ibin++;
  }
  auto *testgraph2 = new TGraph(9);
  int ibin2 = 0;
  for (auto &bin : tpcvct) {
    float y = bin.at(0);
    float xhi = tpcvct.GetAxes().front().GetUpperBinEdge(ibin2);
    float xlo = tpcvct.GetAxes().front().GetLowerBinEdge(ibin2);
    float x = xlo + ((xhi - xlo) / 2);
    testgraph2->SetPoint(ibin2,x,y);
    ibin2++;
  }

  auto *c1 = new TCanvas("c1", "c1", 800, 600);
  c1->cd(1);
  testgraph->SetMarkerStyle(kCircle);
  testgraph->SetMarkerStyle(kOpenSquare);
  testgraph->Draw("AP");
  testgraph2->Draw("P");
  c1->SaveAs("test.pdf");

}

void SimpleTask::Process() {
  std::vector<Qn::Axis> noaxes;

  auto comp1 = (**values_.at("TPC"));
  auto comp1projected = (**values_.at("TPC")).Projection([](Qn::QVector &a, Qn::QVector &b) {return a +b;});
  auto comp1normalized = comp1.Map([](Qn::QVector &a){return a.Normal(Qn::QVector::Normalization::QOVERM);});
  auto comp2 = (**values_.at("TPC_reference")).Map([](Qn::QVector &a){return a.DeNormal();});;
  int sum = 0;
  for (auto &bin : comp1) {
    sum +=bin.n();
  }
  if (sum == comp1projected.GetElement(0).n()) std::cout << "yes" << std::endl;

  std::vector<float> eventparameters;
  eventparameters.push_back(*eventvalues_.at("CentralityVZERO"));
  auto eventbin = Qn::CalculateEventBin(eventaxes_, eventparameters);

  auto psi_n = [](Qn::QVector a, int n) {
    return 1 / (float) n * atan2(a.y(n), a.x(n));
  };

  auto cos_n = [](int n, float a, float b) {
    return cos((float) n  * (a - b));
  };

  auto correlate = [psi_n, cos_n](Qn::QVector a, Qn::QVector b) {
    return cos_n(2, psi_n(a, 2), psi_n(b, 2));
  };
  for (auto &corr : correlations_) {
    auto proja = (**std::get<1>(corr.second)).Projection(noaxes,[](const Qn::QVector &a,const Qn::QVector &b) { return a + b; });
    auto projb = (**std::get<2>(corr.second)).Projection(noaxes,[](const Qn::QVector &a,const Qn::QVector &b) { return a + b; });
    Qn::FillCorrelation(std::get<0>(corr.second),
                        proja,
                        projb,
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
  std::shared_ptr<TTreeReaderValue<Qn::DataContainerQVector>>
      value(new TTreeReaderValue<Qn::DataContainerQVector>(reader_, name.data()));
  auto pair = std::make_pair(name, value);
  values_.insert(pair);
}
void SimpleTask::AddEventVariable(std::string name) {
  TTreeReaderValue<float> value(reader_, name.data());
  eventvalues_.insert(std::make_pair(name, value));
}
