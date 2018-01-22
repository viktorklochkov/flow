//
// Created by Lukas Kreis on 29.06.17.
//

#include "Axis.h"
ClassImp(Qn::Axis);

long Qn::Axis::FindBin(const float value) const {
  long bin = 0;
  if (value < *bin_edges_.begin()) {
    bin = -1;
  } else {
    auto lb = std::lower_bound(bin_edges_.begin(), bin_edges_.end(), value);
    if (lb == bin_edges_.begin() || *lb == value)
      bin = (lb - bin_edges_.begin());
    else
      bin = (lb - bin_edges_.begin()) - 1;
  }
  if (bin >= (long) bin_edges_.size() - 1 || bin < 0)
    bin = -1;
  return bin;
}

Qn::Axis::citerator Qn::Axis::FindBinIter(const float value) {
  citerator bin;
  if (value < *bin_edges_.begin()) {
    bin = end();
  } else {
    auto lb = std::lower_bound(bin_edges_.begin(), bin_edges_.end(), value);
    if (lb == bin_edges_.begin() || *lb == value)
      bin = lb;
    else
      bin = lb - 1;
  }
  if (*bin >= (long) bin_edges_.size() - 1 || *bin < 0) bin = end();
  return bin;
}
