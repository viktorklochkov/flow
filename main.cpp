#include <iostream>
#include <vector>
#include "QnDataContainer.h"
#include "QnCorrectionsQnVector.h"
int main() {

  std::vector<int> map = {1,1};
  QnCorrectionsQnVector a("test",2,&map[0]);
  std::string name("data");
  QnDataContainerQn data(name);
  std::vector<double> bins = {0,1,2,3,4,5,6,7,8,9};
  data.AddAxis("name",bins,9);

  std::vector<float> vars = {1};
  data.AddVector(&a,vars,"latest");

  std::vector<int> binsout = {2};

  std::cout << data.GetAxis("name")->GetName() << std::endl;

  for (auto it = data.cbegin(); it != data.cend(); ++it)
  {
    if (*(it)) std::cout << (*it)->GetName() << std::endl;
  }
  return 0;
}