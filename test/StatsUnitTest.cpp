
#include <random>

#include "gtest/gtest.h"
#include "Stats.h"
#include "Sampler.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TPaveText.h"
#include "TPaveStats.h"
#include "TStyle.h"

TEST(StatsTest, Trivial) {
Qn::Stats stats;
Qn::Product prod({1.,2.},0,true);
stats.SetNumberOfSubSamples(1);
stats.Fill(prod,{0});
EXPECT_EQ(0, stats.Mean());
EXPECT_EQ(0, stats.Error());
}

TEST(StatsTest, Printing) {
  Qn::Stats stats;
  stats.SetBits((BIT(16)|BIT(17)));
  stats.ResetBits(BIT(16));
  stats.SetNumberOfSubSamples(3);
  stats.Fill({{1.},1,true},{0,1,2});
  stats.Print();
}

TEST(StatsTest, Addition) {
  Qn::Stats stats_A;
  Qn::Stats stats_B;
  stats_A.Fill({{1.},1,true},{});
  stats_B.Fill({{2.},2,true},{});
  auto stats_C = stats_A + stats_B;
  EXPECT_EQ(5./3, stats_C.Mean());
  EXPECT_EQ(0.351364, stats_C.Error());
}

TEST(StatsTest, Gaussian) {
  std::default_random_engine generator;
  std::normal_distribution<double> gauss(0,1);
  Qn::Stats stats;
  int nsamples = 1000;
  int nevents = 100000;
  stats.SetNumberOfSubSamples(nsamples);
  Qn::Sampler sampler(nsamples,Qn::Sampler::Method::BOOTSTRAP);
  sampler.SetNumberOfEvents(nevents);
  sampler.CreateBootstrapSamples();
  for (int i = 0; i < nevents; ++i) {
  stats.Fill({{1.},gauss(generator),true}, sampler.GetFillVector(i));
  }
  stats.Print();
  auto histo = stats.SampleMeanHisto("SampleMean");
  TCanvas c1("c","c",800,600);
  c1.cd();
  histo.Scale(1/histo.GetEntries());
  histo.Draw();
  auto g1 = new TF1("g1","gaus",-1,1);
  histo.Fit(g1);
  g1->Draw("SAME");
  gStyle->SetOptFit();
  stats.ResetBits(Qn::Stats::CORRELATEDERRORS);
  auto mean = stats.Mean();
  auto err = stats.Error();
  TPaveText pave(0.6,0.1,0.9,0.3,"NDCBR");
  pave.AddText(TString::Format("original:  %f +- %f",mean,err));
  stats.SetBits(Qn::Stats::CORRELATEDERRORS);
  auto bmean = stats.BootstrapMean();
  auto berr = stats.Error();
  pave.AddText(TString::Format("bootstrap: %f +- %f",bmean,berr));
  pave.Draw();
  c1.SaveAs("BSGauss.png");
  EXPECT_NEAR(err,berr,err*0.1);
}

TEST(StatsTest, GaussianAddition) {
  std::default_random_engine generator;
  std::normal_distribution<double> gauss_a(-0.5,1);
  std::normal_distribution<double> gauss_b(0.5,1);
  Qn::Stats stats_a;
  Qn::Stats stats_b;
  Qn::Stats stats_d;
  int nsamples = 100;
  int nevents = 1000;
  stats_a.SetNumberOfSubSamples(nsamples);
  stats_b.SetNumberOfSubSamples(nsamples);
  stats_d.SetNumberOfSubSamples(nsamples);
  Qn::Sampler sampler(nsamples,Qn::Sampler::Method::BOOTSTRAP);
  sampler.SetNumberOfEvents(nevents);
  sampler.CreateBootstrapSamples();
  for (int i = 0; i < nevents; ++i) {
    auto a = gauss_a(generator);
    auto b = gauss_b(generator);
    stats_a.Fill({{1.},a,true}, sampler.GetFillVector(i));
    stats_b.Fill({{1.},b,true}, sampler.GetFillVector(i));
    stats_d.Fill({{1.},a,true}, sampler.GetFillVector(i));
    stats_d.Fill({{1.},b,true}, sampler.GetFillVector(i));
  }
  auto stats_c = stats_a + stats_b;
  auto err_c = stats_c.Error();
  auto err_d = stats_d.Error();
  EXPECT_FLOAT_EQ(err_c,err_d);
}