//
// Created by Lukas Kreis on 30.01.18.
//
#include <gtest/gtest.h>

#include "DataContainer.h"

#include <TList.h>
#include <TFile.h>
#include <TRandom3.h>
#include <random>
#include <TProfile.h>
#include <ROOT/RDataFrame.hxx>
#include "FlowDataFrame.h"

TEST(DataContainerTest, TestHelper) {

//  ROOT::EnableImplicitMT(24);
  ROOT::RDataFrame df0("tree", "~/testhelper/mergedtree.root");
  auto df = df0.Cache({"CentralityV0M","VtxZ","ZNA_PLAIN","Trigger"}).Filter("Trigger==0", "minbias");
  auto axes1 = Qn::EventAxes(Qn::AxisD{"CentralityV0M", 100, 0, 100});

//  auto file = TFile::Open("~/testhelper/tt.root","READ");
  auto treef = TFile::Open("~/testhelper/mergedtree.root","READ");
  TTreeReader reader("tree",treef);
  using RecenterStep1 = Qn::Correction::RecenterAction<decltype(axes1), decltype(axes1)::AxisValueTypeTuple>;
  std::vector<RecenterStep1> correctionstep1{};
  correctionstep1.push_back(Qn::Correction::Recentering("cent", axes1,"ZNA_PLAIN",""));
  std::vector<ROOT::RDF::RResultPtr<RecenterStep1>> resultptrstep1;
  std::vector<std::string> names_;
  for (auto &correction : correctionstep1) {
    resultptrstep1.push_back(Qn::EventAverage(correction).SetExternalTTreeReader(&reader).BookMe(df));
//    correction.LoadCorrectionFromFile(file, reader);
    names_.push_back(correction.GetName());
  }
  auto correctedstep1 = Qn::Correction::ApplyCorrectionsVector(df, resultptrstep1);
  for (auto & name : correctedstep1.GetColumnNames()) {
    std::cout << name << " ";
  }
  std::cout << std::endl;
  auto axes2 = Qn::EventAxes(Qn::AxisD{"VtxZ", 5, -10, 10});
  using RecenterStep2 = Qn::Correction::RecenterAction<decltype(axes2), decltype(axes2)::AxisValueTypeTuple>;
  std::vector<RecenterStep2> correctionstep2{};
  correctionstep2.push_back(Qn::Correction::Recentering("z", axes2, "ZNA_PLAIN", "cent"));
  std::vector<ROOT::RDF::RResultPtr<RecenterStep2>> resultptrstep2;
  for (auto &correction : correctionstep2) {
    resultptrstep2.push_back(Qn::EventAverage(correction).SetExternalTTreeReader(&reader).BookMe(correctedstep1));
//    correction.LoadCorrectionFromFile(file, reader);
    names_.push_back(correction.GetName());
  }
  auto correctedstep2 = Qn::Correction::ApplyCorrectionsVector(correctedstep1, resultptrstep2);
  for (auto & name : correctedstep2.GetColumnNames()) {
    std::cout << name << " ";
  }
  std::cout << std::endl;
  auto other_branches = {"CentralityV0M", "VtxZ"};
  names_.insert(std::end(names_), std::begin(other_branches), std::end(other_branches));
  auto dfcorrected = correctedstep2.Snapshot("tree", "~/testhelper/rectree.root", names_);
//  auto dfsamples = Qn::Correlation::Resample(*dfcorrected, 100);
//  std::vector<ROOT::RDF::RResultPtr<Qn::Correlation::CorrelationActionBase>> vecs;
//      vecs.push_back(Qn::EventAverage(Qn::Correlation::Correlation("test",
//                                                         Qn::Correlation::TwoParticle::ScalarProduct(2, 2),
//                                                         {"TPCPT_PLAIN_test", "V0C_PLAIN_test"},
//                                                         {Qn::Stats::Weights::REFERENCE, Qn::Stats::Weights::REFERENCE},
//                                                         Qn::EventAxes(Qn::AxisD{"CentralityV0M", 10, 0., 100.}),
//                                                         100)).BookMe(dfsamples));
//
//  auto &ttr = vecs.front()->GetDataContainer();
  auto file = TFile::Open("~/testhelper/tt.root", "RECREATE");
  file->cd();
////  ttr.Write("test");
  resultptrstep1[0]->Write(file);
  resultptrstep2[0]->Write(file);
  file->Close();
  delete file;
//  using Weights = Qn::Stats::Weights;
//  using Qn::Correlation::Correlation;
// file->Close();
// delete file;
 treef->Close();
 delete treef;
}

