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
#ifndef FLOW_CORRELATIONACTION_H
#define FLOW_CORRELATIONACTION_H

#include <functional>
#include <string>
#include <array>
#include <vector>
#include <algorithm>

#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "ROOT/RVec.hxx"

#include "DataContainer.h"
#include "QVector.h"
#include "Common/TemplateMagic.h"

namespace Qn {
namespace Correlation {

template<typename Function, typename InputDataContainers, typename AxesConfig, typename EventParameters>
class CorrelationAction;

template<typename Function, typename... InputDataContainers, typename AxesConfig, typename... EventParameters>
class CorrelationAction<Function, std::tuple<InputDataContainers...>, AxesConfig, std::tuple<EventParameters...>> {
 public:
  constexpr static std::size_t NumberOfInputs = sizeof...(InputDataContainers);
  using EventParameterTuple = std::tuple<ROOT::RVec<ULong64_t>, EventParameters...>;
 private:
  using DataContainerRef = std::reference_wrapper<const DataContainerQVector>;
 public:

  /**
   * Constructor
   * @param function Correlation function of type (const Qn::QVector &q...) -> double
   * @param input_names Array of Q-vectors going into the correlation
   * @param weights Array of weights of the Q-vectors
   * @param correlation_name Name of the correlation
   * @param event_axes Event axes for the binning of the correlation.
   * @param n_samples Number of samples used for the bootstrapping
   */
  CorrelationAction(Function function,
                    const std::array<std::string, NumberOfInputs> &input_names,
                    const std::array<Qn::Stats::Weights, NumberOfInputs> &weights,
                    const std::string &correlation_name,
                    AxesConfig event_axes,
                    unsigned int n_samples) :
      n_samples_(n_samples),
      action_name_(correlation_name),
      input_names_(input_names),
      event_axes_(event_axes),
      function_(function) {
    std::transform(std::begin(weights), std::end(weights), std::begin(use_weights_),
                   [](Qn::Stats::Weights weight) { return weight==Qn::Stats::Weights::OBSERVABLE; });
  }

  /**
   * Initializes the CorrelationAction using the input Q-vectors in the input TTree.
   * @param reader The TTreeReader gives access to the input TTree.
   */
  void Initialize(TTreeReader &reader) {
    std::vector<TTreeReaderValue<DataContainerQVector>> input_data;
    std::transform(std::begin(input_names_),std::end(input_names_),std::back_inserter(input_data),
        [&reader](const std::string &name){return TTreeReaderValue<DataContainerQVector>(reader,name.data());});
    reader.SetLocalEntry(1);
    for (std::size_t i = 0; i < input_data.size(); ++i) {
      auto &i_data = input_data[i];
      if (i_data.GetSetupStatus() < 0) {
        std::ostringstream error_message;
        error_message << "The Q-Vector " << i_data.GetBranchName() << " in the tree is not valid. Aborting.";
        throw std::runtime_error(error_message.str());
      }
      if (!i_data->IsIntegrated()) AddAxes(input_data, i);
    }
    stride_ = correlation_.size();
    correlation_.AddAxes(event_axes_.GetVector());
    for (auto &bin : correlation_) {
      bin.SetNumberOfReSamples(n_samples_);
      if (IsObservable()) bin.SetWeights(Qn::Stats::Weights::OBSERVABLE);
      else                bin.SetWeights(Qn::Stats::Weights::REFERENCE);
    }
    reader.Restart();
  }

  /**
   * Returns the name of the columns used in the correction step. This includes both the input Q-vector and
   * the name of the axes parameters. This function is required by the AverageHelper.
   * @return returns a vector of the column names.
   */
  std::vector<std::string> GetColumnNames() const {
    std::vector<std::string> columns;
    std::copy(std::begin(input_names_), std::end(input_names_), std::back_inserter(columns));
    columns.emplace_back("samples");
    const auto event_axes_names = event_axes_.GetNames();
    std::copy(std::begin(event_axes_names), std::end(event_axes_names), std::back_inserter(columns));
    return columns;
  }

  /**
   * Calculates the result of the correlation function.
   * @param input_q input Q-vectors
   * @param sample_ids frequency map of the resamples.
   * @param event_parameters event parameters for event classification.
   */
  void CalculateAction(InputDataContainers... input_q,
                       const ROOT::RVec<ULong64_t> &sample_ids,
                       EventParameters... event_parameters) {
    long initial_offset = event_axes_.GetLinearIndex(event_parameters...)*stride_;
    if (initial_offset < 0) return;
    std::array<const Qn::QVector*, NumberOfInputs> q_vectors;
    const std::array<const DataContainerRef, NumberOfInputs> input_array = {std::cref(input_q)...};
    LoopOverBins(initial_offset, q_vectors, input_array, sample_ids, 0);
  }

  /**
   * Merges the correction histogram after the collection of statistics is complete.
   * This function is required by the AverageHelper class.
   * @param results the other results which are to be merged with this one.
   */
  void Merge(const std::vector<std::shared_ptr<CorrelationAction>> &results) {
    TList correlation_list;
    for (auto &result : results) {
      correlation_list.Add(&result->correlation_);
    }
    correlation_.Merge(&correlation_list);
  }

  /**
   * Returns the name of the output Q-vector.
   * @return name of the output Q-vector
   */
  std::string GetName() const { return action_name_; }
  /**
   * Gives a const reference to the result of the correlation. Triggers the evaluation of the event loop.
   * @return returns const reference.
   */
  const Qn::DataContainerStats &GetDataContainer() const { return correlation_; }

