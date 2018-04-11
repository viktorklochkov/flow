//
// Created by Lukas Kreis on 22.02.18.
//

#ifndef FLOW_BOOTSTRAPSAMPLER_H
#define FLOW_BOOTSTRAPSAMPLER_H

#include <vector>
#include <array>
#include <TRandom3.h>
#include <iostream>

class BootstrapSampler {
 public:
  explicit BootstrapSampler(int nevents, int nsamples) :
      n_events_(nevents),
      n_samples_(nsamples) {
    samples_.resize(nevents);
  }
  void CreateBootstrapSamples() {
    TRandom3 random;
    for (int isample = 0; isample < n_samples_; ++isample) {
      for (int ievent = 0; ievent < n_events_; ++ievent) {
        samples_[(int) (random.Rndm()*n_events_)].push_back(isample);
      }
    }
  }

  void CreateDividedBootstrapSamples(const int ndivisions) {
    TRandom3 random;
    std::vector<int> divisions = {0};
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
      for (int isample = 0; isample < n_samples_; ++isample) {
        for (int ievent = divisions[isa]; ievent < divisions[isa + 1]; ++ievent) {
          samples_[(int) (random.Rndm()*chunk + divisions[isa])].push_back(isample);
        }
      }
    }
    isa = ndivisions - 1;
    for (int isample = 0; isample < n_samples_; ++isample) {
      for (int ievent = divisions[isa]; ievent < divisions[isa + 1]; ++ievent) {
        samples_[(int) (random.Rndm()*(chunk + remainder) + divisions[isa])].push_back(isample);
      }
    }
  }

  void CreateResamples() {
    TRandom3 random;
    for (int ievent = 0; ievent < n_events_; ++ievent) {
      samples_[ievent].push_back((int) (random.Rndm()*n_samples_));
    }
  }

  std::vector<int> GetFillVector(const int ievent) const {
    return samples_[ievent];
  }

 public:
  int n_events_;
  int n_samples_;
  std::vector<std::vector<int>> samples_;
};

#endif //FLOW_BOOTSTRAPSAMPLER_H
