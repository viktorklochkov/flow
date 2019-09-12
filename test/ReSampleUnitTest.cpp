//
// Created by Lukas Kreis on 18.04.18.
//
#include <random>
#include <TPaveText.h>
#include <TLegend.h>
#include <algorithm>

#include "gtest/gtest.h"

#include "TCanvas.h"
#include "TAxis.h"
#include "ReSamples.h"

TEST(ReSampleUnitTest, test) {

  std::random_device rd;
  std::mt19937 gen(rd());

  std::normal_distribution<> distribution(1., 1.);
  //using poissonian sampling
  std::poisson_distribution<> poisson_bootstrap(1);

  Qn::Statistic stats;
  const unsigned int nsamples = 10000;
  Qn::ReSamples samples(nsamples);
  std::vector<std::size_t> ids(nsamples);
  for (int i = 0; i < 10000; ++i) {
    for (unsigned int j = 0; j < nsamples; ++j) {
      ids[j] = poisson_bootstrap(gen);
    }
    auto value = distribution(gen);
    stats.Fill(value, 1.0);
    samples.FillPoisson({value, true, 1.0}, ids);
  }
  std::cout << stats.Mean() << std::endl;
  samples.CalculateMeans();
  auto bserror = samples.GetConfidenceInterval(stats.Mean(), Qn::ReSamples::CIMethod::pivot).Uncertainty();
  std::cout << stats.MeanError() << " " << bserror << std::endl;
}

TEST(ReSampleUnitTest, vsnsample) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::normal_distribution<> distribution(1., 1.);
  //using poissonian sampling
  std::poisson_distribution<> poisson_bootstrap(1);
  Qn::Statistic stats;
  const unsigned int nsamples = 1000;
  Qn::ReSamples samples(nsamples);
  std::vector<std::size_t> ids(nsamples);
  for (int i = 0; i < 10000; ++i) {
    for (unsigned int j = 0; j < nsamples; ++j) {
      ids[j] = poisson_bootstrap(gen);
    }
    auto value = distribution(gen);
    stats.Fill(value, 1.0);
    samples.FillPoisson({value, true, 1.0}, ids);
  }
  std::cout << stats.Mean() << std::endl;
  samples.CalculateMeans();
  auto pivot = samples.CIvsNSamples(stats.Mean(), Qn::ReSamples::CIMethod::pivot);
  auto percentile = samples.CIvsNSamples(stats.Mean(), Qn::ReSamples::CIMethod::percentile);
  auto normal = samples.CIvsNSamples(stats.Mean(), Qn::ReSamples::CIMethod::normal);
  TGraphAsymmErrors statistical(2);
  statistical.SetPoint(0, 0, stats.Mean());
  statistical.SetPointError(0, 0, 0, stats.MeanError(), stats.MeanError());
  statistical.SetPoint(1, nsamples, stats.Mean());
  statistical.SetPointError(1, 0, 0, stats.MeanError(), stats.MeanError());
  TCanvas c1("CIvsNSamples", "CI", 600, 400);
  statistical.Draw("AL3");
  statistical.SetFillColorAlpha(kBlack, 0.2);
  statistical.SetLineColorAlpha(kBlack, 0.4);
  statistical.GetYaxis()->SetRangeUser((stats.Mean() - stats.MeanError()*2), (stats.Mean() + stats.MeanError()*2));
  statistical.GetXaxis()->SetRangeUser(0., nsamples);
  statistical.SetNameTitle("", ";number of bootstrap samples; x");
  auto style = [](std::pair<TGraph *, TGraph *> &pair, int color) {
    pair.first->SetLineWidth(2);
    pair.second->SetLineWidth(2);
    pair.first->SetLineColorAlpha(color, 0.8);
    pair.second->SetLineColorAlpha(color, 0.8);
  };
  style(pivot, kRed);
  pivot.first->GetYaxis()->SetRangeUser(0.8, 1.2);
  pivot.first->Draw("L");
  pivot.second->Draw("L");
  style(percentile, kGreen + 2);
  percentile.first->Draw("L");
  percentile.second->Draw("L");
  style(normal, kBlue);
  normal.first->Draw("L");
  normal.second->Draw("L");
  TLegend legend(0.15, 0.15, 0.3, 0.25);
  legend.AddEntry(pivot.first, "Pivot", "L");
  legend.AddEntry(percentile.first, "Percentile", "L");
  legend.AddEntry(normal.first, "Normal", "L");
  legend.SetFillStyle(4000);
  legend.SetLineWidth(0);
  legend.Draw();
  c1.SaveAs("CIvsNSamples.pdf");
}

