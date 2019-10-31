#include "ROOT/RDataFrame.hxx"
#include "Correlation.h"
#include "CorrelationHelper.h"
#include "ReSampler.h"

#include "CorrectionManager.h"
#include "ParticleGenerator.h"
#include "ChannelDetector.h"
#include "TrackingDetector.h"

inline std::vector<std::string> CreateDetectorNames(int n_detectors,
                                                    std::string base_name,
                                                    std::vector<std::string> correction_steps) {
  std::vector<std::string> detector_names;
  std::vector<std::string> names_temp;
  for (std::size_t i = 0; i < n_detectors; ++i) {
    names_temp.push_back(base_name + std::to_string(i) + "_");
  }
  for (const auto &name : names_temp) {
    for (const auto &correction_step : correction_steps) {
      detector_names.push_back(name + correction_step);
    }
  }
  return detector_names;
}

int main() {
  bool correct = true;
  bool correlate = true;
  std::vector<ChannelDetector> detectors_ch_a;
  std::vector<ChannelDetector> detectors_ch_b;
  std::vector<TrackingDetector> detectors_trk;

  const int kNchannels = 8;
  enum variables {
    kEvent = 0,
    kPhiCh,
    kWeightCh = kPhiCh + kNchannels,
  };

  Qn::CorrectionManager man;
  man.SetFillOutputTree(true);
  man.SetFillCalibrationQA(true);
  man.SetFillValidationQA(true);
  man.AddVariable("Event", kEvent, 1);
  man.AddVariable("phi_ch", kPhiCh, kNchannels);

  //Channel A
  std::vector<std::vector<double>> efficiencies_ch_a{
      {1., 1., 1., 1., 1, 1, 1, 1},
      {0., 0., 1., 1., 1, 1, 1, 1},
  };
  for (std::size_t i = 0; i < efficiencies_ch_a.size(); ++i) {
    ChannelDetector det({"DetChA" + std::to_string(i), kNchannels, 0, 2*M_PI}, [](const double phi) { return true; });
    det.SetChannelEfficencies(efficiencies_ch_a.at(i));
    detectors_ch_a.push_back(det);
    man.AddVariable("weight_ch_a_" + std::to_string(i), kWeightCh + i*kNchannels, 1);
  }

  auto after_ch_a = kWeightCh + detectors_ch_a.size()*kNchannels;
  //Channel B
  std::vector<std::vector<double>> efficiencies_ch_b{
      {1., 1., 1., 1., 1, 1, 1, 1},
      {0., 0., 1., 1., 1, 1, 1, 1},
  };
  for (std::size_t i = 0; i < efficiencies_ch_b.size(); ++i) {
    ChannelDetector det({"DetChB" + std::to_string(i), kNchannels, 0, 2*M_PI}, [](const double phi) { return true; });
    det.SetChannelEfficencies(efficiencies_ch_b.at(i));
    detectors_ch_b.push_back(det);
    man.AddVariable("weight_ch_b_" + std::to_string(i), after_ch_a + i*kNchannels, 1);
  }

  auto displaced_phi = after_ch_a + detectors_ch_b.size()*kNchannels;
  auto displaced_weights = displaced_phi + 1;
  man.AddVariable("phi", displaced_phi, 1);

  //Add Tracking
  const int kGranularity = 16;
  std::vector<double> modulation1(kGranularity);
  std::vector<double> modulation2(kGranularity);
  std::vector<double> modulation3(kGranularity);
  std::vector<double> modulation4(kGranularity);
  std::vector<double> baseline(kGranularity);
  for (int i = 0; i < kGranularity; ++i) {
    auto phi_step = (2.*M_PI/kGranularity)*i;
    auto x = M_PI/6;
    auto efficiency_bin1 = 0.9*std::fabs(std::sin(2*phi_step + x)) + 0.1*std::fabs(std::cos(phi_step));
    auto efficiency_bin3 = std::fabs(std::sin(2*phi_step + x));
    auto efficiency_bin4 = std::fabs(std::sin(3*phi_step + x));

//    auto efficiency_bin2 = 0.8+0.2*std::fabs(std::cos(phi_step));
    auto efficiency_bin2 = 1.;
    if (i == 0 || i == 8 || i == 7 || i == 15)  efficiency_bin2 = 0.6;

    modulation1[i] = efficiency_bin1;
    modulation2[i] = efficiency_bin2;
    modulation3[i] = efficiency_bin3;
    modulation4[i] = efficiency_bin4;
    baseline[i] = 1.0;
  }
  std::vector<std::vector<double>> efficiencies{
      baseline,
      modulation1,
      modulation2,
      modulation3,
      modulation4
  };
  for (std::size_t i = 0; i < efficiencies.size(); ++i) {
    TrackingDetector
        det({"DetTrk" + std::to_string(i), kGranularity, 0, 2*M_PI}, [](const double phi) { return true; });
    det.SetChannelEfficencies(efficiencies.at(i));
    detectors_trk.push_back(det);
    man.AddVariable("weight_trk_" + std::to_string(i), displaced_weights + i, 1);
  }

  int seed = 123;
  ParticleGenerator<2> gen(seed, {0.00, 0.2});
  std::default_random_engine engine(123);
  std::uniform_real_distribution<> psi_distribution(0, 2*M_PI);
  std::uniform_int_distribution<> n_distribution(2000, 2000);

  auto outfile = TFile::Open("correctionout.root", "RECREATE");
  auto tree = new TTree();
  man.ConnectOutputTree(tree);
  man.SetCalibrationInputFileName("correctionin.root");

  man.AddEventVariable("Event");
  man.AddCorrectionAxis({"Event", 1, 0, 1});

  //Channel A
  for (unsigned int i = 0; i < detectors_ch_a.size(); ++i) {
    std::string name = detectors_ch_a.at(i).Name();
    std::string weight_name = std::string("weight_ch_a_") + std::to_string(i);
    man.AddDetector(name, Qn::DetectorType::CHANNEL, "phi_ch", weight_name, {}, {1, 2});
    Qn::Recentering recentering;
    recentering.SetApplyWidthEqualization(false);
    man.AddCorrectionOnQnVector(name, recentering);
    Qn::TwistAndRescale twist_and_rescale;
    twist_and_rescale.SetApplyRescale(true);
    twist_and_rescale.SetTwistAndRescaleMethod(Qn::TwistAndRescale::Method::DOUBLE_HARMONIC);
    man.AddCorrectionOnQnVector(name, twist_and_rescale);
    man.SetOutputQVectors(name, {Qn::QVector::CorrectionStep::PLAIN,
                                 Qn::QVector::CorrectionStep::RECENTERED,
                                 Qn::QVector::CorrectionStep::TWIST,
                                 Qn::QVector::CorrectionStep::RESCALED
    });
    man.AddHisto1D(name, {"phi", 10, 0, 2*M_PI}, weight_name);
  }

  //Channel B
  for (unsigned int i = 0; i < detectors_ch_b.size(); ++i) {
    std::string name = detectors_ch_b.at(i).Name();
    std::string weight_name = std::string("weight_ch_b_") + std::to_string(i);
    man.AddDetector(name, Qn::DetectorType::CHANNEL, "phi_ch", weight_name, {}, {1, 2});
    Qn::Recentering recentering;
    recentering.SetApplyWidthEqualization(false);
    man.AddCorrectionOnQnVector(name, recentering);
    Qn::TwistAndRescale twist_and_rescale;
    twist_and_rescale.SetApplyRescale(true);
    twist_and_rescale.SetTwistAndRescaleMethod(Qn::TwistAndRescale::Method::DOUBLE_HARMONIC);
    man.AddCorrectionOnQnVector(name, twist_and_rescale);
    man.SetOutputQVectors(name, {Qn::QVector::CorrectionStep::PLAIN,
                                 Qn::QVector::CorrectionStep::RECENTERED,
                                 Qn::QVector::CorrectionStep::TWIST,
                                 Qn::QVector::CorrectionStep::RESCALED
    });
    man.AddHisto1D(name, {"phi", 10, 0, 2*M_PI}, weight_name);
  }

  //Tracking
  for (unsigned int i = 0; i < detectors_trk.size(); ++i) {
    std::string name = detectors_trk.at(i).Name();
    std::string weight_name = std::string("weight_trk_") + std::to_string(i);
    man.AddDetector(name, Qn::DetectorType::TRACK, "phi", weight_name, {}, {1, 2});
    Qn::Recentering recentering;
    recentering.SetApplyWidthEqualization(false);
    man.AddCorrectionOnQnVector(name, recentering);
    Qn::TwistAndRescale twist_and_rescale;
    twist_and_rescale.SetApplyRescale(true);
    twist_and_rescale.SetTwistAndRescaleMethod(Qn::TwistAndRescale::Method::DOUBLE_HARMONIC);
    man.AddCorrectionOnQnVector(name, twist_and_rescale);
    man.SetOutputQVectors(name, {Qn::QVector::CorrectionStep::PLAIN,
                                 Qn::QVector::CorrectionStep::RECENTERED,
                                 Qn::QVector::CorrectionStep::TWIST,
                                 Qn::QVector::CorrectionStep::RESCALED
    });
    man.AddHisto1D(name, {"phi", kGranularity, 0, 2*M_PI}, weight_name);
  }

  man.InitializeOnNode();
  auto correction_list = man.GetCorrectionList();
  auto correction_qa_list = man.GetCorrectionQAList();
  man.SetCurrentRunName("test");

  double *values = man.GetVariableContainer();
  for (int iev = 0; iev < 10000; ++iev) {
    man.Reset();
    for (auto &det : detectors_ch_a) {
      det.Reset();
    }
    for (auto &det : detectors_ch_b) {
      det.Reset();
    }
    values[kEvent] = 0.5;
    if (man.ProcessEvent()) {
      auto psi = psi_distribution(engine);
      //Tracking
      for (int n = 0; n < n_distribution(engine); ++n) {
        auto phi = gen.GetPhi(psi);
        for (std::size_t j = 0; j < detectors_trk.size(); ++j) {
          detectors_trk.at(j).Detect(phi);
          detectors_trk.at(j).FillDataRec(values, displaced_phi, displaced_weights + j);
        }
        man.FillTrackingDetectors();
      }
      //Channel A
      for (int n = 0; n < n_distribution(engine); ++n) {
        auto phi = gen.GetPhi(psi);
        for (auto &j : detectors_ch_a) {
          j.Detect(phi);
        }
      }
      for (std::size_t j = 0; j < detectors_ch_a.size(); ++j) {
        detectors_ch_a[j].FillDataRec(values, kPhiCh, kWeightCh + j*kNchannels);
      }
      //Channel B
      for (int n = 0; n < n_distribution(engine); ++n) {
        auto phi = gen.GetPhi(psi);
        for (auto &j : detectors_ch_b) {
          j.Detect(phi);
        }
      }
      for (std::size_t j = 0; j < detectors_ch_b.size(); ++j) {
        detectors_ch_b[j].FillDataRec(values, kPhiCh, after_ch_a + j*kNchannels);
      }
      man.FillChannelDetectors();
    }
    man.ProcessCorrections();
  }
  man.Finalize();
  outfile->cd();
  tree->Write("tree");
  correction_list->Write("CorrectionHistograms", TObject::kSingleKey);
  correction_qa_list->Write("CorrectionQAHistograms", TObject::kSingleKey);
  outfile->Close();

  auto begin = std::chrono::high_resolution_clock::now(); // start of timing

  std::string tree_file_name = "correctionout.root";
  ROOT::EnableImplicitMT();
  auto file = TFile::Open(tree_file_name.data());
  if (!file) return 1;
  TTreeReader reader("tree", file);

  std::vector<std::string> correction_steps_t{"PLAIN", "RECENTERED", "TWIST", "RESCALED"};
  std::vector<std::string> correction_steps;
  for (const auto &step : correction_steps_t) {
    auto det = detectors_trk.at(0);
    std::string name(det.Name() + "_" + step);
    TTreeReaderValue<Qn::DataContainerQVector> value_test(reader, name.data());
    reader.SetLocalEntry(1);
    if (value_test.GetSetupStatus() < 0) {
      std::cout << "Branch " << value_test.GetBranchName() << std::endl;
      std::cout << "correction_step " + step + " not available." << std::endl;
    } else {
      correction_steps.push_back(step);
    }
    reader.Restart();
  }

  const std::size_t n_samples = 1000;

  auto v2_2 = [](const Qn::QVector &a) {
    auto Q = a.DeNormal();
    auto M = Q.sumweights();
    return (Qn::ScalarProduct(Q, Q, 2) - M)/(M*(M - 1));
  };

  auto scalar = [](const Qn::QVector &a, const Qn::QVector &b) {
    return Qn::ScalarProduct(a, b, 2);
  };

  auto v2_11 = [](const Qn::QVector &a, const Qn::QVector &b, const Qn::QVector &c) {
    return a.x(2)*b.x(1)*c.x(1);
  };

  auto xx = [](const Qn::QVector &a, const Qn::QVector &b) {
    return b.x(1)*a.x(1);
  };

  std::cout << reader.GetEntries(true) << std::endl;
  Qn::AxisD event("Event", 1, 0, 1);
  Qn::Correlation::ReSampler re_sampler(n_samples);
  ROOT::RDataFrame df("tree", tree_file_name);
  auto df_samples = df.Define("Samples", re_sampler, {});

  auto detector_names_trk = CreateDetectorNames(detectors_trk.size(), "DetTrk", correction_steps);
//  auto detector_names_cha = CreateDetectorNames(detectors_trk.size(), "DetChA", correction_steps);
//  auto detector_names_chb = CreateDetectorNames(detectors_trk.size(), "DetChB", correction_steps);

  std::map<std::string, ROOT::RDF::RResultPtr<Qn::DataContainerStats>> correlations;
  for (const auto &detector : detector_names_trk) {
    std::string correlation_name{"v22"};
    auto c22 = Qn::Correlation::MakeCorrelation(correlation_name, v2_2, Qn::Correlation::MakeAxes(event))
        .SetInputNames(detector)
        .SetWeights(Qn::Stats::Weights::OBSERVABLE).BookMe(df_samples, reader, n_samples);
    correlations.emplace(detector + "_" + correlation_name, c22);
  }

//  for (std::size_t i = 0; i < detector_names_cha.size(); ++i) {
//    std::string correlation_name{"v2scalar"};
//    auto v2 = Qn::Correlation::MakeCorrelation(correlation_name, scalar, Qn::Correlation::MakeAxes(event))
//        .SetInputNames(detector_names_trk.at(i), detector_names_cha.at(i))
//        .SetWeights(Qn::Stats::Weights::OBSERVABLE, Qn::Stats::Weights::REFERENCE)
//        .BookMe(df_samples, reader, n_samples);
//    correlations.emplace(detector_names_cha.at(i) + "_" + correlation_name, v2);
//  }
//
//  for (std::size_t i = 0; i < detector_names_cha.size(); ++i) {
//    std::string correlation_name{"v2_11"};
//    auto v2 = Qn::Correlation::MakeCorrelation(correlation_name, v2_11, Qn::Correlation::MakeAxes(event))
//        .SetInputNames(detector_names_trk.at(i), detector_names_cha.at(i), detector_names_chb.at(i))
//        .SetWeights(Qn::Stats::Weights::OBSERVABLE, Qn::Stats::Weights::REFERENCE, Qn::Stats::Weights::REFERENCE)
//        .BookMe(df_samples, reader, n_samples);
//    correlations.emplace(detector_names_cha.at(i) + "_" + correlation_name, v2);
//  }

//  for (std::size_t i = 0; i < detectors_ch_b.size(); ++i) {
//    std::string correlation_name{"xx"};
//    auto v2 = Qn::Correlation::MakeCorrelation(correlation_name, xx, Qn::Correlation::MakeAxes(event))
//        .SetInputNames(detector_names_cha.at(i), detector_names_chb.at(i))
//        .SetWeights(Qn::Stats::Weights::REFERENCE, Qn::Stats::Weights::REFERENCE)
//        .BookMe(df_samples, reader, n_samples);
//    correlations.emplace(detector_names_cha.at(i) + "_" + correlation_name, v2);
//  }

  auto out_file = new TFile("correlationout.root", "RECREATE");
  out_file->cd();
  for (auto &correlation : correlations) {
    correlation.second.GetValue().Write(correlation.first.data());
  }
  out_file->Close();

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> fp_time = end - begin;
  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(fp_time);
  auto minutes = std::chrono::duration_cast<std::chrono::minutes>(fp_time);
  std::cout << minutes.count() << " minutes " << seconds.count() - minutes.count()*60 << " seconds" << std::endl;
}

