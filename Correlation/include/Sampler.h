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

#ifndef FLOW_BOOTSTRAPSAMPLER_H
#define FLOW_BOOTSTRAPSAMPLER_H

#include <vector>
#include <ctime>

namespace Qn {
class Sampler {
 public:
  using size_type = std::size_t;
  enum class Method {
    NONE,
    BOOTSTRAP,
    SUBSAMPLING,
  };

  enum class Resample {
    ON = true,
    OFF = false
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

  void Configure(Method method, size_type nsamples, unsigned long seed = time(0)) {
    method_ = method;
    n_samples_ = nsamples;
    seed_ = seed;
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

  void CreateMoutofNBootstrapSamples(float m_fraction);
  void CreateDividedBootstrapSamples(size_type ndivisions);

  void CreateResamples();

  inline const std::vector<size_type> &GetFillVector(size_type ievent) const { return samples_[ievent]; }
  inline void GetFillVector(std::vector<size_type> &vector) { vector = samples_[ievent_]; }
  inline void UpdateEvent() { ievent_++; }
  inline void ResetEvent() { ievent_ = 0; }
  inline std::vector<std::vector<size_type>> GetSamples() const { return samples_; }
  void SetSamples(const std::vector<std::vector<size_type>> &samples) {
    samples_ = samples;
  }

 private:
  unsigned long seed_ = static_cast<unsigned long>(time(0));
  Method method_ = Method::NONE;
  size_type ievent_ = 0;
  size_type n_events_ = 0;
  size_type n_samples_ = 0;
  std::vector<std::vector<size_type>> samples_;
};
}
#endif //FLOW_BOOTSTRAPSAMPLER_H
