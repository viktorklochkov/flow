//
// Created by Lukas Kreis on 30.01.18.
//


#include <gtest/gtest.h>
#include <TChain.h>
#include "StatsResult.h"
#include "CorrelationManager.h"

TEST(CorrelationManagerTest,test) {
  auto begin = std::chrono::steady_clock::now(); // start of timing
  // abbreviations for convenience
  using QVectors = Qn::QVectors;
  auto constexpr kRef = Qn::kRef;
  auto constexpr kObs = Qn::kObs;
  auto constexpr ese = false;
  std::string inputname("/Users/lukas/zdc/test/list2");
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