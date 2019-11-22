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

#include "gtest/gtest.h"
#include "ROOT/RDataFrame.hxx"
#include "Correlation.h"
#include "CorrelationHelper.h"
#include "ReSampler.h"
#include "AxesConfiguration.h"

TEST(DataFrameAlgorithmUnitTest, axisnaming) {

  Qn::AxisD a1("a1",10,0,100);
  Qn::AxisD a2("a2",10,0,100);
  Qn::AxisD a3("a3",10,0,100);
  auto axes = Qn::Correlation::MakeAxes(a1, a2, a3);

  auto index = axes.GetLinearIndexFromCoordinates(20.,30.,-1);

//  auto file = TFile::Open("~/flowtest/mergedtree.root");
//  if (!file) return;
//  TTreeReader reader("tree", file);
//  ROOT::RDataFrame df("tree", "~/flowtest/smalltree.root");
//
//  auto xx = [](const Qn::QVector &a, const Qn::QVector &b) {
//    return a.x(1)*b.x(1);
//  };

//  Qn::MakeCorrelation("test", xx, Qn::AxisD{"t", 1, 0, 10});

//  DataFrameCreateCorrelation<double> creator;
//  creator.SetEventAxis({{"CentralityV0M", 100, 0., 100.}});
//  creator.SetResampler(20);
//  creator.ConnectDataFrame(&df);
//  creator.ConnectDataInput(&reader);
//  creator.AddCorrelation<>("ZNAZNC",xx,{"ZNA_PLAIN", "ZNC_PLAIN"},{Qn::Stats::Weights::REFERENCE, Qn::Stats::Weights::OBSERVABLE});

//  auto correlation = Qn::MakeCorrelation("ZNAZNC",xx);
//  auto df1 = correlation.SetInputNames("ZNA_PLAIN", "ZNC_PLAIN")
//      .SetWeights(Qn::Stats::Weights::REFERENCE, Qn::Stats::Weights::OBSERVABLE)
//      .Initialize(reader,&df).Define();

//  const std::size_t n_samples = 10;
//  Qn::DataFrameReSampler<n_samples> re_sampler;
//
//  Qn::CorrelationHelper<double> correlationhelper(5);
//  correlationhelper.AddEventAxes(Qn::AxisD{"CentralityV0M", 100, 0., 100.});
//  correlationhelper.Configure(correlation, n_samples);

//  auto df1 = df.Define("ZNATPCPT", correlation, correlation.GetInputNames())
//      .Define("Samples",re_sampler,{});
//  auto x = df1.Display("Samples", 2);
//  auto mean = df1.Book<Qn::DataContainerCorrelation, ROOT::RVec<ULong64_t>, double>(
//      std::move(correlationhelper), {"ZNATPCPT", "Samples", "CentralityV0M"});
//  auto val = mean.GetValue();
}