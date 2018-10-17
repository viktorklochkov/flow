//
// Created by Lukas Kreis on 01.02.18.
//

#include <gtest/gtest.h>
#include "Efficiency.h"

TEST(EfficiencyTest, Trivial) {
  Qn::Efficiency eff;
  eff.Update(Qn::Efficiency::Passed::No);
  EXPECT_EQ(0, eff.GetEfficiency());
}

TEST(EfficiencyTest, Simple) {
  Qn::Efficiency eff;
  eff.Update(Qn::Efficiency::Passed::No);
  eff.Update(Qn::Efficiency::Passed::Yes);
  EXPECT_EQ(0.5, eff.GetEfficiency());
  EXPECT_FLOAT_EQ(0.35159397, eff.GetErrorUp());
  EXPECT_FLOAT_EQ(0.35159397, eff.GetErrorLow());
}

