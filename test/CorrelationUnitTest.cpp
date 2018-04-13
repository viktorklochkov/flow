//
// Created by Lukas Kreis on 30.01.18.
//


#include <gtest/gtest.h>
#include "Correlation/Correlation.h"

TEST(CorrelationTest, IntegratedCorrelation) {
  Qn::DataContainer<Qn::QVector> container_a;
//  container_a.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
  for (auto &bin : container_a) {
    Qn::QVec qvec(1.0, 1.0);
    std::array<Qn::QVec, 4> vecarray = {{qvec, qvec, qvec, qvec}};
    bin = Qn::QVector(Qn::QVector::Normalization::NOCALIB, 1, 1, vecarray);
  }
  Qn::DataContainer<Qn::QVector> container_b(container_a);

  std::vector<Qn::Axis> axes = {{"eva1", 10, 0, 10}};

  auto lambda = [](std::vector<Qn::QVector> &a) { return a[0].x(1) + a[1].x(1); };
  std::vector<Qn::DataContainer<Qn::QVector>> vector{container_a, container_b};
  Qn::Correlation correlation(vector, axes, lambda);
  correlation.Fill(vector, {1});
  correlation.Fill(vector, {2});
  EXPECT_FLOAT_EQ(2.0, correlation.GetCorrelation().GetElement(1));
  EXPECT_EQ(10, correlation.GetCorrelation().size());
}

TEST(CorrelationTest, DifferentialCorrelation) {
  Qn::DataContainer<Qn::QVector> container_a;
  container_a.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
  for (auto &bin : container_a) {
    Qn::QVec qvec(1.0, 1.0);
    std::array<Qn::QVec, 4> vecarray = {{qvec, qvec, qvec, qvec}};
    bin = Qn::QVector(Qn::QVector::Normalization::QOVERM, 1, 1, vecarray);
  }
  Qn::DataContainer<Qn::QVector> container_b(container_a);
  std::vector<Qn::DataContainer<Qn::QVector>> vector{container_a, container_b};
  std::vector<Qn::Axis> axes = {{"eva1", 10, 0, 10}};

  auto lambda = [](std::vector<Qn::QVector> &a) { return a[0].x(1) + a[1].x(1); };

  Qn::Correlation correlation(vector, axes, lambda);
  for (float i = 0; i < 10000; ++i) {
    correlation.Fill(vector, {(int)(i/1000.0)});
  }
  for (auto bin : correlation.GetCorrelation()) {
    EXPECT_FLOAT_EQ(2.0, bin);
  }
  EXPECT_EQ(100000, correlation.GetCorrelation().size());
}

TEST(CorrelationTest, DiffPlusIntCorrelation) {
  Qn::DataContainer<Qn::QVector> container_a;
  container_a.AddAxes({{"a1", 100, 0, 10}, {"a2", 100, 0, 10}});
  for (auto &bin : container_a) {
    Qn::QVec qvec(1.0, 1.0);
    std::array<Qn::QVec, 4> vecarray = {{qvec, qvec, qvec, qvec}};
    bin = Qn::QVector(Qn::QVector::Normalization::NOCALIB, 1, 1, vecarray);
  }
  Qn::DataContainer<Qn::QVector> container_b;
  for (auto &bin : container_b) {
    Qn::QVec qvec(1.0, 1.0);
    std::array<Qn::QVec, 4> vecarray = {{qvec, qvec, qvec, qvec}};
    bin = Qn::QVector(Qn::QVector::Normalization::NOCALIB, 1, 1, vecarray);
  }
  std::vector<Qn::Axis> axes = {{"eva1", 10, 0, 10}};

  auto lambda = [](std::vector<Qn::QVector> &a) { return a[0].x(1) + a[1].x(1); };
  std::vector<Qn::DataContainer<Qn::QVector>> vector{container_a, container_b};

  Qn::Correlation correlation(vector, axes, lambda);
  for (int i = 0; i < 10; ++i) {
    correlation.Fill(vector, {i});
  }
  for (auto bin : correlation.GetCorrelation()) {
    EXPECT_FLOAT_EQ(2, bin);
  }
  EXPECT_EQ(100000, correlation.GetCorrelation().size());
}