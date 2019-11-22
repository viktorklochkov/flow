// Flow Vector Correction Framework
//
// Copyright (C) 2019  Lukas Kreis Ilya Selyuzhenkov
// Contact: l.kreis@gsi.de; ilya.selyuzhenkov@gmail.com
// For a full list of contributors please see docs/Credits
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#include <TH1F.h>
#include <Axis.h>
#include <CorrectionManager.h>
#include "gtest/gtest.h"
#include "ParticleGenerator.h"
#include "ChannelDetector.h"

TEST(ParticleGeneratorUnitTest, test1) {
  ParticleGenerator<4> gen(1,{0.0, 0.5, 0., 0.});
  Qn::AxisD axis("phi", 10, 0, 2*M_PI);
  std::vector<double> data_(10);
  for (int i = 0; i < 10000; ++i) {
    auto bin = axis.FindBin(gen.GeneratePhi());
    if (bin > -1) {
      data_[bin] = data_[bin] + 1;
    }
  }
  for (const auto &x : data_) {
    for (unsigned int i = 0; i < x/100; ++i) {
      std::cout << "*";
    }
    std::cout << std::endl;
  }
}

TEST(ParticleGeneratorUnitTest, Detector) {
  ParticleGenerator<4> gen(1,{0.0, 0.5, 0., 0.});
  Qn::AxisD axis("A", 20, 0, 2*M_PI);
  ChannelDetector det(axis, [](double x) {
    return true;
  });
  for (int i = 0; i < 10000; ++i) {
    det.Detect(gen.GeneratePhi());
  }
  det.Print();
}

TEST(ParticleGeneratorUnitTest, RandomPsi) {
  ParticleGenerator<4> gen(1,{0.0, 0.5, 0., 0.});
  Qn::AxisD axis("A", 20, 0, 2*M_PI);
  std::random_device device;
  std::default_random_engine engine(device());
  ChannelDetector det(axis, [](double x) { return true; });
  std::uniform_real_distribution<> psi_distribution(0, 2*M_PI);
  for (int i = 0; i < 10000; ++i) {
    auto psi = psi_distribution(engine);
    auto phi = gen.GetPhi(psi);
    det.Detect(phi);
  }
  det.Print();
}

TEST(ParticleGeneratorUnitTest, Detectorvalues) {
  const int kChannelsA = 8;
  enum variables {
    kPhi = 0,
    kWeightRec = kPhi + kChannelsA,
    kWeightTrue = kWeightRec + kChannelsA,
    kEvent = kWeightTrue + kChannelsA
  };
  ParticleGenerator<2> gen(1,{0.0, 0.07});
  Qn::AxisD axis("A", kChannelsA, 0, 2*M_PI);
  ChannelDetector det(axis, [](const double phi) { return true; });
  det.SetChannelEfficencies({0.5,0.5,1.,1.,0.4,0.4,1.,1.});

  std::random_device device;
  std::default_random_engine engine(device());
  std::uniform_real_distribution<> psi_distribution(0, 2*M_PI);
  std::uniform_int_distribution<> n_distribution(2000,2000);

  Qn::CorrectionManager man;
  man.SetFillOutputTree(true);
  man.SetFillCalibrationQA(true);
  man.SetFillValidationQA(true);
  man.AddVariable("phi_A",kPhi,kChannelsA);
  man.AddVariable("weight_rec_A",kWeightRec,kChannelsA);
  man.AddVariable("weight_true_A",kWeightTrue,kChannelsA);
  man.AddVariable("Event",kEvent,1);
  auto outfile = TFile::Open("correctionout.root", "RECREATE");
  auto tree = new TTree();
  man.ConnectOutputTree(tree);
  man.SetCalibrationInputFileName("correctionin.root");

  man.AddEventVariable("Event");
  man.AddCorrectionAxis({"Event",1,0,1});

  man.AddDetector("A_true", Qn::DetectorType::CHANNEL, "phi_A", "weight_true_A", {}, {1, 2, 3, 4});
  man.AddDetector("A_rec", Qn::DetectorType::CHANNEL, "phi_A", "weight_rec_A", {}, {1, 2, 3, 4});

  std::vector<std::string> detectors{"A_rec","A_true"};
  for (const auto & detector : detectors) {
    man.AddCorrectionOnQnVector(detector,  Qn::Recentering());
    man.SetOutputQVectors(detector, {Qn::QVector::CorrectionStep::PLAIN, Qn::QVector::CorrectionStep::RECENTERED});
  }
  man.AddHisto1D("A_true",{"phi_A",8,0,2*M_PI},"weight_true_A");
  man.AddHisto1D("A_rec",{"phi_A",8,0,2*M_PI},"weight_rec_A");


  man.InitializeOnNode();
  auto correction_list = man.GetCorrectionList();
  auto correction_qa_list = man.GetCorrectionQAList();
  man.SetCurrentRunName("test");

  double* values = man.GetVariableContainer();
  for (int iev = 0; iev < 50000; ++iev) {
    man.Reset();
    det.Reset();
    values[kEvent] = 0.5;
    if (man.ProcessEvent()) {
      auto psi = psi_distribution(engine);
      for (int i = 0; i < n_distribution(engine); ++i) {
        auto phi = gen.GetPhi(psi);
        det.Detect(phi);
      }
      det.FillDataRec(values, kPhi, kWeightRec);
      det.FillDataTruth(values,kPhi,kWeightTrue);
      man.FillChannelDetectors();
      man.ProcessCorrections();
    }
  }
  man.Finalize();
  outfile->cd();
  tree->Write("tree");
  correction_list->Write("CorrectionHistograms",TObject::kSingleKey);
  correction_qa_list->Write("CorrectionQAHistograms",TObject::kSingleKey);
  outfile->Close();
}