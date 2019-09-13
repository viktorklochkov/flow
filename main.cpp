#include <random>
#include "TFile.h"
#include "TChain.h"
#include "Correction/include/CorrectionManager.h"
#include "Correlation/include/CorrelationManager.h"
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
auto begin = std::chrono::steady_clock::now(); // start of timing
// abbreviations for convenience
using QVectors = Qn::QVectors;
auto constexpr kRef = Qn::kRef;
auto constexpr kObs = Qn::kObs;
auto constexpr ese = false;
std::string inputname("/Users/lukas/flowtest/list");
std::ifstream infile(inputname);
auto chain = new TChain("tree");
std::string line;
while (std::getline(infile, line)) {
std::cout << "Adding to TChain: " << line << std::endl;
chain->Add(line.data());
}

std::string out_correlations{"correlations.root"};

Qn::CorrelationManager man(chain);
man.EnableDebug();
man.SetOutputFile(out_correlations);

auto xx = [](QVectors q) {return q[0].x(1)*q[1].x(1);};
auto yy = [](QVectors q) {return q[0].y(1)*q[1].y(1);};
auto xy = [](QVectors q) {return q[0].x(1)*q[1].y(1);};
auto yx = [](QVectors q) {return q[0].y(1)*q[1].x(1);};

man.AddEventAxis({"CentralityV0M",70,0.,70.});

  man.AddCorrelation("ZNXNX", {"ZNA_RECENTERED", "ZNC_RECENTERED"}, xx, {kRef, kRef});
  man.AddCorrelation("ZNYNY", {"ZNA_RECENTERED", "ZNC_RECENTERED"}, yy, {kRef, kRef});
  man.AddCorrelation("ZNYNX", {"ZNA_RECENTERED", "ZNC_RECENTERED"}, yx, {kRef, kRef});
  man.AddCorrelation("ZNXNY", {"ZNA_RECENTERED", "ZNC_RECENTERED"}, xy, {kRef, kRef});
  man.AddCorrelation("ZPXPX", {"ZPA_RECENTERED", "ZPC_RECENTERED"}, xx, {kRef, kRef});
  man.AddCorrelation("ZPXNX", {"ZPA_RECENTERED", "ZNC_RECENTERED"}, xx, {kRef, kRef});
  man.AddCorrelation("ZNXPX", {"ZNA_RECENTERED", "ZPC_RECENTERED"}, xx, {kRef, kRef});
  man.AddCorrelation("ZPXNY", {"ZPA_RECENTERED", "ZNC_RECENTERED"}, xy, {kRef, kRef});
  man.AddCorrelation("ZNYPX", {"ZNA_RECENTERED", "ZPC_RECENTERED"}, yx, {kRef, kRef});

// Global configuration of the resampling method applied to all correlations if not explicitly disabled.
man.SetResampling(Qn::Sampler::Method::BOOTSTRAP, 20);
man.Run();
auto end = std::chrono::steady_clock::now(); //end of timing
std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::minutes> (end - begin).count() << " minutes" << std::endl;
}