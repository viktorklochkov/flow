//
// Created by Lukas Kreis on 22.02.18.
//

#include <gtest/gtest.h>
#include <Sampler.h>
#include <TH1F.h>
#include <DataContainer.h>
#include "TRandom3.h"

TEST(BootStrapSamplerTest, Constructor) {
  const int nsamples = 3;
  const int nevents = 10;
  Qn::Sampler test(nevents, nsamples);
  test.CreateBootstrapSamples();
  std::array<int, nsamples> size_of_sample{{0}};
  for (auto vec : test.GetSamples()) {
    for (auto b : vec) {
      for (int i = 0; i < nsamples; ++i) {
        if ((int) b==i) size_of_sample[i] = size_of_sample[i] + 1;
      }
    }
  }
  for (int i = 0; i < nsamples; ++i) {
    EXPECT_EQ(nevents, size_of_sample[i]);
  }
}
//
//TEST(BootStrapSamplerTest, Constructor3) {
//  const int nsamples = 50;
//  const int nevents = 100000;
//  Qn::Sampler test(nevents, nsamples);
//  test.CreateDividedBootstrapSamples(5);
//  std::array<int, nsamples> size_of_sample{{0}};
//  for (auto vec : test.GetSamples()) {
//    for (auto b : vec) {
//      for (int i = 0; i < nsamples; ++i) {
//        if ((int) b==i) size_of_sample[i] = size_of_sample[i] + 1;
//      }
//    }
//  }
//  for (int i = 0; i < nsamples; ++i) {
//    EXPECT_EQ(nevents, size_of_sample[i]);
//  }
//}
//
//TEST(BootStrapSamplerTest, Constructor2) {
//  Qn::Sampler test(10, 10);
//  test.CreateResamples();
//  int isamples = 0;
//  for (auto vec : test.GetSamples()) {
//    for (auto b : vec) {
//      if (b) ++isamples;
//    }
//  }
//  EXPECT_EQ(10, isamples);
//}
//
//TEST(BootStrapSamplerTest, SubSampling) {
//  int nevents = 907;
//  int nsamples = 100;
//  Qn::Sampler test(nevents, nsamples);
//  test.CreateSubSamples();
//  std::vector<int> samplesizes;
//  samplesizes.resize(nsamples);
//  for (int ievent = 0; ievent < nevents; ++ievent) {
//    auto sample = test.GetFillVector(ievent);
//    auto position = sample.at(0);
//    samplesizes.at(position)++;
//  }
//  for (auto &samplesize : samplesizes) {
//    std::cout << samplesize << std::endl;
//    ASSERT_NEAR(samplesize, nevents/nsamples, (nevents/nsamples) - 1);
//  }
//}
//
//TEST(BootStrapSamplerTest, Bootstraptest) {
//  TRandom3 rndm;
//  int nevents = 100000;
//  int nsamples = 10;
//  Qn::Sampler test(nevents, nsamples);
//  test.CreateSubSamples();
//  Qn::DataContainerSample a;
//  a.At(0).SetNumberOfSamples(10);
//  TH1D hdist("dist", "dist", 100, -2, 2);
//  TH1D hboot("boot", "boot", 100, -2, 2);
//  for (int i = 0; i < nevents; ++i) {
//    auto g1 = rndm.Gaus(0., 1.);
//    a.At(0).Fill(g1, test.GetFillVector(i));
//    hdist.Fill(g1);
//  }
//  for (int i = 0; i < nsamples; ++i) {
//    hboot.Fill(a.At(0).SampleMean(i));
//  }
//  auto d = hdist.Integral("width");
//  auto b = hboot.Integral("width");
//  hdist.Scale(1./d*b);
//  hboot.Scale(1./b*d);
//  TCanvas c1("c1", "c1", 800, 600);
//  c1.cd();
////  hdist.Draw();
//  hboot.Draw();
//  c1.SaveAs("boottest1.root");
//  a.At(0).CalculateCorrelatedError();
////  ASSERT_NEAR(a.At(0).Error(),a.At(0).CorrelatedError(),0.00001);
//}
//
//TEST(BootStrapSamplerTest, FillTest) {
//  int nevents = 10000;
//  int nsamples = 2;
//  Qn::Sampler test(nevents, nsamples);
//  test.CreateResamples();
//  TH1F hist("test", "test", 2, 0, 2);
//  for (int ievent = 0; ievent < nevents; ++ievent) {
//    auto sample = test.GetFillVector(ievent);
//    bool flag = 0;
//    for (auto i : sample) {
//      if (i==0 || i==1) {
//        if (sample.size()==1) {
//          flag = true;
//          hist.Fill(i);
//        }
//      }
//    }
//    EXPECT_EQ(true, flag);
//  }
//  EXPECT_NEAR(0.5, hist.GetMean(), 0.1);
//}
