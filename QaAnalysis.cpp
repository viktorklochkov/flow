//
// Created by Lukas Kreis on 24.10.17.
//

#include <THnSparse.h>
#include <TCanvas.h>
#include <TH2F.h>
#include <iostream>
#include <TStyle.h>
#include <TTreeReader.h>
#include "QaAnalysis.h"

void QaAnalysis::TrackQa() {
  THnSparseF *histogram = nullptr;
  TList *list = (TList *) file_->Get("histograms");
  if (!list) {
    std::cout << "no histogram found." << std::endl;
    return;
  }
  histogram = (static_cast<THnSparseF *>(list->FindObject("trackqa")));
//  "tracks;pT;eta;phi;dcaxy;dcaz;dEdx;charge;chi2/ndf"
  trackhistograms_->Add(histogram->Projection(1, 2));
  trackhistograms_->Add(histogram->Projection(4, 3));
  trackhistograms_->Add(histogram->Projection(6, 0));
  trackhistograms_->Add(histogram->Projection(6, 1));

  TCanvas *c1 = new TCanvas("c_trackqa", "track qa");
  int canvasdivisions = (int) TMath::Ceil(TMath::Sqrt(trackhistograms_->GetSize()));
  c1->Divide(canvasdivisions, canvasdivisions);
  for (int i = 1; i < trackhistograms_->GetSize() + 1; ++i) {
    c1->cd(i);
    gStyle->SetOptStat(0);
    c1->SetLogz(true);
    trackhistograms_->At(i - 1)->Draw("COLZ");

  }
}
void QaAnalysis::EventQa() {
  TTree *tree = nullptr;
  tree = (TTree *) file_->Get("tree");
  if (!tree) return;
  TTreeReader reader(tree);
  TTreeReaderValue<Qn::DataContainerQn> qntpc(reader, "TPC_reference");
  TTreeReaderValue<Qn::DataContainerQn> qnzdca(reader, "ZDCA_reference");
  TTreeReaderValue<Qn::DataContainerQn> qnzdcc(reader, "ZDCC_reference");
  TTreeReaderValue<Qn::DataContainerQn> qnv0a(reader, "VZEROA_reference");
  TTreeReaderValue<Qn::DataContainerQn> qnv0c(reader, "VZEROC_reference");
  TTreeReaderValue<Qn::DataContainerQn> qnfmda(reader, "FMDA_reference");
  TTreeReaderValue<Qn::DataContainerQn> qnfmdc(reader, "FMDC_reference");
  TTreeReaderValue<float> centspd(reader, "CentralitySPD");
  TTreeReaderValue<float> centtpc(reader, "CentralityTPC");
  TTreeReaderValue<float> centzdc(reader, "CentralityZEMvsZDC");
  TTreeReaderValue<float> centvzero(reader, "CentralityVZERO");
  TTreeReaderValue<float> vtxz(reader, "VtxZ");
  std::string binnames =
      "signals; TPC; ZDCA; ZDCC; VZEROA; VZEROC; FMDA; FMDC; CentralitySPD; CentralityTPC; CentralityZEMvsZDC; CentralityVZERO; VtxZ; ";
  int ndim = 12;
  int nbins[12] = {50, 50, 50, 50, 50, 50, 50, 100, 100, 100, 100, 50};
  double binmin[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -6};
  double binmax[12] = {4000, 200000, 200000, 10000, 20000, 4000, 3000, 100, 100, 100, 100, 6};
  auto *histogram = new THnSparseF("signals", binnames.data(), ndim, nbins, binmin, binmax);

  while (reader.Next()) {
    double signal[12] = {(double) qntpc->GetElement(0).GetN(), (double) qnzdca->GetElement(0).GetSumOfWeights(),
                         (double) qnzdcc->GetElement(0).GetSumOfWeights(),
                         (double) qnv0a->GetElement(0).GetSumOfWeights(),
                         (double) qnv0c->GetElement(0).GetSumOfWeights(),
                         (double) qnfmda->GetElement(0).GetSumOfWeights(),
                         (double) qnfmdc->GetElement(0).GetSumOfWeights(), (double) *centspd, (double) *centtpc,
                         (double) *centzdc, (double) *centvzero, (double) *vtxz};
    histogram->Fill(signal);
  }
  eventhistograms_->Add(histogram->Projection(0, 1));
  eventhistograms_->Add(histogram->Projection(0, 2));
  eventhistograms_->Add(histogram->Projection(0, 3));
  eventhistograms_->Add(histogram->Projection(0, 4));
  eventhistograms_->Add(histogram->Projection(0, 5));
  eventhistograms_->Add(histogram->Projection(0, 6));
  eventhistograms_->Add(histogram->Projection(0, 8));
  eventhistograms_->Add(histogram->Projection(0, 11));
  eventhistograms_->Add(histogram->Projection(1, 9));
  eventhistograms_->Add(histogram->Projection(2, 9));
  eventhistograms_->Add(histogram->Projection(3, 10));
  eventhistograms_->Add(histogram->Projection(4, 10));
  eventhistograms_->Add(histogram->Projection(7));
  eventhistograms_->Add(histogram->Projection(8));
  eventhistograms_->Add(histogram->Projection(9));
  eventhistograms_->Add(histogram->Projection(10));

  TCanvas *c1 = new TCanvas("c_eventqa", "event qa");
  int canvasdivisions = (int) TMath::Ceil(TMath::Sqrt(eventhistograms_->GetSize()));
  c1->Divide(canvasdivisions, canvasdivisions);
  for (int i = 1; i < eventhistograms_->GetSize() + 1; ++i) {
    c1->cd(i);
    gStyle->SetOptStat(0);
    eventhistograms_->At(i - 1)->Draw("COLZ");

  }
}