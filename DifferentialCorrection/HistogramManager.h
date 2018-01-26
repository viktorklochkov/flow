//
// Created by Lukas Kreis on 24.01.18.
//

#ifndef FLOW_HISTOGRAMMANAGER_H
#define FLOW_HISTOGRAMMANAGER_H

#include <TList.h>
#include <Base/Axis.h>
#include <TH1F.h>
#include <TH2F.h>
#include <array>
#include <map>
#include "VariableManager.h"

namespace Qn {

struct HistogramHolder {
  enum class Type {
    Dim1,
    Dim2
  };

  Type type;
  std::vector<std::string> var_names;
  std::vector<std::vector<float>> binning;
};

class HistogramManager {

 public:

  HistogramManager(std::shared_ptr<VariableManager> vm) : histograms_(new TList()), var_manager_(vm) {}

  void AddHist1D(const std::string &x, std::vector<float> bin_edges) {
    histos_.push_back({HistogramHolder::Type::Dim1, {x}, {bin_edges}});
  }

  void CreateHistograms() {
    for (const auto &histo : histos_) {
      if (histo.type==HistogramHolder::Type::Dim1) {
        CreateHist1D(histo);
      }
    }
    for (const auto &histo : histos_) {
      if (histo.type==HistogramHolder::Type::Dim2) {
        CreateHist2D(histo);
      }
    }
  }

  void CreateHist1D(const HistogramHolder &a) {
    int kMaxNbin = 1000;
    float bin_edges_array[kMaxNbin];
    int ibin = 0;
    for (auto bin : a.binning.at(0)) {
      bin_edges_array[ibin] = bin;
      ibin++;
    }
    auto histogram =
        new TH1F((a.var_names.at(0)).data(),
                 ("; " + a.var_names.at(0) + "; ").data(),
                 static_cast<int>(a.binning.at(0).size() - 1),
                 bin_edges_array);
    histograms_->Add(histogram);
    auto value = var_manager_->FindNum(a.var_names.at(0));
    hist1d_fill_.push_back(value);
  }

  void CreateHist2D(const HistogramHolder &a) {
    auto x_edges = a.binning.at(0);
    auto y_edges = a.binning.at(1);
    auto x = a.var_names.at(0);
    auto y = a.var_names.at(1);
    int kMaxNbin = 1000;
    float x_edges_array[kMaxNbin];
    float y_edges_array[kMaxNbin];
    int ibin = 0;
    for (auto bin : x_edges) {
      x_edges_array[ibin] = bin;
      ibin++;
    }
    ibin = 0;
    for (auto bin : y_edges) {
      y_edges_array[ibin] = bin;
      ibin++;
    }
    auto histogram =
        new TH2F((x + "_" + y).data(),
                 ("; " + x + "; " + y).data(),
                 static_cast<int>(x_edges.size() - 1),
                 x_edges_array,
                 static_cast<int>(y_edges.size() - 1),
                 y_edges_array);
    histograms_->Add(histogram);
    auto xx = var_manager_->FindNum(x);
    auto yy = var_manager_->FindNum(y);
    hist2d_fill_.push_back({{xx, yy}});
  }
  void AddHist2D(const std::string &x, std::vector<float> x_edges, const std::string &y, std::vector<float> y_edges) {
    histos_.push_back({HistogramHolder::Type::Dim2, {x, y}, {x_edges, y_edges}});
  }

  void FillHist1D(float *values) {
    int i = 0;
    for (const auto &hist : hist1d_fill_) {
      auto histogram = (TH1F*) histograms_->At(i);
      histogram->Fill(values[hist]);
      i++;
    }
    for (const auto &hist : hist2d_fill_) {
      auto histogram = (TH2F*) histograms_->At(i);
      histogram->Fill(values[hist[0]],values[hist[1]]);
      i++;
    }
  }

  std::unique_ptr<TList> &GetList() { return histograms_; }

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

  std::vector<HistogramHolder> histos_;
  std::vector<int> hist1d_fill_;
  std::vector<std::array<int, 2>> hist2d_fill_;
  std::unique_ptr<TList> histograms_;
  std::shared_ptr<VariableManager> var_manager_;
};
}
#endif //FLOW_HISTOGRAMMANAGER_H
