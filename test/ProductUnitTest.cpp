
#include <gtest/gtest.h>
#include "Product.h"
#include "DataContainer.h"
#include "QVector.h"
#include "Correlation.h"
#include "Correlator.h"

TEST(ProductTest, constructor) {
  Qn::Product p(10, 5, true);
  EXPECT_EQ(p.validity, true);
}

TEST(ProductTest, Correlation) {
  Qn::DataContainer<Qn::QVector> container_a;
  int size = 10;
  container_a.AddAxes({{"a1", size, 0, 100}, {"a2", size, 0, 100}});
  for (auto &bin : container_a) {
    Qn::QVec qvec(1.0, 1.0);
    std::vector<Qn::QVec> vecarray = {{qvec, qvec, qvec, qvec}};
    bin = Qn::QVector(Qn::QVector::Normalization::M, 1, 1, vecarray);
  }
  Qn::DataContainer<Qn::QVector> container_b(container_a);
  std::vector<Qn::DataContainer<Qn::QVector>> vector{container_a, container_b};
  std::vector<Qn::Axis> axes = {{"eva1", size, 0, 10}};

  auto lambda = [](std::vector<Qn::QVector> &a) { return a[0].x(1) + a[1].x(1); };

  Qn::Correlation correlation(vector, axes, lambda);
  for (int i = 0; i < 1000; ++i) {
    correlation.Fill(vector, {(unsigned long) (i/1000.0)});
    EXPECT_FLOAT_EQ(2.0,
                    correlation.GetCorrelation().At(((int) i/1000.0)*container_a.size()*container_b.size()).result);
  }
  EXPECT_EQ(size*size*size*size*size, correlation.GetCorrelation().size());
}

TEST(ProductTest, Correlator) {
  int nevents = 1000;
  int nsamples = 100;
  Qn::DataContainer<Qn::QVector> container_a;
  container_a.AddAxes({{"a1", 2, 0, 10}});
  container_a.At(0) = Qn::QVector(Qn::QVector::Normalization::M, 200, 1, {{{1.0, 1.0}, {1.0, 1.0}}});
  container_a.At(1) = Qn::QVector(Qn::QVector::Normalization::M, 400, 1, {{{2.0, 2.0}, {2.0, 2.0}}});
  Qn::DataContainer<Qn::QVector> container_b(container_a);
  std::vector<Qn::DataContainer<Qn::QVector>> vector{container_a, container_b};
  std::vector<Qn::Axis> axes = {{"eva1", 2, 0, 10}};
  std::vector<std::string> names = {"DET1", "DET1"};
  auto lambda = [](std::vector<Qn::QVector> &a) { return a[0].x(1) + a[1].x(1); };
  Qn::Correlator correlator(names, lambda);
  correlator.ConfigureCorrelation(vector, axes);
  correlator.ConfigureSampler(Qn::Sampler::Method::SUBSAMPLING, nsamples);
  correlator.BuildSamples(nevents);
  for (int i = 0; i < nevents; ++i) {
//    auto isample = static_cast<unsigned int>(i / 100);
    correlator.FillCorrelation(vector, {0}, i);
    correlator.FillCorrelation(vector, {1}, i);
  }
  auto correlation = correlator.GetResult();
//  std::cout << "mean " << correlation.At(7).Mean() << std::endl;
//  auto proj1 = correlation.Projection({"0_DET1_a1", "eva1"});
//  auto proj2 = correlation.Projection({"eva1"});
//  EXPECT_FLOAT_EQ((correlation.At(0).Mean()*correlation.At(0).MultWeight()
//                      + correlation.At(1).MultWeight()*correlation.At(1).Mean())
//                          /(correlation.At(0).MultWeight() + correlation.At(1).MultWeight()), proj1.At(0).Mean());
}