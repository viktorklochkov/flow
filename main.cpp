#include <iostream>
#include <vector>
#include "QnAnalysisVector.h"
#include "TAxis.h"
int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
  double b = 100;
  QnAnalysisVector h("name");
  std::vector<double> vector = {0,1,2,3,4,5,6,7,8,9};
  h.AddAxis("name",&vector[0],10);
  int a = h.GetAxis("name")->GetNbins();

}