TEST(DataContainerTest, equalbinning) {
  int nbins = 10;
  auto
      axes = Qn::EventAxes(Qn::AxisD{"t2", nbins, -2, 2}, Qn::AxisD{"t1", nbins, -2, 2}, Qn::AxisD{"t3", nbins, -2, 2});
  std::mt19937_64 gen(10);
  std::normal_distribution<> normal_distribution(0, 0.1);
  std::vector<double> hist1(nbins);
  std::vector<double> values;
  ROOT::RDataFrame df(1000);
  auto df1 = df.Define("t1", [&normal_distribution, &gen]() { return normal_distribution(gen); }, {})
      .Define("t2", [&normal_distribution, &gen]() { return normal_distribution(gen); }, {});
  df1.Foreach([& hist1, &values, axes](const double x) {
    auto bin = axes.GetLinearIndex(x);
    if (bin!=-1) {
      ++hist1[bin];
    }
    values.push_back(x);
  }, {"t1"});
  Qn::EqualizeBinning(axes, df1, {"t1", "t2"});
//  Qn::EqualEntriesBinner binner;
//  auto bins = binner.CalculateBins(values, nbins, -2., 2.);
//  Qn::AxisD axis2("t2", bins);
//  std::vector<double> hist2(nbins);
//  for (auto &value : values) {
//    auto bin = axis2.FindBin(value);
//    if (bin!=-1) {
//      ++hist2[bin];
//    }
//  }
//  for (int i = 0; i < hist1.size(); ++i) {
//    std::cout << hist1[i] << " " << hist2[i] << std::endl;
//  }
//  axis2.Print();
//}

//TEST(DataContainerTest, Copy) {
//  Qn::DataContainerQVector container;
//  container.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
//  Qn::DataContainerQVector copy(container);
//  EXPECT_EQ(copy.size(), container.size());
//}
//

//
//
}

TEST(DataContainerTest, function) {
  Qn::DataContainerStats stats({{"foo",10,0,10}});
  auto scale = new TF1("scale","x*x",0,10);
  for (auto &bin : stats) { bin.Fill(1,1,std::vector<int>{}); }
  auto stats_scaled = stats.ApplyTF1(scale);
  for (const auto& bin : stats_scaled) {
    std::cout << bin.Mean() << std::endl;
  }
}

TEST(DataContainerTest, Strides) {
  Qn::DataContainerStats stats;
  Qn::AxisD foo("foo",10,0,10);
  Qn::AxisD bar("bar",5,0,5);
  stats.AddAxis(bar);
  stats.AddAxis(foo);
  auto stride = foo.size();
  auto axesconfig = Qn::EventAxes(bar);
  auto lin = axesconfig.GetLinearIndex(1.5);
  for (int i = 0; i < 10; ++i) {
    std::cout << lin*stride + i << std::endl;
    stats.At(lin*stride + i).Fill(-1,1,std::vector<int>{});
  }
  for (unsigned long i = 0; i < 10; ++i) {
    std::cout << stats[std::vector<double>{1.5,i*1.}].Mean() << std::endl;
  }
  auto x = Qn::Abs(stats);
}

