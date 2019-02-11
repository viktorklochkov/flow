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

namespace Qn {

struct Product {
  Product() = default;
  Product(long long ent, double res, bool val) : entries(ent), result(res), validity(val) {}
  long long entries;
  double result;
  bool validity;

  inline Product Sqrt() const {
    Product a(*this);
    a.result = std::sqrt(a.result);
    return a;
  }
};

inline void SetToZero(Qn::Product &a) {
  a.result = 0;
  a.entries = 0;
  a.validity = false;
}

inline Qn::Product operator*(Qn::Product a, double b) {
  Product c;
  c.entries = a.entries;
  c.result = a.result * b;
  c.validity = a.validity;
  return c;
}

inline Qn::Product operator+(Qn::Product a, Qn::Product b) {
  Product c;
  c.entries = a.entries + b.entries;
  c.result = a.result + b.result;
  c.validity = a.validity && b.validity;
  return c;
}

inline Qn::Product Merge(Qn::Product a, Qn::Product b) {
  Product c;
  c.entries = a.entries + b.entries;
  c.result = a.result + b.result;
  c.validity = a.validity && b.validity;
  return c;
}

inline Qn::Product operator-(Qn::Product a, Qn::Product b) {
  Product c;
  c.entries = a.entries + b.entries;
  c.result = a.result - b.result;
  c.validity = a.validity && b.validity;
  return c;
}

inline Qn::Product operator*(Qn::Product a, Qn::Product b) {
  Product c;
  c.entries = a.entries + b.entries;
  c.result = a.result * b.result;
  c.validity = a.validity && b.validity;
  return c;
}

inline Qn::Product operator/(Qn::Product a, Qn::Product b) {
  Product c;
  c.entries = a.entries + b.entries;
  c.result = a.result / b.result;
  c.validity = a.validity && b.validity;
  return c;
}

}

#endif