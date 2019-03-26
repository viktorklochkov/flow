
#include <gtest/gtest.h>
#include "Product.h"
#include "DataContainer.h"
#include "QVector.h"
#include "Correlation.h"


TEST(ProductTest, Trivial) {
  Qn::Product p(5, true,20.);
  EXPECT_EQ(p.validity, true);
  EXPECT_FLOAT_EQ(p.result, 5);
  EXPECT_FLOAT_EQ(p.weight,20.);
}
