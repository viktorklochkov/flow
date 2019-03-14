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

#include "Sampler.h"

#include <random>
#include <algorithm>

#include "TRandom3.h"

void Qn::Sampler::CreateSubSamples() {
  TRandom3 random;
  std::vector<int> event_vector;
  event_vector.reserve(n_events_);
  for (unsigned int i = 0; i < n_events_; ++i) {
    event_vector.push_back(i);
  }
  std::random_device rd;
  std::mt19937 g(rd());
  g.seed(seed_);
  std::shuffle(event_vector.begin(), event_vector.end(), g);
  auto div = n_events_/n_samples_;
  for (unsigned int i = 0; i < n_events_ - (n_events_%n_samples_); ++i) {
    samples_[event_vector[i]].push_back(i/(div));
  }
  for (unsigned long i = n_events_ - (n_events_%n_samples_); i < n_events_; ++i) {
    samples_[event_vector[i]].push_back((n_samples_ - 1));
  }
}
void Qn::Sampler::CreateBootstrapSamples() {
  TRandom3 random;
  random.SetSeed(seed_);
  for (unsigned int isample = 0; isample < n_samples_; ++isample) {
    for (unsigned int ievent = 0; ievent < n_events_; ++ievent) {
      samples_[(int) (random.Rndm()*n_events_)].push_back(isample);
    }
  }
}

void Qn::Sampler::CreateMoutofNBootstrapSamples(float m_fraction) {
  TRandom3 random;
  random.SetSeed(seed_);
  for (unsigned int isample = 0; isample < n_samples_; ++isample) {
    for (unsigned int ievent = 0; ievent < n_events_*m_fraction; ++ievent) {
      samples_[(int) (random.Rndm()*n_events_)].push_back(isample);
    }
  }
}

void Qn::Sampler::CreateDividedBootstrapSamples(const size_type ndivisions) {
  TRandom3 random;
  random.SetSeed(seed_);
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
void Qn::Sampler::CreateResamples() {
  TRandom3 random;
  random.SetSeed(seed_);
  for (unsigned int ievent = 0; ievent < n_events_; ++ievent) {
    samples_[ievent].push_back((unsigned int) (random.Rndm()*n_samples_));
  }
}
