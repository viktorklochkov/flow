#include "ROOT/RDataFrame.hxx"
#include "DataFrameCorrelation.h"
#include "DataFrameHelper.h"
#include "DataFrameReSampler.h"

//
//int main() {
//  using namespace Qn;
//  enum values {
//    kCent,
//    kPhiA,
//    kPhiB = kPhiA+4,
//    kMultA = kPhiB+4,
//    kMultB = kMultA+4,
//    kPhiT,
//    kPt
//  };
//  Qn::CorrectionManager man;
//  auto calibfile = new TFile("corr.root");
//  auto treefile = new TFile("tree.root","RECREATE");
//  treefile->cd();
//  auto tree = new TTree("tree","tree");
//  man.SetTree(tree);
//  man.AddVariable("PhiA",kPhiA,4);
//  man.AddVariable("PhiB",kPhiB,4);
//  man.AddVariable("PhiT",kPhiT,1);
//  man.AddVariable("Pt",kPt,1);
//  man.AddVariable("MultA",kMultA,4);
//  man.AddVariable("MultB",kMultB,4);
//  man.AddVariable("Cent",kCent,1);
//
//  auto correction = [](Qn::SubEvent *config) {
//    config->SetNormalization(Qn::QVector::Normalization::M);
//    config->AddCorrectionOnQnVector(new Qn::Recentering());
//    auto channels = new bool[4]{true, true, true, true};
//    auto group = new int[4]{0, 0, 0, 0};
//    config->SetChannelsScheme(channels, group,{});
//  };
//
//  auto correctiontrk = [](Qn::SubEvent *config) {
//    config->SetNormalization(Qn::QVector::Normalization::M);
//    config->AddCorrectionOnQnVector(new Qn::Recentering());
//    auto rescaling = new TwistAndRescale();
//    rescaling->SetApplyRescale(true);
//    rescaling->SetApplyTwist(true);
//    rescaling->SetTwistAndRescaleMethod(TwistAndRescale::Method::DOUBLE_HARMONIC);
//    config->AddCorrectionOnQnVector(rescaling);
//  };
//
//
//  man.AddDetector("BB",DetectorType::CHANNEL,"PhiB","MultB",{},{1});
//  man.SetCorrectionSteps("BB",correction);
//  man.AddDetector("CC",DetectorType::TRACK,"PhiT","Ones",{{"Pt",20,0.,100.}},{1});
//  man.SetCorrectionSteps("CC",correctiontrk);
//  man.AddDetector("AA",DetectorType::CHANNEL,"PhiA","MultA",{},{1});
//  man.SetCorrectionSteps("AA",correction);
//  man.AddEventVariable("Cent");
//  man.AddCorrectionAxis({"Cent",100,0,100});
//
//  man.Initialize(nullptr);
//
//  auto caliblist = man.GetCalibrationList();
//  auto calibqalist = man.GetCalibrationQAList();
//
//  man.SetProcessName("test");
//
//  const unsigned int nevents = 1000;
//  std::default_random_engine gen;
//  std::uniform_real_distribution<double> uniform(0,100);
//  std::uniform_int_distribution<unsigned int> trk(0,1000);
//  std::uniform_real_distribution<double> piform(0,2*TMath::Pi());
//  std::normal_distribution<double> gauss(0,20);
//  constexpr std::array<double, 4> vectorX = {{1.75, -1.75, 1.75, -1.75}};
//  constexpr std::array<double, 4> vectorY = {{-1.75, -1.75, 1.75, 1.75}};
//  auto values = man.GetVariableContainer();
//  for (unsigned int iev = 0; iev < nevents; ++iev) {
//    man.Reset();
//    values[kCent] = uniform(gen);
//    for (unsigned int ich = 0; ich < 4; ++ich) {
//      values[kPhiA+ich] = std::atan2(vectorY[ich],vectorX[ich]);
//      values[kPhiB+ich] = std::atan2(vectorY[ich],vectorX[ich]);
//      values[kMultA+ich] = std::abs(gauss(gen));
//      values[kMultB+ich] = std::abs(gauss(gen));
//    }
//    man.ProcessEvent();
//    man.FillChannelDetectors();
//    unsigned int ntracks = trk(gen);
//    for (unsigned int itrack = 0; itrack < ntracks; ++itrack) {
//      values[kPhiT] = piform(gen);
//      values[kPt] = uniform(gen);
//      man.FillTrackingDetectors();
//    }
//    man.ProcessQnVectors();
//  }
//  man.Finalize();
//  calibfile->Close();
//  delete calibfile;
//  treefile->cd();
//  tree->Write();
//  caliblist->Write(caliblist->GetName(),TDirectoryFile::kSingleKey);
//  calibqalist->Write(calibqalist->GetName(),TDirectoryFile::kSingleKey);
//  treefile->Close();
//  delete treefile;
//}
int main() {
  auto begin = std::chrono::high_resolution_clock::now(); // start of timing

  ROOT::EnableImplicitMT();
  auto file = TFile::Open("~/flowtest/mergedtree.root");
  if (!file) return 1;
  TTreeReader reader("tree", file);

  const std::size_t n_samples = 100;

  auto xaxc = [](const Qn::QVector &a, const Qn::QVector &c) {
    return a.x(1)*c.x(1);
  };

  std::cout << reader.GetEntries(true) << std::endl;

  Qn::AxisD centrality("CentralityV0M", 100, 0, 100);
  Qn::AxisD Trigger("Trigger", 3, 0., 3.);
  Qn::AxisD VertexX("VtxX",10,0.088,0.096);
  Qn::AxisD VertexY("VtxY",10,0.364,0.372);
  Qn::DataFrameReSampler re_sampler(n_samples);

  ROOT::RDataFrame df("tree", "~/flowtest/mergedtree.root");

  auto ntrackfilter = [](const Qn::DataContainerQVector &a) {
    return a.At(0).sumweights() > 1;
  };

  auto df_samples = df.Define("Samples", re_sampler, {});

  auto stats = Qn::MakeCorrelation("znaX_zncX_recentered", xaxc, Qn::MakeEventAxes(Trigger,centrality,VertexX,VertexY))
      .SetInputNames("ZNA_RECENTERED", "ZNC_RECENTERED")
      .SetWeights(Qn::Stats::Weights::REFERENCE, Qn::Stats::Weights::REFERENCE)
      .BookMe(df_samples, reader, n_samples);

//  auto stats2 = Qn::MakeCorrelation("znaX_zncX_plain", xaxc, Qn::MakeEventAxes(centrality))
//      .SetInputNames("ZNA_PLAIN", "ZNC_PLAIN")
//      .SetWeights(Qn::Stats::Weights::REFERENCE, Qn::Stats::Weights::REFERENCE)
//      .BookMe(df_samples, reader, n_samples);

  auto histo = df_samples.Histo1D({"h", "centrality", 100, 0, 100},"CentralityV0M");

  auto val = stats.GetValue();
  auto out_file = new TFile("test.root", "RECREATE");
  out_file->cd();
  stats->Write("recentered");
//  stats2->Write("plain");
  histo->Write("centrality");
  out_file->Close();

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> fp_time = end - begin;
  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(fp_time);
  auto minutes = std::chrono::duration_cast<std::chrono::minutes>(fp_time);
  std::cout << minutes.count() << " minutes " << seconds.count() - minutes.count()*60 << " seconds" << std::endl;

}
