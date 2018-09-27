#include <iostream>
#include <vector>
#include <random>
#include <TSystem.h>
#include <TROOT.h>
#include <array>
#include "CorrelationTask.h"
#include "CorrectionTask.h"
#include <iomanip>
#include <chrono>

int main(int argc, char **argv) {
  std::cout << "go" << std::endl;
  auto start = std::chrono::system_clock::now();
  if (strcmp(argv[3], "correct")==0) {
    Qn::CorrectionTask task(argv[1], argv[2], "DstTree");
    task.Run();
  }
  if (strcmp(argv[3], "analysis")==0) {
    CorrelationTask st(argv[1], "tree");
    st.Run();
  }
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  std::cout << "elapsed time: " << elapsed_seconds.count() << " s\n";
  return 0;
}
