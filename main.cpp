#include <iostream>
#include <vector>
#include <random>
#include <TSystem.h>
#include "DataContainer.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "Task.h"
#include "CorrectionInterface.h"
using DATA = std::vector<Qn::DataVector>;

int main(int argc, char **argv) {
  Qn::Task task(argv[1], argv[2]);
  task.Run();

  return 0;
}