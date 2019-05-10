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

void Qn::Correlation::FillCorrelation(size_type initial_offset,
                                      unsigned int n) {
  const auto &i_input = **inputs_[n];
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
      qvector_ptrs_[n] = Qn::QVectorPtr(qvector);
      // Checks that all Q-Vectors are valid.
      auto valid =
          std::all_of(qvector_ptrs_.begin(), qvector_ptrs_.end(), [](const Qn::QVectorPtr &q) { return q.n() > 0; });
      // Store result of the correlation.
      if (valid) current_event_result_.At(c_index_) = Qn::Product(function_(qvector_ptrs_), valid, CalculateWeight());
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
    qvector_ptrs_[n] = QVectorPtr(qvector);
    // next step of recursion
    FillCorrelation(offset, n + 1);
    ++ibin;
  }
}

void Qn::Correlation::Fill(const std::vector<unsigned long> &eventindices) {
  // Update eventindices in the result correlation index.
  size_type ieventvar = 0;
  for (auto &bin : current_event_result_) { bin.validity = false; }
  for (auto eventindex : eventindices) {
    c_index_[ieventvar] = eventindex;
    ++ieventvar;
  }
  // Fill the per-event-correlation result recursively.
  FillCorrelation(ieventvar, 0);
}

void Qn::Correlation::Configure(std::map<std::string, Qn::DataContainerQVector *> *qvectors,
                                const std::vector<Qn::Axis> &event_axes) {
  for (const auto &cname : names_) {
    inputs_.push_back(&qvectors->at(cname));
  }
  qvector_ptrs_.resize(inputs_.size());
  // Adds all the axes of event variables to the result of the correlation.
  try {
    current_event_result_.AddAxes(event_axes);
  } catch (std::logic_error &e) {
    std::string errormsg = ("correlation ") + name_ + "trying to add axes, but they already exist.";
    throw std::logic_error(errormsg);
  }
  // Prepare a map of all indices of the correlation
  auto dimension = event_axes.size();
  size_type i_input = 0;
  for (const auto &inputptr : inputs_) {
    auto input = *inputptr;
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
        unsigned int j_input = 0;
        bool found_other = false;
        bool shared_name = false;
        for (const auto &otherptr : inputs_) {
          auto other = *otherptr;
          if (i_input!=j_input) {
            if (!other->IsIntegrated()) {
              auto j_axes = other->GetAxes();
              for (auto &j_axis :j_axes) {
                if (j_axis.Name()==axis.Name()) {
                  found_other = true;
                  shared_name = names_.at(i_input)==names_.at(j_input);
                }
              }
            }
          }
          ++j_input;
        }
        if (found_other) {
          if (shared_name) {
            axis.SetName(std::to_string(i_input) + "_" + names_.at(i_input) + "_" + axis.Name());
          } else {
            axis.SetName(names_.at(i_input) + "_" + axis.Name());
          }
          current_event_result_.AddAxis(axis);
        } else {
          current_event_result_.AddAxis(axis);
        }
        ++i_input;
      }
    }
  }
  c_index_.resize(dimension);
}
