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

#ifndef FLOW_PRODUCT_H
#define FLOW_PRODUCT_H

#include <cmath>
#include <vector>
#include <numeric>

#include "Rtypes.h"

namespace Qn {

struct Product {
  Product() = default;

  Product(std::vector<double> vec, double res, bool val) :
      result(res),
      validity(val),
      w_vect(vec) {}

  virtual ~Product() = default;

  double result = 0.;                   ///!<! value of the product
  bool validity = false;                ///!<! flag to show if product is valid
  std::vector<double> w_vect = {1.};    ///!<! vector of the weights

  inline Product Sqrt() const {
    Product a(*this);
    a.result = std::sqrt(a.result);
    return a;
  }

  double GetWeight() const {return std::accumulate(w_vect.begin(), w_vect.end(),1.,std::multiplies<double>());}

  void SetDim(size_t dim) { w_vect.resize(dim); }
  size_t GetDim() const { return w_vect.size(); }
};

inline Qn::Product MergeBins(Qn::Product a, Qn::Product b) {
  Product c;
  int i = 0;
  c.SetDim(a.GetDim());
  if (a.GetDim()==b.GetDim()) {
    for (auto &weight : c.w_vect) {
      weight = a.w_vect[i] + b.w_vect[i];
      ++i;
    }
  }
  c.result = a.result + b.result;
  c.validity = a.validity && b.validity;
  return c;
}

}

#endif