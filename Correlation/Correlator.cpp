// Flow Vector Correction Framework
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

#include "Correlator.h"

void Qn::Correlator::FillCorrelation(const std::vector<unsigned long> &eventindex, std::size_t event_id) {
  correlation_.Fill(inputs_, eventindex);
  int ibin = 0;
  for (const auto &corr_in_event : *correlation_.GetCorrelation()) {
    if (corr_in_event.validity) {
      if (use_resampling_) {
        result_.At(ibin).Fill(corr_in_event, sampler_->GetFillVector(event_id));
      } else {
        result_.At(ibin).Fill(corr_in_event);
      }
    }
    if (corr_in_event.validity && binned_result_) {
      binned_result_->At(ibin).Fill(corr_in_event.result);
    }
    ++ibin;
  }
}

void Qn::Correlator::ConfigureCorrelation(const std::vector<Qn::DataContainerQVector> &input,
                                          std::vector<Qn::Axis> event) {
  inputs_.resize(input.size());
  correlation_.ConfigureCorrelation(input, event, lambda_correlation_, input_names_, use_weights_);
  result_.AddAxes(correlation_.GetCorrelation()->GetAxes());
  auto use_weights = std::any_of(use_weights_.begin(), use_weights_.end(), [](bool x) { return x; });
  for (auto &bin : result_) {
    if (use_weights) {
      bin.SetStatus(Stats::Status::OBSERVABLE);
    } else {
      bin.SetStatus(Stats::Status::REFERENCE);
    }
  }
  if (binned_result_) {
    auto base_hist = binned_result_->At(0);
    binned_result_->AddAxes(correlation_.GetCorrelation()->GetAxes());
    binned_result_->InitializeEntries(base_hist);
  }
}

//void Qn::Correlator::FindAutoCorrelations() {
//  std::vector<std::vector<size_type>> auto_correlations;
//  auto n_event_axes = correlation_.GetEventAxes().size();
//  for (unsigned long i_input = 0; i_input < input_names_.size(); ++i_input) {
//    std::vector<unsigned long> correlated_inputs;
//    correlated_inputs.push_back(i_input + n_event_axes);
//    for (unsigned long j_input = i_input + 1; j_input < input_names_.size(); ++j_input) {
//      if (input_names_[i_input]==input_names_[j_input]) {
//        correlated_inputs.push_back(j_input + n_event_axes);
//      }
//    }
//    if (correlated_inputs.size() > 1) auto_correlations.push_back(correlated_inputs);
//  }
//  auto correlation_axes = correlation_.GetCorrelation()->GetAxes();
//  for (const auto &correlation : auto_correlations) {
//    std::vector<Qn::Axis> axes;
//    for (const auto id : correlation) {
//      axes.push_back(correlation_axes[id]);
//    }
//    autocorrelated_bins_.push_back(correlation_.GetCorrelation()->GetDiagonal(axes));
//  }
//}

//void Qn::Correlator::RemoveAutoCorrelation() {
//  for (auto bins : autocorrelated_bins_) {
//    for (auto bin : bins) {
//      correlation_.GetCorrelation().ClearDataAt(bin);
//    }
//  }
//}
