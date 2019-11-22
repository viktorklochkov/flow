
#include <Statistic.h>
#include <random>
#include "gtest/gtest.h"
#include "TStatistic.h"

/**
 * Test the statistical algorithm used to calculate the variance.
 */
TEST(StatisticUnitTest, SumSquareCalculation) {
  double weight = 2.0;
  // root implementation
  TStatistic tstatistic;
  // own implementation
  Qn::Statistic qnstatistic;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::poisson_distribution<> d(4);
  // text book implementation
  double sum_w = 0.; /// sum of weights
  double sum_wx = 0.; /// sum of weights * values;
  std::vector<double> vector_x; /// values for second iteration of calculation
  std::vector<double> vector_w; /// values for second iteration of calculation
  for (int n = 0; n < 10000; ++n) {
    auto value = d(gen);
    sum_w += weight;
    vector_x.push_back(value);
    vector_w.push_back(weight);
    sum_wx += weight*value;
    tstatistic.Fill(value, weight);
    qnstatistic.Fill(value, weight);
  }
  // textbook formula
  // second iteration
  double s2 = 0.;
  for (unsigned int i = 0; i< vector_x.size();++i) {
    auto x = vector_x[i];
    auto w = vector_w[i];
    s2 += w*(x - sum_wx/sum_w)*(x - sum_wx/sum_w);

  }
  // standard root to merged root result
  EXPECT_FLOAT_EQ(tstatistic.GetM2(), qnstatistic.SumSq());
  // textbook to own result
  EXPECT_FLOAT_EQ(s2, qnstatistic.SumSq());
  // textbook to own result
  EXPECT_FLOAT_EQ(s2, tstatistic.GetM2());

}

/**
 * Checks if the merging algorithm for the sum of (values*weights)^2 is working as expected with different weights for
 * different partitions of the complete sample.
 */
TEST(StatisticUnitTest, Merging) {
  // own implementation
  Qn::Statistic statistic;
  Qn::Statistic statistic_a;
  Qn::Statistic statistic_b;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::poisson_distribution<> d(4);
  // text book implementation
  double sum_w = 0.; /// sum of weights
  double sum_wx = 0.; /// sum of weights * values;
  std::vector<double> vector_x; /// values for second iteration of calculation
  std::vector<double> vector_w; /// weights for second iteration of calculation
  // partition a
  auto weight = 2.;
  for (int n = 0; n < 10000; ++n) {
    auto value = d(gen);
    sum_wx += weight*value;
    sum_w += weight;
    vector_x.push_back(value);
    vector_w.push_back(weight);
    statistic.Fill(value, weight);
    statistic_a.Fill(value, weight);
  }
  // partition b
  weight = 3.;
  for (int n = 0; n < 10000; ++n) {
    auto value = d(gen);
    sum_wx += weight*value;
    sum_w += weight;
    vector_x.push_back(value);
    vector_w.push_back(weight);
    statistic.Fill(value, weight);
    statistic_b.Fill(value, weight);
  }
  // merging of own implementation
  auto statistic_merged = Qn::Statistic::Merge(statistic_a, statistic_b);
  // textbook formula
  // second iteration
  double s2 = 0.;
  for (unsigned int i = 0; i < vector_w.size(); ++i) {
    auto x = vector_x[i];
    auto w = vector_w[i];
    s2 += w*(x - sum_wx/sum_w)*(x - sum_wx/sum_w);
  }
  // standard to merged result
  EXPECT_FLOAT_EQ(statistic.SumSq(), statistic_merged.SumSq());
  // textbook to own result
  EXPECT_FLOAT_EQ(s2, statistic.SumSq());
  // textbook to own result
  EXPECT_FLOAT_EQ(s2, statistic_merged.SumSq());
}