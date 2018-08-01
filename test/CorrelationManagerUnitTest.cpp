//
// Created by Lukas Kreis on 30.01.18.
//


#include <gtest/gtest.h>
#include "Correlation/Correlation.h"
#include "Correlation/CorrelationManager.h"
#include "DifferentialCorrection/EventInfo.h"

TEST(CorrelationManagerTest, AddingCorrelation) {
  auto data1 = new Qn::DataContainer<Qn::QVector>();
  for (auto &bin : *data1) {
    Qn::QVec qvec(1.0, 1.0);
    std::array<Qn::QVec, 4> vecarray = {{qvec, qvec, qvec, qvec}};
    bin = Qn::QVector(Qn::QVector::Normalization::NOCALIB, 1, 1, vecarray);
  }
  TTree tree;
  auto event = new Qn::EventInfoF();
  event->AddVariable("Ev1");
  event->SetToTree(tree);

  tree.Branch("Det1", &data1);

  event->SetVariable("Ev1", 0.5);
  tree.Fill();

  event->Reset();
  event->SetVariable("Ev1", 1.5);
  tree.Fill();

  event->Reset();
  event->SetVariable("Ev1", 2.5);
  tree.Fill();

  EXPECT_EQ(3, tree.GetEntries());
  std::cout << "create manager" << std::endl;
  std::shared_ptr<TTreeReader> reader(new TTreeReader(&tree));
  Qn::CorrelationManager manager(reader);
  std::cout << "add variables" << std::endl;

  manager.AddEventVariable({"Ev1", 3, 0, 3});
  manager.AddDataContainer("Det1");
  std::cout << "add correlation" << std::endl;

  manager.AddCorrelation("Correlation1",
                         "Det1, Det1",
                         [](std::vector<Qn::QVector> &q) { return q[0].x(1) + q[1].x(1); },
                         10,
                         Qn::Sampler::Method::SUBSAMPLING);
  TTreeReaderValue<float> eventvalue(*reader, "Ev1");

  manager.AddCorrelation("Correlation2",
                         "Det1",
                         [](std::vector<Qn::QVector> &q) { return q[0].mag(2)/sqrt(q[0].n()); },
                         10,
                         Qn::Sampler::Method::SUBSAMPLING);

  int events = 0;
  reader->SetEntry(0);
  std::cout << "init" << std::endl;

  manager.Initialize();
  reader->Restart();
  while (reader->Next()) {
    manager.UpdateEvent();
    if (!manager.CheckEvent()) continue;
    manager.FillCorrelations();
    events++;
  }
  EXPECT_EQ(3, events);
  auto correlation = manager.GetResult("Correlation1");
  for (auto &bin : correlation) {
    EXPECT_FLOAT_EQ(2.0, bin.Mean());
  }
  auto average = manager.GetResult("Correlation2");
  for (auto &bin : correlation) {
    EXPECT_FLOAT_EQ(2.0, bin.Mean());
  }
}