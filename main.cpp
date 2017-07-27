#include <iostream>
#include <vector>
#include <random>
#include <TSystem.h>
#include "DataContainer.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "Task.h"
#include "Correlator.h"
#include "TList.h"

int main(int argc, char **argv) {
  // open files
//  std::shared_ptr<TFile> in_file(TFile::Open(argv[1], "READ"));
//  std::shared_ptr<TFile> out_file(TFile::Open(argv[2], "RECREATE"));
//  std::shared_ptr<TFile> out_calibration_file(TFile::Open(argv[3], "RECREATE"));
//  std::shared_ptr<TFile> in_calibration_file(TFile::Open(argv[4], "READ"));
//  std::array<std::shared_ptr<TFile>, 4> files= {in_file, out_file, out_calibration_file, in_calibration_file};
  // run task
  gSystem->cd("/Users/lukas/phd/analysis/testfiles/");
  Qn::Task task("/Users/lukas/phd/analysis/testfiles/filenames.txt", "/Users/lukas/phd/analysis/testfiles/input.root");
  task.Run();

  TFile file("test.root","RECREATE");
  TTree tree;
  auto data = new Qn::DataContainerQn();
  tree.Branch("Test", data);
  tree.Fill();
  tree.Write();
  file.Close();


  return 0;
}