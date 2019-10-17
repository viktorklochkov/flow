#include "ROOT/RDataFrame.hxx"
#include "Correlation.h"
#include "CorrelationHelper.h"
#include "ReSampler.h"

#include "CorrectionManager.h"
#include "ParticleGenerator.h"
#include "ChannelDetector.h"
#include "TrackingDetector.h"

int main() {
  bool correct = false;
  bool correlate = true;
  if (correlate) {
    auto begin = std::chrono::high_resolution_clock::now(); // start of timing

    std::string tree_file_name = "~/phd/analysis/flow/build/correctionout.root";
    ROOT::EnableImplicitMT();
    auto file = TFile::Open(tree_file_name.data());
    if (!file) return 1;
    TTreeReader reader("tree", file);

    const std::size_t n_samples = 100;

//    auto xaxc = [](const Qn::QVector &a, const Qn::QVector &c) {
//      return a.x(1)*c.x(1);
//    };

    auto v2_2 = [](const Qn::QVector &a) {
      auto Q = a.DeNormal();
      auto M = Q.sumweights();
      return (Qn::ScalarProduct(Q, Q, 2) - M)/(M*(M - 1));
    };

//    auto RealPart = [](const Qn::QVector &q, std::size_t h) {
//      return q.x(2*h)*q.x(h)*q.x(h) - q.x(2*h)*q.y(h)*q.y(h) + 2*(q.y(2*h)*q.x(h)*q.y(h));
//    };
//    auto v2_4 = [RealPart](const Qn::QVector &n) {
//      auto Q = n.DeNormal();
//      auto M = Q.sumweights();
//      auto SP_Q_2 = Qn::ScalarProduct(Q, Q, 2);
//      auto SP_Q_4 = Qn::ScalarProduct(Q, Q, 4);
//      return (SP_Q_2*SP_Q_2 + SP_Q_4 - 2*RealPart(Q, 2))/(M*(M - 1)*(M - 2)*(M - 3))
//          - 2*(2*(M - 2)*SP_Q_2 - M*(M - 3))/(M*(M - 1)*(M - 2)*(M - 3));
//    };

    std::cout << reader.GetEntries(true) << std::endl;

    Qn::AxisD event("Event", 1, 0, 1);
    Qn::Correlation::ReSampler re_sampler(n_samples);

    ROOT::RDataFrame df("tree", tree_file_name);

    auto df_samples = df.Define("Samples", re_sampler, {});

    std::vector<std::string> detectors{"C_true_PLAIN", "C_rec_PLAIN", "C_true_RECENTERED", "C_rec_RECENTERED"};
    std::map<std::string, ROOT::RDF::RResultPtr<Qn::DataContainerStats>> correlations;
    for (const auto &detector : detectors) {
      std::string correlation_name;
      auto c22 = Qn::Correlation::MakeCorrelation(correlation_name, v2_2, Qn::Correlation::MakeAxes(event))
          .SetInputNames(detector)
          .SetWeights(Qn::Stats::Weights::OBSERVABLE).BookMe(df_samples, reader, n_samples);
      correlations.emplace(detector+"_"+correlation_name,c22);
    }

    auto out_file = new TFile("correlationout.root", "RECREATE");
    out_file->cd();
    for (auto &correlation : correlations) {
      auto v22 = Qn::Sqrt(correlation.second.GetValue());
      v22.Write(correlation.first.data());
    }

    out_file->Close();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> fp_time = end - begin;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(fp_time);
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(fp_time);
    std::cout << minutes.count() << " minutes " << seconds.count() - minutes.count()*60 << " seconds" << std::endl;
  }
  if (correct) {

    const int kChannelsA = 8;
    const int kChannelsB = 8;
    const int kGranularity = 10;
    enum variables {
      kPhi_A = 0,
      kWeightRec_A = kPhi_A + kChannelsA,
      kWeightTrue_A = kWeightRec_A + kChannelsA,
      kPhi_B = kWeightTrue_A + kChannelsA,
      kWeightRec_B = kPhi_B + kChannelsB,
      kWeightTrue_B = kWeightRec_B + kChannelsB,
      kPhi_C = kWeightTrue_B + kChannelsB,
      kWeightTrue_C,
      kWeightRec_C,
      kEvent
    };
    ParticleGenerator gen({0.02, 0.07, 0.03, 0.});
    ChannelDetector det_A({"A", kChannelsA, 0, 2*M_PI}, [](const double phi) { return true; });
    det_A.SetChannelEfficencies({0.5, 0.5, 1., 0.7, 0.4, 0.0, 1., 1.});
    ChannelDetector det_B({"B", kChannelsA, 0, 2*M_PI}, [](const double phi) { return true; });
    det_B.SetChannelEfficencies({1, 1, 1, 1, 1, 1, 1, 1});
    TrackingDetector det_C({"C", kGranularity, 0, 2*M_PI}, [](const double phi) { return true; });
    det_C.SetChannelEfficencies({0., 1., 1., 1., 1, 1, 1, 1, 1, 1});

    std::random_device device;
    std::default_random_engine engine(device());
    std::uniform_real_distribution<> psi_distribution(0, 2*M_PI);
    std::uniform_int_distribution<> n_distribution(2000, 2000);

    Qn::CorrectionManager man;
    man.SetFillOutputTree(true);
    man.SetFillCalibrationQA(true);
    man.SetFillValidationQA(true);

    man.AddVariable("phi_A", kPhi_A, kChannelsA);
    man.AddVariable("weight_rec_A", kWeightRec_A, kChannelsA);
    man.AddVariable("weight_true_A", kWeightTrue_A, kChannelsA);

    man.AddVariable("phi_B", kPhi_B, kChannelsB);
    man.AddVariable("weight_rec_B", kWeightRec_B, kChannelsB);
    man.AddVariable("weight_true_B", kWeightTrue_B, kChannelsB);

    man.AddVariable("phi_C", kPhi_C, 1);
    man.AddVariable("weight_rec_C", kWeightRec_C, 1);
    man.AddVariable("weight_true_C", kWeightTrue_C, 1);
    man.AddVariable("Event", kEvent, 1);

    auto outfile = TFile::Open("correctionout.root", "RECREATE");
    auto tree = new TTree();
    man.ConnectOutputTree(tree);
    man.SetCalibrationInputFileName("correctionin.root");

    man.AddEventVariable("Event");
    man.AddCorrectionAxis({"Event", 1, 0, 1});

    man.AddDetector("A_true", Qn::DetectorType::CHANNEL, "phi_A", "weight_true_A", {}, {1, 2});
    man.AddDetector("A_rec", Qn::DetectorType::CHANNEL, "phi_A", "weight_rec_A", {}, {1, 2});

    man.AddDetector("B_true", Qn::DetectorType::CHANNEL, "phi_B", "weight_true_B", {}, {1, 2});
    man.AddDetector("B_rec", Qn::DetectorType::CHANNEL, "phi_B", "weight_rec_B", {}, {1, 2});

    man.AddDetector("C_true", Qn::DetectorType::TRACK, "phi_C", "weight_true_C", {}, {1, 2});
    man.AddDetector("C_rec", Qn::DetectorType::TRACK, "phi_C", "weight_rec_C", {}, {1, 2});

    std::vector<std::string> detectors{"A_rec", "A_true", "B_true", "B_rec", "C_true", "C_rec"};
    for (const auto &detector : detectors) {
      Qn::Recentering recentering;
      recentering.SetApplyWidthEqualization(true);
      man.AddCorrectionOnQnVector(detector, recentering);
      man.SetOutputQVectors(detector, {Qn::QVector::CorrectionStep::PLAIN,
                                       Qn::QVector::CorrectionStep::RECENTERED,
                                       Qn::QVector::CorrectionStep::TWIST,
                                       Qn::QVector::CorrectionStep::RESCALED
      });
      Qn::TwistAndRescale twist_and_rescale;
      twist_and_rescale.SetApplyRescale(true);
      twist_and_rescale.SetTwistAndRescaleMethod(Qn::TwistAndRescale::Method::DOUBLE_HARMONIC);
      man.AddCorrectionOnQnVector(detector, twist_and_rescale);
    }
    man.AddHisto1D("A_true", {"phi_A", 8, 0, 2*M_PI}, "weight_true_A");
    man.AddHisto1D("A_rec", {"phi_A", 8, 0, 2*M_PI}, "weight_rec_A");
    man.AddHisto1D("C_rec", {"phi_C", 10, 0, 2*M_PI}, "weight_rec_C");
    man.AddHisto1D("C_true", {"phi_C", 10, 0, 2*M_PI}, "weight_true_C");

    man.InitializeOnNode();
    auto correction_list = man.GetCorrectionList();
    auto correction_qa_list = man.GetCorrectionQAList();
    man.SetCurrentRunName("test");

    double *values = man.GetVariableContainer();
    for (int iev = 0; iev < 50000; ++iev) {
      man.Reset();
      det_A.Reset();
      det_B.Reset();
      values[kEvent] = 0.5;
      if (man.ProcessEvent()) {
        auto psi = psi_distribution(engine);
        for (int i = 0; i < n_distribution(engine); ++i) {
          auto phi = gen.GetPhi(psi);
          det_A.Detect(phi);
          det_B.Detect(phi);
          if (det_C.Detect(phi)) {
            det_C.FillDataRec(values, kPhi_C, kWeightRec_C);
            det_C.FillDataTruth(values, kPhi_C, kWeightTrue_C);
            man.FillTrackingDetectors();
          }
        }
        det_A.FillDataRec(values, kPhi_A, kWeightRec_A);
        det_A.FillDataTruth(values, kPhi_A, kWeightTrue_A);
        det_B.FillDataRec(values, kPhi_B, kWeightRec_B);
        det_B.FillDataTruth(values, kPhi_B, kWeightTrue_B);
        man.FillChannelDetectors();
        man.ProcessCorrections();
      }
    }
    man.Finalize();
    outfile->cd();
    tree->Write("tree");
    correction_list->Write("CorrectionHistograms", TObject::kSingleKey);
    correction_qa_list->Write("CorrectionQAHistograms", TObject::kSingleKey);
    outfile->Close();
  }
}
