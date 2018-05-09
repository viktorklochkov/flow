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
#include <chrono>

namespace Qn {
class Sampler {
 public:
  enum class Method {
    NONE,
    BOOTSTRAP,
    SUBSAMPLING,
  };

  Sampler() = default;

  explicit Sampler(unsigned int nevents, unsigned int nsamples) :
      n_events_(nevents),
      n_samples_(nsamples) {
    samples_.resize(nevents);
    if (nevents < nsamples) n_samples_ = 1;
  }
  explicit Sampler(unsigned int nsamples, Method met) :
      method_(met),
      n_events_(0),
      n_samples_(nsamples) {
    samples_.resize(0);
  }

  void Configure(Method method, unsigned int nsamples) {
    method_ = method;
    n_samples_ = nsamples;
  }
  void SetNumberOfEvents(unsigned long num) {
    n_events_ = num;
    samples_.resize(num);
  }

  void CreateSamples() {
    if (method_==Method::BOOTSTRAP) CreateBootstrapSamples();
    if (method_==Method::SUBSAMPLING) CreateSubSamples();
  }

  int GetNumSamples() const { return n_samples_; }

  void CreateSubSamples() {
    TRandom3 random;
    std::vector<int> event_vector;
    event_vector.reserve(n_events_);
    for (unsigned int i = 0; i < n_events_; ++i) {
      event_vector.push_back(i);
    }
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(event_vector.begin(), event_vector.end(), std::default_random_engine(seed));
    auto div = n_events_/n_samples_;
    for (unsigned int i = 0; i < n_events_ - (n_events_%n_samples_); ++i) {
      samples_[event_vector[i]].push_back(i/(div));
    }
    for (unsigned long i = n_events_ - (n_events_%n_samples_); i < n_events_; ++i) {
      samples_[event_vector[i]].push_back((n_samples_ - 1));
    }
  }

  void CreateBootstrapSamples() {
    TRandom3 random;
    for (unsigned int isample = 0; isample < n_samples_; ++isample) {
      for (unsigned int ievent = 0; ievent < n_events_; ++ievent) {
        samples_[(int) (random.Rndm()*n_events_)].push_back(isample);
      }
    }
  }

  void CreateDividedBootstrapSamples(const unsigned int ndivisions) {
    TRandom3 random;
    std::vector<unsigned long> divisions = {0};
    divisions.resize(ndivisions + 1);
    std::vector<std::vector<std::vector<int>>> chunks = {{{0}}};
    auto chunk = n_events_/ndivisions;
    auto remainder = n_events_%ndivisions;
    for (unsigned int i = 1; i < ndivisions; ++i) {
      divisions[i] = i*chunk;
    }
    divisions[0] = 0;
    divisions[ndivisions] += n_events_;
    unsigned int isa;
    for (isa = 0; isa < ndivisions - 1; ++isa) {
      for (unsigned int isample = 0; isample < n_samples_; ++isample) {
        for (unsigned long ievent = divisions[isa]; ievent < divisions[isa + 1]; ++ievent) {
          samples_[(int) (random.Rndm()*chunk + divisions[isa])].push_back(isample);
        }
      }
    }
    isa = ndivisions - 1;
    for (unsigned int isample = 0; isample < n_samples_; ++isample) {
      for (unsigned long ievent = divisions[isa]; ievent < divisions[isa + 1]; ++ievent) {
        samples_[(int) (random.Rndm()*(chunk + remainder) + divisions[isa])].push_back(isample);
      }
    }
  }

  void CreateResamples() {
    TRandom3 random;
    for (unsigned int ievent = 0; ievent < n_events_; ++ievent) {
      samples_[ievent].push_back((unsigned int) (random.Rndm()*n_samples_));
    }
  }

  const std::vector<unsigned int> &GetFillVector(const int ievent) const {
    return samples_[ievent];
  }

  void GetFillVector(std::vector<unsigned int> &vector) {
    vector = samples_[ievent_];
  }

  void UpdateEvent() {
    ievent_++;
  }

  void ResetEvent() {
    ievent_ = 0;
  }
  std::vector<std::vector<unsigned int>> GetSamples() const {return samples_;}

 private:
  Method method_ = Method::NONE;
  unsigned long ievent_ = 0;
  unsigned long n_events_ = 0;
  unsigned int n_samples_ = 0;
  std::vector<std::vector<unsigned int>> samples_;
};
}
#endif //FLOW_BOOTSTRAPSAMPLER_H
