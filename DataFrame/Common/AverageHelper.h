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

namespace Qn {

template<typename Helper>
using RActionImpl =  ROOT::Detail::RDF::RActionImpl<Helper>;

template<typename Action, typename EventParameters, typename DataContainers>
class AverageHelper;
/**
 * Averaging helper class for the use in the RDataFrame
 * Allows to perform an averaging over events with an Action
 * @tparam Action Action which specifies the way to perform the average
 * @tparam EventParameters Event parameters used for classification of events
 * @tparam DataContainers Input data containers / Q-vectors
 */
template<typename Action, typename... EventParameters, typename... DataContainers>
class AverageHelper<Action,
                    std::tuple<EventParameters...>,
                    std::tuple<DataContainers...>> : public RActionImpl<AverageHelper<Action,
                                                                                      std::tuple<EventParameters...>,
                                                                                      std::tuple<DataContainers...> >> {
 public:
  using Result_t = Action; /// Result of the averaging operation.

 private:
  bool is_configured_ = false; /// flag for tracking if the helper has been configured using the input data.
  std::vector<std::shared_ptr<Action>> results_; /// vector of results.

 public:
  /**
   * Constructor
   * takes an action and creates as many results as the ROOT MT pool size demands.
   * @param action Action which determines the operation and event classification.
   */
  explicit AverageHelper(Action action) {
    const auto n_slots = ROOT::IsImplicitMTEnabled() ? ROOT::GetImplicitMTPoolSize() : 1;
    for (std::size_t i = 0; i < n_slots; ++i) {
      results_.emplace_back(std::make_shared<Action>(action));
    }
  }

  /**
   * Adds the helper to the datacontainer using the Book function.
   * Wraps this in this function to omit template parameters in it's usage.
   * @tparam DataFrame type of a RDataFrame.
   * @param df input data frame
   * @return returns the result of the averaging. This Action can in the following be used to perform the corrections.
   */
  template<typename DataFrame>
  ROOT::RDF::RResultPtr<Result_t> BookMe(DataFrame &df) {
    return df.template Book<DataContainers..., EventParameters...>(std::move(*this), results_[0]->GetColumnNames());
  }

  /**
   * Main analysis loop. This function is run for every event. Forwards the inputs to the action.
   * @param slot slot in the MT pool.
   * @param data_containers input Q-vectors.
   * @param coordinates input event parameters for event classification.
   */
  void Exec(unsigned int slot, DataContainers... data_containers, EventParameters... coordinates) {
    results_[slot]->CalculateAction(data_containers..., coordinates...);
  }

  /**
   * Finalizes the results and merges all slots into the first.
   */
  void Finalize() {
    auto result = results_.at(0);
    const auto number_of_slots = results_.size();
    std::vector<std::shared_ptr<Result_t>> others;
    for (std::size_t slot = 1; slot < number_of_slots; ++slot) {
      others.push_back(others[slot]);
    }
    result->Merge(others);
  }

  /**
   * Initializes the helper and the action using the first event in the input tree.
   * @param reader
   */
  void InitTask(TTreeReader *reader, unsigned int) {
    if (!is_configured_) {
      auto entry = reader->GetCurrentEntry();
      for (auto &result : results_) {
        reader->Restart();
        result->Initialize(*reader);
      }
      is_configured_ = true;
      reader->Restart();
      reader->SetLocalEntry(entry);
    }
  }

  void Initialize() { /* no-op */}
  Result_t &PartialUpdate(unsigned int slot) { return *results_.at(slot); }
  std::shared_ptr<Result_t> GetResultPtr() const { return results_[0]; }
  std::string GetActionName() const { return results_[0]->GetName(); }
};

/**
 * Helper function which creates the AverageHelper without needing to specify the template parameters.
 * @tparam Action Action which carries the needed information to derive the template parameters and create the AverageHelper.
 */
template<typename Action>
auto inline EventAverage(Action action) {
  using EventParameterTuple = typename Action::EventParameterTuple;
  using DataContainerTuple = TemplateMagic::TupleOf<Action::NumberOfInputs, Qn::DataContainerQVector>;
  return AverageHelper<Action, EventParameterTuple, DataContainerTuple>{action};
}

}

#endif