  /**
   * Writes the result of the correlation to file. Triggers the evaluation of the event loop
   */
  void Write() const { correlation_.Write(GetName().data()); }

 private:
  /**
   * Checks if at least one Q-vector is not a reference Q-vector.
   * @return true if at least one of them is not a reference Q-vector.
   */
  inline bool IsObservable() const {
    return std::any_of(std::begin(use_weights_), std::end(use_weights_), [](bool a) { return a; });
  }

  /**
   * Calculates the total weight of the Q-Vector going into the correlation.
   * @param q_array Array of Q-vectors.
   * @return Weight of the correlation result for this event.
   */
  double CalculateWeight(const std::array<const Qn::QVector *, NumberOfInputs> &q_array) const {
    int i = 0;
    double weight = 1.0;
    for (const auto &q : q_array) {
      if (use_weights_[i]) weight *= q->sumweights();
      ++i;
    }
    return weight;
  }

  /**
   * Loops recursivly over all bins of the correlation result.
   * @param out_bin Initial offset based on the current event parameters.
   * @param q_array Holder for the Q-vectors entering the calculation of the current bin.
   * @param input_array Holder of the DataContainers for the current event.
   * @param sample_ids Frequency map of the resamples of the current event.
   * @param step Recursion step.
   */
  void LoopOverBins(long &out_bin,
                    std::array<const Qn::QVector *, NumberOfInputs> &q_array,
                    const std::array<const DataContainerRef, NumberOfInputs> &input_array,
                    const ROOT::RVec<ULong64_t> &sample_ids,
                    std::size_t step) {
    if (step + 1==NumberOfInputs) { /// base case
      for (const auto &bin : input_array[step].get()) {
        if (bin.sumweights() < 1.) continue;
        q_array[step] = &bin;
        correlation_[out_bin].Fill(TemplateMagic::Call(function_, q_array), CalculateWeight(q_array), sample_ids);
        ++out_bin;
      }
    } else { /// recursion
      for (const auto &bin : input_array[step].get()) {
        if (bin.sumweights() < 1.) continue;
        q_array[step] = &bin;
        LoopOverBins(out_bin, q_array, input_array, sample_ids, step + 1);
      }
    }
  }

  /**
   * Adds axes of the Q-vectors to the correlation DataContainer.
   * If similar axes are found the axis is renamed to make sure the name is unique.
   * @param data_containers Datacontainers going into the correlation
   * @param i Current position of the data container.
   */
  void AddAxes(std::vector<TTreeReaderValue<DataContainerQVector>> &data_containers, std::size_t i) {
    for (auto axis :data_containers[i]->GetAxes()) {
      std::string name = axis.Name();
      for (std::size_t j = 0; j < NumberOfInputs; ++j) {
        if (i==j) continue;
        auto &other = data_containers[j];
        if (other->IsIntegrated()) continue;
        for (const auto &other_axis : other->GetAxes()) {
          if (axis!=other_axis) continue;
          auto this_name = std::string(data_containers[i].GetBranchName());
          if (this_name==other.GetBranchName()) name = this_name+"_"+std::to_string(i)+"_"+axis.Name();
          else                                  name = this_name+"_"+axis.Name();
        }
      }
      axis.SetName(name);
      correlation_.AddAxis(axis);
    }
  }

  unsigned int stride_ = 1; /// Offset of due to the non-event axes.
  unsigned int n_samples_ = 1; /// Number of samples used in the ReSampler.
  std::string action_name_; /// Name of the correlation.
  std::array<std::string, NumberOfInputs> input_names_; /// Names of the input Qvectors.
  std::array<bool, NumberOfInputs> use_weights_; /// Array to track which weight is being used.
  AxesConfig event_axes_; /// Configuration of the event axes.
  Function function_; /// Correlation function.
  Qn::DataContainerStats correlation_; /// Result data container.
};

/**
 * Helper function to create a Correlation without specifying the template parameters directly.
 * @tparam Function type of the correlation function.
 * @tparam AxesConfig Type of the axis configuration.
 * @param function Correlation function
 * @param input_names Array of input Q-vector names.
 * @param weights Array of weights.
 * @param correlation_name Name of the correlation
 * @param event_axes Event axes used in the correlation.
 * @param n_samples Number of samples used in the Resampling step.
 * @return result of the correlation.
 */
template<typename Function, typename AxesConfig>
auto Correlation(Function function,
                     const std::array<std::string, TemplateMagic::FunctionTraits<Function>::Arity> input_names,
                     const std::array<Qn::Stats::Weights, TemplateMagic::FunctionTraits<Function>::Arity> weights,
                     const std::string& correlation_name,
                     AxesConfig event_axes,
                     unsigned int n_samples) {
  constexpr auto n_parameters = TemplateMagic::FunctionTraits<Function>::Arity;
  using DataContainerTuple = TemplateMagic::TupleOf<n_parameters, Qn::DataContainerQVector>;
  using EventParameterTuple = typename AxesConfig::AxisValueTypeTuple;
  return CorrelationAction<Function, DataContainerTuple, AxesConfig, EventParameterTuple>(function, input_names, weights, correlation_name, event_axes, n_samples);
}

}
}
#endif //FLOW_CORRELATIONACTION_H
