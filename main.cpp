#include <iostream>
#include <vector>
#include <random>
#include <TSystem.h>
#include <TROOT.h>
#include <array>
#include "Base/DataContainer.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "Correlation/Correlation.h"
#include "SimpleTask.h"
#include "Base/Stats.h"
#include "TestTask.h"
#include <iomanip>
#include <chrono>

int main(int argc, char **argv) {
  auto start = std::chrono::system_clock::now();
  ROOT::EnableImplicitMT(2);
  if (strcmp(argv[3], "correct")==0) {
    Qn::TestTask task(argv[1], argv[2], "DstTree");
    task.Run();
  }
  if (strcmp(argv[3], "analysis")==0) {
    SimpleTask st(argv[1], "tree");
    st.Run();
  }
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  std::cout << "elapsed time: " << elapsed_seconds.count() << " s\n";
  return 0;
}
