#include <iostream>
#include <vector>
#include <random>
#include "QnDataContainer.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TFile.h"
#include "QnTask.h"
#include "TROOT.h"

int main(int argc, char **argv) {
  std::unique_ptr<TFile> in_file(TFile::Open(argv[1], "READ"));
//  std::unique_ptr<TFile> out_file(TFile::Open(argv[2], "RECREATE"));
//  std::unique_ptr<TFile> in_calibration_file(TFile::Open(argv[3], "READ"));
//  std::unique_ptr<TFile> out_calibration_file(TFile::Open(argv[4], "RECREATE"));
  std::unique_ptr<TFile> out_file(nullptr);
  std::unique_ptr<TFile> in_calibration_file(nullptr);
  std::unique_ptr<TFile> out_calibration_file(nullptr);
  std::array<std::unique_ptr<TFile>, 4>
      files =
      {std::move(in_file), std::move(out_file), std::move(in_calibration_file), std::move(out_calibration_file)};
  QnTask task(std::move(files));
  task.Run();
  return 0;
}