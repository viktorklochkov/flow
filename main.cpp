#include <iostream>
#include <vector>
#include <random>
#include "DataContainer.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "QnTask.h"
#include "Correlator.h"

int main(int argc, char **argv) {
//  std::unique_ptr<TFile> in_file(TFile::Open(argv[1], "READ"));
//  std::unique_ptr<TFile> out_file(TFile::Open(argv[2], "RECREATE"));
////  std::unique_ptr<TFile> in_calibration_file(TFile::Open(argv[3], "READ"));
////  std::unique_ptr<TFile> out_calibration_file(TFile::Open(argv[4], "RECREATE"));
////  std::unique_ptr<TFile> out_file(nullptr);
//  std::unique_ptr<TFile> in_calibration_file(nullptr);
//  std::unique_ptr<TFile> out_calibration_file(nullptr);
//  std::array<std::unique_ptr<TFile>, 4>
//      files =
//      {std::move(in_file), std::move(out_file), std::move(in_calibration_file), std::move(out_calibration_file)};
//  QnTask task(std::move(files));
//  task.Run();
//  return 0;
  int test[2] = {1, 1};
  std::unique_ptr<QnCorrectionsQnVector> qn(new QnCorrectionsQnVector("name", 2, test));
  std::unique_ptr<QnCorrectionsQnVector> qn2(new QnCorrectionsQnVector("name", 2, test));
  qn->SetQx(2, 2.0);
  qn2->SetQx(2, 3.0);
  qn->SetQy(2, 2.0);
  qn2->SetQy(2, 3.0);

  Qn::DataContainerQn datacontainer;
  Qn::Axis axis("const",{1.0,2.0,3.0});
  datacontainer.AddAxis(axis);
  std::vector<float> vec = {1.0};
  std::vector<float> vec2 = {2.0};
  datacontainer.SetElement(qn, (std::vector<float>){1.0});
  datacontainer.SetElement(qn2,  (std::vector<float>){2.0});
  Qn::DataContainerC correlationcontainer;
  correlationcontainer.AddAxis("test", {0, 1, 2});

  typedef std::tuple<const Qn::DataContainerQn &, int> qnharmonictuple;

  Qn::Correlate(correlationcontainer, qnharmonictuple(datacontainer, 2),
                qnharmonictuple(datacontainer, 2), qnharmonictuple(datacontainer, 2)
  );

  return 0;
}