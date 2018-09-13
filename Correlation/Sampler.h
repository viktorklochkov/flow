//
// Created by Lukas Kreis on 22.02.18.
//

#ifndef FLOW_BOOTSTRAPSAMPLER_H
#define FLOW_BOOTSTRAPSAMPLER_H

#include <vector>
#include <array>
#include <TRandom3.h>
#include <iostream>
#include <random>
#include <algorithm>

namespace Qn {
class Sampler {
 public:
  using size_type = std::size_t;
  enum class Method {
    NONE,
    BOOTSTRAP,
    SUBSAMPLING,
  };

  Sampler() = default;

  explicit Sampler(size_type nevents, size_type nsamples) :
      n_events_(nevents),
      n_samples_(nsamples) {
    samples_.resize(nevents);
    if (nevents < nsamples) n_samples_ = 1;
  }
  explicit Sampler(size_type nsamples, Method met) :
      method_(met),
      n_events_(0),
      n_samples_(nsamples) {
    samples_.resize(0);
  }

  void Configure(Method method, size_type nsamples) {
    method_ = method;
    n_samples_ = nsamples;
  }
  void SetNumberOfEvents(size_type num) {
    n_events_ = num;
    samples_.resize(num);
  }

  void CreateSamples() {
    if (method_==Method::BOOTSTRAP) CreateBootstrapSamples();
    if (method_==Method::SUBSAMPLING) CreateSubSamples();
  }

  inline size_type GetNumSamples() const { return n_samples_; }

  void CreateSubSamples();

  void CreateBootstrapSamples();

  void CreateDividedBootstrapSamples(size_type ndivisions);

  void CreateResamples();

  inline const std::vector<size_type> &GetFillVector(size_type ievent) const { return samples_[ievent]; }
  inline void GetFillVector(std::vector<size_type> &vector) { vector = samples_[ievent_]; }
  inline void UpdateEvent() { ievent_++; }
  inline void ResetEvent() { ievent_ = 0; }
  inline std::vector<std::vector<size_type>> GetSamples() const { return samples_; }

 private:
  Method method_ = Method::NONE;
  size_type ievent_ = 0;
  size_type n_events_ = 0;
  size_type n_samples_ = 0;
  std::vector<std::vector<size_type>> samples_;
};
}
#endif //FLOW_BOOTSTRAPSAMPLER_H
