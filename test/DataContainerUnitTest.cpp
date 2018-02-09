//
// Created by Lukas Kreis on 30.01.18.
//




#include <gtest/gtest.h>
#include "Base/DataContainer.h"
#include <TList.h>
#include <TFile.h>

TEST(DataContainerTest, Copy) {
  Qn::DataContainer<Qn::QVector> container;
  container.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
  Qn::DataContainer<Qn::QVector> copy(container);
  EXPECT_EQ(copy.size(), container.size());
}

TEST(DataContainerTest, AddAxes) {
  Qn::DataContainer<Qn::QVector> container;
  container.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
  EXPECT_EQ(100, container.size());
}

TEST(DataContainerTest, Projection) {
  Qn::DataContainer<float> container;
  container.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
  for (auto &bin : container) {
    bin = 1;
  }
  auto integration = container.Projection([](float a, float b) { return a + b; });
  for (const auto &bin : integration) {
    EXPECT_EQ(100, bin);
  }
  auto projection = container.Projection({{"a2", 10, 0, 10}}, [](float a, float b) { return a + b; });
  for (const auto &bin : projection) {
    EXPECT_EQ(10, bin);
  }
}

TEST(DataContainerTest, Rebin) {
  Qn::DataContainer<float> container;
  container.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
  for (auto &bin : container) {
    bin = 1;
  }
  auto rebin = container.Rebin({"a1", 5, 0, 10}, [](float a, float b) { return a + b; });
  int numberofbins = 0;
  for (const auto &bin : rebin) {
    EXPECT_EQ(2, bin);
    numberofbins++;
  }
  EXPECT_EQ(50, numberofbins);
}

TEST(DataContainerTest, Select) {
  Qn::DataContainer<float> container;
  container.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
  for (auto &bin : container) {
    bin = 1;
  }
  auto rebin = container.Select({"a1", 5, 0, 5});
  int numberofbins = 0;
  for (const auto &bin : rebin) {
    EXPECT_EQ(1, bin);
    numberofbins++;
  }
  EXPECT_EQ(50, numberofbins);
}

TEST(DataContainerTest, Addition) {
  Qn::DataContainer<float> container_a;
  container_a.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
  for (auto &bin : container_a) {
    bin = 1;
  }
  Qn::DataContainer<float> container_b;
  container_b.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
  for (auto &bin : container_b) {
    bin = 1;
  }
  auto container_sum = container_a + container_b;
  int numberofbins = 0;
  for (const auto &bin : container_sum) {
    EXPECT_EQ(2, bin);
    numberofbins++;
  }
  EXPECT_EQ(100, numberofbins);
}

TEST(DataContainerTest, Hadd) {
  auto file_a = new TFile("testa.root","RECREATE");
  file_a->cd();
  auto container_a = new Qn::DataContainerProfile();
  container_a->AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
  for (auto &bin : *container_a) {
    bin.Update(3.0);
  }
  file_a->WriteObject(container_a,"container");
  file_a->Write();
  auto file_b = new TFile("testb.root","RECREATE");
  file_b->cd();
  auto container_b = new Qn::DataContainerProfile();
  container_b->AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
  for (auto &bin : *container_b) {
    bin.Update(1.0);
  }
  file_b->WriteObject(container_b,"container");
  file_b->Write();
  system("rm test.root");
  system("hadd test.root testa.root testb.root");
  auto file_c = new TFile("test.root","OPEN");
  auto container_c = (Qn::DataContainerProfile*) file_c->Get("container");
  for (auto &bin : *container_c) {
    EXPECT_FLOAT_EQ(2.0, bin.Mean());
  }
}