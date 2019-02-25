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

#ifndef FLOW_STATS_H
#define FLOW_STATS_H

#include <vector>
#include <iostream>
#include <bitset>

#include "Rtypes.h"

#include "Profile.h"
#include "SubSamples.h"

namespace Qn {

class Stats {
 public:

  enum Settings {
    CORRELATEDERRORS = BIT(16),
    PRODAVGWEIGHTS = BIT(17),
    ASYMMERRORS = BIT(18)
  };

  enum class Status {
    REFERENCE,
    OBSERVABLE,
    POINTAVERAGE,
  };

  using size_type = std::size_t;

  Stats() = default;

  double Mean() const { return profile_.Mean(); }
  double BootstrapMean() const { return subsamples_.Mean(); }
  double Error() const {
    if (bits_ & Settings::CORRELATEDERRORS) {
      return (subsamples_.ErrorHi(profile_.Mean()) + subsamples_.ErrorLo(profile_.Mean()))/2;
    } else { return profile_.Error(); }
  }
  double ErrorLo() const { return subsamples_.ErrorLo(profile_.Mean()); }
  double ErrorHi() const { return subsamples_.ErrorHi(profile_.Mean()); }

  friend Stats Merge(const Stats &, const Stats &);
  friend Stats operator+(const Stats &, const Stats &);
  friend Stats operator-(const Stats &, const Stats &);
  friend Stats operator*(const Stats &, const Stats &);
  friend Stats operator*(const Stats &, double);
  friend Stats operator*(double, const Stats &);
  friend Stats operator/(const Stats &, const Stats &);
  friend Stats Sqrt(const Stats &);

  virtual ~Stats() = default;

  void SetBits(unsigned int bits) { bits_ = bits; }
  void ResetBits(UInt_t bits) { bits_ &= ~(bits & 0x00ffffff); }

  void Print();

  void Fill(const Product &product, const std::vector<size_type> &samples) {
    subsamples_.Fill(product, samples);
    profile_.Fill(product);
  }

  void SetNumberOfSubSamples(size_type nsamples) {
    subsamples_.SetNumberOfSamples(nsamples);
  }

  TH1F SampleMeanHisto(const std::string &name) {
    return subsamples_.SubSampleMeanHisto(name);
  }

  void SetStatus(Stats::Status status) { status_ = status; }
  Status GetStatus(Stats::Status status) const{ return status; }

 private:
  SubSamples subsamples_;
  Profile profile_;
  unsigned int bits_ = 0;
  Status status_ = Status::REFERENCE;

  /// \cond CLASSIMP
 ClassDef(Stats, 1);
  /// \endcond
};

 Stats Merge(const Stats &, const Stats &);
 Stats operator+(const Stats &, const Stats &);
 Stats operator-(const Stats &, const Stats &);
 Stats operator*(const Stats &, const Stats &);
 Stats operator*(const Stats &, double);
 Stats operator*(double, const Stats &);
 Stats operator/(const Stats &, const Stats &);
 Stats Sqrt(const Stats &);
}

#endif //FLOW_STATS_H