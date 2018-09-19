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

#include "Axis.h"
ClassImp(Qn::Axis);

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
