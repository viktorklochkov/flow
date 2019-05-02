//
// Created by Lukas Kreis on 30.01.18.
//


#include <gtest/gtest.h>
#include "StatsResult.h"
#include "CorrelationManager.h"

TEST(CorrelationManagerTest, FullCorrelationWithESE) {
  auto begin = std::chrono::steady_clock::now();
  using QVectors = Qn::QVectors;
  auto constexpr kRef = Qn::kRef;
  auto constexpr kObs = Qn::kObs;
  auto constexpr ese = true;

  auto input_file = TFile::Open("25testtree.root");
  auto tree = dynamic_cast<TTree *>(input_file->Get("tree"));

  std::string out_correlations{"correlations.root"};
  std::string in_calib{"calib.root"};
  std::string out_calib{"calib.root"};
  std::string in_ese{"esetree.root"};
  std::string out_ese{"esetree.root"};

  Qn::CorrelationManager man(tree);
  man.SetOutputFile(out_correlations);
  man.EnableDebug();
  if (ese) {
    man.SetESEInputFile(in_calib, in_ese);
    man.SetESEOutputFile(out_calib, out_ese);
    auto v1mag = [](QVectors q) { return q[0].DeNormal().mag(1)/std::sqrt(q[0].sumweights()); };
    man.AddEventShape("ZDCAq1", {"ZDCA"}, v1mag, {"h", "", 50, 0, 200});
    man.SetRunEventId("RunNumber","EventNumber");
  }
  auto v2zdc_yxx = [](QVectors q) { return q[0].y(2)*q[1].x(1)*q[2].x(1); };
  auto v2zdc_yyy = [](QVectors q) { return q[0].y(2)*q[1].y(1)*q[2].y(1); };
  auto v2zdc_xxy = [](QVectors q) { return q[0].x(2)*q[1].x(1)*q[2].y(1); };
  auto v2zdc_xyx = [](QVectors q) { return q[0].x(2)*q[1].y(1)*q[2].x(1); };
  auto v2zdc_xxx = [](QVectors q) { return q[0].x(2)*q[1].x(1)*q[2].x(1); };
  auto v2zdc_xyy = [](QVectors q) { return q[0].x(2)*q[1].y(1)*q[2].y(1); };
  auto v2zdc_yxy = [](QVectors q) { return q[0].y(2)*q[1].x(1)*q[2].y(1); };
  auto v2zdc_yyx = [](QVectors q) { return q[0].y(2)*q[1].y(1)*q[2].x(1); };
  auto scalar = [](QVectors q) { return q[0].x(2)*q[1].x(2) + q[0].y(2)*q[1].y(2); };

  man.AddEventAxis({"CentralityV0M", 70, 0., 70.});

  man.AddCorrelation("TPCPTV0A", {"TPCPT", "V0A"}, scalar, {kObs, kRef});
  man.AddCorrelation("TPCPTV0C", {"TPCPT", "V0C"}, scalar, {kObs, kRef});
  man.AddCorrelation("TPCV0A", {"TPC", "V0A"}, scalar, {kRef, kRef});
  man.AddCorrelation("TPCV0C", {"TPC", "V0C"}, scalar, {kRef, kRef});
  man.AddCorrelation("V0CV0A", {"V0A", "V0C"}, scalar, {kRef, kRef});
  man.AddCorrelation("ZDCAC_XX", {"ZDCA", "ZDCC"}, [](QVectors q) { return -q[0].x(1)*q[1].x(1); }, {kRef, kRef});
  man.AddCorrelation("ZDCAC_YY", {"ZDCA", "ZDCC"}, [](QVectors q) { return q[0].y(1)*q[1].y(1); }, {kRef, kRef});
  man.AddCorrelation("ZDCAC_YX", {"ZDCA", "ZDCC"}, [](QVectors q) { return q[0].y(1)*q[1].x(1); }, {kRef, kRef});
  man.AddCorrelation("ZDCAC_XY", {"ZDCA", "ZDCC"}, [](QVectors q) { return q[0].x(1)*q[1].y(1); }, {kRef, kRef});

  man.AddCorrelation("ZDCAC_YXX", {"TPC", "ZDCA", "ZDCC"}, v2zdc_yxx, {kObs, kRef, kRef});
  man.AddCorrelation("ZDCAC_YYY", {"TPC", "ZDCA", "ZDCC"}, v2zdc_yyy, {kObs, kRef, kRef});
  man.AddCorrelation("ZDCAC_XYX", {"TPC", "ZDCA", "ZDCC"}, v2zdc_xyx, {kObs, kRef, kRef});
  man.AddCorrelation("ZDCAC_XXY", {"TPC", "ZDCA", "ZDCC"}, v2zdc_xxy, {kObs, kRef, kRef});
  man.AddCorrelation("ZDCAC_XXX", {"TPC", "ZDCA", "ZDCC"}, v2zdc_xxx, {kObs, kRef, kRef});
  man.AddCorrelation("ZDCAC_XYY", {"TPC", "ZDCA", "ZDCC"}, v2zdc_xyy, {kObs, kRef, kRef});
  man.AddCorrelation("ZDCAC_YXY", {"TPC", "ZDCA", "ZDCC"}, v2zdc_yxy, {kObs, kRef, kRef});
  man.AddCorrelation("ZDCAC_YYX", {"TPC", "ZDCA", "ZDCC"}, v2zdc_yyx, {kObs, kRef, kRef});

  man.SetResampling(Qn::Sampler::Method::BOOTSTRAP, 10);
  man.Run();
  auto end = std::chrono::steady_clock::now();
  std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::minutes>(end - begin).count() << " minutes"
            << std::endl;

}

