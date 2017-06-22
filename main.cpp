#include <iostream>
#include <vector>
#include "QnDataContainer.h"
#include "TTree.h"
#include "TFile.h"
#include <chrono>

class Timer {
 public:
  Timer() : beg_(clock_::now()) {}
  void reset() { beg_ = clock_::now(); }
  double elapsed() const {
    return std::chrono::duration_cast<second_>
        (clock_::now() - beg_).count();
  }

 private:
  typedef std::chrono::high_resolution_clock clock_;
  typedef std::chrono::duration<double, std::ratio<1> > second_;
  std::chrono::time_point<clock_> beg_;
};

int main() {

  auto file = TFile::Open("test.root","RECREATE");

  Timer timer;
  std::vector<int> map = {1, 1};
  std::unique_ptr<QnCorrectionsQnVector> a(new QnCorrectionsQnVector("test", 2, &map[0]));
  std::unique_ptr<QnCorrectionsQnVector> a2(new QnCorrectionsQnVector("test2", 2, &map[0]));
  std::string name("data");
  QnDataContainerQn data(name);
  std::vector<float> bins = {0, 1, 2};
  data.AddAxis("name", bins);
  data.AddAxis("names", bins);

  std::vector<float> vars = {0.5, 0.5};
  data.AddVector(a, vars);
  std::vector<float> vars2 = {1.5, 1.5};
  data.AddVector(a2, vars2);

  for (auto it = data.cbegin(); it != data.cend(); ++it) {
    if (*(it)) std::cout << (*it)->GetName() << std::endl;
  }

  TTree tree("name","title");
  TBranch *branch = tree.Branch("branch",&data);
  tree.Fill();
  file->Write();
  return 0;
}