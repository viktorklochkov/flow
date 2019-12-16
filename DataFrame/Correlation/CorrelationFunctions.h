// Flow Vector Correction Framework
//
// Copyright (C) 2019  Lukas Kreis Ilya Selyuzhenkov
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
#ifndef FLOW_CORRELATIONFUNCTIONS_H_
#define FLOW_CORRELATIONFUNCTIONS_H_

namespace Qn {
namespace Correlation {
/**
 * Two particle correlation functions
 */
namespace TwoParticle {
inline auto xx(unsigned int h_a, unsigned int h_b) {
  return [h_a, h_b](const Qn::QVector &a, const Qn::QVector &b) { return a.x(h_a)*b.x(h_b); };
}
inline auto yy(unsigned int h_a, unsigned int h_b) {
  return [h_a, h_b](const Qn::QVector &a, const Qn::QVector &b) { return a.y(h_a)*b.y(h_b); };
}
inline auto yx(unsigned int h_a, unsigned int h_b) {
  return [h_a, h_b](const Qn::QVector &a, const Qn::QVector &b) { return a.y(h_a)*b.x(h_b); };
}
inline auto xy(unsigned int h_a, unsigned int h_b) {
  return [h_a, h_b](const Qn::QVector &a, const Qn::QVector &b) { return a.x(h_a)*b.y(h_b); };
}
inline auto ScalarProduct(unsigned int h_u, unsigned int h_Q) {
  return [h_u, h_Q](const Qn::QVector &u, const Qn::QVector &Q) { return u.x(h_u)*Q.x(h_Q) + u.y(h_u)*Q.y(h_Q); };
}
inline auto Cumulant(const Qn::QVector &u, unsigned int h_u) {
  return [h_u](const Qn::QVector &u) {
    auto m = u.sumweights();
    auto Q_mag = u.DeNormal().mag(h_u);
    return (Q_mag*Q_mag - m)/(m*(m - 1));
  };
}
} /// TwoParticle

/**
 * Four particle correlation functions
 */
namespace FourParticle {
inline auto Cumulant(unsigned int h_u) {
  return [h_u](const Qn::QVector &u) {
    auto Q = u.DeNormal();
    auto M = u.sumweights();
    auto x = Q.x(h_u);
    auto y = Q.y(h_u);
    auto x2 = Q.x(2*h_u);
    auto y2 = Q.y(2*h_u);
    auto Q_mag = std::sqrt(x*x + y*y);
    auto Q_2n_mag = std::sqrt(x2*x2 + y2*y2);
    auto real = x2*x*x - x2*y*y + y2*y*x + y2*x*y;
    auto term_1 = (Q_mag*Q_mag*Q_mag*Q_mag + Q_2n_mag*Q_2n_mag - 2*real)/(M*(M - 1)*(M - 2)*(M - 3));
    auto term_2 = (2*(M - 2)*Q_mag*Q_mag - M*(M - 3))/(M*(M - 1)*(M - 2)*(M - 3));
    return term_1 - 2*term_2;
  };
}
} /// FourParticle

}
}
#endif //FLOW_DATAFRAME_CORRELATION_CORRELATIONFUNCTIONS_H_
