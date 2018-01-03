//
// Created by Lukas Kreis on 03.11.17.
//

#include "Correlation.h"

namespace Qn {

void Correlation::FillCorrelation(const std::vector<long> &eventindex,
                                  std::vector<std::vector<long>> &index,
                                  std::vector<QVector> &contents,
                                  u_int iteration,
                                  std::vector<long> &cindex) {
  auto &datacontainer = *(inputs_.begin() + iteration);
  if (iteration + 1 == inputs_.size()) {
    int ibin = 0;
    for (auto &bin : datacontainer) {
      auto binindex = datacontainer.GetIndex(ibin);
      if (!datacontainer.IsIntegrated()) index.push_back(binindex);
      std::for_each(std::begin(index), std::end(index), [&cindex](const std::vector<long> &element) {
        for (const auto &a : element) { cindex.push_back(a); }
      });
      contents.at(iteration) = bin;
      data_correlation_.CallOnElement(cindex,
                                      [this, &contents](Qn::Statistics &a) -> void {
                                        if (std::all_of(contents.begin(), contents.end(), [](QVector qv) { return qv.n() != 0; })) {
                                          a.Update(function_(contents));
                                        }
                                      });
      if (!datacontainer.IsIntegrated()) index.erase(index.end() - 1);
      ++ibin;
      cindex.clear();
    }
    index.clear();
    return;
  }
  int ibin = 0;
  for (auto &bin : datacontainer) {
    index.push_back(eventindex);
    auto binindex = datacontainer.GetIndex(ibin);
    if (!datacontainer.IsIntegrated()) index.push_back(binindex);
    contents.at(iteration) = bin;
    FillCorrelation(eventindex, index, contents, iteration + 1, cindex);
    ++ibin;
  }
}
void Correlation::Fill(const std::vector<Correlation::CONTAINERS> &input, const std::vector<long> &eventindex) {
  std::vector<std::vector<long>> index;
  std::vector<QVector> contents;
  contents.resize(input.size());
  inputs_ = input;
  uint iteration = 0;
  std::vector<long> cindex;
  cindex.reserve(10);
  index.reserve(10);
  FillCorrelation(eventindex, index, contents, iteration, cindex);
}
}