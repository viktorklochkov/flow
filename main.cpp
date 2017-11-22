#include <iostream>
#include <vector>
#include <random>
#include <TSystem.h>
#include <TROOT.h>
#include <array>
#include "DataContainer.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "Task.h"
#include "CorrectionInterface.h"
#include "Correlation.h"
#include "SimpleTask.h"
#include "Stats.h"
int main(int argc, char **argv) {
  ROOT::EnableImplicitMT(2);
//  Qn::Task task(argv[1], argv[2],"DstTree");
//  task.Run();


  SimpleTask st("/Users/lukas/phd/analysis/flow/cmake-build-debug/outlist", "tree");
  st.Run();
//
//  std::vector<float> makemean= {1.0,2};
//  auto mean = Qn::Stats::Mean(makemean);
//  auto rms = Qn::Stats::Error(makemean);

//  Qn::QVector a(Qn::QVector::Normalization::NOCALIB,2,2,{{{1,1}}});
//  Qn::DataContaine rF data;
  std::vector<Qn::Axis> axes;
//  std::vector<Qn::Axis> noaxes;
//  axes.emplace_back("axis1",4,0,1,1);
//  axes.emplace_back("axis2",2,0,1,1);
//  data.AddAxes(axes);
//
//  std::vector<Qn::Axis> eventaxes;
//  eventaxes.emplace_back("eventaxes",2,0,1,1);
//
//  for (auto &bin : data) {
//    bin = 1.0;
//  }
//  Qn::Axis rebin("one",4,0,1,1);
//  data.Rebin([](float &a, float &b){return a + b;},rebin);

//
//  auto projection = data.Projection([](Qn::QVector &a, Qn::QVector &b){return a + b;});
//
//  std::vector<Qn::DataContainerQVector> inputs;
//  inputs.emplace_back(projection);
//  inputs.emplace_back(projection);
//
//  Qn::Correlation correlation(inputs, eventaxes);
//  std::vector<long> eventindex = {1};
//  correlation.Fill(eventindex,[](std::vector<Qn::QVector> vec){
//    return vec.at(0).x(0) * vec.at(0).y(0);
//  });

//  auto projection = data.Projection([](Qn::QVector &a, Qn::QVector &b){return a + b;});

  Qn::QVec q(2.0,2.0);
  std::vector<Qn::QVector> vectors;
  std::array<Qn::QVec,4> qvecs = {{q,q,q,q,}};
  vectors.emplace_back(Qn::QVector::Normalization::NOCALIB,2,2,qvecs);
  vectors.emplace_back(Qn::QVector::Normalization::NOCALIB,2,2,qvecs);
  vectors.emplace_back(Qn::QVector::Normalization::NOCALIB,2,2,qvecs);

  auto result = Qn::Multiply(vectors,{2,2,2});

  return 0;

}