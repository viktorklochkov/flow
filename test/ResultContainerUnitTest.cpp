//
// Created by Lukas Kreis on 19.03.18.
//

#include <gtest/gtest.h>
#include <Base/ResultContainer.h>
#include <TRandom3.h>
#include <TProfile.h>
#include <TCanvas.h>

TEST(ResultContainerTest, Construct) {
  std::vector<Qn::Axis> axes = {{"test", 10, 0, 10}};
  Qn::ResultContainer a("Container", axes, 2);
}

TEST(ResultContainerTest, Fill) {
  Qn::ResultContainer a("Container", 100);
  TRandom3 rndm;
  TProfile profile("test", "test", 3, 0, 3);
  TH1F hist("jtest", "test", 1000, -10, 10);
  double value = 0.;
  unsigned long subsample = 0;
  for (int i = 0; i < 100000; ++i) {
    subsample = rndm.Integer(100);
    value = rndm.Gaus(0, 1);
    a.Fill(value, {subsample});
//    profile.Fill(subsample+1.,value);
    profile.Fill(0., value);
    hist.Fill(value);
  }
  std::cout << std::endl;
  TCanvas canvas("c1", "c1", 800, 600);
  canvas.cd();
  hist.Draw();
  canvas.SaveAs("test.pdf");
  a.UseCorrelatedErrors(true);
  a.RecalculateError();
  ASSERT_NEAR(a.At(0).Mean(), profile.GetBinContent(1), 0.000000005);
  ASSERT_NEAR(a.At(0).Error(), profile.GetBinError(1), profile.GetBinError(1)*0.1);
}

TEST(ResultContainerTest, CorrelatedErrors) {
  unsigned long sample_size_N = 1000;
  unsigned long subsamples = 100;
  Qn::ResultContainer a("Container", subsamples);
  TRandom3 rndm;
  TProfile profile("test", "test", 3, 0, 3);
  TH1F hist("jtest", "test", 1000, -10, 10);
  double value = 0.;
  unsigned long subsample = 0;
  for (int i = 0; i < sample_size_N; ++i) {
    subsample = rndm.Integer(subsamples);
    value = rndm.Gaus(0, 1);
    unsigned long isample = i/(sample_size_N/subsamples);
    a.Fill(value, {isample});
//    profile.Fill(subsample+1.,value);
    profile.Fill(0., value);
    hist.Fill(value);
  }
  std::cout << std::endl;
  TCanvas canvas("c1", "c1", 800, 600);
  canvas.cd();
  hist.Draw();
  canvas.SaveAs("test.pdf");
  double uncorr_error = a.At(0).Error();
  a.UseCorrelatedErrors(true);
//  a.RecalculateError();
  ASSERT_NEAR(a.At(0).Mean(), profile.GetBinContent(1), 0.000000005);
  ASSERT_NEAR(a.At(0).Error(), uncorr_error, uncorr_error*0.001);
}