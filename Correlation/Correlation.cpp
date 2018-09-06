//
// Created by Lukas Kreis on 03.11.17.
//

#include "Correlation.h"

namespace Qn {

void Correlation::FillCorrelation(const std::vector<unsigned long> &eventindex,
                                  std::vector<QVector> &contents,
                                  int iterationoffset,
                                  u_int iteration,
                                  const std::vector<Correlation::CONTAINERS> &input) {
  const auto &datacontainer = input[iteration];
  iterationoffset += iteration;
  if (iteration + 1==input.size()) {
    int ibin = 0;
    for (auto &bin : datacontainer) {
      if (!datacontainer.IsIntegrated()) {
        int i_index = 0;
        for (auto index : index_[iteration][ibin]) {
          int pos = i_index + iterationoffset;
          c_index_[pos] = index;
          ++i_index;
        }
      }
      contents[iteration] = bin;
      auto is_n = std::all_of(contents.begin(),contents.end(),[](QVector q) {return q.n() > 0;});
      data_correlation_.At(c_index_).first = is_n;
      data_correlation_.At(c_index_).second = function_(contents);
      ++ibin;
    }
    return;
  }
  int ibin = 0;
  for (const auto &bin : datacontainer) {
    int offset = iterationoffset;
    if (!datacontainer.IsIntegrated()) {
      int i_index = 0;
      for (auto index : index_[iteration][ibin]) {
        offset += i_index;
        c_index_[offset] = index;
        ++i_index;
      }
    }
    contents[iteration] = bin;
    FillCorrelation(eventindex, contents, offset, iteration + 1, input);
    ++ibin;
  }
}

void Correlation::Fill(const std::vector<Correlation::CONTAINERS> &input, const std::vector<unsigned long> &eventindex) {
  data_correlation_.ClearData();
  std::vector<QVector> contents;
  contents.resize(input.size());
  uint iteration = 0;
  auto size = eventindex.size();
  for (const auto &i : input) {
    size += i.GetAxes().size();
  }
  int ii = 0;
  for (auto eventind : eventindex) {
    c_index_[ii] = eventind;
    ++ii;
  }
  FillCorrelation(eventindex, contents, ii, iteration, input);
}

void Correlation::CreateCorrelationContainer(const std::vector<Correlation::CONTAINERS> &inputs) {
  int i = 0;
  data_correlation_.AddAxes(axes_event_);
  auto size = axes_event_.size();
  for (auto &input : inputs) {
    size += input.GetAxes().size();
    std::vector<std::vector<unsigned long>> indexmap;
    for (std::size_t j = 0; j < input.size(); ++j) {
      std::vector<unsigned long> indices;
      input.GetIndex(indices, j);
      indexmap.push_back(indices);
    }
    index_.push_back(indexmap);
    if (!input.IsIntegrated()) {
      auto axes = input.GetAxes();
      for (auto &axis : axes) {
        auto original_name = axis.Name();
        if (!names_.empty()) {axis.SetName(std::to_string(i) + "_" + names_[i] + "_" + original_name);}
        else {axis.SetName(std::to_string(i) + "_" + original_name);}
        data_correlation_.AddAxis(axis);
      }
      ++i;
    }
  }
  c_index_.resize(size);
}

}