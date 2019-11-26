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
#ifndef FLOW_CORRECTION_INCLUDE_CORRECTIONFILLHELPER_H_
#define FLOW_CORRECTION_INCLUDE_CORRECTIONFILLHELPER_H_

#include "ROOT/RDF/ActionHelpers.hxx"

class CorrectionFillHelper : public ROOT::Detail::RDF::RActionImpl<CorrectionFillHelper> {
 public:
  using Result_t = Qn::DataContainerStatistic;
 private:
  std::vector<std::shared_ptr<Result_t>> data_containers_;
 public:
  CorrectionFillHelper(CorrectionFillHelper &&) = default;
  CorrectionFillHelper(const CorrectionFillHelper &) = delete;
  explicit CorrectionFillHelper(const Qn::DataContainerStatistic &h) {
    const auto n_slots = ROOT::IsImplicitMTEnabled() ? ROOT::GetImplicitMTPoolSize() : 1;
    for (unsigned int i = 0; i < n_slots; ++i) {
      data_containers_.emplace_back(std::make_shared<Result_t>(h));
    }
  }

  void InitTask(TTreeReader *, unsigned int) { /* noop */}
  void Exec(unsigned int slot, const double value, const double weight, const std::vector<double> &coordinates) {
    data_containers_[slot]->Fill(value, weight, coordinates);
  }
  void Initialize() { /* noop */}
  void Finalize() {
    auto result = data_containers_.at(0);
    const auto nslots = data_containers_.size();
    TList l;
    l.SetOwner(); // The list will free the memory associated to its elements upon destruction
    for (unsigned int slot = 1; slot < nslots; ++slot) {
      l.Add(data_containers_[slot].get());
    }
    result->Merge(&l);
  }
  Result_t &PartialUpdate(unsigned int slot) { return *data_containers_[slot]; }
  std::string GetActionName() { return "FillStatisticsHistogram"; }
  std::shared_ptr<Result_t> GetResultPtr() const {
    return data_containers_.at(0);
  }
};


#endif //FLOW_CORRECTION_INCLUDE_CORRECTIONFILLHELPER_H_
