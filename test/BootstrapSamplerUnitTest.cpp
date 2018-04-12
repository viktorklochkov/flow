//
// Created by Lukas Kreis on 22.02.18.
//

#include <gtest/gtest.h>
#include <Correlation/BootstrapSampler.h>
#include <TH1F.h>
#include <Base/ResultContainer.h>

TEST(BootStrapSamplerTest, Constructor) {
  const int nsamples = 50;
  const int nevents = 100000;
  BootstrapSampler test(nevents, nsamples);
  test.CreateBootstrapSamples();
  std::array<int, nsamples> size_of_sample{{0}};
  for (auto vec : test.samples_) {
    for (auto b : vec) {
      for (int i = 0; i < nsamples; ++i) {
        if (b==i) size_of_sample[i] = size_of_sample[i] + 1;
      }
    }
  }
  for (int i = 0; i < nsamples; ++i) {
    EXPECT_EQ(nevents, size_of_sample[i]);
  }
}

TEST(BootStrapSamplerTest, Constructor3) {
  const int nsamples = 50;
  const int nevents = 100000;
  BootstrapSampler test(nevents, nsamples);
  test.CreateDividedBootstrapSamples(5);
  std::array<int, nsamples> size_of_sample{{0}};
  for (auto vec : test.samples_) {
    for (auto b : vec) {
      for (int i = 0; i < nsamples; ++i) {
        if (b==i) size_of_sample[i] = size_of_sample[i] + 1;
      }
    }
  }
  for (int i = 0; i < nsamples; ++i) {
    EXPECT_EQ(nevents, size_of_sample[i]);
  }
}

TEST(BootStrapSamplerTest, Constructor2) {
  BootstrapSampler test(10, 10);
  test.CreateResamples();
  int isamples = 0;
  for (auto vec : test.samples_) {
    for (auto b : vec) {
      if (b) ++isamples;
    }
  }
  EXPECT_EQ(10, isamples);
}

TEST(BootStrapSamplerTest, SubSampling) {
  int nevents = 907;
  int nsamples = 100;
  BootstrapSampler test(nevents, nsamples);
  test.CreateSubSamples();
  std::vector<int> samplesizes;
  samplesizes.resize(nsamples);
  for (int ievent = 0; ievent < nevents; ++ievent) {
    auto sample = test.GetFillVector(ievent);
    auto position = sample.at(0);
    std::cout << position << std::endl;
    samplesizes.at(position)++;
  }
  for (auto & samplesize : samplesizes) {
    std::cout << samplesize << std::endl;
    ASSERT_NEAR(samplesize,nevents/nsamples,(nevents/nsamples)-1);
  }
}

TEST(BootStrapSamplerTest, FillTest) {
  int nevents = 10000;
  int nsamples = 2;
  BootstrapSampler test(nevents, nsamples);
  test.CreateResamples();
  TH1F hist("test", "test", 2, 0, 2);
  for (int ievent = 0; ievent < nevents; ++ievent) {
    auto sample = test.GetFillVector(ievent);
    bool flag = 0;
    for (auto i : sample) {
      if (i==0 || i==1) {
        if (sample.size()==1) {
          flag = true;
          hist.Fill(i);
        }
      }
    }
    EXPECT_EQ(true, flag);
  }
  EXPECT_NEAR(0.5, hist.GetMean(), 0.1);
}

TEST(BootStrapSamplerTest, ResultContainer) {
  int nevents = 10000;
  int nsamples = 1000;
  TRandom3 rndm;
  BootstrapSampler sampler(nevents, nsamples);
  Qn::ResultContainer container("test", nsamples);
  sampler.CreateBootstrapSamples();
  for (int i = 0; i < nevents; ++i) {
    auto sample = sampler.GetFillVector(i);
    container.Fill(rndm.Gaus(0,1), sample);
  }

  EXPECT_NEAR(container.At(0).Mean(),0,0.01);
}