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
  Qn::Task task(argv[1], argv[2]);
  task.Run();
  return 0;
}