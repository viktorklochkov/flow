

#include <TFile.h>
#include "gtest/gtest.h"

#include "QAHistogram.h"
#include "VariableManager.h"

TEST(QAHistogramUnitTest, ptrtest) {

  Qn::VariableManager manager;
  manager.CreateVariable("a", 0, 1);
  manager.CreateVariable("b", 1, 1);
  auto vars = manager.GetVariableContainer();
  manager.CreateVariableOnes();
  vars[0] = 1;
  vars[1] = 1;

//  Qn::QAHisto1D  hist({manager.FindVariable("a"),manager.FindVariable("b")},TH1F("t","",10,0,10));

  Qn::QAHisto1DPtr histptr({manager.FindVariable("a"), manager.FindVariable("Ones")}, new TH1F("t", "", 10, 0, 10));

  Qn::QAHisto2DPtr hist2dptr({manager.FindVariable("a"),manager.FindVariable("a"), manager.FindVariable("Ones")}, new TH2F("t2d", "", 10, 0, 10,10,0,10));

  auto listaxis = std::make_unique<Qn::Axis>("b",std::vector<float>{0.6666900, 1.2340,4.,33.,345023.23453453000,345345.00000343});
  Qn::QAHisto2DPtr hist2dptrlist({manager.FindVariable("a"),manager.FindVariable("a"), manager.FindVariable("Ones")},
      new TH2F("t2dd", "", 10, 0, 10,10,0,10),std::move(listaxis),manager.FindVariable("b"));

  auto file = TFile::Open("qatest.root", "RECREATE");
  file->cd();
  auto list = new TList();
  list->SetName("qalist");
  histptr.AddToList(list);
  hist2dptr.AddToList(list);
  hist2dptrlist.AddToList(list);
  histptr.Fill();
  hist2dptr.Fill();
  hist2dptrlist.Fill();
  vars[0] = 2;
  vars[1] = 2;
  histptr.Fill();
  hist2dptr.Fill();
  hist2dptrlist.Fill();
  list->Write("qalist", TObject::kSingleKey);
  file->Close();
}