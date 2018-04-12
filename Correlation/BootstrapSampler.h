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

class BootstrapSampler {
 public:
  explicit BootstrapSampler(int nevents, int nsamples) :
      n_events_(nevents),
      n_samples_(nsamples) {
    samples_.resize(nevents);
    if (nevents < nsamples) n_samples_ = 1;
  }

  void CreateSubSamples() {
    TRandom3 random;
    std::vector<int> event_vector;
    event_vector.reserve(n_events_);
    for (int i = 0; i < n_events_; ++i) {
      event_vector.push_back(i);
    }
    long long int seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(event_vector.begin(), event_vector.end(), std::default_random_engine(seed));
    int div = n_events_/n_samples_;
    for (int i = 0; i < n_events_ - (n_events_ % n_samples_); ++i) {
      samples_[event_vector[i]].push_back(i/(div));
    }
    for (int i = n_events_ - (n_events_ % n_samples_); i < n_events_; ++i) {
      samples_[event_vector[i]].push_back((n_samples_-1));
    }
  }

  void CreateBootstrapSamples() {
    TRandom3 random;
    for (unsigned long isample = 0; isample < n_samples_; ++isample) {
      for (int ievent = 0; ievent < n_events_; ++ievent) {
        samples_[(int) (random.Rndm()*n_events_)].push_back(isample);
      }
    }
  }

  void CreateDividedBootstrapSamples(const unsigned long ndivisions) {
    TRandom3 random;
    std::vector<unsigned long> divisions = {0};
    divisions.resize(ndivisions + 1);
    std::vector<std::vector<std::vector<int>>> chunks = {{{0}}};
    auto chunk = n_events_/ndivisions;
    auto remainder = n_events_%ndivisions;
    for (int i = 1; i < ndivisions; ++i) {
      divisions[i] = i*chunk;
    }
    divisions[0] = 0;
    divisions[ndivisions] += n_events_;
    int isa;
    for (isa = 0; isa < ndivisions - 1; ++isa) {
      for (unsigned long isample = 0; isample < n_samples_; ++isample) {
        for (int ievent = divisions[isa]; ievent < divisions[isa + 1]; ++ievent) {
          samples_[(int) (random.Rndm()*chunk + divisions[isa])].push_back(isample);
        }
      }
    }
    isa = ndivisions - 1;
    for (unsigned long isample = 0; isample < n_samples_; ++isample) {
      for (int ievent = divisions[isa]; ievent < divisions[isa + 1]; ++ievent) {
        samples_[(int) (random.Rndm()*(chunk + remainder) + divisions[isa])].push_back(isample);
      }
    }
  }

  void CreateResamples() {
    TRandom3 random;
    for (int ievent = 0; ievent < n_events_; ++ievent) {
      samples_[ievent].push_back((unsigned long) (random.Rndm()*n_samples_));
    }
  }

  std::vector<unsigned long> GetFillVector(const int ievent) const {
    return samples_[ievent];
  }

 public:
  int n_events_;
  int n_samples_;
  std::vector<std::vector<unsigned long>> samples_;
};

#endif //FLOW_BOOTSTRAPSAMPLER_H
