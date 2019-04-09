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

#ifndef FLOW_CORRELATIONBASE_H
#define FLOW_CORRELATIONBASE_H

#include <utility>

#include "DataContainer.h"

namespace Qn {

/**
 * @class Weight
 * @brief Enumerator for setting if weights of a detector are used in the correlation.
 */
enum class Weight {
  REFERENCE,
  OBSERVABLE
};

auto constexpr kRef = Qn::Weight::REFERENCE;
auto constexpr kObs = Qn::Weight::OBSERVABLE;

/**
 * @class Correlation
 * @brief abstract baseclass of the correlation
 * Implements all the methods to calculate correlations of qvectors.
 */
class Correlation {
 protected:
  using size_type = std::size_t;
  // all inputs of the correlation in a vector
  using inputs_type = std::vector<DataContainerQVector *const *>;
  // The Pointers to the input qvectors are updated once by the CorrelationManager.
  // They always point to a valid entry in the TTree.

 public:
  using function_type = std::function<double(const std::vector<Qn::QVectorPtr> &)>;
  using function_t  = const function_type &;

  Correlation(std::string name,
              std::vector<std::string> names,
              function_type function,
              std::vector<Qn::Weight> use_weights) :
      name_(std::move(name)),
      names_(std::move(names)),
      function_(std::move(function)) {
    for (size_type i = 0; i < names_.size(); ++i) { use_weights_.push_back(use_weights[i]==Qn::kObs); }
  }

  Correlation(std::string name, std::vector<std::string> names, function_type function) :
      name_(std::move(name)),
      names_(std::move(names)),
      function_(std::move(function)) {
    for (size_type i = 0; i < names_.size(); ++i) { use_weights_.push_back(false); }
  }

  const std::vector<std::string> &GetInputNames() const { return names_; }
  const Qn::DataContainerProduct &GetResult() const { return current_event_result_; };
  bool UsingWeights() const { return std::any_of(use_weights_.begin(), use_weights_.end(), [](bool x) { return x; }); }

  void Fill(const std::vector<unsigned long> &eventindices);
  void Configure(std::map<std::string, Qn::DataContainerQVector *> *qvectors, const std::vector<Qn::Axis> &eventaxes);
  std::string GetName() const { return name_; }

 private:
  std::string name_; ///< name of the correlation
  std::vector<std::string> names_; ///< vector of input names
  inputs_type inputs_; ///< pointer to the Q-Vector inputs during the correlation step.
  std::vector<QVectorPtr> qvector_ptrs_; ///< vector holding pointers to the Q-Vector during FillCorrelation step
  std::vector<bool> use_weights_; ///< vector of input weights
  function_type function_; ///< correlation function
  std::vector<std::vector<std::vector<size_type>>> index_; ///< map of multi-dimensional indices of all inputs
  std::vector<size_type> c_index_; ///<  multi-dimensional indices of a bin of the resulting correlation
  Qn::DataContainerProduct current_event_result_; ///< result of the correlation of the current event
  void FillCorrelation(size_type initial_offset, unsigned int n = 0);

  /**
   * Calculate weight for correlation;
   * @return weight for the current event
   */
  inline double CalculateWeight() {
    size_type i_weight = 0;
    double weight = 1.;
    for (const auto &q : qvector_ptrs_) {
      if (use_weights_[i_weight]) {
        weight *= q.sumweights();
      }
      ++i_weight;
    }
    return weight;
  }

};

}

#endif
