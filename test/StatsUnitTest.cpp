
#include <random>
#include <algorithm>

#include "gtest/gtest.h"
#include "Stats.h"
#include "Sampler.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TPaveText.h"
#include "TPaveStats.h"
#include "TStyle.h"
#include "TProfile.h"

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
  EXPECT_FLOAT_EQ(5./3, stats_C.Mean());
  EXPECT_FLOAT_EQ(0.35136418, stats_C.Error());
}

TEST(StatsTest, MultiplicationPA) {
  Qn::Stats stats_A;
  Qn::Stats stats_B;
  stats_A.Fill({{1.},1,true},{});
  stats_B.Fill({{2.},2,true},{});
  stats_A.SetStatus(Qn::Stats::Status::OBSERVABLE);
  stats_B.SetStatus(Qn::Stats::Status::OBSERVABLE);
  auto stats_C = stats_A * stats_B;
  EXPECT_FLOAT_EQ(2, stats_C.Mean());
  EXPECT_FLOAT_EQ(0, stats_C.Error());
}

TEST(StatsTest, Division) {
  Qn::Stats stats_A;
  Qn::Stats stats_B;
  stats_A.SetStatus(Qn::Stats::Status::OBSERVABLE);
  stats_B.SetStatus(Qn::Stats::Status::REFERENCE);
  stats_A.Fill({{6.},1,true},{});
  stats_B.Fill({{10.},2,true},{});
  auto stats_C = stats_A / stats_B;
  EXPECT_FLOAT_EQ(1./2 , stats_C.Mean());
  EXPECT_FLOAT_EQ(0, stats_C.Error());
}

