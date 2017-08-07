#include <iostream>
#include <vector>
#include <random>
#include <TSystem.h>
#include "DataContainer.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "Task.h"
#include "CorrectionInterface.h"
using DATA = std::vector<Qn::DataVector>;

int main(int argc, char **argv) {
//  Qn::Task task(argv[1], argv[2]);
//  task.Run();
  std::mt19937 eng{42};
  std::uniform_real_distribution<float> rnd{0, 2 * M_PI};
  std::uniform_real_distribution<float> rndcent{0, 100};
  std::map<int, Qn::DataContainerDataVector> vectordata;
  std::map<int, std::unique_ptr<Qn::DataContainerQn>> vectorqndata;
  for (int ii = 0; ii < 2; ++ii) {
    Qn::DataContainerDataVector data;
    std::unique_ptr<Qn::DataContainerQn> qndata(new Qn::DataContainerQn);
    Qn::Axis axis("test", 10, 0, 10);
    data.AddAxis(axis);
    qndata->AddAxis(axis);
    for (auto &element : data) {
      DATA datavectors;
      for (int i = 0; i < 10; ++i) {
        Qn::DataVector a(rnd(eng), 1);
        datavectors.push_back(a);
      }
      element = datavectors;
    }
    vectorqndata.insert(std::pair<int, std::unique_ptr<Qn::DataContainerQn>>(ii, std::move(qndata)));
    vectordata.insert(std::pair<int, Qn::DataContainerDataVector>(ii, data));
  }
  QnCorrectionsManager manager;
  Qn::Internal::AddDetectorToFramework(manager, Qn::DetectorType::Track, vectordata);
  manager.SetShouldFillQAHistograms();
  manager.SetShouldFillOutputHistograms();
  manager.InitializeQnCorrectionsFramework();
  manager.SetCurrentProcessListName("test");
  TTree tree("tree", "tree");
  Qn::Internal::SaveToTree(tree, vectorqndata);
  Qn::EventInfoF eventinfo;
  eventinfo.AddVariable("centTPC");
  eventinfo.AddVariable("centVZERO");
  eventinfo.SetToTree(tree);

#pragma omp parallel for
  for (int i = 0; i < 100; ++i) {
    manager.ClearEvent();
    vectorqndata[0]->ClearData();
    vectordata.clear();
    eventinfo.Reset();

    for (int ii = 0; ii < 2; ++ii) {
      Qn::DataContainerDataVector data;
      Qn::Axis axis("test", 10, 0, 10);
      data.AddAxis(axis);
      for (auto &element : data) {
        DATA datavectors;
        for (int j = 0; j < 10; ++j) {
          Qn::DataVector a(rnd(eng), 1);
          datavectors.push_back(a);
        }
        element = datavectors;
      }
      vectordata.insert(std::pair<int, Qn::DataContainerDataVector>(ii, data));
    }
    Qn::Internal::FillDataToFramework(manager, vectordata);
    float *a = manager.GetDataContainer();
    a[1] = rndcent(eng);
    eventinfo.SetVariable("centTPC",a[1]);
    eventinfo.SetVariable("centVZERO",a[1]);
    manager.ProcessEvent();
    Qn::Internal::GetQnFromFramework(manager, vectorqndata);
    tree.Fill();

  }
  manager.FinalizeQnCorrectionsFramework();
  TFile f("test.root", "RECREATE");
  f.cd();
  manager.GetQAHistogramsList()->Write(manager.GetQAHistogramsList()->GetName(), TObject::kSingleKey);
  manager.GetOutputHistogramsList()->Write(manager.GetOutputHistogramsList()->GetName(), TObject::kSingleKey);
  tree.Write("tree");
  f.Close();

  eventinfo.Clear();
  eventinfo.AddVariable("centTPC");
  eventinfo.AddVariable("centVZERO");

  TFile ff("test.root","READ");
  auto treetest = (TTree*) ff.Get("tree");
  eventinfo.AttachToTree(*treetest);
  for (Int_t i = 0; i < 10; ++i) {
    treetest->GetEvent(i);
    auto cent = eventinfo.GetVariable("centVZERO");
    if (cent.second) std::cout << cent.first << std::endl;
  }

  return 0;
}