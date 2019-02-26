
#include <gtest/gtest.h>
#include "Product.h"
#include "DataContainer.h"
#include "QVector.h"
#include "Correlation.h"
#include "Correlator.h"

TEST(ProductTest, Trivial) {
  Qn::Product p({2.,10.}, 5, true);
  EXPECT_EQ(p.validity, true);
  EXPECT_FLOAT_EQ(p.result, 5);
  EXPECT_FLOAT_EQ(p.GetWeight(),20.);
  EXPECT_EQ(p.GetDim(),2);
}