TEST(DataContainerTest, AddAxes) {
  Qn::DataContainer<double, Qn::AxisD> container;
  container.AddAxes({{"ev1", 2, 0, 2},
                     {"ev2", 2, 0, 2},
                     {"a1", 2, 0, 10},
                     {"a2", 2, 0, 10}});
  Qn::DataContainer<double, Qn::AxisD> c2;
  c2.AddAxes({{"ev1", 2, 0, 2}, {"ev2", 2, 0, 2}});
//  EXPECT_EQ(100, container.size());
  for (std::size_t i = 0; i < container.size(); ++i) {
    auto index = container.GetIndex(i);
    std::cout << "linear: " << i << " multi: ";
    for (const auto &in :index) {
      std::cout << in << " ";
    }
    std::cout << std::endl;
  }
  std::vector<double> vec(2);
  for (unsigned int i = 0; i < 2; ++i) {
    for (unsigned int j = 0; j < 2; ++j) {
      vec = {(double) i, (double) j};
      auto ibin = c2.FindBin(vec)*c2.size();
      for (unsigned int k = 0; k < 4; ++k) {
//        std::cout << i << " " << j << " " << ibin << " " << k << std::endl;
        std::cout << ibin + k << std::endl;
        container.At(ibin + k) = i + 2*j;
      }
    }
  }
}
//
//TEST(DataContainerTest, Filter) {
//  Qn::DataContainer<float,float> container;
//  container.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
//  for (auto &bin : container) {
//    bin = 1;
//  }
//  const auto lambda = [](const std::vector<Qn::AxisF> &axes, const std::vector<std::size_t> binindex) {
//    return axes[0].GetLowerBinEdge(binindex[0]) >= 4.9999 && axes[1].GetLowerBinEdge(binindex[1]) < 2.;
//  };
//  auto filtered = container.Filter(lambda);
//  auto proj = container.Filter(lambda).Projection();
//  EXPECT_EQ(proj.At(0), 10);
//  for (unsigned int ibin = 0; ibin < container.size(); ++ibin) {
//    if (ibin==50 || ibin==51 || ibin==60 || ibin==61 || ibin==70 || ibin==71
//        || ibin==80 || ibin==81 || ibin==90 || ibin==91) {
//      EXPECT_EQ(filtered.At(ibin), 1);
//    } else {
//      EXPECT_EQ(filtered.At(ibin), 0);
//    }
//  }
//}
//
//TEST(DataContainerTest, Projection) {
//  Qn::DataContainer<float, float> container;
//  container.AddAxes({{"a1", 30, 0, 10}, {"a2", 20, 0, 10}, {"a3", 10, 0, 10}});
//  for (auto &bin : container) {
//    bin = 1;
//  }
//  auto projection = container.Projection({"a1", "a2"}, [](float a, float b) { return a + b; });
//  for (const auto &bin : projection) {
//    EXPECT_EQ(10, bin);
//  }
//}
//
//TEST(DataContainerTest, ProjectionStats) {
//  Qn::DataContainerStats container;
//  container.AddAxes({{"a1", 30, 0, 10}});
//  for (auto &bin : container) {
//    bin.SetNumberOfSubSamples(10);
//    bin.Fill({1, true, 2.}, {});
//  }
//  auto projection = container.Projection();
//  for (const auto &bin : projection) {
//    EXPECT_EQ(bin.GetNSamples(), 10);
//  }
//}
//
//TEST(DataContainerTest, ProjectionStats2d) {
//  Qn::DataContainerStats container;
//  container.AddAxes({{"a1", 30, 0, 10}, {"a2", 30, 0, 10}});
//  for (auto &bin : container) {
//    bin.SetNumberOfSubSamples(10);
//    bin.Fill({1, true, 2.});
//  }
//  auto projection = container.Projection({"a2"});
//  for (const auto &bin : projection) {
//    EXPECT_EQ(bin.GetNSamples(), 10);
//  }
//}

//TEST(DataContainerTest, ProjectionStats) {
//  Qn::DataContainerStats container;
//  container.AddAxes({{"a1", 30, 0, 10}});
//  for (auto &bin : container) {
//    bin.Fill({{2.},1,true},{});
//  }
//  auto projection = container.Projection();
////  for (const auto &bin : projection) {
////    EXPECT_EQ(10, bin);
////  }
//}

