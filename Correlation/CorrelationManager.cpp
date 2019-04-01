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
void CorrelationManager::AddProjection(const std::string &input_name,
                                       const std::string &output_name,
                                       const std::string &axis_names) {
  DataContainerQVector projection;
  std::vector<std::string> name_vector;
  tokenize(axis_names, name_vector, ", ", true);
  projections_.emplace(output_name, std::make_tuple(input_name, name_vector));
  qvectors_->emplace(output_name, nullptr);
  qvectors_proj_.emplace(output_name, projection);
}

/**
 * Adds a new event variable to the correlation manager.
 * Actual value is retrieved when the tree is read from the file.
 * @param eventaxis Event variable defined as a Axis, which is used in the correlations.
 */

void CorrelationManager::AddEventVariable(const Qn::Axis &eventaxis) {
  TTreeReaderValue<float> value(*reader_, eventaxis.Name().data());
  tree_event_values_.emplace(eventaxis.Name(), value);
  event_values_.emplace_back(-999);
  eventbin_.emplace_back(-1);
  event_axes_->push_back(eventaxis);
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
                                        CorrelationManager::function_type &&lambda,
                                        const std::vector<Qn::Weight> &use_weights,
                                        Qn::Sampler::Resample use_resampling) {
  if (correlations_.find(name)==correlations_.end()) {
    auto correlation = std::make_unique<Qn::Correlation>(name, input, lambda, use_weights);
    correlations_.emplace(name, std::move(correlation));
  }
  StatsResult result(use_resampling, correlations_[name].get());
  stats_results_.emplace(name, result);
}

void CorrelationManager::ConfigureResampling(Sampler::Method method,
                                             CorrelationManager::size_type nsamples,
                                             unsigned long seed) {
  sampler_ = std::make_unique<Qn::Sampler>(num_events_, method, nsamples, seed);
  resampling_status_ = ResamplingStatus::ResamplingEnabled;
}

/**
 * Adds a ESE axis to all correlations
 * @param name Name of the datacontainer used to calculate the Q-vector magnitude.
 * @param input Name of the input Q-Vectors used for the correlation
 * @param lambda Correlation function
 * @param histo binning used for the calibration
 */
void CorrelationManager::AddESE(const std::string &name, const std::vector<std::string> &input,
                                CorrelationManager::function_type &&lambda, const TH1F &histo) {
  ese_manager_.AddESE(name, input, std::forward<function_type>(lambda), histo);
  ese_state_ = ESEState::ESEEnabled;
}

/**
 * Initializes the Correlation task.
 */
void CorrelationManager::Initialize() {
  //Check if the data container is found in the inputs.
//  for (const auto &corr : correlations_) {
//    for (const auto &name :  corr.second.GetInputNames()) {
//      if (tree_values_.find(name)==tree_values_.end()) {
//        AddDataContainer(name);
//      }
//    }
//  }
//  ese_manager_.CheckInputQVectors();
// Read in the first event to determine the binning of the Q-vector inputs.
  reader_->SetEntry(0);
// read ese file to determine if already calibrated
//  if (use_ese_) {
//    ese_file_ = std::make_unique<TFile>(ese_file_name_.data(), "READ");
//    fill_ese_ = true;
//    if (ese_file_->IsOpen()) {
//      event_shape_.reset((Qn::DataContainerEventShape *) (ese_file_->Get("ESE"))->Clone("ESE"));
//      fill_ese_ = false;
//    }
//    if (event_shape_) eventbin_.push_back(-1);
//  }

// initialize values to be able to build the correlations.
  for (auto &value: tree_values_) {
    qvectors_->at(value.first) = value.second.Get();
  }
  int i = 0;
  for (auto &value: tree_event_values_) {
    event_values_.at(i) = *value.second.Get();
    i++;
  }
  MakeProjections();
// configure the resampling using the number of event of the
  if (resampling_status_==ResamplingStatus::ResamplingEnabled) {
    sampler_->CreateSamples();
    std::cout << sampler_->Report() << std::endl;
  }
//
  BuildCorrelations();
// reset to the first event before the processing step.
  reader_->Restart();
}
/**
 * Process the correlation. Needs to be called in the event loop.
 */
void CorrelationManager::Process() {
  UpdateEvent();
  if (CheckEvent()) {
//    if (use_ese_) {
//      if (event_shape_) FillESE(ese_correlations_);
//      if (IsESE()) {
//        CheckESEEvent();
//      }
//    }
    FillCorrelations();
  }
}
/**
 * Finalizes the correlation task
 * @param correlation_file name of the correlation file.
 * @param ese_file name of the ese q-vector magnitude calibration file.
 */
void CorrelationManager::Finalize() {
//  if (use_ese_) {
//    FitEventShape();
//    if (!ese_file_name_.empty()) SaveEventShape(ese_file_name_);
//  }
  if (!correlation_file_name_.empty()) SaveToFile(correlation_file_name_);
}

void CorrelationManager::SaveToFile(std::string name) {
  auto outputfile = TFile::Open(name.data(), "RECREATE");
  for (const auto &stats : stats_results_) {
    stats.second.GetResult().Write(stats.first.data());
  }
  outputfile->Close();
}

void CorrelationManager::MakeProjections() {
  for (const auto &projection : projections_) {
    qvectors_proj_[projection.first] =
        (*qvectors_)[std::get<0>(projection.second)]->Projection(std::get<1>(projection.second),
                                                                 [](Qn::QVector a, const Qn::QVector &b) {
                                                                   a.CopyHarmonics(b);
                                                                   auto norm = b.GetNorm();
                                                                   return (a + b).Normal(norm);
                                                                 });
    (*qvectors_)[projection.first] = &qvectors_proj_[projection.first];
  }
}

