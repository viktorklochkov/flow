void resolution() {

  TFile *file = TFile::Open("output.root");
  TTreeReader *reader = new TTreeReader("tree",file);

  Qn::Resolution resolution(reader);
  Qn::Axis axis("CentralityVZERO",9,0,100,1);
  resolution.AddDetector("TPC_reference","VZEROA_reference","VZEROC_reference", axis);

  while (reader->Next()) {
    resolution.Process();
  }
//  resolution.PostProcess();
//
//  auto list = resolution.GetHistograms();
//
//  TCanvas *c1 = new TCanvas("c1","c1",800,600);
//  c1->cd();
//  auto test = list->Get("rTPC_referenceVZEROA_referenceVZEROC_reference");
//  test->Draw();
}