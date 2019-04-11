

#include <TFile.h>
#include "gtest/gtest.h"

#include "QAHistogram.h"
#include "VariableManager.h"


TEST(QAHistogramUnitTest, ptrtest) {

  Qn::VariableManager manager;
  manager.CreateVariable("a",0,1);
  manager.CreateVariable("b",1,1);
  auto vars = manager.GetVariableContainer();
  manager.CreateVariableOnes();
vars[0] = 1;
vars[1] = 1;

//  Qn::QAHisto1D  hist({manager.FindVariable("a"),manager.FindVariable("b")},TH1F("t","",10,0,10));

  Qn::QAHisto1DPtr histptr({manager.FindVariable("a"),manager.FindVariable("Ones")},new TH1F("t","",10,0,10));

  auto file = TFile::Open("qatest.root","RECREATE");
  file->cd();
  auto list = new TList();
  list->SetName("qalist");
  histptr.AddToList(list);
  histptr.Fill();
  vars[0] = 2;
  vars[1] = 1;
  histptr.Fill();
  list->Write();
  file->Close();
}