#include <iostream>
#include <vector>
#include <random>
#include "DataContainer.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TFile.h"
#include "QnTask.h"
#include "EventInfo.h"
#include "TROOT.h"
#include <map>

int main(int argc, char **argv) {
  std::unique_ptr<TFile> in_file(TFile::Open(argv[1], "READ"));
  std::unique_ptr<TFile> out_file(TFile::Open(argv[2], "RECREATE"));
////  std::unique_ptr<TFile> in_calibration_file(TFile::Open(argv[3], "READ"));
////  std::unique_ptr<TFile> out_calibration_file(TFile::Open(argv[4], "RECREATE"));
//  std::unique_ptr<TFile> out_file(nullptr);
  std::unique_ptr<TFile> in_calibration_file(nullptr);
  std::unique_ptr<TFile> out_calibration_file(nullptr);
  std::array<std::unique_ptr<TFile>, 4>
      files =
      {std::move(in_file), std::move(out_file), std::move(in_calibration_file), std::move(out_calibration_file)};
  QnTask task(std::move(files));
  task.Run();
//  return 0;
//  int test[2] = {1,1};
//  std::unique_ptr<QnCorrectionsQnVector> qn(new QnCorrectionsQnVector("name",2,test));
//  QnDataContainerQn datacontainer;
//  datacontainer.AddAxis("test",5,0,5);
//  QnAxis axis = datacontainer.GetAxis("test");
//  datacontainer.SetElement(qn,{4});
//  std::vector<float> t = {4};
//  const auto & a = datacontainer.GetElement(t);

  return 0;
}