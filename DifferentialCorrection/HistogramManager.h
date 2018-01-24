//
// Created by Lukas Kreis on 24.01.18.
//

#ifndef FLOW_HISTOGRAMMANAGER_H
#define FLOW_HISTOGRAMMANAGER_H

#include <TList.h>
#include <Base/Axis.h>
#include <TH1F.h>
#include <array>
#include <map>
#include "VariableManager.h"

namespace Qn {
class HistogramManager {

 public:

  HistogramManager(std::shared_ptr<VariableManager> vm) : histograms_(new TList()), var_manager_(vm) {}

  void AddHist1D(const std::string &x, int nbins, float *bins) {
    auto histogram = new TH1F((x).data(), ("; " + x + "; ").data(), nbins, bins);
    histograms_->Add(histogram);
    auto value = var_manager_->FindNum(x);
    hist1d_fill_.push_back(value);
  }
  void FillHist1D(float *values) {
    int i = 0;
    for (const auto &hist : hist1d_fill_) {
      auto histogram = (TH1 *) histograms_->At(i);
      histogram->Fill(values[hist]);
      i++;
    }
  }

  std::unique_ptr<TList>& GetList() { return histograms_; }

 private:
  /**
 * Tokenize input string
 * @tparam ContainerT
 * @param str
 * @param tokens
 * @param delimiters
 * @param trimEmpty
 */
  template<class ContainerT>
  void tokenize(const std::string &str, ContainerT &tokens,
                const std::string &delimiters = " ", bool trimEmpty = false) {
    std::string::size_type pos, lastPos = 0, length = str.length();
    using value_type = typename ContainerT::value_type;
    using size_type  = typename ContainerT::size_type;
    while (lastPos < length + 1) {
      pos = str.find_first_of(delimiters, lastPos);
      if (pos==std::string::npos) {
        pos = length;
      }
      if (pos!=lastPos || !trimEmpty)
        tokens.push_back(value_type(str.data() + lastPos,
                                    (size_type) pos - lastPos));
      lastPos = pos + 1;
    }
  }

  std::vector<int> hist1d_fill_;
  std::unique_ptr<TList> histograms_;
  std::shared_ptr<VariableManager> var_manager_;
};
}
#endif //FLOW_HISTOGRAMMANAGER_H
