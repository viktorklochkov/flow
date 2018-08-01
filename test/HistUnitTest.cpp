//
// Created by Lukas Kreis on 12.07.18.
//

#include <TRandom2.h>
#include <Base/EventShape.h>
#include "gtest/gtest.h"
#include "Base/Hist.h"
#include "Base/DataContainer.h"


TEST(HistTest, basic) {

  Qn::Hist a({"test",10,0,10});
  a.Fill(9.);
  a.Fill(1.);
  a.Fill(9.1);
  a.GetBinValue(10);
}

TEST(HistTest, th1f) {

  Qn::DataContainerTH1F a({{"test",10,0,10}});
  a.InitializeEntries({"pt","pt",100,-10,10});

  TRandom2 rndm;

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 100000; ++j) {
      a.At(i).Fill( rndm.Gaus(0,1));
    }
  }
  a.At(0).SaveAs("testhist.root");

  Qn::EventShape shape("name");
  auto hist = a.At(0);
  Double_t norm = 1;
  hist.Scale(norm/hist.Integral(), "width");
  shape.FitWithSpline(hist);

  std::cout << shape.GetPercentile(0.1) << "\n";
  std::cout << shape.GetPercentile(-2.) << "\n";

}