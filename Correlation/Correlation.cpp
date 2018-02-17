//
// Created by Lukas Kreis on 03.11.17.
//

#include "Correlation.h"

namespace Qn {

void Correlation::FillCorrelation(const std::vector<long> &eventindex,
                                  std::vector<QVector> &contents,
                                  int ipos,
                                  u_int iteration,
                                  std::vector<long> &cindex,
                                  const std::vector<Correlation::CONTAINERS> &input) {
  const auto &datacontainer = input[iteration];
  ipos += iteration;
  if (iteration + 1==input.size()) {
    int ibin = 0;
    for (auto &bin : datacontainer) {
      if (bin.n()==0) continue;
      if (!datacontainer.IsIntegrated()) {
        int ii = 0;
        for (auto i : index_[iteration][ibin]) {
          int pos = ii + ipos;
          cindex[pos] = i;
          ++ii;
        }
      }
      contents[iteration] = bin;
      data_correlation_.At(cindex) = function_(contents);
      ++ibin;
    }
    return;
  }
  int ibin = 0;
  for (const auto &bin : datacontainer) {
    int pos = ipos;
    if (bin.n()==0) continue;
    if (!datacontainer.IsIntegrated()) {
      int ii = 0;
      for (auto i : index_[iteration][ibin]) {
        pos += ii;
        cindex[pos] = i;
        ++ii;
      }
    }
    contents[iteration] = bin;
    FillCorrelation(eventindex, contents, pos, iteration + 1, cindex, input);
    ++ibin;
  }
}

void Correlation::Fill(const std::vector<Correlation::CONTAINERS> &input, const std::vector<long> &eventindex) {
  std::vector<QVector> contents;
  contents.resize(input.size());
  uint iteration = 0;
  std::vector<long> cindex;
  int size = eventindex.size();
  for (const auto &i : input) {
    size += i.GetAxes().size();
  }
  cindex.resize(size);
  int ii = 0;
  for (auto eventind : eventindex) {
    cindex[ii] = eventind;
    ++ii;
  }
  FillCorrelation(eventindex, contents, ii, iteration, cindex, input);
}

void Correlation::CreateCorrelationContainer(const std::vector<Correlation::CONTAINERS> &inputs) {
  int i = 0;
  data_correlation_.AddAxes(axes_event_);
  for (auto &input : inputs) {
    std::vector<std::vector<long>> indexmap;
    for (int i = 0; i < input.size(); ++i) {
      std::vector<long> indices;
      input.GetIndex(indices,i);
      indexmap.push_back(indices);
    }
    index_.push_back(indexmap);
    if (!input.IsIntegrated()) {
      auto axes = input.GetAxes();
      for (auto &axis : axes) {
        axis.SetName(std::to_string(i) + axis.Name());
      }
      data_correlation_.AddAxes(axes);
      ++i;
    }
  }
  c_index_.reserve(i);
}

}