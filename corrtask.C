void corrtask(std::string filename) {
  R__LOAD_LIBRARY(/Users/lukas/phd/analysis/flow/cmake - build - debug/libFlow.dylib)
  TFile *file = TFile::Open(filename.data());
  file->ls();

  auto tpcva = file->Get("TPCVA");
  auto tpcvc = file->Get("TPCVC");
  auto vavc = file->Get("VAVC");

  auto multiply = [](Qn::Statistics a, Qn::Statistics b) {
    return a*b;
  };

  auto divide = [](Qn::Statistics a, Qn::Statistics b) {
    return a/b;
  };

  auto sqrt = [](Qn::Statistics a) {
    return a.Sqrt();
  };

  auto add = [](Qn::Statistics a, Qn::Statistics b) {
    return a + b;
  };

  auto rvatpcvc = tpcva.Apply(vavc, multiply).Apply(tpcvc, divide).Map(sqrt);
  auto v2tpcva = tpcva.Apply(rvatpcvc, divide);

}