#include <iostream>
#include <vector>
#include <random>
#include "QnDataContainer.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TFile.h"

int main() {

  auto file = TFile::Open("test.root","RECREATE");

  std::vector<int> map = {1, 1};
  std::unique_ptr<QnCorrectionsQnVector> a(new QnCorrectionsQnVector("test", 2, &map[0]));
  std::unique_ptr<QnCorrectionsQnVector> a2(new QnCorrectionsQnVector("test2", 2, &map[0]));
  std::string name("data");
  QnDataContainerQn *data = new QnDataContainerQn(name);
  std::vector<float> bins = {0, 1, 2};
  data->AddAxis("name", bins);
  data->AddAxis("names", bins);

//  std::vector<float> vars = {0.5, 0.5};
//  data.AddVector(a, vars);
//  std::vector<float> vars2 = {1.5, 1.5};
//  data.AddVector(a2, vars2);
//
//  for (auto it = data.cbegin(); it != data.cend(); ++it) {
//    if (*(it)) std::cout << (*it)->GetName() << std::endl;
//  }

  TTree tree("name","title");
  TBranch *branch = tree.Branch("branch",&data);
  for (int i = 0; i < 1; i++) {
    std::vector<float> bins = {0, 1, 2};
    std::vector<float> vars = {0.5,0.5};
    data->AddVector(a, vars);
    std::vector<float> vars2 = {1.5,1.5};
    data->AddVector(a2, vars2);
    tree.Fill();
    data->ClearData();
  }

  TTreeReader theReader("name", file);
  TTreeReaderValue<QnDataContainerQn> datat(theReader, "branch");
  std::vector<int> pos = {1,1};
  while(theReader.Next()) {
    std::string n("name");
    auto const & qn = datat->GetVector(pos);
    std::cout << datat->GetAxis("name").Name() << std::endl;
    std::string name = qn->GetName();
  }
  file->Write();
  return 0;
}