//
// Created by Lukas Kreis on 12.07.18.
//

#ifndef FLOW_HIST_H
#define FLOW_HIST_H

#include <vector>
#include "Axis.h"
namespace Qn {
class Hist {
 public:
  Hist(Qn::Axis axis) :
      axis_(axis) {
    data_.resize(axis_.size() + 1);
  }

  void Fill(float value, float weight = 1) {
    auto bin = axis_.FindBin(value);
    data_.at(bin + 1) += weight;
  }

  float GetBinValue(int bin) {
    return data_.at(bin);
  }

  long FindBin(float value) {
    return axis_.FindBin(value);
  }

  using iterator = typename std::vector<float>::iterator;
  using const_iterator = typename std::vector<float>::const_iterator;
  const_iterator begin() const { return data_.cbegin() + 1; } ///< iterator for external use
  const_iterator end() const { return data_.cend(); } ///< iterator for external use
  iterator begin() { return data_.begin() + 1; } ///< iterator for external use
  iterator end() { return data_.end(); } ///< iterator for external use

 private:
  std::vector<float> data_;
  Qn::Axis axis_;
};
}

#endif //FLOW_HIST_H
