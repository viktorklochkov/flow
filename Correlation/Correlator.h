//
// Created by Lukas Kreis on 19.04.18.
//

#ifndef FLOW_CORRELATOR_H
#define FLOW_CORRELATOR_H

#include <Base/QVector.h>
#include "Correlation.h"
#include "Sampler.h"

namespace Qn {
class Correlator {
 public:
  Correlator(std::vector<std::string> input_names,
             std::function<double(std::vector<Qn::QVector> &)> lambda
  ) : lambda_correlation_(std::move(lambda)), input_names_(std::move(input_names)) {
    binned_result_.InitializeEntries(TH1F("no","no",1,0,1));
  }

  Correlator(std::vector<std::string> input_names,
             std::function<double(std::vector<Qn::QVector> &)> lambda, const TH1F &base
  ) : lambda_correlation_(std::move(lambda)), input_names_(std::move(input_names)) {
    binned_result_.InitializeEntries(base);
  }

  void ConfigureSampler(Sampler::Method method, int nsamples) {
    sampler_.Configure(method, nsamples);
  }

  void BuildSamples(int nevents) {
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
                       int event_id) {
    correlation_.Fill(inputs, eventindex);
    FillResult(event_id);
  }

  void FindAutoCorrelations() {
    std::vector<std::vector<unsigned long>> auto_correlations;
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
        Qn::SetToZero(correlation_.At(bin));
      }
    }
  }

  void FillResult(int event_id) {
    auto sample_id_vector = sampler_.GetFillVector(event_id);
    int ibin = 0;
    for (const auto &bin : correlation_.GetCorrelation()) {
      if (bin.first) result_.At(ibin).Fill(bin.second, sample_id_vector);
      if (bin.first) binned_result_.At(ibin).Fill(bin.second);
      ++ibin;
    }
  }

  void ConfigureCorrelation(const std::vector<DataContainerQVector> &input, std::vector<Qn::Axis> event) {
    correlation_.ConfigureCorrelation(input, event, lambda_correlation_, input_names_);
    result_.AddAxes(correlation_.GetCorrelation().GetAxes());
    auto base_hist = binned_result_.At(0);
    binned_result_.AddAxes(correlation_.GetCorrelation().GetAxes());
    binned_result_.InitializeEntries(base_hist);
  }

  Qn::Correlation GetCorrelation() const { return correlation_; }

  DataContainerSample GetResult() const { return result_; }

  DataContainerTH1F GetBinnedResult() const { return binned_result_; }

 private:
  Qn::Sampler sampler_;
  Qn::Correlation correlation_;
  Qn::DataContainerSample result_;
  Qn::DataContainerTH1F binned_result_;
  std::function<double(std::vector<Qn::QVector> &)> lambda_correlation_;
  std::vector<std::string> input_names_;
  std::vector<std::vector<int>> autocorrelated_bins_;
};
}

#endif //FLOW_CORRELATOR_H
