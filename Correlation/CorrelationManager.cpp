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
#include <include/CorrelationManager.h>

#include "CorrelationManager.h"

#include "ROOT/RMakeUnique.hxx"
#include "ROOT/RIntegerSequence.hxx"

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
 * Adds a list of detectors to the correlation manager.
 * The actual values are retrieved when the tree is read from the file.
 * @param name list of qvectors
 */
void CorrelationManager::AddQVectors(const std::vector<std::string> &qvectors) {
  for (const auto &name : qvectors) {
    AddDataContainer(name);
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
  event_axes_.AddEventVariable(eventaxis);
//  TTreeReaderValue<float> value(*reader_, eventaxis.Name().data());
//  tree_event_values_.push_back(value);
//  event_values_.emplace_back(-999);
//  eventbin_.emplace_back(-1);
//  event_axes_.push_back(eventaxis);
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
                                        const std::vector<std::string> &inputs,
                                        CorrelationManager::function_p lambda,
                                        const std::vector<Qn::Weight> &use_weights,
                                        Qn::Sampler::Resample use_resampling) {
  std::for_each(inputs.begin(), inputs.end(), [this](const std::string &item) { this->AddDataContainer(item); });
  if (correlations_.find(name)==correlations_.end()) {
    auto correlation = std::make_unique<Qn::Correlation>(name, inputs, lambda, use_weights);
    correlations_.emplace(name, std::move(correlation));
  }
  StatsResult result(use_resampling, correlations_[name].get());
  stats_results_.emplace(name, result);
}

/**
 * Adds a ESE axis to all correlations
 * @param name Name of the datacontainer used to calculate the Q-vector magnitude.
 * @param input Name of the input Q-Vectors used for the correlation
 * @param lambda Correlation function
 * @param histo binning used for the calibration
 */
void CorrelationManager::AddESE(const std::string &name, const std::vector<std::string> &input,
                                CorrelationManager::function_p lambda, const TH1F &histo) {
  ese_handler_.AddESE(name, input, lambda, histo);
  ese_state_ = ESEState::ESEEnabled;
}

void CorrelationManager::ConfigureResampling(Sampler::Method method,
                                             CorrelationManager::size_type nsamples,
                                             unsigned long seed) {
  sampler_ = std::make_unique<Qn::Sampler>(num_events_, method, nsamples, seed);
  resampling_status_ = ResamplingStatus::ResamplingEnabled;
}

/**
 * @brief Initializes the CorrelationManager. Called during Run().
 */
void CorrelationManager::Initialize() {
  if (ese_state_==ESEState::ESEEnabled) {
    ese_handler_.Connect();
  }
// Read in the first event to determine the binning of the Q-vector inputs.
  reader_->SetEntry(1);
// initialize values to be able to build the correlations.
  UpdateEvent();
  MakeProjections();
// configure the resampling using the number of event of the
  if (resampling_status_==ResamplingStatus::ResamplingEnabled) {
    sampler_->CreateSamples();
    std::cout << sampler_->Report() << std::endl;
  }
  if (ese_state_==ESEState::ESEEnabled) {
    ese_handler_.Initialize();
    std::cout << ese_handler_.Report() << std::endl;
  }
  ConfigureCorrelations();
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
  if (ese_calib_out_) ese_calib_out_->Close();
  if (ese_calib_in_) ese_calib_in_->Close();
  if (ese_treefile_out_) ese_treefile_out_->Close();
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
    auto output = std::get<0>(projection.second);
    auto axes = std::get<1>(projection.second);
    qvectors_proj_[projection.first] = (*qvectors_)[std::get<0>(projection.second)]->Projection(axes, function);
    (*qvectors_)[projection.first] = &qvectors_proj_[input];
  }
}

void CorrelationManager::ConfigureCorrelations() {
  for (auto &corr : correlations_) {
    corr.second->Configure(qvectors_.get(), event_axes_.GetAxes());
  }
  if (ese_state_==ESEState::ESEEnabled) ese_handler_.Configure();
  for (auto &stats : stats_results_) {
    try {
      stats.second.ConfigureStats(sampler_.get());
    } catch (NoResamplerException &e) {
      std::cout << stats.first << " " << e.what() << std::endl;
    }
  }
}

Qn::Correlation *CorrelationManager::AddCorrelationOnly(const std::string &name, const std::vector<std::string> &inputs,
                                                        CorrelationManager::function_p lambda) {
  std::for_each(inputs.begin(), inputs.end(), [this](const std::string &item) { this->AddDataContainer(item); });
  if (correlations_.find(name)==correlations_.end()) {
    std::vector<Qn::Weight> use_weights(inputs.size());
    for_each(use_weights.begin(), use_weights.end(), [](Qn::Weight &a) { a = Qn::Weight::REFERENCE; });
    auto correlation = std::make_unique<Qn::Correlation>(name, inputs, lambda, use_weights);
    correlations_.emplace(name, std::move(correlation));
  }
  return correlations_.at(name).get();
}

}