// Flow std::vector Correction Framework
//
// Copyright (C) 2018  Lukas Kreis, Ilya Selyuzhenkov
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

#include <memory>

#include "CorrelationManager.h"

namespace Qn {

/**
 * Adds a new DataContainer to the correlation manager.
 * Actual value is retrieved when the tree is read from the file.
 * @param name
 */
void CorrelationManager::AddDataContainer(const std::string &name) {
  if (tree_values_.find(name)==tree_values_.end()) {
    TTreeReaderValue<Qn::DataContainerQVector> value(*reader_, name.data());
    tree_values_.emplace(name, value);
    qvectors_->emplace(name, nullptr);
  }
}

/**
 * Adds new Projection to the correlation manager.
 * Projects the DataContainer on the specified axes and creates a new Datacontainer with a new name.
 * @param input_name Name of the input datacontainer.
 * @param output_name  Name of the output projection
 * @param axis_names Names of axes to be projected upon.
 */
void CorrelationManager::AddProjection(const std::string &name,
                                       const std::string &input,
                                       const std::vector<std::string> &axes) {
  DataContainerQVector projection;
  projections_.emplace(name, std::make_tuple(input, axes));
  qvectors_->emplace(name, nullptr);
  qvectors_proj_.emplace(name, projection);
}

/**
 * Adds a new event variable to the correlation manager.
 * Actual value is retrieved when the tree is read from the file.
 * @param eventaxis Event variable defined as a Axis, which is used in the correlations.
 */

void CorrelationManager::AddEventAxis(const Qn::Axis &eventaxis) {
  event_axes_.RegisterEventAxis(eventaxis);
}

/**
 * Adds a correlation to the output.
 * @param name Name of the correlation under which it is saved to the file
 * @param input Names of the input datacontainers.
 * @param lambda Function which is used to calculate the correlation.
 * @param nsamples number of samples used in the subsampling
 * @param method method which is used for the subsampling
 */
void CorrelationManager::AddCorrelation(std::string name,
                                        const std::vector<std::string> &input,
                                        CorrelationManager::function_t lambda,
                                        const std::vector<Qn::Weight> &use_weights,
                                        Qn::Sampler::Resample resample) {
  stats_results_.emplace(name, StatsResult{resample, RegisterCorrelation(name, input, lambda, use_weights)});
}

/**
 * Adds a ESE axis to all correlations
 * @param name Name of the datacontainer used to calculate the Q-vector magnitude.
 * @param input Name of the input Q-Vectors used for the correlation
 * @param lambda Correlation function
 * @param histo binning used for the calibration
 */
void CorrelationManager::AddEventShape(const std::string &name, const std::vector<std::string> &input,
                                       CorrelationManager::function_t lambda, const TH1F &histo) {
  ese_handler_.AddESE(name, input, lambda, histo);
}

void CorrelationManager::SetResampling(Sampler::Method method,
                                       CorrelationManager::size_type nsamples,
                                       unsigned long seed) {
  sampler_ = std::make_unique<Qn::Sampler>(num_events_, method, nsamples, seed);
}

/**
 * @brief Initializes the CorrelationManager. Called during Run().
 */
void CorrelationManager::Initialize() {
  ese_handler_.Connect();
// Read in the first event to determine the binning of the Q-vector inputs.
  reader_->SetEntry(1);
// initialize values to be able to build the correlations.
  UpdateEvent();
  MakeProjections();
// configure the resampling using the number of event of the
  if (sampler_) {
    sampler_->CreateSamples();
    std::cout << sampler_->Report() << std::endl;
  }

  ese_handler_.Initialize();
  ConfigureCorrelations();
  std::cout << ese_handler_.Report() << std::endl;
// reset to the first event before the processing step.
  if (event_axes_.GetAxes().empty()) { throw std::logic_error("no event axes added. aborting."); }
  reader_->Restart();
}

/**
 * Finalizes the correlation task
 * @param correlation_file name of the correlation file.
 * @param ese_file name of the ese q-vector magnitude calibration file.
 */
void CorrelationManager::Finalize() {
  ese_handler_.Finalize();
  if (!correlation_file_name_.empty()) {
    auto outputfile = TFile::Open(correlation_file_name_.data(), "RECREATE");
    for (const auto &stats : stats_results_) {
      stats.second.GetResult().Write(stats.first.data());
    }
    outputfile->Close();
  }
}

void CorrelationManager::Run() {
  Initialize();
  while (reader_->Next()) {
    UpdateEvent();
    if (event_axes_.CheckEvent()) {
      auto bin = event_axes_.GetBin();
      ese_handler_.Process(bin);
      for (auto &pair : correlations_) {
        pair.second->Fill(bin);
      }
      for (auto &stats : stats_results_) {
        stats.second.Fill(static_cast<size_t>(reader_->GetCurrentEntry()));
      }
    }
    // Fill tree regardless of validity of event. Friend tree needs to have the same number of entries.
    // TODO maybe it can be fixed by using an Index without saving useless data.
    ese_handler_.FillTree();
  }
  Finalize();
}

void CorrelationManager::UpdateEvent() {
  for (auto &value : tree_values_) {
    (*qvectors_)[value.first] = value.second.Get();
  }
  event_axes_.UpdateEvent();
  MakeProjections();
}

void CorrelationManager::MakeProjections() {
  auto function = [](Qn::QVector a, const Qn::QVector &b) {
    a.CopyHarmonics(b);
    auto norm = b.GetNorm();
    return (a + b).Normal(norm);
  };
  for (const auto &projection : projections_) {
    auto input = projection.first;
    const auto &output = std::get<0>(projection.second);
    const auto &axes = std::get<1>(projection.second);
    qvectors_proj_[projection.first] = (*qvectors_)[output]->Projection(axes, function);
    (*qvectors_)[projection.first] = &qvectors_proj_[input];
  }
}

void CorrelationManager::ConfigureCorrelations() {
  for (auto &corr : correlations_) {
    corr.second->Configure(qvectors_.get(), event_axes_.GetAxes());
  }
  ese_handler_.Configure();
  for (auto &stats : stats_results_) {
    try {
      stats.second.ConfigureStats(sampler_.get());
    } catch (NoResamplerException &e) {
      std::cout << stats.first << " " << e.what() << std::endl;
    }
  }
}

Qn::Correlation *CorrelationManager::RegisterCorrelation(const std::string &name,
                                                         const std::vector<std::string> &inputs,
                                                         CorrelationManager::function_t lambda,
                                                         std::vector<Qn::Weight> use_weights) {
  std::for_each(inputs.begin(), inputs.end(), [this](const std::string &item) { this->AddDataContainer(item); });
  if (correlations_.find(name)==correlations_.end()) {
    auto correlation = std::make_unique<Qn::Correlation>(name, inputs, lambda, use_weights);
    correlations_.emplace(name, std::move(correlation));
  }
  return correlations_.at(name).get();
}

}