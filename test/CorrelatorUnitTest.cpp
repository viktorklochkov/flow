//
// Created by Lukas Kreis on 19.04.18.
//

#include <gtest/gtest.h>
#include "Correlator.h"

//TEST(CorrelatorUnitTest, Basic) {
//
//  int nevents = 99;
//  int nsamples = 10;
//  Qn::DataContainer<Qn::QVector> container_a;
//  container_a.AddAxes({{"a1", 2, 0, 10}, {"a2", 2, 0, 10}});
//  for (auto &bin : container_a) {
//    Qn::QVec qvec(1.0, 1.0);
//    std::vector<Qn::QVec> vecarray = {{qvec, qvec, qvec, qvec}};
//    bin = Qn::QVector(Qn::QVector::Normalization::QOVERM, 1, 1, vecarray);
//  }
//  Qn::DataContainer<Qn::QVector> container_b(container_a);
////  container_b = container_b.Projection({"a1"},[](const Qn::QVector &a, const Qn::QVector &b) {return a + b;});
//  std::vector<Qn::DataContainer<Qn::QVector>> vector{container_a, container_b};
//  std::vector<Qn::Axis> axes = {{"eva1", 2, 0, 10}};
//  std::vector<std::string> names = {"DET1", "DET2"};
//  auto lambda = [](std::vector<Qn::QVector> &a) { return a[0].x(1) + a[1].x(1); };
//  Qn::Correlator correlator(names, lambda);
//  correlator.ConfigureCorrelation(vector,axes);
//  correlator.ConfigureSampler(Qn::Sampler::Method::SUBSAMPLING, nsamples);
//  correlator.BuildSamples(nevents);
//  for (int i = 0; i < nevents; ++i) {
//  correlator.FillCorrelation(vector,{0},i);
//  }
//  auto test = correlator.GetResult();
//  int i = 0;
//  for (auto & bin : test) {
//    if (i <16) EXPECT_EQ(2.0,bin.Mean());
//    else EXPECT_EQ(0,bin.Mean());
//    i++;
//  }
//  EXPECT_EQ(test.size(),32);
//}
//
//TEST(CorrelatorUnitTest, AutoCorrelation) {
//  int nevents = 99;
//  int nsamples = 10;
//  Qn::DataContainer<Qn::QVector> container_a;
//  container_a.AddAxes({{"a1", 2, 0, 10}});
//  for (auto &bin : container_a) {
//    Qn::QVec qvec(1.0, 1.0);
//    std::vector<Qn::QVec> vecarray = {{qvec, qvec, qvec, qvec}};
//    bin = Qn::QVector(Qn::QVector::Normalization::QOVERM, 1, 1, vecarray);
//  }
//  Qn::DataContainer<Qn::QVector> container_b(container_a);
//  std::vector<Qn::DataContainer<Qn::QVector>> vector{container_a, container_b};
//  std::vector<Qn::Axis> axes = {{"eva1", 2, 0, 10}};
//  std::vector<std::string> names = {"DET1", "DET1"};
//  auto lambda = [](std::vector<Qn::QVector> &a) { return a[0].x(1) + a[1].x(1); };
//  Qn::Correlator correlator(names, lambda);
//  correlator.ConfigureCorrelation(vector,axes);
//  correlator.ConfigureSampler(Qn::Sampler::Method::SUBSAMPLING, nsamples);
//  correlator.BuildSamples(nevents);
//  correlator.FindAutoCorrelations();
//  correlator.FillCorrelation(vector,{1},1);
//  correlator.RemoveAutoCorrelation();
//}
//
//TEST(CorrelatorUnitTest, OneDataContainerOnly) {
//  int nevents = 100;
//  Qn::DataContainer<Qn::QVector> container_a;
//  for (auto &bin : container_a) {
//    Qn::QVec qvec(1.0, 1.0);
//    std::vector<Qn::QVec> vecarray = {{qvec, qvec, qvec, qvec}};
//    bin = Qn::QVector(Qn::QVector::Normalization::QOVERSQRTM, 1, 1, vecarray);
//  }
//  std::vector<Qn::DataContainer<Qn::QVector>> vector{container_a};
//  std::vector<Qn::Axis> axes = {{"eva1", 3, 0, 10}};
//  std::vector<std::string> names = {"DET1", "DET2"};
//  auto lambda = [](std::vector<Qn::QVector> &a) { return a[0].x(1) * a[0].x(1); };
//  Qn::Correlator correlator(names, lambda);
//  correlator.ConfigureCorrelation(vector,axes);
//  correlator.ConfigureSampler(Qn::Sampler::Method::NONE, 1);
//  correlator.BuildSamples(nevents);
//
//  correlator.FillCorrelation(vector,{0},0);
//  for (auto &bin : vector.at(0)) {
//    Qn::QVec qvec(2.0, 2.0);
//    std::vector<Qn::QVec> vecarray = {{qvec, qvec, qvec, qvec}};
//    bin = Qn::QVector(Qn::QVector::Normalization::QOVERSQRTM, 1, 1, vecarray);
//  }
//  correlator.FillCorrelation(vector,{1},0);
//  for (auto &bin : vector.at(0)) {
//    Qn::QVec qvec(3.0, 3.0);
//    std::vector<Qn::QVec> vecarray = {{qvec, qvec, qvec, qvec}};
//    bin = Qn::QVector(Qn::QVector::Normalization::QOVERSQRTM, 1, 1, vecarray);
//  }
//  correlator.FillCorrelation(vector,{2},0);
//
//
//  auto test = correlator.GetResult();
//  EXPECT_EQ(1.0,test.At(0).Mean());
//  EXPECT_EQ(4.0,test.At(1).Mean());
//  EXPECT_EQ(9.0,test.At(2).Mean());
//  EXPECT_EQ(test.size(),3);
//}