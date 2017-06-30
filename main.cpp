#include <iostream>
#include <vector>
#include <random>
#include "QnDataContainer.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TFile.h"
#include "QnTask.h"

int main() {

  auto file = TFile::Open("test.root","RECREATE");

  std::vector<int> map = {1, 1};
  std::unique_ptr<QnCorrectionsQnVector> a(new QnCorrectionsQnVector("test", 2, &map[0]));
  std::unique_ptr<QnCorrectionsQnVector> a2(new QnCorrectionsQnVector("test2", 2, &map[0]));
  QnDataContainerQn *data = new QnDataContainerQn("data");
  std::vector<float> bins = {0, 1, 2};
  data->AddAxis("name", bins);
  data->AddAxis("names", bins);

  std::unique_ptr<TTree> tree(new TTree("name","title"));
  tree->Branch("event",&data);
  for (int i = 0; i < 1; i++) {
    std::vector<float> bins = {0, 1, 2};
    std::vector<float> vars = {0.5,0.5};
    data->AddVector(a, vars);
    std::vector<float> vars2 = {1.5,1.5};
    data->AddVector(a2, vars2);
    tree->Fill();
    data->ClearData();
  }

  QnTask task(std::move(tree));
  task.Run();

  file->Write();
  return 0;
}