//TEST(DataContainerTest, SqrtStats) {
//  Qn::DataContainerStats container;
//  container.AddAxes({{"a1", 30, 0, 10}});
//  for (auto &bin : container) {
//    bin.Fill({2., true, 2.}, {});
//  }
//  auto srt = Sqrt(container);
//  EXPECT_FLOAT_EQ(sqrt(2), srt.At(0).Mean());
//}
//
//TEST(DataContainerTest, Division) {
//  Qn::DataContainer<float, float> container;
//  container.AddAxes({{"a1", 30, 0, 10}, {"a2", 40, 0, 20}, {"a3", 40, 0, 20}});
//  for (auto &bin : container) {
//    bin = 4;
//  }
//  Qn::DataContainer<float, float> b;
//  b.AddAxes({{"a1", 30, 0, 10}, {"a2", 40, 0, 20}});
//  for (auto &bin : b) {
//    bin = 2;
//  }
//  auto projection = container/b;
//  for (const auto &bin : projection) {
//    EXPECT_EQ(2, bin);
//  }
//}


//TEST(DataContainerTest, ExclusiveSum) {
//  Qn::DataContainerF container;
//  container.AddAxes({{"a1", 2, 0, 10}, {"a2", 2, 0, 10}});
//  for (auto &bin : container) {
//    bin = 1;
//  }
//  auto exsum = Qn::ExclusiveSum(container);
//}

