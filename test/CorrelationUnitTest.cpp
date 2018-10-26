//
// Created by Lukas Kreis on 30.01.18.
//


#include <gtest/gtest.h>
#include "Correlation.h"

//TEST(CorrelationTest, IntegratedCorrelation) {
//  Qn::DataContainer<Qn::QVector> container_a;
////  container_a.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
//  for (auto &bin : container_a) {
//    Qn::QVec qvec(1.0, 1.0);
//    std::vector<Qn::QVec> vecarray = {{qvec, qvec, qvec, qvec}};
//    bin = Qn::QVector(Qn::QVector::Normalization::NOCALIB, 1, 1, vecarray);
//  }
//  Qn::DataContainer<Qn::QVector> container_b(container_a);
//
//  std::vector<Qn::Axis> axes = {{"eva1", 10, 0, 10}};
//
//  auto lambda = [](std::vector<Qn::QVector> &a) { return a[0].x(1) + a[1].x(1); };
//  std::vector<Qn::DataContainer<Qn::QVector>> vector{container_a, container_b};
//  Qn::Correlation correlation(vector, axes, lambda);
//  correlation.Fill(vector, {1});
//  correlation.Fill(vector, {2});
//  EXPECT_FLOAT_EQ(2.0, correlation.GetCorrelation().At(2).second);
//  EXPECT_FLOAT_EQ(0.0, correlation.GetCorrelation().At(1).second);
//  EXPECT_EQ(10, correlation.GetCorrelation().size());
//}
//
//TEST(CorrelationTest, AveragingCorrelation) {
//  Qn::DataContainer<Qn::QVector> container_a;
//  for (auto &bin : container_a) {
//    Qn::QVec qvec(1.0, 1.0);
//    std::vector<Qn::QVec> vecarray = {{qvec, qvec, qvec, qvec}};
//    bin = Qn::QVector(Qn::QVector::Normalization::NOCALIB, 1, 1, vecarray);
//  }
//  std::vector<Qn::Axis> axes = {{"eva1", 10, 0, 10}};
//  auto lambda = [](std::vector<Qn::QVector> &a) { return sqrt(a[0].x(1) * a[0].x(1)); };
//  std::vector<Qn::DataContainer<Qn::QVector>> vector{container_a};
//  Qn::Correlation correlation(vector, axes, lambda);
//  correlation.Fill(vector, {1});
//  auto data = correlation.GetCorrelation();
//  EXPECT_FLOAT_EQ(1.0, data.At(1).second);
//  EXPECT_EQ(10, correlation.GetCorrelation().size());
//}
//
//TEST(CorrelationTest, DifferentialCorrelation) {
//  Qn::DataContainer<Qn::QVector> container_a;
//  int size = 10;
//  container_a.AddAxes({{"a1", size, 0, 100}, {"a2", size, 0, 100}});
//  for (auto &bin : container_a) {
//    Qn::QVec qvec(1.0, 1.0);
//    std::vector<Qn::QVec> vecarray = {{qvec, qvec, qvec, qvec}};
//    bin = Qn::QVector(Qn::QVector::Normalization::QOVERM, 1, 1, vecarray);
//  }
//  Qn::DataContainer<Qn::QVector> container_b(container_a);
//  std::vector<Qn::DataContainer<Qn::QVector>> vector{container_a, container_b};
//  std::vector<Qn::Axis> axes = {{"eva1", size, 0, 10}};
//
//  auto lambda = [](std::vector<Qn::QVector> &a) { return a[0].x(1) + a[1].x(1); };
//
//  Qn::Correlation correlation(vector, axes, lambda);
//  for (int i = 0; i < 10000; ++i) {
//    correlation.Fill(vector, {(unsigned long)(i/1000.0)});
//    EXPECT_FLOAT_EQ(2.0, correlation.GetCorrelation().At(((int)i/1000.0)*container_a.size()*container_b.size()).second);
//  }
//  EXPECT_EQ(size*size*size*size*size, correlation.GetCorrelation().size());
//}
//
//TEST(CorrelationTest, DiffPlusIntCorrelation) {
//  Qn::DataContainer<Qn::QVector> container_a;
//  container_a.AddAxes({{"a1", 2, 0, 10}, {"a2", 2, 0, 10}});
//  for (auto &bin : container_a) {
//    Qn::QVec qvec(1.0, 1.0);
//    std::vector<Qn::QVec> vecarray = {{qvec, qvec, qvec, qvec}};
//    bin = Qn::QVector(Qn::QVector::Normalization::NOCALIB, 1, 1, vecarray);
//  }
//  Qn::DataContainer<Qn::QVector> container_b;
//  for (auto &bin : container_b) {
//    Qn::QVec qvec(1.0, 1.0);
//    std::vector<Qn::QVec> vecarray = {{qvec, qvec, qvec, qvec}};
//    bin = Qn::QVector(Qn::QVector::Normalization::NOCALIB, 1, 1, vecarray);
//  }
//  std::vector<Qn::Axis> axes = {{"eva1", 10, 0, 10}};
//
//  auto lambda = [](std::vector<Qn::QVector> &a) { return a[0].x(1) + a[1].x(1); };
//  std::vector<Qn::DataContainer<Qn::QVector>> vector{container_a, container_b};
//
//  Qn::Correlation correlation(vector, axes, lambda);
//  for (unsigned long i = 0; i < 10; ++i) {
//    correlation.Fill(vector, {i});
//    EXPECT_FLOAT_EQ(2, correlation.GetCorrelation().At(4*i).second);
//  }
//  EXPECT_EQ(40, correlation.GetCorrelation().size());
//}
//
//TEST(CorrelationTest, BuildUnequalNames) {
//  Qn::DataContainer<Qn::QVector> container_a;
//  container_a.AddAxes({{"a1", 2, 0, 10}, {"a2", 2, 0, 10}});
//  for (auto &bin : container_a) {
//    Qn::QVec qvec(1.0, 1.0);
//    std::vector<Qn::QVec> vecarray = {{qvec, qvec, qvec, qvec}};
//    bin = Qn::QVector(Qn::QVector::Normalization::NOCALIB, 1, 1, vecarray);
//  }
//  Qn::DataContainer<Qn::QVector> container_b;
//  for (auto &bin : container_b) {
//    Qn::QVec qvec(1.0, 1.0);
//    std::vector<Qn::QVec> vecarray = {{qvec, qvec, qvec, qvec}};
//    bin = Qn::QVector(Qn::QVector::Normalization::NOCALIB, 1, 1, vecarray);
//  }
//  auto lambda = [](std::vector<Qn::QVector> &a) { return a[0].x(1) + a[1].x(1); };
//  std::vector<Qn::Axis> axes = {{"eva1", 10, 0, 10}};
//
//  Qn::Correlation correlation({"c1","c2"},{container_a, container_b},axes,lambda);
//
//  auto a = correlation.GetCorrelation().GetAxes();
//
//  EXPECT_STREQ(a[1].Name().data(),"0_c1_a1");
//  EXPECT_STREQ(a[2].Name().data(),"0_c1_a2");
//}
//
//TEST(CorrelationTest, BuildEqualNames) {
//  Qn::DataContainer<Qn::QVector> container_a;
//  container_a.AddAxes({{"a1", 2, 0, 10}, {"a2", 2, 0, 10}});
//  for (auto &bin : container_a) {
//    Qn::QVec qvec(1.0, 1.0);
//    std::vector<Qn::QVec> vecarray = {{qvec, qvec, qvec, qvec}};
//    bin = Qn::QVector(Qn::QVector::Normalization::NOCALIB, 1, 1, vecarray);
//  }
//  Qn::DataContainer<Qn::QVector> container_b;
//  container_b.AddAxes({{"a1", 2, 0, 10}, {"a2", 2, 0, 10}});
//  for (auto &bin : container_b) {
//    Qn::QVec qvec(1.0, 1.0);
//    std::vector<Qn::QVec> vecarray = {{qvec, qvec, qvec, qvec}};
//    bin = Qn::QVector(Qn::QVector::Normalization::NOCALIB, 1, 1, vecarray);
//  }
//  auto lambda = [](std::vector<Qn::QVector> &a) { return a[0].x(1) + a[1].x(1); };
//  std::vector<Qn::Axis> axes = {{"eva1", 10, 0, 10}};
//
//  Qn::Correlation correlation({"c","c"},{container_a, container_b},axes,lambda);
//
//  auto a = correlation.GetCorrelation().GetAxes();
//
//  EXPECT_STREQ(a[1].Name().data(),"0_c_a1");
//  EXPECT_STREQ(a[3].Name().data(),"1_c_a1");
//}