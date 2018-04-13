//
// Created by Lukas Kreis on 13.04.18.
//

#include <gtest/gtest.h>
#include <Correlation/CorrelationHolder.h>

TEST(CorrelationHolderUnitTest, adding) {
  int nsamples = 10;
  int nevents = 1000;
  Qn::DataContainer<Qn::QVector> container_a;
  container_a.AddAxes({{"a1", 2, 0, 10}, {"a2", 2, 0, 10}});
  for (auto &bin : container_a) {
    Qn::QVec qvec(1.0, 1.0);
    std::array<Qn::QVec, 4> vecarray = {{qvec, qvec, qvec, qvec}};
    bin = Qn::QVector(Qn::QVector::Normalization::NOCALIB, 1, 1, vecarray);
  }
  Qn::DataContainer<Qn::QVector> container_b(container_a);

  std::vector<Qn::Axis> axes = {{"eva1", 1, 0, 10}};

  auto lambda = [](std::vector<Qn::QVector> &a) { return a[0].x(1) + a[1].x(1); };
  std::vector<Qn::DataContainer<Qn::QVector>> vector{container_a, container_b};
  Qn::CorrelationHolder holder("test",{"a1","a2"},lambda,10,Qn::BootstrapSampler::Method::SUBSAMPLING);
  holder.Initialize(nevents, vector,axes);
  TRandom3 rndm;
  for (int i = 0; i < nevents; ++i) {
    holder.FillCorrelation(vector,{0},i);
//    holder.FillToResult(i);
  }
  auto test = *holder.GetResult();
  test.UseCorrelatedErrors(true);
  test.At(0);
}