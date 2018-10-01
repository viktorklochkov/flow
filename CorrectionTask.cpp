//
// Created by Lukas Kreis on 29.06.17.
//

#include <iostream>
#include <QnCorrections/QnCorrectionsLog.h>
#include <memory>
#include <random>
#include <THnSparse.h>
#include "CorrectionTask.h"

namespace Qn {

CorrectionTask::CorrectionTask(std::string filelist, std::string incalib, std::string treename) :
    out_file_(new TFile("output.root", "RECREATE")),
    in_calibration_file_(new TFile(incalib.data(), "READ")),
    out_calibration_file_(new TFile("qn.root", "RECREATE")),
    in_tree_(this->MakeChain(filelist, treename)),
    out_tree_(nullptr),
    tree_reader_(in_tree_.get()),
    event_(tree_reader_, "Event"),
    manager_(),
    write_tree_(true) {
  out_file_->cd();
  out_tree_ = new TTree("tree", "tree");
}

void CorrectionTask::Run() {
  Initialize();
  QnCorrectionsSetTracingLevel(kError);
  std::cout << "Processing..." << std::endl;
  int i = 0;
  while (tree_reader_.Next()) {
    Process();
    ++i;
  }
  std::cout << "Analyzed " << i << " events." << std::endl;
  Finalize();
}

void CorrectionTask::Initialize() {
  constexpr bool FMD = false;
  SetVariables({VAR::kVtxZ, VAR::kPt, VAR::kEta, VAR::kP,
                VAR::kPhi, VAR::kTPCncls, VAR::kDcaXY,
                VAR::kDcaZ, VAR::kVZEROTotalMult, VAR::kNtracksTotal,
                VAR::kTPCsignal, VAR::kTPCchi2, VAR::kCharge,
                VAR::kTrackQualityFlag, VAR::kVZEROChannelMult, VAR::kMultEstimatorSPDClusters,
                VAR::kSPDntracklets, VAR::kITSncls, VAR::kNTracksPerTrackingStatus, VAR::kVZEROTotalMult});

  // Add Variables to variable manager needed for filling
  manager_.AddVariable("CentralityV0M", VAR::kCentVZERO, 1);
  manager_.AddVariable("CentralitySPD", VAR::kCentSPD, 1);
  manager_.AddVariable("ITSNClusters", VAR::kITSncls, 1);
  manager_.AddVariable("SPDNTracklets", VAR::kSPDntracklets, 1);
  manager_.AddVariable("CentralitySPD", VAR::kCentSPD, 1);
  manager_.AddVariable("NTracksTPCout", VAR::kNTracksPerTrackingStatus + VAR::kTPCout, 1);
  manager_.AddVariable("VTXX", VAR::kVtxX, 1);
  manager_.AddVariable("VTXY", VAR::kVtxY, 1);
  manager_.AddVariable("VTXZ", VAR::kVtxZ, 1);
  manager_.AddVariable("NTracks", VAR::kNtracksSelected, 1);
  manager_.AddVariable("TPCPt", VAR::kPt, 1);
  manager_.AddVariable("TPCEta", VAR::kEta, 1);
  manager_.AddVariable("TPCPhi", VAR::kPhi, 1);
  manager_.AddVariable("TPCNCls", VAR::kTPCncls, 1);
  manager_.AddVariable("TPCTrackFlag", VAR::kQualityTrackFlags, 1);
  manager_.AddVariable("TPCHybrid", VAR::kFilterBit + 8, 1);
  manager_.AddVariable("TPCHybrid+", VAR::kFilterBit + 9, 1);
  manager_.AddVariable("V0Mult", VAR::Variables::kVZEROTotalMult, 1);
  manager_.AddVariable("V0CMultChannel", VAR::Variables::kVZEROChannelMult, 32);
  manager_.AddVariable("V0AMultChannel", VAR::Variables::kVZEROChannelMult + 32, 32);
  manager_.AddVariable("V0CPhiChannel", VAR::Variables::kVZEROChannelPhi, 32);
  manager_.AddVariable("V0APhiChannel", VAR::Variables::kVZEROChannelPhi + 32, 32);
  manager_.AddVariable("V0ARingChannel", VAR::Variables::kVZEROChannelRing + 32, 32);
  manager_.AddVariable("V0CRingChannel", VAR::Variables::kVZEROChannelRing, 32);
  manager_.AddVariable("ZDCAPhi", VAR::Variables::kZDCPhi + 6, 4);
  manager_.AddVariable("ZDCAMult", VAR::Variables::kZDCnEnergyCh + 6, 4);
  manager_.AddVariable("ZDCCPhi", VAR::Variables::kZDCPhi + 1, 4);
  manager_.AddVariable("ZDCCMult", VAR::Variables::kZDCnEnergyCh + 1, 4);
  manager_.AddVariable("T0APhi", VAR::Variables::kTZEROPhiCh + 12, 12);
  manager_.AddVariable("T0AMult", VAR::Variables::kTZEROAmplitudeCh + 12, 12);
  manager_.AddVariable("T0CPhi", VAR::Variables::kTZEROPhiCh, 12);
  manager_.AddVariable("T0CMult", VAR::Variables::kTZEROAmplitudeCh, 12);


  //Correction eventvariables
  manager_.SetEventVariable("CentralityV0M");
  manager_.SetEventVariable("CentralitySPD");
  manager_.SetEventVariable("VTXZ");
  manager_.SetEventVariable("VTXY");
  manager_.SetEventVariable("VTXX");
  manager_.AddCorrectionAxis({"CentralitySPD", 80, 0, 80});

  //Config TPC
  Axis pt("TPCPt",
          {0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1., 1.25, 1.5, 1.75, 2.0, 2.25, 2.5, 3., 3.5, 4., 5., 6., 8., 10.});
  Axis eta("TPCEta", {-0.8, -0.5, 0.5, 0.8});

  auto cut_hybrid = [](const double &flag, const double &flagPlus) {
    return flag==1 || flagPlus==1;
  };
  auto cut_eta = [](const double &eta) { return -0.8 < eta && eta < 0.8; };
  //Config of TPC corrections
  auto confTPC = [](QnCorrectionsDetectorConfigurationBase *config) {
    config->SetQVectorNormalizationMethod(QnCorrectionsQnVector::QVNORM_QoverM);
    auto recenter = new QnCorrectionsQnVectorRecentering();
    recenter->SetApplyWidthEqualization(true);
    config->AddCorrectionOnQnVector(recenter);
  };
  auto confTPCR = [](QnCorrectionsDetectorConfigurationBase *config) {
    config->SetQVectorNormalizationMethod(QnCorrectionsQnVector::QVNORM_QoverM);
    auto recenter = new QnCorrectionsQnVectorRecentering();
    recenter->SetApplyWidthEqualization(true);
    config->AddCorrectionOnQnVector(recenter);
  };
  // TPC pT-dependence
  manager_.AddDetector("TPC", DetectorType::TRACK, "TPCPhi", "Ones", {pt,eta}, {2, 3, 4});
  manager_.AddCut("TPC", {"TPCHybrid", "TPCHybrid+"}, cut_hybrid);
  manager_.AddCut("TPC", {"TPCEta"}, cut_eta);
  manager_.AddCut("TPC", {"TPCPt"}, [](const double &pt) { return pt > 0.2 && pt < 10.; });
  manager_.SetCorrectionSteps("TPC", confTPC);
  manager_.AddHisto2D("TPC", {{"TPCPt", 50, 0., 10.}, {"TPCNCls", 160, 0, 160}});
  manager_.AddHisto2D("TPC", {{"TPCEta", 50, -1., 1.}, {"TPCPhi", 50, 0, 2*TMath::Pi()}});
  manager_.AddHisto2D("TPC", {{"TPCEta", 50, -1., 1.}, {"TPCPt", 50, 0., 10.}});
  //TPC pT-integrated
  manager_.AddDetector("TPC_R", DetectorType::TRACK, "TPCPhi", "Ones", {eta}, {2, 3, 4});
  manager_.AddCut("TPC_R", {"TPCHybrid", "TPCHybrid+"}, cut_hybrid);
  manager_.AddCut("TPC_R", {"TPCEta"}, cut_eta);
  manager_.AddCut("TPC_R", {"TPCPt"}, [](const double &pt) { return pt > 0.2 && pt < 10.; });
  manager_.AddCut("TPC_R", {"TPCNCls"}, [](const double &ncls) { return ncls > 70; });
  manager_.SetCorrectionSteps("TPC_R", confTPCR);

  //Config VZERO A- and C-side
  auto cut_mult = [](double &mult) {
    return mult > 0.1;
  };
  //Config of VZERO A- and C-side corrections
  auto confV0 = [](QnCorrectionsDetectorConfigurationBase *config) {
    config->SetQVectorNormalizationMethod(QnCorrectionsQnVector::QVNORM_QoverM);
    auto recenter = new QnCorrectionsQnVectorRecentering();
    recenter->SetApplyWidthEqualization(true);
    config->AddCorrectionOnQnVector(recenter);
    auto *V0Channels = new bool[32];
    for (int ich = 0; ich < 32; ich++) V0Channels[ich] = kTRUE;
    auto *channelGroups = new int[32];
    for (int ich = 0; ich < 32; ich++) {
      channelGroups[ich] = ich/8;
    }
    ((QnCorrectionsDetectorConfigurationChannels *) config)->SetChannelsScheme(V0Channels, channelGroups, nullptr);
    auto equal = new QnCorrectionsInputGainEqualization();
    equal->SetEqualizationMethod(QnCorrectionsInputGainEqualization::GEQUAL_averageEqualization);
    equal->SetUseChannelGroupsWeights(true);
    config->AddCorrectionOnInputData(equal);
  };

  //VZERO A
  manager_.AddDetector("V0A", DetectorType::CHANNEL, "V0APhiChannel", "V0AMultChannel", {}, {2, 3});
  manager_.AddCut("V0A", {"V0AMultChannel"}, cut_mult);
  manager_.SetCorrectionSteps("V0A", confV0);
  manager_.AddHisto1D("V0A", {{"V0AChannels", 32, 0, 32}}, "V0AMultChannel");
  manager_.AddHisto2D("V0A", {{"V0ARingChannel", 4, 0, 4}, {"V0AChannels", 32, 0, 32}}, "V0AMultChannel");
  manager_.AddHisto2D("V0A", {{"V0APhiChannel", 8, 0, 2*TMath::Pi()}, {"V0ARingChannel", 4, 0, 4}}, "V0AMultChannel");
  //VZERO C
  manager_.AddDetector("V0C", DetectorType::CHANNEL, "V0CPhiChannel", "V0CMultChannel", {}, {2, 3});
  manager_.AddCut("V0C", {"V0CMultChannel"}, cut_mult);
  manager_.SetCorrectionSteps("V0C", confV0);
  manager_.AddHisto1D("V0C", {{"V0CChannels", 32, 0, 32}}, "V0CMultChannel");
  manager_.AddHisto2D("V0C", {{"V0CRingChannel", 4, 0, 4}, {"V0CChannels", 32, 0, 32}}, "V0CMultChannel");
  manager_.AddHisto2D("V0C", {{"V0CPhiChannel", 8, 0, 2*TMath::Pi()}, {"V0CRingChannel", 4, 0, 4}}, "V0CMultChannel");

//   Config for ZDC A and ZDC C
  auto confZDC = [](QnCorrectionsDetectorConfigurationBase *config) {
    config->SetQVectorNormalizationMethod(QnCorrectionsQnVector::QVNORM_QoverM);
    auto recenter = new QnCorrectionsQnVectorRecentering();
    recenter->SetApplyWidthEqualization(true);
    config->AddCorrectionOnQnVector(recenter);
    auto *channels = new bool[4];
    for (int ich = 0; ich < 4; ich++) channels[ich] = kTRUE;
    auto *channelGroups = new int[4];
    for (int ich = 0; ich < 4; ich++) {
      channelGroups[ich] = 0;
    }
    ((QnCorrectionsDetectorConfigurationChannels *) config)->SetChannelsScheme(channels, nullptr, nullptr);
  };
  // ZDC A
  manager_.AddDetector("ZDCA", DetectorType::CHANNEL, "ZDCAPhi", "ZDCAMult", {}, {1});
  manager_.SetCorrectionSteps("ZDCA", confZDC);
  manager_.AddHisto1D("ZDCA", {{"ZDCAChannels", 4, 0, 4}}, "ZDCAMult");
  manager_.AddHisto1D("ZDCA", {{"ZDCAPhi", 4, 0, 2*TMath::Pi()}}, "ZDCAMult");
  manager_.AddCut("ZDCA", {"ZDCAMult"}, [](double &mult) { return mult > 100; });
  // ZDC C
  manager_.AddDetector("ZDCC", DetectorType::CHANNEL, "ZDCCPhi", "ZDCCMult", {}, {1});
  manager_.SetCorrectionSteps("ZDCC", confZDC);
  manager_.AddHisto1D("ZDCC", {{"ZDCCChannels", 4, 0, 4}}, "ZDCCMult");
  manager_.AddHisto1D("ZDCC", {{"ZDCCPhi", 4, 0, 2*TMath::Pi()}}, "ZDCCMult");
  manager_.AddCut("ZDCC", {"ZDCCMult"}, [](double &mult) { return mult > 100; });

  auto confT0 = [](QnCorrectionsDetectorConfigurationBase *config) {
    config->SetQVectorNormalizationMethod(QnCorrectionsQnVector::QVNORM_QoverM);
    auto recenter = new QnCorrectionsQnVectorRecentering();
    recenter->SetApplyWidthEqualization(true);
    config->AddCorrectionOnQnVector(recenter);
    auto *channels = new bool[12];
    for (int ich = 0; ich < 12; ich++) channels[ich] = kTRUE;
    auto *channelGroups = new int[12];
    for (int ich = 0; ich < 12; ich++) {
      channelGroups[ich] = 0;
    }
    ((QnCorrectionsDetectorConfigurationChannels *) config)->SetChannelsScheme(channels, nullptr, nullptr);
  };
  // T0A
  manager_.AddDetector("T0A", DetectorType::CHANNEL, "T0APhi", "T0AMult", {}, {2, 3});
  manager_.SetCorrectionSteps("T0A", confT0);
  manager_.AddCut("T0A", {"T0AMult"}, cut_mult);
  manager_.AddHisto1D("T0A", {{"T0AChannels", 12, 0, 12}}, "T0AMult");
  manager_.AddHisto1D("T0A", {{"T0APhi", 12, 0, 2*TMath::Pi()}}, "T0AMult");
  // T0C
  manager_.AddDetector("T0C", DetectorType::CHANNEL, "T0CPhi", "T0CMult", {}, {2, 3});
  manager_.SetCorrectionSteps("T0C", confT0);
  manager_.AddCut("T0C", {"T0CMult"}, cut_mult);
  manager_.AddHisto1D("T0C", {{"T0CChannels", 12, 0, 12}}, "T0CMult");
  manager_.AddHisto1D("T0C", {{"T0CPhi", 12, 0, 2*TMath::Pi()}}, "T0CMult");


  if (FMD) {
    manager_.AddVariable("FMDAPhi", VAR::Variables::kFMDAPhi, 1200);
    manager_.AddVariable("FMDAMult", VAR::Variables::kFMDAWeight, 1200);
    manager_.AddVariable("FMDAEta", VAR::Variables::kFMDAEta, 1200);

    manager_.AddVariable("FMDCPhi", VAR::Variables::kFMDCPhi, 1200);
    manager_.AddVariable("FMDCMult", VAR::Variables::kFMDCWeight, 1200);
    manager_.AddVariable("FMDCEta", VAR::Variables::kFMDCEta, 1200);

    auto confFMD = [](QnCorrectionsDetectorConfigurationBase *config) {
      config->SetQVectorNormalizationMethod(QnCorrectionsQnVector::QVNORM_QoverM);
      auto recenter = new QnCorrectionsQnVectorRecentering();
      recenter->SetApplyWidthEqualization(true);
      config->AddCorrectionOnQnVector(recenter);
      auto *channels = new bool[1200];
      for (int ich = 0; ich < 1200; ich++) channels[ich] = kTRUE;
      ((QnCorrectionsDetectorConfigurationChannels *) config)->SetChannelsScheme(channels, nullptr);
    };
    manager_.AddDetector("FMDA", DetectorType::CHANNEL, "FMDAPhi", "FMDAMult", {}, {2, 3});
    manager_.SetCorrectionSteps("FMDA", confFMD);
    manager_.AddHisto1D("FMDA", {{"FMDAChannels", 1200, 0, 1200}}, "FMDAMult");
    manager_.AddHisto1D("FMDA", {{"FMDAPhi", 20, 0, 2*TMath::Pi()}}, "FMDAMult");
    manager_.AddHisto2D("FMDA", {{"FMDAPhi", 20, 0, 2*TMath::Pi()},{"FMDAEta", 100, -4, 6}}, "FMDAMult");
    manager_.AddCut("FMDA", {"FMDAMult"}, [](double &mult) { return mult > 1e-6; });

    manager_.AddDetector("FMDC", DetectorType::CHANNEL, "FMDCPhi", "FMDCMult", {}, {2, 3});
    manager_.SetCorrectionSteps("FMDC", confFMD);
    manager_.AddHisto1D("FMDC", {{"FMDCChannels", 1200, 0, 1200}}, "FMDCMult");
    manager_.AddHisto2D("FMDC", {{"FMDCPhi", 20, 0, 2*TMath::Pi()},{"FMDCEta", 100, -4, 6}}, "FMDCMult");
    manager_.AddHisto1D("FMDC", {{"FMDCPhi", 20, 0, 2*TMath::Pi()}}, "FMDCMult");

    manager_.AddCut("FMDC", {"FMDCMult"}, [](double &mult) { return mult > 1e-6; });
  }



//  Event selection configuration
  manager_.AddEventCut({"NTracks"}, [](const double &ntracks) { return ntracks > 0; });
  manager_.AddEventCut({"VTXZ"}, [](const double &z) { return -10 < z && z < 10; });
  manager_.AddEventCut({"CentralityV0M"}, [](const double &cent) { return 0 < cent && cent < 100; });

  manager_.AddEventHisto2D({{"VTXZ", 200, -30, 30}, {"VTXX", 200, -0.3, 0.3}});
  manager_.AddEventHisto2D({{"VTXZ", 200, -30, 30}, {"VTXY", 200, -0.3, 0.3}});
  manager_.AddEventHisto2D({{"VTXX", 200, -0.3, 0.3}, {"VTXY", 200, -0.3, 0.3}});
  manager_.AddEventHisto2D({{"NTracks", 100, 0, 1800}, {"CentralityV0M", 100, 0, 100}});
  manager_.AddEventHisto2D({{"CentralitySPD", 100, 0, 100}, {"CentralityV0M", 100, 0, 100}});
  manager_.AddEventHisto2D({{"SPDNTracklets", 100, 0, 4000}, {"CentralitySPD", 100, 0, 100}});
  manager_.AddEventHisto2D({{"SPDNTracklets", 100, 0, 4000}, {"CentralityV0M", 100, 0, 100}});
  manager_.AddEventHisto2D({{"SPDNTracklets", 100, 0, 4000}, {"NTracks", 100, 0, 1800}});
  manager_.AddEventHisto2D({{"NTracks", 100, 0, 1800}, {"V0Mult", 100, 0, 30000}});
  manager_.AddEventHisto2D({{"NTracksTPCout", 100, 0, 3000}, {"V0Mult", 100, 0, 30000}});
  manager_.SetTree(out_tree_);
  //Initialization of framework
  manager_.Initialize(in_calibration_file_);
}

void CorrectionTask::Process() {
  manager_.Reset();
  auto event = event_.Get();
  if (event->IsA()!=AliReducedEventInfo::Class()) return;
  manager_.Process(DataFiller(event));
}

void CorrectionTask::Finalize() {
  manager_.Finalize();
  manager_.SaveHistograms(out_calibration_file_);
  manager_.SaveTree(out_file_);
}

std::unique_ptr<TChain> CorrectionTask::MakeChain(std::string filename, std::string treename) {
  std::unique_ptr<TChain> chain(new TChain(treename.data()));
  std::ifstream in;
  in.open(filename);
  std::string line;
  std::cout << "Adding files to chain:" << std::endl;
  while ((in >> line).good()) {
    if (!line.empty()) {
      chain->AddFile(line.data());
      std::cout << line << std::endl;
    }
  }
  return chain;
}
}
