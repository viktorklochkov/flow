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
  ROOT::EnableImplicitMT(2);
  Qn::Task task(argv[1], argv[2],"DstTree");
  task.Run();

  std::vector<Qn::Axis> axes;
  axes.emplace_back("one",2,0,2,1);
  axes.emplace_back("two",2,0,2,2);
  axes.emplace_back("three",2,0,2,3);
  std::vector<Qn::Axis> axesprojected;
  axesprojected.emplace_back("one",2,0,1,1);

  auto set = [](double &element){element = 1;};


  Qn::DataContainer<double> data;
  data.AddAxes(axes);
  data.CallOnElement((std::vector<long>){0,0,1},set);
  data.CallOnElement((std::vector<long>){0,1,1},set);
  data.CallOnElement((std::vector<long>){1,1,1},set);
  data.CallOnElement((std::vector<long>){1,0,0},set);
  data.CallOnElement((std::vector<long>){1,1,0},set);
  data.CallOnElement((std::vector<long>){1,0,1},set);
  data.CallOnElement((std::vector<long>){0,1,0},set);
  data.CallOnElement((std::vector<long>){0,0,0},set);

  auto add = [](double &element, double& elementa){element += elementa;};

//  data.AddElement({1,1},add,1.0);
  auto projection = data.Projection(axesprojected,add);
  auto integration = data.Projection(add);


  return 0;
}