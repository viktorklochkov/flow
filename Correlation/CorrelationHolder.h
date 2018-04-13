//
// Created by Lukas Kreis on 13.04.18.
//

#ifndef FLOW_CORRELATIONHOLDER_H
#define FLOW_CORRELATIONHOLDER_H
#include <Base/ResultContainer.h>
#include <utility>

#include "Correlation.h"
#include "BootstrapSampler.h"

namespace Qn {
class CorrelationHolder {
 public:

  CorrelationHolder(std::string name,
                    std::vector<std::string> inputs,
                    std::function<double(std::vector<Qn::QVector> &)> lambda,
                    int nsamples,
                    BootstrapSampler::Method method) :
      lambda_(std::move(lambda)),
      name_(std::move(name)),
      input_names_(std::move(inputs)),
      sampler_(new BootstrapSampler(nsamples, method)) {

  }

  void Initialize(int nevents, const std::vector<DataContainerQVector> &input,
                  const std::vector<Axis> &event) {
    sampler_->SetNumberOfEvents(nevents);
    sampler_->CreateSamples();
    correlation_.reset(new Correlation(input, event, lambda_));
    result_.reset(new ResultContainer(name_, correlation_->GetCorrelation().GetAxes(), sampler_->GetNumSamples()));
  }

  void FillCorrelation(const std::vector<DataContainerQVector> &input, const std::vector<long> &eventindex, int event_id) {
    correlation_->Fill(input, eventindex);
    FillToResult(event_id);
  }

  void FillToResult(int event_Id) {
    auto samples = sampler_->GetFillVector(event_Id);
    int ibin = 0;
    for (const auto &bin : correlation_->GetCorrelation()) {
      result_->Fill(ibin, bin, samples);
      ++ibin;
    }
  }

  const std::unique_ptr<ResultContainer> &GetResult() const { return result_; }

  std::vector<std::string> GetInputNames() const { return input_names_; }

 private:
  std::function<double(std::vector<Qn::QVector> &)> lambda_;
  std::string name_;
  std::vector<std::string> input_names_;
  std::unique_ptr<Qn::Correlation> correlation_ = nullptr;
  std::unique_ptr<Qn::BootstrapSampler> sampler_ = nullptr;
  std::unique_ptr<Qn::ResultContainer> result_ = nullptr;
};
}
#endif //FLOW_CORRELATIONHOLDER_H
