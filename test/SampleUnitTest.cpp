//
// Created by Lukas Kreis on 18.04.18.
//

#include <gtest/gtest.h>

#include "TRandom3.h"
#include "SubSamples.h"


TEST(SampleUnitTest, create) {
  TRandom3 rndm;
  Qn::SubSamples a(10);
  Qn::SubSamples b(10);
  auto c = Qn::SubSamples::MergeNormal(a,b);
  EXPECT_EQ(std::distance(c.begin(),c.end()),std::distance(a.begin(),a.end()));
}