TEST(StatsTest, Sqrt) {
  std::default_random_engine generator;
  std::normal_distribution<double> gauss(2,1);
  Qn::Stats stats;
  int nsamples = 1000;
  int nevents = 10000;
  stats.SetNumberOfSubSamples(nsamples);
  Qn::Sampler sampler(nsamples,Qn::Sampler::Method::BOOTSTRAP);
  sampler.SetNumberOfEvents(nevents);
  sampler.CreateBootstrapSamples();
  for (int i = 0; i < nevents; ++i) {
    stats.Fill({{1.},gauss(generator),true}, sampler.GetFillVector(i));
  }
//  stats.SetStatus(Qn::Stats::Status::POINTAVERAGE);
  stats = Qn::Sqrt(stats);
  auto histo = stats.SampleMeanHisto("SampleMean");
  TCanvas c1("c","c",800,600);
  c1.cd();
  histo.Scale(1/histo.GetEntries());
  histo.Draw();
  auto g1 = new TF1("g1","gaus",-0,3);
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

TEST(StatsTest, DivisionPA) {
  Qn::Stats stats_A;
  Qn::Stats stats_B;
  stats_A.Fill({{1.},1,true},{});
  stats_B.Fill({{2.},2,true},{});
  stats_A.SetStatus(Qn::Stats::Status::OBSERVABLE);
  stats_B.SetStatus(Qn::Stats::Status::OBSERVABLE);
  auto stats_C = stats_A / stats_B;
  EXPECT_FLOAT_EQ(1./2 , stats_C.Mean());
  EXPECT_FLOAT_EQ(0, stats_C.Error());
}

TEST(StatsTest, Merging) {
  Qn::Stats stats_A;
  Qn::Stats stats_B;
  stats_A.SetNumberOfSubSamples(10);
  stats_B.SetNumberOfSubSamples(10);
  auto stats_C = Qn::Merge(stats_A,stats_B);
  EXPECT_EQ(stats_A.GetNSamples(),stats_C.GetNSamples());
}

TEST(StatsTest, MergingConcate) {
  Qn::Stats stats_A;
  Qn::Stats stats_B;
  stats_A.SetBits(Qn::Stats::MERGESUBSAMPLES);
  stats_A.SetNumberOfSubSamples(10);
  stats_B.SetNumberOfSubSamples(10);
  auto stats_C = Qn::Merge(stats_A,stats_B);
  EXPECT_EQ(20,stats_C.GetNSamples());
}

TEST(StatsTest, Gaussian) {
  std::default_random_engine generator;
  std::normal_distribution<double> gauss(0,1);
  Qn::Stats stats;
  int nsamples = 1000;
  int nevents = 100;
  stats.SetNumberOfSubSamples(nsamples);
  Qn::Sampler sampler(nsamples,Qn::Sampler::Method::BOOTSTRAP);
  sampler.SetNumberOfEvents(nevents);
  sampler.CreateBootstrapSamples();
  auto gausshisto = new TH1F("gauss","gauss",50,-10,10);
  for (int i = 0; i < nevents; ++i) {
    auto a = gauss(generator);
    stats.Fill({{1.},a,true}, sampler.GetFillVector(i));
    gausshisto->Fill(a);
  }
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
  gausshisto->SetLineColor(kGreen+2);
//  gausshisto->Draw("HISTO");
  c1.SaveAs("BSGauss.png");
  EXPECT_NEAR(err,berr,err*0.1);
}

TEST(StatsTest, BSMoutofN) {
  const int ntests = 100;
  std::array<double, ntests> tests;
  for (int j = 0; j < ntests; ++j) {
    constexpr float m_step = 0.1;
    constexpr int m_size = static_cast<int>(1./m_step + 1);
    std::array<double, m_size> deviations;
    for (int m = 1; m < m_size + 1; ++m) {
      std::default_random_engine generator;
      generator.seed(j);
      std::normal_distribution<double> gauss_1(0.1, 2.);
      std::normal_distribution<double> gauss_0(1.0, 0.5);
      Qn::Stats stats_A;
      Qn::Stats stats_B;
      int nsamples = 1000;
      int nevents = 100;
      stats_A.SetNumberOfSubSamples(nsamples);
      stats_B.SetNumberOfSubSamples(nsamples);
      Qn::Sampler sampler(nsamples, Qn::Sampler::Method::BOOTSTRAP);
      sampler.SetNumberOfEvents(nevents);
      sampler.CreateMoutofNBootstrapSamples(m*m_step);
      auto histo1 = new TProfile((std::to_string(m) + std::to_string(j)).data(), "gauss2", 1, 0, 1);
      auto histo2 = new TProfile((std::to_string(m) + std::to_string(j)+"den").data(), "gauss2", 1, 0, 1);
      for (int i = 0; i < nevents; ++i) {
        double a = gauss_0(generator);
        double b = gauss_1(generator);
        stats_A.Fill({{1.}, a, true}, sampler.GetFillVector(i));
        stats_B.Fill({{1.}, b, true}, sampler.GetFillVector(i));
        histo1->Fill(0., a);
        histo2->Fill(0., b);
      }
      auto m1 = histo1->GetBinContent(1);
      auto m2 = histo2->GetBinContent(1);
      auto err1 = histo1->GetBinError(1);
      auto err2 = histo2->GetBinError(1);
      auto errtotal = std::sqrt((err1/m2)*(err1/m2) + (m1*err2/m2/m2)*(m1*err2/m2/m2));
      auto stats_C = stats_A/stats_B;
      stats_C.ResetBits(Qn::Stats::CORRELATEDERRORS);
      auto mean = stats_C.Mean();
      stats_C.SetBits(Qn::Stats::CORRELATEDERRORS);
      auto bsmean = stats_C.BootstrapMean();
      auto bserr = stats_C.Error();
      deviations.at(m - 1) = std::abs((errtotal - bserr)/errtotal);
    }
    auto min = std::min_element(deviations.begin(), deviations.end());
    std::cout << (std::distance(deviations.begin(), min) + 1)*m_step << " " << *min << std::endl;
    tests.at(j) = (std::distance(deviations.begin(), min) + 1)*m_step;
  }
}

TEST(StatsTest, GaussianRatio) {
  std::default_random_engine generator;
  generator.seed(123);
  std::normal_distribution<double> gauss_1(0.1,2.);
  std::normal_distribution<double> gauss_0(1.0,0.5);
  Qn::Stats stats_A;
  Qn::Stats stats_B;
  int nsamples = 1000;
  int nevents = 1000;
  stats_A.SetNumberOfSubSamples(nsamples);
  stats_B.SetNumberOfSubSamples(nsamples);
  Qn::Sampler sampler(nsamples,Qn::Sampler::Method::BOOTSTRAP);
  sampler.SetNumberOfEvents(nevents);
  sampler.CreateMoutofNBootstrapSamples(.2);
  auto histo1 = new TProfile("gauss1","gauss2",1,0,1);
  auto histo2 = new TProfile("gauss2","gauss2",1,0,1);

  for (int i = 0; i < nevents; ++i) {
    double a = gauss_0(generator);
    double b = gauss_1(generator);
    stats_A.Fill({{1.},a,true}, sampler.GetFillVector(i));
    stats_B.Fill({{1.},b,true}, sampler.GetFillVector(i));
    histo1->Fill(0.,a);
    histo2->Fill(0.,b);
  }
  std::cout << "histo1 " << histo1->GetBinContent(1) << " " << histo1->GetBinError(1) << std::endl;
  std::cout << "histo2 " << histo2->GetBinContent(1) << " " << histo2->GetBinError(1) << std::endl;
  auto m1 = histo1->GetBinContent(1);
  auto m2 = histo2->GetBinContent(1);
  auto err1 = histo1->GetBinError(1);
  auto err2 = histo2->GetBinError(1);
  auto errtotal = std::sqrt((err1/m2)*(err1/m2)+(m1*err2/m2/m2)*(m1*err2/m2/m2));
  std::cout << "properror " << errtotal << std::endl;
  auto stats_C = stats_A / stats_B;
  auto histo = stats_C.SampleMeanHisto("SampleMean");
  TCanvas c1("c","c",800,600);
  c1.SetLogy();
  c1.cd();
//  histo.Scale(1/histo.GetEntries());
  histo.Draw();
  auto g1 = new TF1("g1","gaus",-1,1);
  histo.Fit(g1);
  g1->Draw("SAME");
  gStyle->SetOptFit();
  stats_C.ResetBits(Qn::Stats::CORRELATEDERRORS);
  auto mean = stats_C.Mean();
  auto err = stats_C.Error();
  TPaveText pave(0.6,0.1,0.9,0.3,"NDCBR");
  pave.AddText(TString::Format("original:  %f +- %f",mean,err));
  stats_C.SetBits(Qn::Stats::ASYMMERRORS|Qn::Stats::CORRELATEDERRORS);
  std::cout << stats_C.ErrorLo() << " - " << stats_C.ErrorHi() << std::endl;
  auto bmean = stats_C.BootstrapMean();
  stats_C.ResetBits(Qn::Stats::ASYMMERRORS);
  auto berr = stats_C.Error();
  pave.AddText(TString::Format("bootstrap: %f +- %f",bmean,berr));
  pave.Draw();
  c1.SaveAs("BSGaussDivision.png");
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
  stats_a.SetStatus(Qn::Stats::Status::OBSERVABLE);
  stats_b.SetStatus(Qn::Stats::Status::OBSERVABLE);
  auto stats_c = Qn::Merge(stats_a,stats_b);
  stats_c.SetBits(Qn::Stats::CORRELATEDERRORS);
  stats_d.SetBits(Qn::Stats::CORRELATEDERRORS);
  auto err_c = stats_c.Error();
  auto err_d = stats_d.Error();
  EXPECT_FLOAT_EQ(stats_d.Mean(),stats_c.Mean());
  EXPECT_FLOAT_EQ(stats_d.BootstrapMean(),stats_c.BootstrapMean());
  EXPECT_FLOAT_EQ(err_c,err_d);
}

TEST(StatsTest, GaussianAdditionWeighted) {
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
    stats_b.Fill({{2.},b,true}, sampler.GetFillVector(i));
    stats_d.Fill({{1.},a,true}, sampler.GetFillVector(i));
    stats_d.Fill({{2.},b,true}, sampler.GetFillVector(i));
  }
  stats_a.SetStatus(Qn::Stats::Status::OBSERVABLE);
  stats_b.SetStatus(Qn::Stats::Status::REFERENCE);
  auto stats_c = Qn::Merge(stats_a,stats_b);
  stats_c.SetBits(Qn::Stats::CORRELATEDERRORS);
  stats_d.SetBits(Qn::Stats::CORRELATEDERRORS);
  auto err_c = stats_c.Error();
  auto err_d = stats_d.Error();
  EXPECT_FLOAT_EQ(stats_d.Mean(),stats_c.Mean());
  EXPECT_FLOAT_EQ(stats_d.BootstrapMean(),stats_c.BootstrapMean());
  EXPECT_FLOAT_EQ(err_c,err_d);
}

