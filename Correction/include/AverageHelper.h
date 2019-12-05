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
namespace Correction {

template<typename Helper>
using RActionImpl =  ROOT::Detail::RDF::RActionImpl<Helper>;

template<typename Action, typename EventParameters, typename DataContainers>
class AverageHelper;

template<typename Action, typename... EventParameters, typename... DataContainers>
class AverageHelper<Action,
                    std::tuple<EventParameters...>,
                    std::tuple<DataContainers...>> : public RActionImpl<AverageHelper<Action,
                                                                                      std::tuple<EventParameters...>,
                                                                                      std::tuple<DataContainers...> >> {
 public:
  using Result_t = Action;

 private:
  bool is_configured_ = false;
  std::vector<std::shared_ptr<Action>> results_;

 public:
  explicit AverageHelper(Action action) {
    const auto n_slots = ROOT::IsImplicitMTEnabled() ? ROOT::GetImplicitMTPoolSize() : 1;
    for (std::size_t i = 0; i < n_slots; ++i) {
      results_.emplace_back(std::make_shared<Action>(action));
    }
  }

  template<typename DataFrame>
  ROOT::RDF::RResultPtr<Result_t> BookMe(DataFrame &df) {
    return df.template Book<DataContainers..., EventParameters...>(std::move(*this), results_[0]->GetColumnNames());
  }

  void Exec(unsigned int slot, DataContainers... data_containers, EventParameters... coordinates) {
    results_[slot]->CalculateCorrections(data_containers..., coordinates...);
  }

  void Finalize() {
    auto result = results_.at(0);
    const auto number_of_slots = results_.size();
    std::vector<std::shared_ptr<Result_t>> others;
    for (std::size_t slot = 1; slot < number_of_slots; ++slot) {
      others.push_back(others[slot]);
    }
    result->Merge(others);
  }

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

template<typename Action>
AverageHelper<Action,
              typename Action::EventParameterTuple,
              TemplateHelpers::TupleOf<Action::NumberOfInputs, Qn::DataContainerQVector>
             >
MakeAverageHelper(Action action) {
  using EventParameterTuple = typename Action::EventParameterTuple;
  using DataContainerTuple = TemplateHelpers::TupleOf<Action::NumberOfInputs, Qn::DataContainerQVector>;
  return AverageHelper<Action, EventParameterTuple, DataContainerTuple>{action};
}

}
}

#endif
