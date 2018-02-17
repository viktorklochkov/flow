//
// Created by Lukas Kreis on 30.01.18.
//


#include <gtest/gtest.h>
#include <Base/DataContainer.h>
#include "Base/Profile.h"

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
  auto error = std::sqrt((a.Error()*a.Error()*a.Entries() + b.Error()*b.Error()*b.Entries() - 2*a.Mean()*b.Mean())/6);
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

TEST(ProfileTest, DataContainerAdding) {
  Qn::DataContainerProfile profile1;
  profile1.AddAxis({"a1", 10, 0, 10});
  for (auto &bin : profile1) {
    bin.Update(1.0);
    bin.Update(2.0);
    bin.Update(3.0);
    EXPECT_FLOAT_EQ(0.47140452,bin.Error());
  }
  Qn::DataContainerProfile profile2;
  profile2.AddAxis({"a1", 10, 0, 10});
  for (auto &bin : profile2) {
    bin.Update(4.0);
    bin.Update(5.0);
    bin.Update(6.0);
    EXPECT_FLOAT_EQ(0.47140452,bin.Error());
  }
  auto profilesum = profile1 + profile2;
  Qn::Profile a;
  a.Update(1.);
  a.Update(2.);
  a.Update(3.);
  EXPECT_FLOAT_EQ(0.47140452,a.Error());
  Qn::Profile b;
  b.Update(1.);
  b.Update(2.);
  b.Update(3.);
  EXPECT_FLOAT_EQ(0.47140452,b.Error());
  auto c = a + b;
  EXPECT_FLOAT_EQ(0.69721669,c.Error());

  for (auto &bin : profilesum) {
    EXPECT_FLOAT_EQ(21.0/6.0,bin.Mean());
  }
}

TEST(ProfileTest, AdditionSubSamples) {
  Qn::Profile a;
  a.Update(1.);
  a.Update(2.);
  a.Update(3.);
  Qn::Profile b;
  b.Update(1.);
  b.Update(2.);
  b.Update(3.);
  auto c = Qn::Add(a,b);
  Qn::Profile d;
  d.Update(1.);
  d.Update(2.);
  d.Update(3.);
  d.Update(1.);
  d.Update(2.);
  d.Update(3.);
  EXPECT_FLOAT_EQ(d.Error(),c.Error());

}

TEST(ProfileTest, ErrorCalculation) {
  Qn::Profile b;
  b.Update(1.);
  b.Update(2.);
  b.Update(3.);
  EXPECT_FLOAT_EQ(0.47140452,b.Error());
}