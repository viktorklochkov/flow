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
#ifndef FLOW_AVERAGEHELPER_H_
#define FLOW_AVERAGEHELPER_H_

#include <vector>
#include <tuple>
#include <string>

#include "ROOT/RResultPtr.hxx"
#include "ROOT/RDF/ActionHelpers.hxx"

#include "DataContainer.h"
#include "Common/TemplateMagic.h"

namespace Qn {

template<typename Helper>
using RActionImpl =  ROOT::Detail::RDF::RActionImpl<Helper>;

/**
 * Averaging helper class for the use in the RDataFrame
 * Allows to perform an averaging over events with an Action
 * @tparam Action Action which specifies the way to perform the average
 */
template<typename Action>
class AverageHelper : public RActionImpl<AverageHelper<Action>> {
 public:
  using Result_t = Action; /// Result of the averaging operation.

 private:
  std::vector<bool> is_configured_; /// flag for tracking if the helper has been configured using the input data.
  std::vector<std::shared_ptr<Action>> results_; /// vector of results.
  TTreeReader *external_reader_; /// non-owning pointer to external TTreeReader needed in case of cached dataframe.

 public:
  /**
   * Constructor
   * takes an action and creates as many results as the ROOT MT pool size demands.
   * @param action Action which determines the operation and event classification.
   */
  explicit AverageHelper(Action action) {
    const auto n_slots = ROOT::IsImplicitMTEnabled() ? ROOT::GetImplicitMTPoolSize() : 1;
    for (std::size_t i = 0; i < n_slots; ++i) {
      is_configured_.push_back(false);
      results_.emplace_back(std::make_shared<Action>(action));
    }
  }

  /**
   * Set External TTreeReader
   */
   auto SetExternalTTreeReader(TTreeReader *reader) {
     external_reader_ = reader;
     return *this;
   }

  /**
   * Adds the helper to the datacontainer using the Book function.
   * Wraps this in this function to omit template parameters in it's usage.
   * @tparam DataFrame type of a RDataFrame.
   * @param df input data frame
   * @return returns the result of the averaging. This Action can in the following be used to perform the corrections.
   */
  template<typename DataFrame>
  ROOT::RDF::RResultPtr<Result_t> BookMe(DataFrame &&df) {
    return results_[0]->BookMe(std::forward<DataFrame>(df), *this);
  }

  /**
   * Main analysis loop. This function is run for every event. Forwards the inputs to the action.
   * @tparam Parameters of the action.
   * @param slot slot in the MT pool.
   * @param parameters of the action.
   */
  template<typename... Parameters>
  void Exec(unsigned int slot, Parameters &&... parameters) {
    results_[slot]->CalculateAction(std::forward<Parameters>(parameters)...);
  }

  /**
   * Finalizes the results and merges all slots into the first.
   * Special case if results_[0] is not initialized.
   * During merging all elements of the merge need to be initialized.
   */
  void Finalize() {
    auto first_configured = std::distance(std::begin(is_configured_), std::find_if(std::begin(is_configured_),
                                                                                   std::end(is_configured_),
                                                                                   [](bool x) { return x; }));
    if (!is_configured_[0]) results_[0]->CopyInitializedState(*results_.at(first_configured));
    std::vector<std::shared_ptr<Result_t>> others;
    for (std::size_t slot = first_configured + 1; slot < results_.size(); ++slot) {
      if (is_configured_[slot]) others.push_back(results_[slot]);
    }
    results_[0]->Merge(others);
  }

  /**
   * Initializes the helper and the action using the first event in the input tree.
   * @param reader
   */
  void InitTask(TTreeReader *reader, unsigned int slot) {
    if (!is_configured_[slot]) {
      if (reader) {
        TTreeReader local_reader(reader->GetTree());
        results_[slot]->Initialize(local_reader);
      } else {
        TTreeReader local_reader(external_reader_->GetTree());
        results_[slot]->Initialize(local_reader);
      }
      is_configured_[slot] = true;
    }
  }

  /**
   * Needed by the RDataFrame do not remove
   */
  void Initialize() { /* no-op */}

  /**
   * Get partial result of a slot
   * @param slot slot index
   * @return partial result
   */
  Result_t &PartialUpdate(unsigned int slot) { return *results_[slot]; }

  /**
   * Get pointer to the merged result.
   * @return pointer to the result
   */
  std::shared_ptr<Result_t> GetResultPtr() const { return results_[0]; }

  /**
   * Returns the name of the Action
   * @return Name of the action
   */
  std::string GetActionName() const { return results_[0]->GetName(); }
};

/**
 * Helper function which creates the AverageHelper without needing to specify the template parameters.
 * @tparam Action Action which carries the needed information to derive the template parameters and create the AverageHelper.
 */
template<typename Action>
auto inline EventAverage(Action action) { return AverageHelper<Action>{action}; }

} /// Qn

#endif