//
//TEST(DataContainerTest, ProjectionProfile) {
//  Qn::DataContainerSample a;
//  a.AddAxes({{"a1",2,0,2},{"b1",2,0,2}});
//  TRandom3 rndm;
//  TProfile pa("pa","pa",1,0,1);
//  TProfile pb("pb","pb",1,0,1);
//  int j = 0;
//  std::vector<float> vec;
//  std::vector<float> vec2;
//  for (int i = 0; i < 100000; ++i) {
//    vec.push_back(rndm.Gaus());
//    vec2.push_back(rndm.Gaus());
//  }
//  for (auto &bin : a) {
//    for (int i = 0; i < 100000; ++i) {
//      if (j==0 || j==2) bin.Update(vec.at(i));
//      if (j==1 || j==3) bin.Update(vec2.at(i));
//      if (j==0) pa.Fill(0.5, vec.at(i));
//      if (j==1) pb.Fill(0.5, vec2.at(i));
//    }
//    j++;
//    std::cout <<"ibin" << j << ": "<< bin.Mean() << " " << bin.Error() << std::endl;
//  }
//  std::cout << "pa" << pa.GetBinContent(1) << " " << pa.GetBinError(1) << std::endl;
//  std::cout << "pb" << pb.GetBinContent(1) << " " << pb.GetBinError(1) << std::endl;
//  auto proj = a.Projection({"a1"},[](const Qn::Sample &a, const Qn::Sample &b){return a+b;});
//  for (auto &bin : proj) {
//    std::cout << bin.Mean() << " " << bin.Error() << std::endl;
//  }
//  pa.Add(&pb);
//  std::cout << pa.GetBinContent(1) << " " << pa.GetBinError(1) << std::endl;
//}
//
//TEST(DataContainerTest, Rebin) {
//  Qn::DataContainer<float, float> container;
//  container.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
//  for (auto &bin : container) {
//    bin = 1;
//  }
//  auto rebin = container.Rebin({"a1", 5, 0, 10}, [](float a, float b) { return a + b; });
//  int numberofbins = 0;
//  for (const auto &bin : rebin) {
//    EXPECT_EQ(2, bin);
//    numberofbins++;
//  }
//  EXPECT_EQ(50, numberofbins);
//}
////
//TEST(DataContainerTest, Select) {
//  Qn::DataContainer<float, float> container;
//  container.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
//  for (auto &bin : container) {
//    bin = 1;
//  }
//  auto rebin = container.Select({"a1", 5, 0, 5});
//  int numberofbins = 0;
//  for (const auto &bin : rebin) {
//    EXPECT_EQ(1, bin);
//    numberofbins++;
//  }
//  EXPECT_EQ(50, numberofbins);
//}
////
//TEST(DataContainerTest, Addition) {
//  Qn::DataContainer<float, float> container_a;
//  container_a.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
//  for (auto &bin : container_a) {
//    bin = 1;
//  }
//  Qn::DataContainer<float, float> container_b;
//  container_b.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
//  for (auto &bin : container_b) {
//    bin = 1;
//  }
//  auto container_sum = container_a + container_b;
//  int numberofbins = 0;
//  for (const auto &bin : container_sum) {
//    EXPECT_EQ(2, bin);
//    numberofbins++;
//  }
//  EXPECT_EQ(100, numberofbins);
//}
//
//TEST(DataContainerTest, Bits) {
//  Qn::DataContainerStats dcstat;
//  dcstat.SetSetting(Qn::Stats::Settings::CORRELATEDERRORS);
//  std::default_random_engine generator;
//  std::normal_distribution<double> gauss(0, 1);
//  int nsamples = 100;
//  int nevents = 1000;
//  std::for_each(dcstat.begin(), dcstat.end(), [nsamples](Qn::Stats &a) { a.SetNumberOfSubSamples(nsamples); });
//  Qn::Sampler sampler(nsamples, Qn::Sampler::Method::BOOTSTRAP);
//  sampler.SetNumberOfEvents(nevents);
//  sampler.CreateBootstrapSamples();
//  for (int i = 0; i < nevents; ++i) {
//    dcstat.At(0).Fill({gauss(generator), true, 1.}, sampler.GetFillVector(i));
//  }
//  dcstat.TestBit(Qn::Stats::Settings::CORRELATEDERRORS);
//  EXPECT_TRUE(dcstat.At(0).TestBit(Qn::Stats::Settings::CORRELATEDERRORS));
//  dcstat.ResetSetting(Qn::Stats::Settings::CORRELATEDERRORS);
//  unsigned int testbit = BIT(1);
//  dcstat.SetBit(testbit);
//  EXPECT_TRUE(dcstat.TestBit(testbit));
//  EXPECT_FALSE(dcstat.At(0).TestBit(testbit));
//  dcstat.ResetSetting(testbit);
//  EXPECT_TRUE(dcstat.TestBit(testbit));
//  dcstat.SetBit(testbit, false);
//  EXPECT_FALSE(dcstat.TestBit(testbit));
//  EXPECT_FALSE(dcstat.At(0).TestBit(testbit));
//  EXPECT_FALSE(dcstat.At(0).TestBit(Qn::Stats::Settings::CORRELATEDERRORS));
//
//}
//
//TEST(DataContainerTest, GetNameTest) {
//  auto file = new TFile("getnametest.root", "RECREATE");
//  TDirectory *dir = file->mkdir("dir");
//  dir->cd();
//  auto a = new Qn::DataContainerEventShape();
//  a->Write("test", TObject::kSingleKey);
//  file->Close();
//  delete a;
//  delete file;
//  auto file2 = TFile::Open("getnametest.root");
//  auto dir2 = (TDirectory *) file2->Get("dir");
//  dir2->ls();
//  for (const auto &obj : *dir2->GetListOfKeys()) {
//    std::cout << obj->GetName() << std::endl;
//    std::cout << dir2->Get(obj->GetName())->GetName() << std::endl;
//  }
//
//}
//
//TEST(DataContainerTest, LinearIndexing) {
//  Qn::DataContainer<float, float> a;
//  a.AddAxis({"A",2,0,2});
//  a.AddAxis({"B",2,0,2});
//  a.At(0) = 99.;
//  a.At(3) = 10.;
//  auto bina = a[std::vector<double>({0.,0.})];
//  EXPECT_FLOAT_EQ(bina,99.);
//  EXPECT_FLOAT_EQ(a[std::vector<double>({1.5,1.5})],10.);
//}

