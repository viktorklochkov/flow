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

#include <SubSamples.h>

#include "SubSamples.h"

namespace Qn {

SubSamples SubSamples::MergeBinsNormal(const SubSamples &lhs, const SubSamples &rhs) {
  SubSamples subsamples(lhs);
  subsamples.samples_.resize(rhs.size());
  int i = 0;
  for (auto &sample : subsamples) {
    sample.sumwy = sample.sumwy + rhs.samples_[i].sumwy;
    sample.sumw = sample.sumw + rhs.samples_[i].sumw;
    ++i;
  }
  return subsamples;
}

SubSamples SubSamples::MergeBinsPointAverage(const SubSamples &lhs, const SubSamples &rhs) {
  SubSamples subsamples(lhs);
  subsamples.samples_.resize(rhs.size());
  int i = 0;
  for (auto &sample : subsamples) {
    sample.sumwy = sample.sumwy + rhs.samples_[i].sumwy;
    sample.sumw = sample.sumw + rhs.samples_[i].sumw;
    ++i;
  }
  return subsamples;
}

SubSamples SubSamples::MergeConcat(const SubSamples &lhs, const SubSamples &rhs) {
  SubSamples subsamples(lhs);
  subsamples.samples_.insert(subsamples.end(),rhs.begin(),rhs.end());
  return subsamples;
}

SubSamples SubSamples::AdditionNormal(const SubSamples &lhs, const SubSamples &rhs) {
  SubSamples subsamples(lhs);
  subsamples.samples_.resize(rhs.size());
  int i = 0;
  for (auto &sample : subsamples) {
    sample.sumwy = sample.sumwy + rhs.samples_[i].sumwy;
    sample.sumw = sample.sumw + rhs.samples_[i].sumw;
    ++i;
  }
  return subsamples;
}

SubSamples SubSamples::AdditionPointAverage(const SubSamples &lhs, const SubSamples &rhs) {
  SubSamples subsamples(lhs);
  subsamples.samples_.resize(rhs.size());
  int i = 0;
  for (auto &sample : subsamples) {
    sample.sumwy = sample.sumwy + rhs.samples_[i].sumwy;
    sample.sumw = sample.sumw + rhs.samples_[i].sumw;
    ++i;
  }
  return subsamples;
}

SubSamples SubSamples::SubtractionNormal(const SubSamples &lhs, const SubSamples &rhs) {
  SubSamples subsamples(lhs);
  subsamples.samples_.resize(rhs.size());
  int i = 0;
  for (auto &sample : subsamples) {
    sample.sumwy = sample.sumwy - rhs.samples_[i].sumwy;
    sample.sumw = sample.sumw - rhs.samples_[i].sumw;
    ++i;
  }
  return subsamples;
}

SubSamples SubSamples::SubtractionPointAverage(const SubSamples &lhs, const SubSamples &rhs) {
  SubSamples subsamples(lhs);
  subsamples.samples_.resize(rhs.size());
  int i = 0;
  for (auto &sample : subsamples) {
    sample.sumwy = sample.sumwy - rhs.samples_[i].sumwy;
    sample.sumw = sample.sumw - rhs.samples_[i].sumw;
    ++i;
  }
  return subsamples;
}

SubSamples SubSamples::MultiplicationNormal(const SubSamples &lhs, const SubSamples &rhs) {
  SubSamples subsamples(lhs);
  subsamples.samples_.resize(rhs.size());
  int i = 0;
  for (auto &sample : subsamples) {
    sample.sumwy = sample.sumwy*rhs.samples_[i].sumwy;
    sample.sumw = sample.sumw*rhs.samples_[i].sumw;
    ++i;
  }
  return subsamples;
}

SubSamples SubSamples::MultiplicationPointAverage(const SubSamples &lhs, const SubSamples &rhs) {
  SubSamples subsamples(lhs);
  subsamples.samples_.resize(rhs.size());
  int i = 0;
  for (auto &sample : subsamples) {
    sample.sumwy = sample.sumwy*rhs.samples_[i].sumwy;
    sample.sumw = sample.sumw*rhs.samples_[i].sumw;
    ++i;
  }
  return subsamples;
}

SubSamples SubSamples::DivisionNormal(const SubSamples &num, const SubSamples &den) {
  SubSamples subsamples(num);
  subsamples.samples_.resize(den.size());
  int i = 0;
  for (auto &sample : subsamples) {
    sample.sumwy = sample.sumwy/den.samples_[i].sumwy;
    sample.sumw = sample.sumw/den.samples_[i].sumw;
    ++i;
  }
  return subsamples;
}

SubSamples SubSamples::DivisionPointAverage(const SubSamples &num, const SubSamples &den) {
  SubSamples subsamples(num);
  subsamples.samples_.resize(den.size());
  int i = 0;
  for (auto &sample : subsamples) {
    sample.sumwy = sample.sumwy/den.samples_[i].sumwy;
    sample.sumw = sample.sumw/den.samples_[i].sumw;
    ++i;
  }
  return subsamples;
}

SubSamples SubSamples::SqrtNormal(const SubSamples &samp) {
  SubSamples subsamples(samp);
  int i = 0;
  for (auto &sum : subsamples) {
    sum.sumwy = sqrt(samp.samples_[i].sumwy);
    sum.sumw = sqrt(samp.samples_[i].sumw);
    ++i;
  }
  return subsamples;
}

SubSamples SubSamples::SqrtPointAverage(const SubSamples &samp) {
  SubSamples subsamples(samp);
  int i = 0;
  for (auto &sum : subsamples) {
    sum.sumwy = sqrt(samp.samples_[i].sumwy);
    sum.sumw = sqrt(samp.samples_[i].sumw);
    ++i;
  }
  return subsamples;
}

SubSamples SubSamples::ScaleNormal(const SubSamples &lhs, double rhs) {
  SubSamples subsamples(lhs);
  int i = 0;
  for (auto &sum : subsamples) {
    sum.sumwy = lhs.samples_[i].sumwy*rhs;
    sum.sumw = lhs.samples_[i].sumw;
    ++i;
  }
  return subsamples;
}

SubSamples SubSamples::ScalePointAverage(const SubSamples &lhs, double rhs) {
  SubSamples subsamples(lhs);
  int i = 0;
  for (auto &sum : subsamples) {
    sum.sumwy = lhs.samples_[i].sumwy*rhs;
    sum.sumw = lhs.samples_[i].sumw;
    ++i;
  }
  return subsamples;
}

void SubSamples::Print(double real_mean) {
  auto isample = samples_.size()/3;
  if (samples_.size() > 3) {
    std::cout << "SAMPLES EXCERPT: " << isample << ", " << isample*2 << ", " << isample*3 << std::endl;
    std::cout << "S" << isample << " ";
    samples_[isample].Print();
    std::cout << "S" << isample*2 << " ";
    samples_[isample*2].Print();
    std::cout << "S" << isample*3 << " ";
    samples_[isample*3].Print();
    std::cout << "SUMMARY   " << std::endl;
    std::cout << "N_samples " << samples_.size() << std::endl;
    std::cout << "Mean      " << Mean() << std::endl;
    std::cout << "Error Hi  " << ErrorHi(real_mean) << std::endl;
    std::cout << "Error Lo  " << ErrorLo(real_mean) << std::endl;
    std::cout << "Error Sym " << (ErrorHi(real_mean) + ErrorLo(real_mean))/2 << std::endl;
  }
}

}