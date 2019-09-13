
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

TEST(StatsUnitTest, Merging) {
  // Check consistency of merging the statistical errors for the case when no prior operations have been performed
  // and the moments of the distributions can be used with merging using gaussian error propagation.
  Qn::Stats noop_a;
  Qn::Stats noop_b;

  Qn::Stats op_a;
  Qn::Stats op_b;

  Qn::Stats truth;

  op_a.ResetBits(Qn::Stats::CORRELATEDERRORS);
  op_b.ResetBits(Qn::Stats::CORRELATEDERRORS);
  noop_a.ResetBits(Qn::Stats::CORRELATEDERRORS);
  noop_b.ResetBits(Qn::Stats::CORRELATEDERRORS);
  truth.ResetBits(Qn::Stats::CORRELATEDERRORS);


  std::random_device rd;
  std::mt19937 gen(rd());
  std::normal_distribution<> d(1., 0.5);

// partition a
  auto weight = 1.;
  for (int n = 0; n < 10000; ++n) {
    double value = d(gen);
    op_a.Fill({value, true, weight}, {});
    noop_a.Fill({value, true, weight}, {});
    truth.Fill({value, true, weight}, {});
  }
// partition b
  weight = 1.;
  for (int n = 0; n < 10000; ++n) {
    double value = d(gen);
    op_b.Fill({value, true, weight}, {});
    noop_b.Fill({value, true, weight}, {});
    truth.Fill({value, true, weight}, {});

  }
// merging of own implementation
  op_a.CalculateMeanAndError();
  op_b.CalculateMeanAndError();

  auto op_merged = Qn::MergeBins(op_a,op_b);
  auto noop_merged = Qn::MergeBins(noop_a,noop_b);

  EXPECT_FLOAT_EQ(op_merged.Mean(), noop_merged.Mean());
  EXPECT_FLOAT_EQ(op_merged.Mean(),truth.Mean());
  EXPECT_FLOAT_EQ(noop_merged.Mean(), truth.Mean());

  EXPECT_NEAR(op_merged.MeanError(), noop_merged.MeanError(),truth.MeanError()*0.001);
  EXPECT_NEAR(op_merged.MeanError(),truth.MeanError(),truth.MeanError()*0.001);
  EXPECT_NEAR(noop_merged.MeanError(), truth.MeanError(),truth.MeanError()*0.001);

}