
#include "EventInfo.h"
#include "CorrelationManager.h"

int main() {
  using namespace Qn;
  auto data1 = new Qn::DataContainer<Qn::QVector>();
  data1->AddAxis({"test",10,0,10});
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

  auto ne = 10000;

  for (int i = 0; i < ne/2; ++i) {
    event->SetVariable("Ev1", 0.5);
    for (auto &bin : *data1) {
      Qn::QVec qvec(1.0, 1.0);
      bin = Qn::QVector(Qn::QVector::Normalization::NONE, 1, 2, {{qvec, qvec, qvec, qvec}});
    }
    tree.Fill();
  }
  for (int i = ne/2; i < ne; ++i) {
    event->SetVariable("Ev1", 0.5);
    for (auto &bin : *data1) {
      Qn::QVec qvec(2.0, 2.0);
      bin = Qn::QVector(Qn::QVector::Normalization::NONE, 1, 2, {{qvec, qvec, qvec, qvec}});
    }
    tree.Fill();
  }

  std::cout << "create manager" << std::endl;
  std::shared_ptr<TTreeReader> reader(new TTreeReader(&tree));
  Qn::CorrelationManager manager(reader);
  std::cout << "add variables" << std::endl;

  manager.EventAxis({"Ev1", 1, 0, 2});
//  manager.Add({"Det1", "Det2"});
//  manager.SetESECalibrationFile("testese.root");
//  manager.EventShape("Det1",[](const std::vector<Qn::QVector> &a){return  a[0].x(1);},10);
  std::cout << "add correlation" << std::endl;
  manager.Correlation("c1",
                      {"Det1", "Det2"},
                      [](QVectors q) { return q[0].y(1) + q[1].x(1); },
                      {Weight::REFERENCE, Weight::REFERENCE});
  manager.Correlation("avg",
                      {"Det1"},
                      [](QVectors q) { return q[0].mag(2)/sqrt(q[0].n()); },
                      {Weight::OBSERVABLE},
                      Sampler::Resample::OFF);
  manager.Correlation("c2",
                      {"Det1", "Det2"},
                      [](QVectors q) { return q[0].x(1) + q[1].x(1); },
                      {Weight::OBSERVABLE, Weight::REFERENCE});
  manager.Resampling(Qn::Sampler::Method::BOOTSTRAP, 100);
  reader->SetEntry(0);
  std::cout << "run" << std::endl;
  manager.Run();
}