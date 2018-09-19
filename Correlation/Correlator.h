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

#ifndef FLOW_CORRELATOR_H
#define FLOW_CORRELATOR_H

#include <Base/QVector.h>
#include "Correlation.h"
#include "Sampler.h"

namespace Qn {
class Correlator {
 public:
  using size_type = std::size_t;
  Correlator(std::vector<std::string> input_names,
             std::function<double(std::vector<Qn::QVector> &)> lambda
  ) : lambda_correlation_(std::move(lambda)), input_names_(std::move(input_names)) {}

  Correlator(std::vector<std::string> input_names,
             std::function<double(std::vector<Qn::QVector> &)> lambda, const TH1F &base
  ) : binned_result_(new Qn::DataContainerTH1F()), lambda_correlation_(std::move(lambda)), input_names_(std::move(input_names)) {
    binned_result_->InitializeEntries(base);
  }

  void ConfigureSampler(Sampler::Method method, size_type nsamples) {
    sampler_.Configure(method, nsamples);
  }

  void BuildSamples(std::size_t nevents) {
    sampler_.SetNumberOfEvents(nevents);
    auto nsamples = sampler_.GetNumSamples();
    result_ = result_.Map([nsamples](Sample sample) {
      sample.SetNumberOfSamples(nsamples);
      return sample;
    });
    sampler_.CreateSamples();
  }

  const std::vector<std::string> &GetInputNames() const { return input_names_; }

  void FillCorrelation(const std::vector<DataContainerQVector> &inputs,
                       const std::vector<unsigned long> eventindex,
                       std::size_t event_id) {
    correlation_.Fill(inputs, eventindex);
    FillResult(event_id);
  }

  void FindAutoCorrelations() {
    std::vector<std::vector<size_type>> auto_correlations;
    auto n_event_axes = correlation_.GetEventAxes().size();
    for (unsigned long i_input = 0; i_input < input_names_.size(); ++i_input) {
      std::vector<unsigned long> correlated_inputs;
      correlated_inputs.push_back(i_input + n_event_axes);
      for (unsigned long j_input = i_input + 1; j_input < input_names_.size(); ++j_input) {
        if (input_names_[i_input]==input_names_[j_input]) {
          correlated_inputs.push_back(j_input + n_event_axes);
        }
      }
      if (correlated_inputs.size() > 1) auto_correlations.push_back(correlated_inputs);
    }
    auto correlation_axes = correlation_.GetCorrelation().GetAxes();
    for (const auto &correlation : auto_correlations) {
      std::vector<Qn::Axis> axes;
      for (const auto id : correlation) {
        axes.push_back(correlation_axes[id]);
      }
      autocorrelated_bins_.push_back(correlation_.GetCorrelation().GetDiagonal(axes));
    }
  }

  void RemoveAutoCorrelation() {
    for (auto bins : autocorrelated_bins_) {
      for (auto bin : bins) {
        correlation_.GetCorrelation().ClearDataAt(bin);
      }
    }
  }

  void FillResult(size_type event_id) {
    auto sample_id_vector = sampler_.GetFillVector(event_id);
    int ibin = 0;
    for (const auto &bin : correlation_.GetCorrelation()) {
      if (bin.first) result_.At(ibin).Fill(bin.second, sample_id_vector);
      if (bin.first && binned_result_) {
        binned_result_->At(ibin).Fill(bin.second);
      }
      ++ibin;
    }
  }

  void ConfigureCorrelation(const std::vector<DataContainerQVector> &input, std::vector<Qn::Axis> event) {
    correlation_.ConfigureCorrelation(input, event, lambda_correlation_, input_names_);
    result_.AddAxes(correlation_.GetCorrelation().GetAxes());
    if (binned_result_) {
      auto base_hist = binned_result_->At(0);
      binned_result_->AddAxes(correlation_.GetCorrelation().GetAxes());
      binned_result_->InitializeEntries(base_hist);
    }
  }

  Qn::Correlation GetCorrelation() const { return correlation_; }

  DataContainerSample GetResult() const { return result_; }

  std::shared_ptr<DataContainerTH1F> GetBinnedResult() const { return binned_result_; }

 private:
  Qn::Sampler sampler_;
  Qn::Correlation correlation_;
  Qn::DataContainerSample result_;
  std::shared_ptr<Qn::DataContainerTH1F> binned_result_;
  std::function<double(std::vector<Qn::QVector> &)> lambda_correlation_;
  std::vector<std::string> input_names_;
  std::vector<std::vector<size_type>> autocorrelated_bins_;
};
}

#endif //FLOW_CORRELATOR_H
