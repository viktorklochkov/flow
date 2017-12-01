//
// Created by Lukas Kreis on 13.11.17.
//

#include <TCanvas.h>
#include <TFile.h>
#include <ReducedEvent/AliReducedVarManager.h>
#include "SimpleTask.h"
#include "DataContainerHelper.h"
SimpleTask::SimpleTask(std::string filelist, std::string treename) :
    in_tree_(this->MakeChain(filelist, treename)),
    reader_(in_tree_.get()) {}

void SimpleTask::Initialize() {
  std::vector<Qn::Axis> noaxes;
  eventaxes_.emplace_back("CentralityVZERO", 10, 0, 80, 1);

  auto tpc = *values_.at("TPC_reference");
  auto vc = *values_.at("VZEROC_reference");
  auto va = *values_.at("VZEROA_reference");
  auto fc = *values_.at("FMDC_reference");
  auto fa = *values_.at("FMDA_reference");
  auto zc = *values_.at("ZDCC_reference");
  auto za = *values_.at("ZDCA_reference");

  correlations_.insert({"rtpcvc", {{tpc, vc}, eventaxes_}});
  correlations_.insert({"rtpcva", {{tpc, va}, eventaxes_}});
  correlations_.insert({"rvcva", {{vc, va}, eventaxes_}});
  correlations_.insert({"rtpcfc", {{tpc, fc}, eventaxes_}});
  correlations_.insert({"rtpcfa", {{tpc, fa}, eventaxes_}});
  correlations_.insert({"rfcfa", {{fc, fa}, eventaxes_}});
  correlations_.insert({"rtpczc", {{tpc, zc}, eventaxes_}});
  correlations_.insert({"rtpcza", {{tpc, za}, eventaxes_}});
  correlations_.insert({"rzcza", {{zc, za}, eventaxes_}});
  correlations_.insert({"v2tpcvaxx", {{tpc, va}, eventaxes_}});
  correlations_.insert({"v2tpcvayy", {{tpc, va}, eventaxes_}});
  correlations_.insert({"vcvaxx", {{vc, va}, eventaxes_}});
  correlations_.insert({"vcvayy", {{vc, va}, eventaxes_}});
}

