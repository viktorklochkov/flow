#include <iostream>
#include <vector>
#include <random>
#include <TSystem.h>
#include <TROOT.h>
#include <complex>
#include "DataContainer.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "Task.h"
#include "CorrectionInterface.h"
#include "Correlation.h"
#include "SimpleTask.h"
#include "statistics.h"
int main(int argc, char **argv) {
  ROOT::EnableImplicitMT(2);
//  Qn::Task task(argv[1], argv[2],"DstTree");
//  task.Run();

//  auto set = [](float &element) { element = 1; };
//  auto add = [](float a, float b) { return a + b; };
//  auto multiply = [](float a, float b) { return a * b; };
//
//  std::vector<Qn::Axis> axesa;
//  axesa.emplace_back("one", 2, 0, 2, 1);
//  axesa.emplace_back("two", 2, 0, 2, 2);
//
//  std::vector<Qn::Axis> axesb;
//  axesb.emplace_back("one", 2, 0, 2, 1);
//  axesb.emplace_back("two", 2, 0, 2, 2);
//
//  std::vector<Qn::Axis> proj;
////  proj.emplace_back("one",2,0,2,1);
//
//  std::vector<Qn::Axis> projb;
//
//  std::vector<Qn::Axis> cent;
//  cent.emplace_back("cent", 2, 0, 2, 1);
//
//  Qn::DataContainerF data;
//  data.AddAxes(axesa);
//  data.CallOnElement((std::vector<long>) {0, 0}, set);
//  data.CallOnElement((std::vector<long>) {0, 1}, set);
//  data.CallOnElement((std::vector<long>) {1, 1}, set);
//  data.CallOnElement((std::vector<long>) {1, 0}, set);
//
//  std::vector<long> eventindex = {1};
//  std::vector<float> centvarsb = {0};

//  auto test = data.Add(data, add);
//  auto test2 = test.Map([](float a) { return sqrt(a); });
//
//  auto correlation = Qn::CreateCorrelation(data, data, proj, proj, [](float a, float b) { return a + b; }, cent);
//  auto projdata = data.Projection(proj, add);
//  auto projdatab = data.Projection(projb, add);
//  Qn::FillCorrelation(correlation, projdata, projdata, multiply, eventindex);
//  eventindex = {0};
//  Qn::FillCorrelation(correlation, projdata, projdata, multiply, eventindex);

  SimpleTask st("/Users/lukas/phd/analysis/flow/cmake-build-debug/outlist", "tree");
  st.Run();
//
//  std::vector<float> makemean= {1.0,2};
//  auto mean = Qn::Stats::Mean(makemean);
//  auto rms = Qn::Stats::Error(makemean);

  Qn::QVector a(Qn::QVector::Normalization::NOCALIB,2,2,{{{1,1},{1,1},{1,1},{1,1}}});
  a = a.Normal(Qn::QVector::Normalization::QOVERM);
  Qn::DataContainerQVector data;
  std::vector<Qn::Axis> axes;
  axes.emplace_back("axis1",2,0,1,1);
  axes.emplace_back("axis2",2,0,1,1);
  data.AddAxes(axes);
  for (auto &bin : data) {
    bin = a;
  }
  auto projection = data.Projection([](Qn::QVector &a, Qn::QVector &b){return a + b;});



  return 0;

}