void CorrelationManager::BuildCorrelations() {
  for (auto &corr : correlations_) {
//    if (event_shape_ && !fill_ese_) {
//      axes.push_back(eventshape_axes_.at(0));
//    }
    corr.second->Configure({qvectors_.get(), event_axes_.get()});
  }
  for (auto &stats : stats_results_) {
    try {
      stats.second.ConfigureStats(sampler_.get());
    } catch (NoResamplerException &e) {
      std::cout << stats.first << " " << e.what() << std::endl;
    }
  }
}

void CorrelationManager::Run() {
  Initialize();
  while (reader_->Next()) {
    Process();
  }
  Finalize();
}

//void CorrelationManager::BuildESECorrelation() {
//  if (state_ == State::ESEEnabled) {
//    for (auto &corr : ese_correlations_) {
//      corr.second.ConfigureCorrelation(qvectors_, event_axes_, &sampler_);
//    }
//
//  }

//  if (!use_ese_) return;
//  for (const auto &ese : ese_correlations_) {
//    if (!event_shape_) {
//      event_shape_ = std::make_unique<Qn::DataContainerEventShape>();
//      event_shape_->AddAxes(event_axes_);
//    }
//    if (event_shape_ && !fill_ese_) {
//      eventshape_axes_.emplace_back(ese.first, 10, 0., 1.);
//      event_values_.emplace_back(-999);
//    }
//  }
//  std::vector<Qn::DataContainerQVector*> qvectors;
//  for (auto &corr : ese_correlations_) {
//    qvectors.clear();
//    qvectors.reserve(corr.second.GetInputNames().size());
//    for (const auto &cname : corr.second.GetInputNames()) {
//      qvectors.push_back(qvectors_.at(cname));
//    }
//    corr.second.ConfigureCorrelation(qvectors, event_axes_);
//    corr.second.SetSampler(&sampler_);
//  }
//}

//void CorrelationManager::FillESE(std::map<std::string, Qn::Correlation> &corr) {
//  for (auto &pair : corr) {
//    u_long i = 0;
//    for (const auto &name : pair.second.GetInputNames()) {
//      (pair.second.GetInputs())[i] = qvectors_[name];
//      ++i;
//    }
//    pair.second.FillCorrelation(eventbin_, static_cast<size_t>(reader_->GetCurrentEntry()));
//  }
//}

void CorrelationManager::FillCorrelations() {
  for (auto &pair : correlations_) {
    pair.second->Fill(eventbin_);
  }
  for (auto &stats : stats_results_) {
    stats.second.Fill(static_cast<size_t>(reader_->GetCurrentEntry()));
  }
}

void CorrelationManager::UpdateEvent() {
  for (auto &value : tree_values_) {
    (*qvectors_)[value.first] = value.second.Get();
  }
  unsigned long i = 0;
  for (auto &value : tree_event_values_) {
    event_values_[i] = *value.second.Get();
    i++;
  }
  MakeProjections();
}

bool CorrelationManager::CheckEvent() {
  u_long ie = 0;
  for (const auto &ae : *event_axes_) {
    const auto &bin = ae.FindBin(event_values_[ie]);
    if (bin!=-1) {
      eventbin_[ie] = (unsigned long) bin;
    } else {
      return false;
    }
    ie++;
  }
  return true;
}

//bool CorrelationManager::CheckESEEvent() {
//  if (use_ese_) {
//    for (auto &bin : ese_correlations_) {
//      const auto &ese = *bin.second.GetCorrelation()->GetCorrelation();
//      const auto &correlation = ese.At(eventbin_);
//      double value = -999.;
//      if (correlation.validity) value = correlation.result;
//      event_values_.back() = event_shape_->At(eventbin_).GetPercentile(value);
//    }
//  }
//  if (!fill_ese_) {
//    u_long ie = 0;
//    auto event_axis_size = event_axes_.size();
//    for (const auto &ae : eventshape_axes_) {
//      const auto &bin = ae.FindBin(event_values_.back());
//      if (bin!=-1) {
//        eventbin_[event_axis_size + ie] = (unsigned long) bin;
//      } else {
//        return false;
//      }
//      ie++;
//    }
//    return true;
//  } else {
//    return false;
//  }
//}

//void CorrelationManager::FitEventShape() {
//  if (!use_ese_ || !fill_ese_) return;
//  int ibin = 0;
//  for (auto correlation : ese_correlations_) {
//    for (auto bin : *correlation.second.GetBinnedResult()) {
//      auto name = correlation.first;
//      event_shape_->At(ibin).SetName("ESE_" + name);;
//      event_shape_->At(ibin).SetHisto(bin);
//      event_shape_->At(ibin).FitWithSpline();
//      event_shape_->At(ibin).SetReady();
//      ++ibin;
//    }
//  }
//}
//
//void CorrelationManager::SaveEventShape(const std::string &filename) {
//  if (fill_ese_ && use_ese_) {
//    auto tmp_event_shape = (Qn::DataContainerEventShape *) event_shape_->Clone("ESE");
//    ese_file_ = std::make_unique<TFile>(filename.data(), "RECREATE");
//    if (event_shape_ && fill_ese_) ese_file_->WriteTObject(tmp_event_shape, "ESE", "");
//    ese_file_->Close();
//  }
//}
//
//bool CorrelationManager::IsESE() const {
//  if (!use_ese_) return false;
//  return std::all_of(event_shape_->begin(),
//                     event_shape_->end(),
//                     [](Qn::EventShape &a) { return a.IsReady(); }) && !fill_ese_;
//}

}