void SimpleTask::Run() {
  AddEventVariable("CentralityVZERO");
  AddDataContainer("TPC_reference");
  AddDataContainer("TPC");
  AddDataContainer("VZEROA_reference");
  AddDataContainer("VZEROC_reference");
  AddDataContainer("FMDA_reference");
  AddDataContainer("FMDC_reference");
  AddDataContainer("ZDCA_reference");
  AddDataContainer("ZDCC_reference");

  int events = 1;
  reader_.SetEntry(0);
  Initialize();
  Process();
  while (reader_.Next()) {
    events++;
    Process();
  }
  std::cout << "number of events: " << events << std::endl;

  auto rtpcvc = correlations_.at("rtpcvc").GetCorrelation();
  auto rtpcva = correlations_.at("rtpcva").GetCorrelation();
  auto rvcva = correlations_.at("rvcva").GetCorrelation();
  auto rtpcfc = correlations_.at("rtpcfc").GetCorrelation();
  auto rtpcfa = correlations_.at("rtpcfa").GetCorrelation();
  auto rfcfa = correlations_.at("rfcfa").GetCorrelation();
  auto rtpczc = correlations_.at("rtpczc").GetCorrelation();
  auto rtpcza = correlations_.at("rtpcza").GetCorrelation();
  auto rzcza = correlations_.at("rzcza").GetCorrelation();
  auto v2tpcvaxx = correlations_.at("v2tpcvaxx").GetCorrelation();
  auto v2tpcvayy = correlations_.at("v2tpcvayy").GetCorrelation();
  auto vcvaxx = correlations_.at("vcvaxx").GetCorrelation();
  auto vcvayy = correlations_.at("vcvayy").GetCorrelation();

  auto multiply = [](Qn::Statistics a, Qn::Statistics b) {
    return a * b;
  };

  auto divide = [](Qn::Statistics a, Qn::Statistics b) {
    return a / b;
  };

  auto sqrt = [](Qn::Statistics a) {
    return a.Sqrt();
  };

  auto multiscalar = [](Qn::Statistics a) {
    return a * 2;
  };


  auto rtpcvcva = rtpcvc.Apply(rtpcva,multiply).Apply(rvcva,divide).Map(sqrt);
  auto rtpcfcfa = rtpcfc.Apply(rtpcfa,multiply).Apply(rfcfa,divide).Map(sqrt);
  auto rtpczcza = rtpczc.Apply(rtpcza,multiply).Apply(rzcza,divide).Map(sqrt);
  auto rvctpcva = rtpcvc.Apply(rvcva,multiply).Apply(rtpcva,divide).Map(sqrt);
  auto rvatpcvc = rtpcva.Apply(rvcva,multiply).Apply(rtpcvc,divide).Map(sqrt);
  auto rfctpcfa = rtpcfa.Apply(rfcfa,multiply).Apply(rtpcfc,divide).Map(sqrt);
  auto rfatpcfc = rtpcfc.Apply(rfcfa,multiply).Apply(rtpcfa,divide).Map(sqrt);
  auto rzctpcza = rtpcfa.Apply(rfcfa,multiply).Apply(rtpcfc,divide).Map(sqrt);
  auto rzatpczc = rtpczc.Apply(rzcza,multiply).Apply(rtpczc,divide).Map(sqrt);
  vcvaxx.Map(sqrt);
  vcvayy.Map(sqrt);
  v2tpcvaxx = v2tpcvaxx.Apply(rvatpcvc,divide);
  v2tpcvayy = v2tpcvayy.Apply(rvatpcvc,divide);


  auto grtpcvcva = Qn::DataToProfileGraph(rtpcvcva);
  auto grtpcfcfa = Qn::DataToProfileGraph(rtpcfcfa);
  auto grtpczcza = Qn::DataToProfileGraph(rtpczcza);
  auto grvctpcva = Qn::DataToProfileGraph(rvctpcva);
  auto grvatpcvc = Qn::DataToProfileGraph(rvatpcvc);
  auto grfctpcfa = Qn::DataToProfileGraph(rfctpcfa);
  auto grfatpcfc = Qn::DataToProfileGraph(rfatpcfc);
  auto grzctpcza = Qn::DataToProfileGraph(rzctpcza);
  auto grzatpczc = Qn::DataToProfileGraph(rzatpczc);
  auto gv2tpcvaxx = Qn::DataToProfileGraph(v2tpcvaxx);
  auto gv2tpcvayy = Qn::DataToProfileGraph(v2tpcvayy);

//
  auto *c1 = new TCanvas("c1", "c1", 1200, 600);
  c1->cd(1);
  grtpcfcfa.SetTitle("Resolution");
  grtpcfcfa.Draw("AP");
  grtpcfcfa.SetMinimum(0);
  grtpcfcfa.SetMaximum(1.0);
  grtpcfcfa.SetLineColor(kRed);
  grtpcfcfa.SetMarkerColor(kRed);
  grtpcfcfa.SetMarkerStyle(kOpenSquare);
  grtpcvcva.Draw("P");
  grtpcvcva.SetLineColor(kRed);
  grtpcvcva.SetMarkerColor(kRed);
  grtpcvcva.SetMarkerStyle(kCircle);
  grtpczcza.Draw("P");
  grvatpcvc.SetLineColor(kBlue);
  grvatpcvc.SetMarkerColor(kBlue);
  grvatpcvc.SetMarkerStyle(kCircle);
  grvatpcvc.Draw("P");
  grvctpcva.SetLineColor(kBlue);
  grvctpcva.SetMarkerColor(kBlue);
  grvctpcva.SetMarkerStyle(kOpenSquare);
  grvctpcva.Draw("P");
  grfatpcfc.SetMarkerStyle(kCircle);
  grfatpcfc.Draw("P");
  grfctpcfa.SetMarkerStyle(kOpenSquare);
  grfctpcfa.Draw("P");
  auto *c2 = new TCanvas("c2", "c2", 1200, 600);
  c2->cd();
  gv2tpcvaxx.SetTitle("v2");
  gv2tpcvaxx.Draw("ALP");
//  gv2tpcvaxx.SetMaximum(0.15);
//  gv2tpcvaxx.SetMinimum(0.0);
  gv2tpcvayy.Draw("LP");

  c1->SaveAs("test.root");
  c1->SaveAs("test.pdf");
  c2->SaveAs("test2.pdf");
  c2->SaveAs("test2.root");

}

