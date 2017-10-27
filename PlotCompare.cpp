//
// Created by Lukas Kreis on 25.10.17.
//

#include <TCanvas.h>
#include <TStyle.h>
#include <TLegend.h>
#include <iostream>
#include <TLine.h>
#include "PlotCompare.h"
void PlotCompare::Plot(std::string name1, std::string name2) {
  std::string canvasname = std::string(hist1_->GetName()) + std::string(hist2_->GetName());
  auto *c = new TCanvas(canvasname.data(),canvasname.data(),600,800);
  gStyle->SetOptStat(0);
  c->cd();
  auto *pad1 = new TPad("pad1", "big",0.0,0.3,1.0,1.0);
  pad1->SetMargin(0.1,0.05,0.0,0.05);
  pad1->cd();
  base_->Draw("AXIS");
  SetText(*base_);
  Style(*hist1_, 1);
  Style(*hist2_, 2);
  hist1_->Draw("SAME");
  hist2_->Draw("SAME");
  base_->SetTitle("; ; Resolution");
  base_->SetMaximum(1.0);
  base_->SetMinimum(0.0001);

  auto *legend = new TLegend(.65,.75,.94,.94);
  legend->SetBorderSize(0);
  legend->SetTextFont(43);
  legend->SetTextSize(18);
  legend->SetFillStyle(0);
  legend->AddEntry(hist1_,name1.data());
  legend->AddEntry(hist2_,name2.data());
  legend->Draw();

  auto *pad2 = new TPad("pad2", "small",0.0,0.0,1.0,0.3);
  pad2->SetMargin(0.1,0.05,0.2,0.0);
  pad2->cd();
  pad2->SetFrameFillColor(0);
  pad2->SetFrameBorderMode(0);
  pad2->SetFrameFillColor(0);
  pad2->SetFrameBorderMode(0);
  pad2->SetTicks(1,0);
  TH1D* ratio = Ratio();
  SetText(*ratio_base_);
  Style(*ratio,3);
  ratio_base_->SetTitle("; Centrality; Ratio ");
  ratio_base_->Draw("AXIS");
  ratio_base_->SetMaximum(1.9999);
  ratio_base_->SetMinimum(0.0);
  ratio_base_->GetYaxis()->SetNdivisions(210,true);
  ratio->Draw("SAME");
  TLine *line = new TLine(0,1,100,1);
  line->SetLineColor(kBlack);
  line->SetLineStyle(kDashed);
  line->Draw();
  c->cd();
  pad1->Draw();
  pad2->Draw();

  c->SaveAs("comp.pdf");
}

void PlotCompare::Style(TH1D &hist,int number) {
  if (number == 1) hist.SetMarkerStyle(kFullCircle);
  if (number == 2) hist.SetMarkerStyle(kCircle);
  if (number == 3) hist.SetMarkerStyle(kDot);
  hist.SetMarkerColor(kBlack);
  hist.SetMarkerSize(0.8);
  hist.SetLineColor(kBlack);
  hist.SetLineWidth(1);
}

void SetText(TH1D* hist) {

}

TH1D* PlotCompare::Ratio() {
  auto *ratio = (TH1D*) hist1_->Clone();
  ratio->Divide(hist2_);
  return ratio;
}
void PlotCompare::SetText(TH1D &hist) {
  hist.GetXaxis()->SetLabelFont(43);
  hist.GetYaxis()->SetLabelFont(43);
  hist.GetXaxis()->SetTitleFont(43);
  hist.GetYaxis()->SetTitleFont(43);
  hist.GetXaxis()->SetTitleSize(18);
  hist.GetXaxis()->SetTitleOffset(2.5);
  hist.GetYaxis()->SetTitleOffset(1.3);
  hist.GetYaxis()->SetTitleSize(18);
  hist.GetXaxis()->SetLabelSize(14);
  hist.GetYaxis()->SetLabelSize(14);
}
