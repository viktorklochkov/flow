//
// Created by Lukas Kreis on 30.01.18.
//


#include <gtest/gtest.h>
#include "Correlation.h"
#include "DataContainer.h"

TEST(CorrelationTest, ConfigSameDetSameAxis) {
  auto lambda = [](const std::vector<Qn::QVectorPtr>& q) {return q[0].x(1) + q[1].x(1);};
  Qn::Correlation correlation("test",{"A","A","B"},lambda,{Qn::Weight::OBSERVABLE, Qn::Weight::REFERENCE});
  auto mappy = new std::map<std::string, Qn::DataContainerQVector*>;
  auto conta = new Qn::DataContainerQVector();
  conta->AddAxes({{"a",10,0,10}});
  mappy->emplace("A",conta);
  auto contb = new Qn::DataContainerQVector();
  contb->AddAxes({{"b",10,0,10}});
  mappy->emplace("B",contb);
  std::vector<Qn::AxisF> eventaxes = {{"Ev",10,0,10}};
  correlation.Configure(mappy, eventaxes);
  auto axes = correlation.GetResult().GetAxes();
  EXPECT_STREQ(axes[0].Name().data(), "Ev");
  EXPECT_STREQ(axes[1].Name().data(), "0_A_a");
  EXPECT_STREQ(axes[2].Name().data(), "1_A_a");
  EXPECT_STREQ(axes[3].Name().data(), "b");
  delete conta;
  delete mappy;
}

TEST(CorrelationTest, ConfigSameAxis) {
  auto lambda = [](const std::vector<Qn::QVectorPtr>& q) {return q[0].x(1) + q[1].x(1);};
  Qn::Correlation correlation("test",{"A","B"},lambda,{Qn::Weight::OBSERVABLE, Qn::Weight::REFERENCE});
  auto mappy = new std::map<std::string, Qn::DataContainerQVector*>;
  auto conta = new Qn::DataContainerQVector();
  conta->AddAxes({{"a",10,0,10}});
  auto contb = new Qn::DataContainerQVector();
  contb->AddAxes({{"a",10,0,10}});
  mappy->emplace("A",conta);
  mappy->emplace("B",contb);
  std::vector<Qn::AxisF> eventaxes = {{"Ev",10,0,10}};
  correlation.Configure(mappy, eventaxes);
  auto axes = correlation.GetResult().GetAxes();
  EXPECT_STREQ(axes[0].Name().data(), "Ev");
  EXPECT_STREQ(axes[1].Name().data(), "A_a");
  EXPECT_STREQ(axes[2].Name().data(), "B_a");
  delete conta;
  delete contb;
  delete mappy;
}

TEST(CorrelationTest, ConfigDifferentAxes) {
  auto lambda = [](const std::vector<Qn::QVectorPtr>& q) {return q[0].x(1) + q[1].x(1);};
  Qn::Correlation correlation("test",{"A","B"},lambda,{Qn::Weight::OBSERVABLE, Qn::Weight::REFERENCE});
  auto mappy = new std::map<std::string, Qn::DataContainerQVector*>;
  auto conta = new Qn::DataContainerQVector();
  conta->AddAxes({{"a",10,0,10}});
  auto contb = new Qn::DataContainerQVector();
  contb->AddAxes({{"b",10,0,10}});
  mappy->emplace("A",conta);
  mappy->emplace("B",contb);
  std::vector<Qn::AxisF> eventaxes = {{"Ev",10,0,10}};
  correlation.Configure(mappy, eventaxes);
  auto axes = correlation.GetResult().GetAxes();
  EXPECT_STREQ(axes[0].Name().data(), "Ev");
  EXPECT_STREQ(axes[1].Name().data(), "a");
  EXPECT_STREQ(axes[2].Name().data(), "b");
  delete conta;
  delete contb;
  delete mappy;
}