TEST(CorrelationManagerTest, AddingCorrelation) {
//  using namespace Qn;
//  auto data1 = new Qn::DataContainer<Qn::QVector>();
//  data1->AddAxis({"test",10,0,10});
  // for (auto &bin : *data1) {
  //   Qn::QVec qvec(1.0, 1.0);
  //   bin = Qn::QVector(Qn::QVector::Normalization::NONE, 1, 2, {{qvec, qvec, qvec, qvec}});
  // }
  // TFile testtree("testtree.root", "RECREATE");
  // TTree tree("tree", "tree");
  // auto event = new Qn::EventInfoF();
  // event->AddVariable("Ev1");
  // event->SetToTree(tree);

  // tree.Branch("Det1", &data1);
  // tree.Branch("Det2", &data1);

  // auto ne = 10000;
  // int counter = 0;
  // for (int i = 0; i < ne/2; ++i) {
  //   ++counter;
  //   event->SetVariable("Ev1", 0.5);
  //   for (auto &bin : *data1) {
  //     Qn::QVec qvec(1.0, 1.0);
  //     bin = Qn::QVector(Qn::QVector::Normalization::NONE, 1, 2, {{qvec, qvec, qvec, qvec}});
  //   }
  //   tree.Fill();
  // }
  // for (int i = ne/2; i < ne; ++i) {
  //   ++counter;
  //   event->SetVariable("Ev1", 0.5);
  //   for (auto &bin : *data1) {
  //     Qn::QVec qvec(2.0, 2.0);
  //     bin = Qn::QVector(Qn::QVector::Normalization::NONE, 1, 2, {{qvec, qvec, qvec, qvec}});
  //   }
  //   tree.Fill();
  // }
  // tree.Write();
  // testtree.Close();
  // auto readtreefile = TFile::Open("testtree.root", "READ");
  // auto reading = (TTree *) readtreefile->Get("tree");
  // std::cout << counter << std::endl;
  // EXPECT_EQ(ne, reading->GetEntries());
  // std::cout << "create manager" << std::endl;
////  reading->AddFriend("ESE", "percentiles.root");
  // Qn::CorrelationManager manager(reading);
  // std::cout << "add variables" << std::endl;

  // manager.AddEventAxis({"Ev1", 2, 0, 2});
  // manager.SetOutputFile("correlation.root");
  // manager.SetESEInputFile("/Users/lukas/phd/analysis/flow/cmake-build-debug/test/calib.root",
  //                         "/Users/lukas/phd/analysis/flow/cmake-build-debug/test/percentiles.root");
  // manager.SetESEOutputFile("calib.root", "percentiles.root");
  // manager.AddEventShape("Det1ESE", {"Det1"}, [](QVectors q) { return q[0].x(1); }, {"h", "h", 2, 0., 3.});
  // manager.AddCorrelation("c1",
  //                        {"Det1", "Det2"},
  //                        [](QVectors q) { return q[0].y(1) + q[1].x(1); },
  //                        {kObs, kObs});
////  manager.AddCorrelation("avg",
////                         {"Det1"},
////                         [](QVectors q) { return q[0].mag(2)/sqrt(q[0].n()); },
////                         {Weight::OBSERVABLE},
////                         Sampler::Resample::OFF);
////  manager.AddCorrelation("c2",
////                         {"Det1", "Det2"},
////                         [](QVectors q) { return q[0].x(1) + q[1].x(1); },
////                         {Weight::OBSERVABLE, Weight::REFERENCE});
  // manager.SetResampling(Qn::Sampler::Method::BOOTSTRAP, 100);
  // std::cout << "run" << std::endl;
  // manager.Run();
  // auto correlation = manager.GetResult("c1");
  // for (unsigned long i = 0; i < 10; ++i) {
  //   auto bin = correlation.At({0, 9, i});
  //   EXPECT_FLOAT_EQ(4.0, bin.Mean());
  //   EXPECT_EQ(100, bin.GetNSamples());
  // }
  // for (unsigned long i = 0; i < 10; ++i) {
  //   auto bin = correlation.At({0, 5, i});
  //   EXPECT_FLOAT_EQ(2.0, bin.Mean());
  //   EXPECT_EQ(100, bin.GetNSamples());
  // }
//  auto average = manager.GetResult("avg");
//  for (auto &bin : average) {
//    EXPECT_FLOAT_EQ((sqrt(8) + sqrt(2))/2, bin.Mean());
//    EXPECT_EQ(0, bin.GetNSamples());
//  }
}
