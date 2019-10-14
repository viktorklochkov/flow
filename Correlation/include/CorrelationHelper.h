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
#ifndef FLOW_DATAFRAMESTATISTICS_H
#define FLOW_DATAFRAMESTATISTICS_H

#include "ROOT/RDF/ActionHelpers.hxx"
#include "ROOT/RStringView.hxx"
#include "ROOT/TypeTraits.hxx"
#include "ROOT/RVec.hxx"

#include "Correlation.h"
#include "AxesConfiguration.h"

#include "DataContainer.h"

namespace Qn {
namespace Correlation {

template<typename helper_t>
using RActionImpl =  ROOT::Detail::RDF::RActionImpl<helper_t>;

enum ConfigurationState {
  Start,
  Input,
  Weight
};

template<ConfigurationState State, typename AxisConfig, typename Correlation, typename EventParameters, typename DataContainers>
class CorrelationHelper;

}
template<typename Function, typename AxisConfig>
auto MakeCorrelation(const std::string &name, Function function, AxisConfig event_axes);

namespace Correlation {
template<ConfigurationState State, typename AxisConfig, typename Correlation, typename... EventParameters, typename... DataContainers>
class CorrelationHelper<State,
                        AxisConfig,
                        Correlation,
                        std::tuple<EventParameters...>,
                        std::tuple<DataContainers...>> :
    public RActionImpl<CorrelationHelper<State, AxisConfig, Correlation,
                                         std::tuple<EventParameters...>,
                                         std::tuple<DataContainers...> >> {
 public:
  using Function = typename Correlation::FunctionType;
  using Result_t = Qn::DataContainerStats;

  template<typename F, typename Axis>
  friend auto Qn::MakeCorrelation(const std::string &name, F function, Axis event_axes);

  template<ConfigurationState OtherState>
  using CorrelationHelperOtherState = CorrelationHelper<OtherState, AxisConfig, Correlation,
                                                        std::tuple<EventParameters...>,
                                                        std::tuple<DataContainers...>>;
 private:
  std::string name_; //!<! Name of the Correlation
  std::size_t stride_ = 0; //!<! size of the correlation data container without event axes
  std::vector<std::shared_ptr<Result_t>> data_containers_; //!<! vector of result data containers
  AxisConfig event_axes_config_; //!<! Axis configuration of the event axes
  Correlation correlation_; //!<! object calculating the event by event correlation
 public:
  CorrelationHelper(std::string name, Correlation correlation, AxisConfig event_axes_config) :
      name_(std::move(name)),
      event_axes_config_(std::move(event_axes_config)),
      correlation_(std::move(correlation)) {
    const auto n_slots = ROOT::IsImplicitMTEnabled() ? ROOT::GetImplicitMTPoolSize() : 1;
    for (std::size_t i = 0; i < n_slots; ++i) {
      data_containers_.emplace_back(std::make_shared<Qn::DataContainerStats>());
    }
  }

  CorrelationHelper(CorrelationHelper &&other) = default;

  template<ConfigurationState FromState>
  CorrelationHelper(CorrelationHelperOtherState<FromState> &&other) :
      name_(std::move(other.name_)),
      stride_(std::move(other.stride_)),
      data_containers_(std::move(other.data_containers_)),
      event_axes_config_(std::move(other.event_axes_config_)),
      correlation_(std::move(other.correlation_)) {}

  friend CorrelationHelperOtherState<ConfigurationState::Start>;
  friend CorrelationHelperOtherState<ConfigurationState::Input>;
  friend CorrelationHelperOtherState<ConfigurationState::Weight>;

  template<typename ...Names>
  CorrelationHelperOtherState<ConfigurationState::Input> SetInputNames(Names... names) &&{
    correlation_.SetInputNames(std::forward<Names>(names)...);
    return {std::move(*this)};
  }

  template<typename ...Weights>
  CorrelationHelperOtherState<ConfigurationState::Weight> SetWeights(Weights... weights) &&{
    static_assert(State==ConfigurationState::Input, "Configure input Q vector names first");
    correlation_.SetWeights(std::forward<Weights>(weights)...);
    return {std::move(*this)};
  }

  void Configure(TTreeReader &reader, const std::size_t n_resamples) {
    correlation_.Initialize(reader);
    auto correlation_axes = correlation_.GetCorrelationAxes();
    auto event_axes = event_axes_config_.GetVector();
    // Initialize output data containers
    for (auto &data : data_containers_) {
      data->AddAxes(event_axes);
      data->AddAxes(correlation_axes);
      for (auto &bin : *data) {
        bin.SetNumberOfReSamples(n_resamples);
      }
    }
    // calculate stride of the resulting container
    Qn::DataContainerStats temp_correlation;
    temp_correlation.AddAxes(correlation_axes);
    stride_ = temp_correlation.size();
  }

  template<typename DATAFRAME>
  auto BookMe(DATAFRAME &df, TTreeReader &reader, const std::size_t n_resamples) &&{
    static_assert(State==ConfigurationState::Weight, "Configure weights first");
    Configure(reader, n_resamples);
    std::vector<std::string> columns;
    columns.emplace_back("Samples");
    auto input_names = correlation_.GetInputNames();
    for (const auto &name : input_names) {
      columns.emplace_back(name);
    }
    auto event_axes = event_axes_config_.GetVector();
    for (const auto &axis : event_axes) {
      columns.emplace_back(axis.Name());
    }
    return df.template Book<ROOT::RVec<ULong64_t>, DataContainers..., EventParameters...>(std::move(*this), columns);
  }

  void Exec(unsigned int slot,
            const ROOT::RVec<ULong64_t> sample_ids,
            DataContainers... data_containers,
            EventParameters... coordinates) {
    const auto &per_event_correlation = correlation_.Correlate(data_containers...);
    auto event_bin = event_axes_config_.GetLinearIndexFromCoordinates(coordinates...);
    if (event_bin < 0) return;
    for (std::size_t ibin = 0; ibin < per_event_correlation.size(); ++ibin) {
      data_containers_[slot]->At(event_bin*stride_ + ibin).FillPoisson(per_event_correlation[ibin], sample_ids);
    }
  }

  void InitTask(TTreeReader *, unsigned int) {

  }

  void Initialize() { /* no-op */}

  void Finalize() {
    auto result = data_containers_.at(0);
    const auto number_of_slots = data_containers_.size();
    TList l;
    for (std::size_t slot = 1; slot < number_of_slots; ++slot) {
      l.Add(data_containers_.at(slot).get());
    }
    result->Merge(&l);
  }

  Result_t &PartialUpdate(unsigned int slot) {
    return *data_containers_.at(slot);
  }

  std::shared_ptr<Result_t> GetResultPtr() const {
    return data_containers_.at(0);
  }

  std::string GetActionName() const {
    return name_;
  }
};

template<typename F, typename AxisConfig>
auto MakeCorrelation(const std::string &name, F function, AxisConfig event_axes) {
  auto constexpr n_parameters = TemplateHelpers::FunctionTraits<decltype(function)>::Arity;
  using QVectorTuple = TemplateHelpers::TupleOf<n_parameters, Qn::QVector>;
  using DataContainerTuple = TemplateHelpers::TupleOf<n_parameters, Qn::DataContainerQVector>;
  auto correlation = Correlation<F, QVectorTuple, DataContainerTuple>(function);
  using EventParameterTuple = typename AxisConfig::AxisValueTypeTuple;
  using CorrelationType = decltype(correlation);
  CorrelationHelper<ConfigurationState::Start,
                    AxisConfig,
                    CorrelationType,
                    EventParameterTuple,
                    DataContainerTuple> helper(name, correlation, event_axes);
  return helper;
}

}
}
#endif //FLOW_DATAFRAMESTATISTICS_H
