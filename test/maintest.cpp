
#include "CorrelationManager.h"

int main() {
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
