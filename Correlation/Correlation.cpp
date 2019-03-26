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

#include "Correlation.h"

namespace Qn {

void Correlation::FillCorrelation(size_type initial_offset,
                                  unsigned int n,
                                  const std::size_t event_id) {
  const auto &i_input = *inputs_[n];
  initial_offset += n;
  if (n + 1==inputs_.size()) {
    size_type ibin = 0;
    for (auto qvector : i_input) {
      if (!i_input.IsIntegrated()) {
        size_type i_index = 0;
        for (const auto &index : index_[n][ibin]) {
          auto pos = i_index + initial_offset;
          c_index_[pos] = index;
          ++i_index;
        }
      }
      qvectors_[n] = &qvector;
      auto valid = std::all_of(qvectors_.begin(), qvectors_.end(), [](QVector *q) { return q->n() > 0; });
      size_type i_weight = 0;
      double weight = 1.;
      for (const auto &q : qvectors_) {
        if (use_weights_[i_weight]) {
          weight *= q->sum_weights_;
        }
        ++i_weight;
      }
      if (valid) {
        result_.At(c_index_).Fill(function_(qvectors_), weight, sampler_->GetFillVector(event_id));
        if (use_histo_result_) histo_result_.At(c_index_).Fill(function_(qvectors_));
      }
      ++ibin;
    }
    return;
  }
  size_type ibin = 0;
  for (auto bin : i_input) {
    size_type offset = initial_offset;
    if (!i_input.IsIntegrated()) {
      size_type i_index = 0;
      for (const auto &index : index_[n][ibin]) {
        offset += i_index;
        c_index_[offset] = index;
        ++i_index;
      }
    }
    qvectors_[n] = &bin;
    FillCorrelation(offset, n + 1, event_id);
    ++ibin;
  }
}

void Correlation::FillCorrelationNoResampling(size_type initial_offset,
                                              unsigned int n,
                                              const std::size_t event_id) {
  const auto &i_input = *inputs_[n];
  initial_offset += n;
  if (n + 1==inputs_.size()) {
    size_type ibin = 0;
    for (auto qvector : i_input) {
      if (!i_input.IsIntegrated()) {
        size_type i_index = 0;
        for (auto index : index_[n][ibin]) {
          auto pos = i_index + initial_offset;
          c_index_[pos] = index;
          ++i_index;
        }
      }
      qvectors_[n] = &qvector;
      auto valid = std::all_of(qvectors_.begin(), qvectors_.end(), [](const Qn::QVector* q) { return q->n() > 0; });
      size_type i_weight = 0;
      double weight = 1.;
      for (const auto &q : qvectors_) {
        if (use_weights_[i_weight]) {
          weight *= q->sum_weights_;
        }
        ++i_weight;
      }
      if (valid) {
        result_.At(c_index_).Fill(function_(qvectors_), weight, {});
        if (use_histo_result_) histo_result_.At(c_index_).Fill(function_(qvectors_));
      }

      ++ibin;
    }
    return;
  }
  size_type ibin = 0;
  for (auto bin : i_input) {
    auto offset = initial_offset;
    if (!i_input.IsIntegrated()) {
      int i_index = 0;
      for (auto index : index_[n][ibin]) {
        offset += i_index;
        c_index_[offset] = index;
        ++i_index;
      }
    }
    qvectors_[n] = &bin;
    FillCorrelationNoResampling(offset, n + 1, event_id);
    ++ibin;
  }
}

void Correlation::Fill(const std::vector<unsigned long> &eventindices,
                       const std::size_t event_id) {
  uint iteration = 0;
  int ii = 0;
  for (auto eventindex : eventindices) {
    c_index_[ii] = eventindex;
    ++ii;
  }
  if (use_resampling_) {
    FillCorrelation(ii, iteration, event_id);
  } else {
    FillCorrelationNoResampling(ii, iteration, event_id);
  }
}

void Correlation::ConfigureCorrelation(const Correlation::DataContainers &inputs,
                                       const std::vector<Qn::Axis> &event,
                                       Sampler *sampler) {
  inputs_ = inputs;
  axes_event_ = event;
  qvectors_.resize(inputs.size());
  result_.AddAxes(axes_event_);

  //prepare correlation indices
  auto event_variables_size = axes_event_.size();
  int i_input = 0;
  for (const auto &input : inputs) {
    if (!input->IsIntegrated()) event_variables_size += input->GetAxes().size();
    std::vector<std::vector<unsigned long>> indexmap;
    for (std::size_t j_input = 0; j_input < input->size(); ++j_input) {
      std::vector<unsigned long> indices;
      input->GetIndex(indices, j_input);
      indexmap.push_back(indices);
    }
    index_.push_back(indexmap);
    if (!input->IsIntegrated()) {
      auto axes = input->GetAxes();
      for (auto &axis : axes) {
        auto original_name = axis.Name();
        if (!names_.empty()) { axis.SetName(std::to_string(i_input) + "_" + names_[i_input] + "_" + original_name); }
        else { axis.SetName(std::to_string(i_input) + "_" + original_name); }
        result_.AddAxis(axis);
      }
      ++i_input;
    }
  }
  c_index_.resize(event_variables_size);

  // configure weights
  for (auto &bin : result_) {
    if (std::any_of(use_weights_.begin(), use_weights_.end(), [](bool x) { return x; })) {
      bin.SetStatus(Stats::Status::OBSERVABLE);
    } else {
      bin.SetStatus(Stats::Status::REFERENCE);
    }
  }

  // configure histo_result
  if (use_histo_result_) {
    auto base_hist = histo_result_.At(0);
    histo_result_.AddAxes(result_.GetAxes());
    histo_result_.InitializeEntries(base_hist);
  }

  // configure sampler
  if (use_resampling_) {
    sampler_ = sampler;
    for (auto &bin :result_) {
      bin.SetNumberOfSubSamples(sampler_->GetNumSamples());
    }
  }

}

}