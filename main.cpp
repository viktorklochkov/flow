#include <iostream>
#include <vector>
#include <random>
#include <TSystem.h>
#include <TROOT.h>
#include "DataContainer.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "Task.h"
#include "CorrectionInterface.h"
int main(int argc, char **argv) {
//  ROOT::EnableImplicitMT(2);
  Qn::Task task(argv[1], argv[2]);
  task.Run();

  auto file = new TFile("output.root","OPEN");
  auto tree = (TTree*) file->Get("tree");
  Qn::DataContainer<QnCorrectionsQnVector> *dc;
  tree->SetBranchAddress("0", &dc);
  tree->GetEvent(0);
  std::cout << dc->size() << std::endl;

  return 0;
}