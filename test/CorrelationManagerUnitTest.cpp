//
// Created by Lukas Kreis on 30.01.18.
//


#include <gtest/gtest.h>
#include "Correlation.h"
#include "CorrelationManager.h"
#include "EventInfo.h"

TEST(CorrelationManagerTest, AddingCorrelation) {
  auto data1 = new Qn::DataContainer<Qn::QVector>();
  for (auto &bin : *data1) {
    Qn::QVec qvec(1.0, 1.0);
    bin = Qn::QVector(Qn::QVector::Normalization::NONE, 1, 2, {{qvec, qvec, qvec, qvec}});
  }
  TTree tree;
  auto event = new Qn::EventInfoF();
  event->AddVariable("Ev1");
  event->SetToTree(tree);

  tree.Branch("Det1", &data1);
  tree.Branch("Det2", &data1);

  auto ne = 100000;

  for (int i = 0; i < ne; ++i) {
    event->SetVariable("Ev1", 0.5);
    tree.Fill();
  }
  for (int i = 0; i < ne; ++i) {
    event->SetVariable("Ev1", 1.5);
    tree.Fill();
  }
  for (int i = 0; i < ne; ++i) {
    event->SetVariable("Ev1", 2.5);
    tree.Fill();
  }

  EXPECT_EQ(3*ne, tree.GetEntries());
  std::cout << "create manager" << std::endl;
  std::shared_ptr<TTreeReader> reader(new TTreeReader(&tree));
  Qn::CorrelationManager manager(reader);
  std::cout << "add variables" << std::endl;

  manager.AddEventVariable({"Ev1", 3, 0, 3});
  manager.AddQVectors("Det1, Det2");
  manager.SetESECalibrationFile("testese.root");
  manager.AddESE("Det1",[](const std::vector<Qn::QVector> &a){return  a[0].x(1);},10);
  std::cout << "add correlation" << std::endl;
  TTreeReaderValue<float> eventvalue(*reader, "Ev1");
  manager.AddCorrelation("c1","Det1, Det2",[](std::vector<Qn::QVector> &q) { return q[0].x(1) + q[1].x(1);});
  manager.SetRefQinCorrelation("c1",{Qn::Weight::REFERENCE, Qn::Weight::REFERENCE});
  manager.AddCorrelation("avg","Det1",[](std::vector<Qn::QVector> &q) { return q[0].mag(2)/sqrt(q[0].n());}, Qn::Sampler::Resample::OFF);
  manager.SetRefQinCorrelation("avg",{Qn::Weight::OBSERVABLE});
  manager.AddCorrelation("c2","Det1, Det2",[](std::vector<Qn::QVector> &q) { return q[0].x(1) + q[1].x(1);});
  manager.ConfigureResampling(Qn::Sampler::Method::BOOTSTRAP,10);
  int events = 0;
  reader->SetEntry(0);
  std::cout << "init" << std::endl;

  manager.Initialize();
  reader->Restart();
  std::cout << "run" << std::endl;
  while (reader->Next()) {
    manager.Process();
    events++;
  }
  EXPECT_EQ(3*ne, events);
  manager.Finalize();
  auto correlation = manager.GetResult("c1");
  auto correlation2 = manager.GetResult("c2");
  for (auto &bin : correlation) {
    EXPECT_FLOAT_EQ(2.0, bin.Mean());
    EXPECT_EQ(10,bin.GetNSamples());
  }
  auto average = manager.GetResult("avg");
  for (auto &bin : average) {
    EXPECT_FLOAT_EQ(sqrt(2.0), bin.Mean());
    EXPECT_EQ(0,bin.GetNSamples());
  }
}