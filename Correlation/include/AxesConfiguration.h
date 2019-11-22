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
#ifndef FLOW_DATAFRAMECORRELATION_INCLUDE_DATAFRAMEAXISCONFIGURATION_H_
#define FLOW_DATAFRAMECORRELATION_INCLUDE_DATAFRAMEAXISCONFIGURATION_H_

#include "TemplateHelpers.h"
namespace Qn {
namespace Correlation {

namespace Impl {
template<std::size_t position>
struct StrideCalculator {
  template<typename Stride, typename Axes>
  static void Calculate(Stride &stride, const Axes &axes) {
    stride[position] = stride[position + 1]*std::get<position>(axes).size();
    StrideCalculator<position - 1>::Calculate(stride, axes);
  }
};
template<>
struct StrideCalculator<0> {
  template<typename Stride, typename Axes>
  static void Calculate(Stride &stride, const Axes &axes) {
    stride[0] = stride[1]*std::get<0>(axes).size();
  }
};
}

template<typename ...Axes>
class AxesConfiguration {
 public:
  static constexpr auto kDimension = sizeof...(Axes);
  AxesConfiguration(Axes... axes) : axes_(axes...) {
    CalculateStride();
  }
  using AxisType = typename std::tuple_element<0, std::tuple<Axes...>>::type;
  using AxisValueTypeTuple = typename TemplateHelpers::TupleOf<sizeof...(Axes), typename AxisType::ValueType>;
  std::vector<AxisType> GetVector() const { return TemplateHelpers::ToVector(axes_); }

  template<typename... Coordinates>
  long GetLinearIndexFromCoordinates(Coordinates... coordinates) const {
    return FindBin(coordinates...);
  }

 private:

  template<typename FirstCoordinate, typename... Rest>
  long FindBin(FirstCoordinate first_coordinate, Rest... rest) const {
    constexpr std::size_t position = kDimension - sizeof...(rest) - 1;
    auto bin = stride_[position + 1]*std::get<position>(axes_).FindBin(first_coordinate);
    auto rest_bin = FindBin(rest...);
    if (rest_bin < 0) return -1;
    return bin + rest_bin;
  }

  template<typename FirstCoordinate>
  long FindBin(FirstCoordinate first_coordinate) const {
    constexpr std::size_t position = kDimension - 1;
    return stride_[position + 1]*std::get<position>(axes_).FindBin(first_coordinate);
  }

  void CalculateStride() {
    stride_[kDimension] = 1;
    Impl::StrideCalculator<kDimension - 1>::Calculate(stride_, axes_);
  }

 private:
  std::tuple<Axes...> axes_;
  std::array<std::size_t, kDimension + 1> stride_;
};

template<typename ...Axes>
auto MakeAxes(Axes... axes) {
  return AxesConfiguration<Axes...>(axes...);
}

}
}
#endif //FLOW_DATAFRAMECORRELATION_INCLUDE_DATAFRAMEAXISCONFIGURATION_H_