void SimpleTask::Process() {
  std::vector<Qn::Axis> noaxes;

  auto tpc = *values_.at("TPC_reference");
  auto vc = *values_.at("VZEROC_reference");
  auto va = *values_.at("VZEROA_reference");
  auto fc = *values_.at("FMDC_reference");
  auto fa = *values_.at("FMDA_reference");
  auto zc = *values_.at("ZDCC_reference");
  auto za = *values_.at("ZDCA_reference");


  std::vector<float> eventparameters;
  eventparameters.push_back(*eventvalues_.at("CentralityVZERO"));
  auto eventbin = Qn::CalculateEventBin(eventaxes_, eventparameters);
  for (auto bin : eventbin) {
    if (bin == -1) return;
  }

  auto resolution = [] (const std::vector<Qn::QVector> &a) {
    return cos(2 * (Qn::Resolution::PsiN(a[0], 2) - Qn::Resolution::PsiN(a[1], 2)));
  };

  auto xx = [] (const std::vector<Qn::QVector> &a) {
    return a[0].Normal(Qn::QVector::Normalization::QOVERNORMQ).x(2) * a[1].x(2);
  };
  auto yy = [] (const std::vector<Qn::QVector> &a) {
    return a[0].Normal(Qn::QVector::Normalization::QOVERNORMQ).y(2) * a[1].y(2);
  };
//
  correlations_.at("rtpcva").Fill({tpc, va}, eventbin, resolution);
  correlations_.at("rtpcvc").Fill({tpc, vc}, eventbin, resolution);
  correlations_.at("rvcva").Fill({va, vc}, eventbin, resolution);
  correlations_.at("rtpcfa").Fill({tpc, fa}, eventbin, resolution);
  correlations_.at("rtpcfc").Fill({tpc, fc}, eventbin, resolution);
  correlations_.at("rfcfa").Fill({fa, fc}, eventbin, resolution);
  correlations_.at("v2tpcvaxx").Fill({tpc, va}, eventbin, xx);
  correlations_.at("v2tpcvayy").Fill({tpc, va}, eventbin, yy);
  correlations_.at("rtpcza").Fill({tpc, za}, eventbin, resolution);
  correlations_.at("rtpczc").Fill({tpc, zc}, eventbin, resolution);
  correlations_.at("vcvaxx").Fill({va, vc}, eventbin, xx);
  correlations_.at("vcvayy").Fill({va, vc}, eventbin, yy);
//  correlations_.at("rzcza").Fill({za, zc}, eventbin, zdccorrelation);
}

std::unique_ptr<TChain> SimpleTask::MakeChain(std::string filename, std::string treename) {
  std::unique_ptr<TChain> chain(new TChain(treename.data()));
  std::ifstream in;
  in.open(filename);
  std::string line;
  std::cout << "files in TChain:" << "\n";
  while ((in >> line).good()) {
    if (!line.empty()) {
      chain->AddFile(line.data());
      std::cout << line << std::endl;
    }
  }
  return chain;
}
void SimpleTask::AddDataContainer(std::string name) {
  TTreeReaderValue<Qn::DataContainerQVector>
      value(reader_, name.data());
  auto pair = std::make_pair(name, value);
  values_.insert(std::make_pair(name, value));
}
void SimpleTask::AddEventVariable(std::string name) {
  TTreeReaderValue<float> value(reader_, name.data());
  eventvalues_.insert(std::make_pair(name, value));
}