//
//TEST(DataContainerTest, Hadd) {
//  auto container_a = new Qn::DataContainerEventShape();
//  container_a->AddAxes({{"a1", 1, 0, 10}});
//  for (auto &bin : *container_a) {
//    bin.SetHisto(TH1F("test","test",1,0,1));
//    bin.histo_->Fill(0.5);
//  }
//  auto file_a = new TFile("testa.root", "RECREATE");
//  file_a->WriteObject(container_a, "container");
//  file_a->Write();
//  system("rm test.root");
//  system("hadd test.root testa.root testa.root");
//  auto file_c = new TFile("test.root", "OPEN");
//  auto container_c = (Qn::DataContainerEventShape *) file_c->Get("container");
//  for (auto &bin : *container_c) {
//    EXPECT_FLOAT_EQ(2, bin.histo_->GetEntries());
//  }
//}
//TEST(DataContainerTest, HaddProfile) {
//  auto file_a = new TFile("testa.root", "RECREATE");
//  file_a->cd();
//  auto container_a = new Qn::DataContainerProfile();
//  container_a->AddAxes({{"a1", 1, 0, 10}});
//  for (auto &bin : *container_a) {
//    bin.Update(1.0);
//  }
//  file_a->WriteObject(container_a, "container");
//  file_a->Write();
//  system("rm test.root");
//  system("hadd test.root testa.root testa.root");
//  auto file_c = new TFile("test.root", "OPEN");
//  auto container_c = (Qn::DataContainerProfile *) file_c->Get("container");
//  for (auto &bin : *container_c) {
//    EXPECT_FLOAT_EQ(2, bin.Sum());
//  }
//}

//TEST(DataContainerTest, ProjectionFile) {
//  auto file_c = new TFile("v2pttest.root", "OPEN");
//  auto tpcpt = (Qn::DataContainerSample *) file_c->Get("TPCPTV0A");
//  auto tpc = (Qn::DataContainerSample *) file_c->Get("TPCV0A");
//  tpcpt->Projection({"CentralityV0M"});
//}

//TEST(DataContainerTest, ProjectionExclude) {
//  Qn::DataContainerF container_a;
//  container_a.AddAxes({{"a1", 2, 0, 10}, {"a2", 2, 0, 10}});
//  for (auto &bin : container_a) {
//    bin = 1.0;
//  }
//  auto container_b = container_a.ProjectionEX({"a1"}, [](float a, float b) { return a + b; }, {0});
//  for (auto &bin : container_b) {
//    std::cout << bin << std::endl;
//  }
//}
//
//TEST(DataContainerTest, Diagonal) {
//  Qn::DataContainerF data;
//  data.AddAxes({{"a2", 2, 0, 2}, {"a3", 2, 0, 2}});
//  for (unsigned long i = 0; i < data.GetAxis("a2").size(); i++) {
//    for (unsigned long j = 0; j < data.GetAxis("a3").size(); j++) {
//      data.At({i, i, j}) = 1;
//    }
//  }
//  std::cout << data.At(0) << std::endl;
//}
//
//TEST(DataContainerTest, DiagonalTest) {
//  Qn::DataContainerF data;
//  data.AddAxes({{"a1", 20, 0, 2}, {"a2", 20, 0, 2}, {"a3", 20, 0, 2}});
//  for (auto &bin : data) {
//    bin = 1;
//  }
//  auto test = data.GetDiagonal({{"a1", 20, 0, 2}, {"a2", 20, 0, 2}});
//}
//
//TEST(DataContainerTest, Apply) {
//  Qn::DataContainerF data1({{"a1", 5, 0, 2}});
//  Qn::DataContainerF data2({{"a1", 5, 0, 2}, {"a2", 5, 0, 2}});
//  int ibin = 0;
//  for (auto &bin : data1) {
//    bin = ++ibin;
//  }
//  for (auto &bin : data2) {
//    bin = 1;
//  }
//  auto result = data1.Apply(data2, [](float a, float b) { return a + b; });
//  ibin = 0;
//  for (const auto &bin : result) {
//    EXPECT_FLOAT_EQ(bin, 2 + (ibin++ / 5));
//  }
//}