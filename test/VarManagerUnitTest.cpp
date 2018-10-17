//
// Created by Lukas Kreis on 24.08.18.
//

#include <gtest/gtest.h>
#include <TMath.h>
#include <Detector.h>
#include <QAHistogram.h>
#include <VariableCutBase.h>
#include "VariableManager.h"

TEST(VarManagerUnitTest, test) {
  using namespace Qn;
  VariableManager a;
  double *vars = new double[10];
  a.SetVariables(vars);
  a.CreateVariable("i=1,2", 1, 2);
  for (int i = 0; i < 10; i++) {
    vars[i] = i*8;
  }
  auto variable = a.FindVariable("i=1,2");
  for (auto var : variable) {
    std::cout << var << std::endl;
  }
  std::cout << *(variable.begin() + 1) << std::endl;
}


//
//TEST(VarManagerUnitTest, dettest) {
//  using namespace Qn;
//  VariableManager a;
//  double *vars = new double[100];
//  a.SetVariables(vars);
//  a.CreateVariable("weight", 0, 4);
//  a.CreateVariable("phi", 5, 4);
//  a.CreateVariable("eta", 9, 4);
//  a.CreateVariable("ev", 13, 1);
//  for (int i = 0; i < 5; i++) {
//    vars[i] = i;
//  }
//  vars[5] = 0;
//  vars[6] = 0.5*TMath::Pi();
//  vars[7] = 1*TMath::Pi();
//  vars[8] = 1.5*TMath::Pi();
//  vars[9] = -1.0;
//  vars[10] = -0.5;
//  vars[11] = 0.5;
//  vars[12] = 1.0;
//  vars[13] = 5;
//  auto weight = a.FindVariable("weight");
//  auto phi = a.FindVariable("phi");
//  auto eta = a.FindVariable("eta");
//  auto ev = a.FindVariable("ev");
//  Detector det(DetectorType::Channel, {}, phi, weight, {});
////  det.AddCut(eta, [](double eta) { return eta < 0; });
//  det.Initialize("Test", a);
//  det.FillData();
//  vars[13] = 6;
//  det.ClearData();
//  det.FillData();
//  TFile *file = new TFile("vartest.root", "RECREATE");
//  file->cd();
//  TDirectory *test = file->mkdir("test", "test");
//  test->cd();
//  det.SaveReport();
//  file->Close();
//}

//TEST(VarManagerUnitTest, cuttest) {
//  using namespace Qn;
//  VariableManager a;
//  double *vars = new double[100];
//  a.SetVariables(vars);
//  a.CreateVariable("weight", 0, 2);
//  a.CreateVariable("mass", 5, 2);
//  a.CreateVariable("phi", 10, 2);
//  for (int i = 0; i < 5; i++) {
//    vars[i] = i;
//  }
//  for (int i = 5; i < 10; i++) {
//    vars[i] = 10 - i;
//  }
//  vars[10] = 2;
//  vars[11] = 2;
//  vars[0] = 1;
//  vars[1] = 1;
//  vars[5] = 1;
//  vars[6] = 0;
//  auto weight = a.FindVariable("weight");
//  auto mass = a.FindVariable("mass");
//  auto phi = a.FindVariable("weight");
//  Qn::VariableCutNDim<double &, double &> cut({weight, mass}, [](double &w, double &m) { return w > 1 && m < 8; });
//  auto a0 = cut.Check(0);
//  auto a1 = cut.Check(1);
//  auto a2 = cut.Check(2);
//  auto a3 = cut.Check(3);
//
//  auto cutN = MakeUniqueNDimCut({weight, mass}, [](double &w, double &m) { return w > 1 && m < 8; });
//
//  TFile file("testreport.root", "RECREATE");
//  {
//    Cuts cuts;
//    cuts.AddCut({weight}, [](double &w) { return w > 0; });
//    cuts.AddCut({mass}, [](double &m) {
//      std::cout << m << std::endl; return m > 0; });
//    cuts.CreateCutReport(phi.length());
//    cuts.CheckCuts(0);
//    cuts.CheckCuts(1);
//    cuts.FillReport();
//    cuts.Write();
//  }
//  file.Close();
//}
//
//TEST(VarManagerUnitTest, testtest) {
//  using namespace Qn;
//  VariableManager a;
//  double vars[4];
//  for (int i = 0; i < 5; i++) {
//    vars[i] = i;
//  }
//  std::array<double, 4> weight;
//  std::copy(std::begin(vars), std::end(vars), std::begin(weight));
//  std::vector<std::unique_ptr<QAHistoBase>> vector;
//  vector.emplace_back(new QAHisto<TH2F, 3, std::array<double, 4>>({{weight, weight, weight}},
//                                                                  {"a", "a", 10, 0, 10, 10, 0, 10}));
//  vector.push_back(std::make_unique<QAHisto<TH1F, 2, std::array<double, 4>>>(std::array<std::array<double, 4>, 2>{
//      {weight, weight}}, TH1F("a", "a", 10, 0, 10)));
//  for (auto &hist : vector) {
//    hist->Fill();
//  }
//  TCanvas c1("c1", "c1", 600, 600);
//  c1.Divide(2);
//  int i = 0;
//  for (auto &hist : vector) {
//    c1.cd(i + 1);
//    hist->Draw("COLZ");
//    ++i;
//  }
//  c1.SaveAs("test.png");
//}