TEST(ReSampleUnitTest, scatterplot) {
  const unsigned int nsamples = 100;
  auto randombin = [](Qn::ReSamples &samples, Qn::Statistic &stats, double gauss_m) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> distribution(gauss_m, 1.);
    //using poissonian sampling
    std::poisson_distribution<> poisson_bootstrap(1);
    std::vector<std::size_t> ids(nsamples);
    for (int i = 0; i < 10000; ++i) {
      for (unsigned int j = 0; j < nsamples; ++j) {
        ids[j] = poisson_bootstrap(gen);
      }
      auto value = distribution(gen);
      stats.Fill(value, 1.0);
      samples.FillPoisson({value, true, 1.0}, ids);
    }
    std::cout << stats.Mean() << std::endl;
    samples.CalculateMeans();
  };

  Qn::ReSamples samples1(nsamples);
  Qn::Statistic stats1;
  double gaussianmean = 0.;
  randombin(samples1, stats1, gaussianmean);
  Qn::ReSamples samples2(nsamples);
  Qn::Statistic stats2;
  randombin(samples2, stats2, gaussianmean + 0.2);

  TGraph scatter(0);
  scatter.SetMarkerStyle(kFullCircle);
  scatter.SetMarkerColorAlpha(kBlack, 0.3);
  scatter.SetMarkerSize(0.2);
  samples1.ScatterGraph(scatter, 1.0, 0.1);
  samples2.ScatterGraph(scatter, 1.2, 0.1);

  TGraphAsymmErrors mean(0);
  mean.SetMarkerStyle(kFullCircle);
  mean.SetMarkerColor(kBlack);
  mean.SetLineWidth(2);
  mean.SetPoint(0, 1.0, stats1.Mean());
  mean.SetPointError(0, 0, 0, stats1.MeanError(), stats1.MeanError());
  mean.SetPoint(1, 1.2, stats2.Mean());
  mean.SetPointError(1, 0, 0, stats2.MeanError(), stats2.MeanError());

  TGraphAsymmErrors bserrors(0);
  bserrors.SetMarkerStyle(kFullCircle);
  bserrors.SetMarkerColor(kBlack);
  bserrors.SetLineWidth(2);
  bserrors.SetPoint(0, 1., stats1.Mean());
  auto error = samples1.GetConfidenceInterval(stats1.Mean(), Qn::ReSamples::CIMethod::normal).Uncertainty();
  bserrors.SetPointError(0, 0, 0, error, error);
  bserrors.SetPoint(1, 1.2, stats2.Mean());
  error = samples1.GetConfidenceInterval(stats1.Mean(), Qn::ReSamples::CIMethod::normal).Uncertainty();
  bserrors.SetPointError(1, 0, 0, error, error);

  TCanvas c1("scattergraph", "", 600, 400);
  scatter.Draw("AP");
  mean.Draw("PZ");
  bserrors.Draw("||");
  c1.SaveAs("Scattergraph.pdf");
}

TEST(ReSampleUnitTest, Bagging) {
  const unsigned int nsamples = 100;
  const unsigned int totalsize = 10000;
  const double n = 0.7;
  const unsigned int b = std::round(std::pow(totalsize, n));
  const unsigned int s = 5;
  const unsigned int r = 10;
  std::vector<double> data(totalsize);
  std::vector<unsigned int> indices(totalsize);
  std::vector<unsigned int> b_indices(b);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::normal_distribution<> distribution(1., 1.);

  for (unsigned int i = 0; i < totalsize; ++i) {
    data[i] = distribution(gen);
    indices[i] = i;
  }

  std::poisson_distribution<> poisson_bootstrap(1);
  for (unsigned int i = 0; i < s; ++i) {
    std::shuffle(indices.begin(), indices.end(), gen);
    indices = std::vector<unsigned int>(indices.begin() + b, indices.end());
    b_indices = std::vector<unsigned int>(indices.begin(), indices.begin() + b);
    //divide the disjunct data into subsamples
    Qn::ReSamples samples(r);
    std::vector<std::size_t> multiplicities(b);
    for (unsigned int l = 0; l < b; ++l) {
      multiplicities[l] = poisson_bootstrap(gen);
    }
    for (unsigned int k = 0; k < b; ++k) {
      samples.FillPoisson({data[b_indices[k]], true, 1.0}, multiplicities);
    }
//    samples.
  }
}