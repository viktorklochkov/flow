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

  return 0;
}