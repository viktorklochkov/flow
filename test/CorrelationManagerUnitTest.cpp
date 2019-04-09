//
// Created by Lukas Kreis on 30.01.18.
//


#include <gtest/gtest.h>
#include "StatsResult.h"
#include "CorrelationManager.h"
#include "EventInfo.h"

TEST(CorrelationManagerTest, AddingCorrelation) {
  using namespace Qn;
  auto data1 = new Qn::DataContainer<Qn::QVector>();
//  data1->AddAxis({"test",10,0,10});
  for (auto &bin : *data1) {
    Qn::QVec qvec(1.0, 1.0);
    bin = Qn::QVector(Qn::QVector::Normalization::NONE, 1, 2, {{qvec, qvec, qvec, qvec}});
  }
  TFile testtree("testtree.root", "RECREATE");
  TTree tree("tree", "tree");
  auto event = new Qn::EventInfoF();
  event->AddVariable("Ev1");
  event->SetToTree(tree);

  tree.Branch("Det1", &data1);
  tree.Branch("Det2", &data1);

  auto ne = 10000;
  int counter = 0;
  for (int i = 0; i < ne/2; ++i) {
    ++counter;
    event->SetVariable("Ev1", 0.5);
    for (auto &bin : *data1) {
      Qn::QVec qvec(1.0, 1.0);
      bin = Qn::QVector(Qn::QVector::Normalization::NONE, 1, 2, {{qvec, qvec, qvec, qvec}});
    }
    tree.Fill();
  }
  for (int i = ne/2; i < ne; ++i) {
    ++counter;
    event->SetVariable("Ev1", 0.5);
    for (auto &bin : *data1) {
      Qn::QVec qvec(2.0, 2.0);
      bin = Qn::QVector(Qn::QVector::Normalization::NONE, 1, 2, {{qvec, qvec, qvec, qvec}});
    }
    tree.Fill();
  }
  tree.Write();
  testtree.Close();
  auto readtreefile = TFile::Open("testtree.root", "READ");
  auto reading = (TTree *) readtreefile->Get("tree");
  std::cout << counter << std::endl;
  EXPECT_EQ(ne, reading->GetEntries());
  std::cout << "create manager" << std::endl;
//  reading->AddFriend("ESE", "percentiles.root");
  Qn::CorrelationManager manager(reading);
  std::cout << "add variables" << std::endl;

  manager.EventAxis({"Ev1", 2, 0, 2});
  manager.SetOutputFile("correlation.root");
  manager.SetESEInputFile("/Users/lukas/phd/analysis/flow/cmake-build-debug/test/calib.root",
                          "/Users/lukas/phd/analysis/flow/cmake-build-debug/test/percentiles.root");
  manager.SetESEOutputFile("calib.root", "percentiles.root");
  manager.EventShape("Det1ESE", {"Det1"}, [](QVectors q) { return q[0].x(1); }, {"h", "h", 2, 0., 3.});
  manager.Correlation("c1",
                      {"Det1", "Det2"},
                      [](QVectors q) { return q[0].y(1) + q[1].x(1); },
                      {kObs, kObs});
//  manager.AddCorrelation("avg",
//                         {"Det1"},
//                         [](QVectors q) { return q[0].mag(2)/sqrt(q[0].n()); },
//                         {Weight::OBSERVABLE},
//                         Sampler::Resample::OFF);
//  manager.AddCorrelation("c2",
//                         {"Det1", "Det2"},
//                         [](QVectors q) { return q[0].x(1) + q[1].x(1); },
//                         {Weight::OBSERVABLE, Weight::REFERENCE});
  manager.Resampling(Qn::Sampler::Method::BOOTSTRAP, 100);
  std::cout << "run" << std::endl;
  manager.Run();
  auto correlation = manager.GetResult("c1");
  for (unsigned long i = 0; i < 10; ++i) {
    auto bin = correlation.At({0, 9, i});
    EXPECT_FLOAT_EQ(4.0, bin.Mean());
    EXPECT_EQ(100, bin.GetNSamples());
  }
  for (unsigned long i = 0; i < 10; ++i) {
    auto bin = correlation.At({0, 5, i});
    EXPECT_FLOAT_EQ(2.0, bin.Mean());
    EXPECT_EQ(100, bin.GetNSamples());
  }
//  auto average = manager.GetResult("avg");
//  for (auto &bin : average) {
//    EXPECT_FLOAT_EQ((sqrt(8) + sqrt(2))/2, bin.Mean());
//    EXPECT_EQ(0, bin.GetNSamples());
//  }
}
