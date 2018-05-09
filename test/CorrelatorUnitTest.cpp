//
// Created by Lukas Kreis on 19.04.18.
//

#include <gtest/gtest.h>
#include <Correlation/Correlator.h>

TEST(CorrelatorUnitTest, Basic) {

  int nevents = 99;
  int nsamples = 10;
  Qn::DataContainer<Qn::QVector> container_a;
  container_a.AddAxes({{"a1", 2, 0, 10}, {"a2", 2, 0, 10}});
  for (auto &bin : container_a) {
    Qn::QVec qvec(1.0, 1.0);
    std::array<Qn::QVec, 4> vecarray = {{qvec, qvec, qvec, qvec}};
    bin = Qn::QVector(Qn::QVector::Normalization::QOVERM, 1, 1, vecarray);
  }
  Qn::DataContainer<Qn::QVector> container_b(container_a);
  std::vector<Qn::DataContainer<Qn::QVector>> vector{container_a, container_b};
  std::vector<Qn::Axis> axes = {{"eva1", 1, 0, 10}};
  std::vector<std::string> names = {"DET1", "DET2"};
  auto lambda = [](std::vector<Qn::QVector> &a) { return a[0].x(1) + a[1].x(1); };
  Qn::Correlator correlator(names, lambda);
  correlator.ConfigureCorrelation(vector,axes);
  correlator.ConfigureSampler(Qn::Sampler::Method::SUBSAMPLING, nsamples);
  correlator.BuildSamples(nevents);
  for (int i = 0; i < nevents; ++i) {
  correlator.FillCorrelation(vector,{0},i);
  }
  auto test = correlator.GetResult();
  for (auto & bin : test) {
    EXPECT_EQ(2.0,bin.Mean());
  }
}

TEST(CorrelatorUnitTest, AutoCorrelation) {
  int nevents = 99;
  int nsamples = 10;
  Qn::DataContainer<Qn::QVector> container_a;
  container_a.AddAxes({{"a1", 2, 0, 10}});
  for (auto &bin : container_a) {
    Qn::QVec qvec(1.0, 1.0);
    std::array<Qn::QVec, 4> vecarray = {{qvec, qvec, qvec, qvec}};
    bin = Qn::QVector(Qn::QVector::Normalization::QOVERM, 1, 1, vecarray);
  }
  Qn::DataContainer<Qn::QVector> container_b(container_a);
  std::vector<Qn::DataContainer<Qn::QVector>> vector{container_a, container_b};
  std::vector<Qn::Axis> axes = {{"eva1", 2, 0, 10}};
  std::vector<std::string> names = {"DET1", "DET1"};
  auto lambda = [](std::vector<Qn::QVector> &a) { return a[0].x(1) + a[1].x(1); };
  Qn::Correlator correlator(names, lambda);
  correlator.ConfigureCorrelation(vector,axes);
  correlator.ConfigureSampler(Qn::Sampler::Method::SUBSAMPLING, nsamples);
  correlator.BuildSamples(nevents);
  correlator.FindAutoCorrelations();
  correlator.FillCorrelation(vector,{1},1);
  correlator.RemoveAutoCorrelation();
}