TEST(StatsTest, GaussianAdditionWeightedRatio) {
  std::default_random_engine generator;
  std::normal_distribution<double> gauss_a(-0.5,1);
  std::normal_distribution<double> gauss_b(0.5,1);
  std::normal_distribution<double> gauss_den(0.0,1);
  Qn::Stats stats_a;
  Qn::Stats stats_b;
  Qn::Stats stats_d;
  Qn::Stats stats_den;
  int nsamples = 100;
  int nevents = 1000;
  stats_a.SetNumberOfSubSamples(nsamples);
  stats_b.SetNumberOfSubSamples(nsamples);
  stats_d.SetNumberOfSubSamples(nsamples);
  stats_den.SetNumberOfSubSamples(nsamples);
  Qn::Sampler sampler(nsamples,Qn::Sampler::Method::BOOTSTRAP);
  sampler.SetNumberOfEvents(nevents);
  sampler.CreateBootstrapSamples();
  for (int i = 0; i < nevents; ++i) {
    auto a = gauss_a(generator);
    auto b = gauss_b(generator);
    auto den = gauss_den(generator);
    stats_a.Fill({{},a,true}, sampler.GetFillVector(i));
    stats_b.Fill({{2.},b,true}, sampler.GetFillVector(i));
    stats_d.Fill({{},a,true}, sampler.GetFillVector(i));
    stats_d.Fill({{2.},b,true}, sampler.GetFillVector(i));
    stats_den.Fill({{},den,true}, sampler.GetFillVector(i));
  }
  stats_a.SetStatus(Qn::Stats::Status::OBSERVABLE);
  stats_b.SetStatus(Qn::Stats::Status::OBSERVABLE);
  stats_den.SetStatus(Qn::Stats::Status::REFERENCE);
  stats_d.SetStatus(Qn::Stats::Status::OBSERVABLE);
  auto ratio_merged = stats_d / stats_den;
  auto ratio_um_a = stats_a / stats_den;
  auto ratio_um_b = stats_b / stats_den;
  auto ratio_ab = Qn::Merge(ratio_um_a,ratio_um_b);
  ratio_ab.SetBits(Qn::Stats::CORRELATEDERRORS);
  ratio_merged.SetBits(Qn::Stats::CORRELATEDERRORS);
  auto err_c = ratio_ab.Error();
  auto err_d = ratio_merged.Error();
  EXPECT_FLOAT_EQ(ratio_merged.Mean(),ratio_ab.Mean());
  EXPECT_FLOAT_EQ(ratio_merged.BootstrapMean(),ratio_ab.BootstrapMean());
  EXPECT_FLOAT_EQ(err_c,err_d);
}