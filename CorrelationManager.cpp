//
// Created by Lukas Kreis on 15.12.17.
//

#include "CorrelationManager.h"
#include <TFile.h>

namespace Qn {

void CorrelationManager::UpdateEvent() {
  for (auto &value : tree_values_) {
    qvectors_.at(value.first) = *value.second.Get();
  }
  int i = 0;
  for (auto &value : tree_event_values_) {
    event_values_.at(i) = *value.second.Get();
    i++;
  }
  MakeProjections();
}

bool CorrelationManager::CheckEvent() {
  u_long ie = 0;
  for (const auto &ae : event_axes_) {
    eventbin_.at(ie) = (ae.FindBin(event_values_[ie]));
    ie++;
  }
  for (auto bin : eventbin_) {
    if (bin==-1) return false;
  }
  return true;
}

void CorrelationManager::MakeProjections() {
  for (auto &projection : projections_) {
    qvectors_.at(std::get<0>(projection.second)) =
        (*tree_values_.at(projection.first)).Projection(std::get<2>(projection.second),
                                                        [](Qn::QVector &a, Qn::QVector &b) {
                                                          return (a + b).Normal(Qn::QVector::Normalization::QOVERM);
                                                        });
  }
}
void CorrelationManager::Initialize() {
  for (auto &value : tree_values_) {
    qvectors_.at(value.first) = *value.second.Get();
  }
  int i = 0;
  for (auto &value : tree_event_values_) {
    event_values_.at(i) = *value.second.Get();
    i++;
  }
  for (auto &projection : projections_) {
    std::list<std::string> axisnames;
    tokenize(std::get<1>(projection.second), axisnames, ", ", true);
    auto toproject = qvectors_.at(projection.first);
    for (auto const &name : axisnames) {
      std::get<2>(projection.second).push_back(toproject.GetAxis(name));
    }
  }
  BuildCorrelation();
}

void CorrelationManager::SaveToFile(std::string name) {
  auto outputfile = TFile::Open(name.data(), "RECREATE");
  for (const auto &correlation : correlations_) {
    correlation.second.second.GetCorrelation().Write(correlation.first.data());
  }
  outputfile->Close();
}

void CorrelationManager::FillCorrelations() {
  for (auto &pair : correlations_) {
    u_long i = 0;
    std::vector<DataContainerQVector> inputs;
    inputs.resize(pair.second.first.size());
    for (const auto &name : pair.second.first) {
      inputs.at(i) = qvectors_.at(name);
      ++i;
    }
    pair.second.second.Fill(inputs, eventbin_);
  }
}

void CorrelationManager::BuildCorrelation() {
  for (auto &corr : build_correlations_) {
    auto containernamelist = corr.second.first;
    auto name = corr.first;
    auto lambda = corr.second.second;
    std::vector<Qn::DataContainerQVector> qvectors;
    qvectors.reserve(containernamelist.size());
    for (auto &cname : containernamelist) {
      qvectors.push_back(qvectors_.at(cname));
    }
    Qn::Correlation correlation(std::move(qvectors), event_axes_, std::move(lambda));
    correlations_.emplace(name, std::make_pair(containernamelist, correlation));
  }
}

}