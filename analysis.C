#include "TFile.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTree.h"
#include <iostream>
#include "DataContainer.h"

void analysis() {
  auto file = TFile::Open("output.root");
  auto tree = (TTree*) file->Get("qn_tree");
  TTreeReader reader(tree);
  TTreeReaderValue<Qn::DataContainerTest> data(reader, "TPC");

  while(reader.Next()) {
    data->GetElement((std::vector<float>){0.0,0.0});
    std::cout << "test" << std::endl;
  }
}
