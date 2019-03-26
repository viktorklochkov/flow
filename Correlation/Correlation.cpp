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
                                  unsigned int n) {
  const auto &i_input = *inputs_[n];
  initial_offset += n;
  // End recursion if last input DataContainer is reached: n+1 = number of inputs.
  if (n + 1==inputs_.size()) {
    size_type ibin = 0;
    // Calculate result with Q-Vectors from previous recursion steps and all Q-Vectors of the current (last) input DataContainer.
    for (const auto &qvector : i_input) {
      // Sets multi-dimensional index in the resulting correlation DataContainer
      // In case of integrated Q-Vectors no index is propagated to the resulting correlation.
      // e.g. a correlation with 1 event variable and two Q-Vectors Q_1 and Q_2
      // For non-integrated: event(h) x Q_1(i, j) x Q_2(k, l) -> (h, i, j, k, l)
      // For integrated:     event(h) x Q_1()     x Q_2(k, l) -> (h, k, l)
      if (!i_input.IsIntegrated()) {
        size_type i_index = 0;
        for (const auto &index : index_[n][ibin]) {
          auto pos = i_index + initial_offset;
          c_index_[pos] = index;
          ++i_index;
        }
      }
      // Adds a pointer to the current Q-Vector to the temporary container used for calculation of the correlation.
      qvectors_[n] = QVectorPtr(qvector);
      // Checks that all Q-Vectors are valid.
      auto valid = std::all_of(qvectors_.begin(), qvectors_.end(), [](const QVectorPtr &q) { return q.n() > 0; });
      // Store result of the correlation.
      if (valid) current_event_result_.At(c_index_) = Qn::Product(function_(qvectors_), valid, CalculateWeight());
      ++ibin;
    }
    // end of recursion
    return;
  }
  size_type ibin = 0;
  for (const auto &qvector : i_input) {
    size_type offset = initial_offset;
    // Sets multi-dimensional index in the resulting correlation DataContainer
    // In case of integrated Q-Vectors no index is propagated to the resulting correlation. See above.
    if (!i_input.IsIntegrated()) {
      size_type i_index = 0;
      for (auto &index : index_[n][ibin]) {
        offset += i_index;
        c_index_[offset] = index;
        ++i_index;
      }
    }
    // Adds a pointer to the current Q-Vector to the temporary container used for calculation of the correlation.
    qvectors_[n] = QVectorPtr(qvector);
    // next step of recursion
    FillCorrelation(offset, n + 1);
    ++ibin;
  }
}

void Correlation::Fill(const std::vector<unsigned long> &eventindices,
                       const size_type event_id) {
  int ieventvar = 0;
  for (auto eventindex : eventindices) {
    c_index_[ieventvar] = eventindex;
    ++ieventvar;
  }
  FillCorrelation(ieventvar, 0);
  if (use_resampling_) {
    unsigned int ibin = 0;
    for (auto &bin : result_) {
      bin.Fill(current_event_result_.At(ibin),
               resampler_->GetFillVector(event_id));
      ++ibin;
    }
  } else {
    unsigned int ibin = 0;
    for (auto &bin : result_) {
      bin.Fill(current_event_result_.At(ibin), {});
      ++ibin;
    }
  }
  if (use_histo_result_) {
    unsigned int ibin = 0;
    for (auto &bin : result_) {
      if (use_histo_result_) bin.Fill(current_event_result_.At(ibin));
      ++ibin;
    }
  }
}

void Correlation::ConfigureCorrelation(const Correlation::INPUTS &inputs,
                                       const std::vector<Qn::Axis> &event,
                                       Sampler *sampler) {
  inputs_ = inputs;
  axes_event_ = event;
  qvectors_.resize(inputs.size());
  // Adds all the axes of event variables to the result of the correlation.
  result_.AddAxes(axes_event_);
  // Prepare a map of all indices of the correlation
  auto dimension = axes_event_.size();
  size_type i_input = 0;
  for (const auto &input : inputs) {
    if (!input->IsIntegrated()) dimension += input->GetAxes().size();
    std::vector<std::vector<unsigned long>> indexmap; // vector of multi-dimensional indices of one Input Q-Vector.
    for (size_type ibin = 0; ibin < input->size(); ++ibin) {
      indexmap.emplace_back(input->GetIndex(ibin));
    }
    index_.push_back(indexmap);
    // Adds remaining axes to the result of the correlation in case of non-integrated Q-Vectors.
    if (!input->IsIntegrated()) {
      auto axes = input->GetAxes();
      for (auto &axis : axes) {
        axis.SetName(std::to_string(i_input) + "_" + names_.at(i_input) + "_" + axis.Name());
        result_.AddAxis(axis);
      }
      ++i_input;
    }
  }
  c_index_.resize(dimension);
  // configure current event result
  current_event_result_.AddAxes(result_.GetAxes());
  // configure weights
  if (std::any_of(use_weights_.begin(), use_weights_.end(), [](Qn::Weight x) { return x==Qn::Weight::OBSERVABLE; })) {
    std::for_each(result_.begin(), result_.end(), [](Qn::Stats &stats) { stats.SetStatus(Stats::Status::OBSERVABLE); });
  } else {
    std::for_each(result_.begin(), result_.end(), [](Qn::Stats &stats) { stats.SetStatus(Stats::Status::REFERENCE); });
  }
  // configure histo_result
  if (use_histo_result_) {
    auto base_hist = histo_result_.At(0);
    histo_result_.AddAxes(result_.GetAxes());
    histo_result_.InitializeEntries(base_hist);
  }
  // configure sampler
  if (use_resampling_) {
    resampler_ = sampler;
    for (auto &bin :result_) {
      bin.SetNumberOfSubSamples(resampler_->GetNumSamples());
    }
  }
}

}