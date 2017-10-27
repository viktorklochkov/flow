//
// Created by Lukas Kreis on 25.10.17.
//

#ifndef FLOW_PLOTCOMPARE_H
#define FLOW_PLOTCOMPARE_H

#include <TH1D.h>
#include <TPad.h>
class PlotCompare {
 public:
  PlotCompare(TH1D *hist1, TH1D *hist2) :
  hist1_(hist1),
  hist2_(hist2),
  base_((TH1D*) hist1->Clone((std::string(hist1->GetName())+std::string("base")).data())),
  ratio_base_((TH1D*) hist1->Clone((std::string(hist1->GetName())+std::string("ratiobase")).data())) {
    base_->Reset();
    ratio_base_->Reset();
  }

  void Plot(std::string name1, std::string name2);
 private:
  void SetText(TH1D& hist);
  TH1D* Ratio();
  void Style(TH1D &hist, int num);
  TH1D *hist1_ = nullptr;
  TH1D *hist2_ = nullptr;
  TH1D *base_ = nullptr;
  TH1D *ratio_base_ = nullptr;

};

#endif //FLOW_PLOTCOMPARE_H
