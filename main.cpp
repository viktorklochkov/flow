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
  auto start = std::chrono::system_clock::now();
  auto start_c = std::chrono::system_clock::to_time_t(start);
  std::cout << "begin timestamp: " << std::put_time(std::localtime(&start_c), "%c") << "\n";

  ROOT::EnableImplicitMT(2);
  if (strcmp(argv[3], "correct") == 0) {
    Qn::Task task(argv[1], argv[2], "DstTree");
    task.Run();
  }
  if (strcmp(argv[3], "analysis") == 0) {
    SimpleTask st(argv[1], "tree");
    st.Run();
  } else {
    std::cout << "doing nothing" << "\n";
  }

  auto end = std::chrono::system_clock::now();
  auto end_c = std::chrono::system_clock::to_time_t(end);
  std::chrono::duration<double> elapsed_seconds = end - start;
  std::cout << "end timestamp: " << std::put_time(std::localtime(&end_c), "%c") << "\n";
  std::cout << "elapsed time: " << elapsed_seconds.count() << " s\n";
  return 0;
}
