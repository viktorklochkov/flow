//
// Created by Lukas Kreis on 30.01.18.
//


#include <gtest/gtest.h>
#include "Base/Stats.h"

TEST(ProfileTest, Trivial) {
  Qn::Profile a;
  a.Update(0.0);
  EXPECT_EQ(0, a.Mean());
  EXPECT_EQ(0, a.Error());
  EXPECT_EQ(0, a.Sum());
  EXPECT_EQ(0, a.Sum2());
}

TEST(ProfileTest, Simple) {
  Qn::Profile a;
  a.Update(1.0);
  a.Update(0.0);
  a.Update(-1.0);
  auto error = sqrt(2.0/3.0)/sqrt(3.0);
  EXPECT_EQ(0, a.Mean());
  EXPECT_EQ(error, a.Error());
  EXPECT_EQ(0, a.Sum());
  EXPECT_EQ(2, a.Sum2());
  EXPECT_EQ(3, a.Entries());
}

TEST(ProfileTest, Averaging) {
  Qn::Profile a;
  a.Update(1.0);
  a.Update(0.0);
  a.Update(-1.0);
  Qn::Profile b;
  b.Update(0.5);
  b.Update(0.0);
  b.Update(-0.5);
  auto c = a + b;
  auto error = std::sqrt((a.Error()*a.Error()*a.Entries() + b.Error()*b.Error()*a.Entries() - 2*a.Mean()*b.Mean())/6);
  auto sum2 = (2.*3. + 0.5*3.)/6;
  EXPECT_EQ(0, c.Mean());
  EXPECT_EQ(error, c.Error());
  EXPECT_EQ(0, c.Sum());
  EXPECT_EQ(sum2, c.Sum2());
}

TEST(ProfileTest, Sqrt) {
  Qn::Profile a;
  a.Update(1.0);
  a.Update(0.0);
  a.Update(-1.0);
  auto c = a.Sqrt();
  EXPECT_EQ(0, c.Mean());
  EXPECT_EQ(1./2.*a.Error()/a.Mean(), c.Error());
  EXPECT_EQ(std::sqrt(std::abs(a.Sum())), c.Sum());
  EXPECT_EQ(std::sqrt(std::abs(a.Sum2())), c.Sum2());
}