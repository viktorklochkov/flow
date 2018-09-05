//
// Created by Lukas Kreis on 29.08.18.
//

#ifndef FLOW_VARIABLECUTBASE_H
#define FLOW_VARIABLECUTBASE_H

#include <array>
#include <vector>
#include <iostream>
#include "VariableManager.h"
#include "QAHistogram.h"

namespace Qn {

struct VariableCutBase {
  virtual ~VariableCutBase() = default;
  virtual bool Check(int i) = 0;
  virtual int GetVariableLength() const = 0;
  virtual std::string Name() const = 0;
};

template<typename... T>
class VariableCutNDim : public VariableCutBase {
 public:
  VariableCutNDim<T...>(Variable const (&arr)[sizeof...(T)], std::function<bool(T...)> lambda)
      : lambda_(lambda) {
    int i = 0;
    for (auto a : arr) {
      variables_[i] = a;
      ++i;
    }
  }
  bool Check(int i) override {
    return CheckImpl(i, variables_, std::make_index_sequence<sizeof...(T)>{});
  }
  int GetVariableLength() const override { return variables_[0].length(); }
 private:
  template<typename ARR, std::size_t... I>
  bool CheckImpl(int i, const ARR &arr, std::index_sequence<I...>) {
    return lambda_(*(arr[I].begin() + i)...);
  }

  std::string Name() const override {
    std::string name;
    for (auto var : variables_) {
      name += var.Name();
    }
    return name;
  }
  std::array<Variable, sizeof...(T)> variables_;
  std::function<bool(T...)> lambda_;
};

namespace Details {
template<std::size_t>
using Type = double &;
template<std::size_t... Is, std::size_t N, typename FUNC>
auto CreateNDimCutImpl(std::index_sequence<Is...>, Variable const (&arr)[N], FUNC &&func) {
  auto pp = std::make_unique<VariableCutNDim<Type<Is>...>>(arr, std::forward<FUNC>(func));
  return pp;
}
}

template<std::size_t N, typename FUNC>
auto MakeUniqueNDimCut(Variable const (&arr)[N], FUNC &&func) {
  return Details::CreateNDimCutImpl(std::make_index_sequence<N>{}, arr, std::forward<FUNC>(func));
}

class Cuts {
 public:
  ~Cuts() { delete[] var_values_; }
  template<std::size_t N, typename FUNC>
  void AddCut(Variable const (&arr)[N], FUNC &&func) {
    cuts_.push_back(MakeUniqueNDimCut(arr, func));
  }
  void AddCut(std::unique_ptr<VariableCutBase> cut) {
    cuts_.push_back(std::move(cut));
  }
  bool CheckCuts(int i) {
    int icut = 1;
    if (cuts_.size()==0) return true;
    *((cut_weight_).begin() + i) = *((cut_weight_).begin() + i) + 1.0;
    bool passed = true;
    for (auto &cut : cuts_) {
      bool ipass = cut->Check(i) && passed;
      if (ipass) {
        ++*cut_weight_.at(i + nchannels_*icut);
      }
      passed = ipass;
      ++icut;
    }
    return passed;
  }

  void FillReport() {
    if (report_) report_->Fill();
    auto offset = nchannels_*(cuts_.size() + 1);
    for (std::size_t i = 0; i < nchannels_; ++i) {
      for (std::size_t j = 0; j < (cuts_.size() + 1); ++j) {
        var_values_[2*offset + i + nchannels_*j] = 0;
      }
    }
  }

  void CreateCutReport(std::string detname, int nchannels = 1) {
    if (!cuts_.empty()) {
      nchannels_ = nchannels;
      auto offset = nchannels*(cuts_.size() + 1);
      cut_i_ = Variable(0, offset);
      cut_channel_ = Variable(offset, offset);
      cut_weight_ = Variable(2*offset, offset);
      var_values_ = new double[3*offset];
      cut_weight_.var_container = var_values_;
      cut_i_.var_container = var_values_;
      cut_channel_.var_container = var_values_;
      for (std::size_t i = 0; i < nchannels; ++i) {
        for (std::size_t j = 0; j < (cuts_.size() + 1); ++j) {
          var_values_[i + nchannels*j] = j;
          var_values_[offset + i + nchannels*j] = i;
          var_values_[2*offset + i + nchannels*j] = 0;
        }
      }
      if (nchannels==1) {
        std::string name = detname + "Cut_Report";
        std::string title = std::string(";cuts;entries");
        auto nbins = cuts_.size() + 1;
        float low = 0.;
        float high = cuts_.size() + 1;
        TH1F histo(name.data(), title.data(), nbins, low, high);
        int icut = 2;
        histo.GetXaxis()->SetBinLabel(1, "all");
        for (auto &cut : cuts_) {
          histo.GetXaxis()->SetBinLabel(icut, cut->Name().data());
          ++icut;
        }
        std::array<Variable, 2> arr = {{cut_i_, cut_weight_}};
        report_ = std::make_unique<QAHisto1D>(arr, histo);
      } else {
        std::string name = detname + "Cut_Report";
        std::string title = std::string(";cuts;channels");
        int x_nbins = cuts_.size() + 1;
        int y_nbins = nchannels_;
        float low = 0.;
        float x_high = cuts_.size() + 1;
        float y_high = nchannels_;
        TH2F histo(name.data(), title.data(), x_nbins, low, x_high, y_nbins, low, y_high);
        histo.GetXaxis()->SetBinLabel(1, "all");
        int icut = 2;
        for (auto &cut : cuts_) {
          histo.GetXaxis()->SetBinLabel(icut, cut->Name().data());
          ++icut;
        }
        std::array<Variable, 3> arr = {{cut_i_, cut_channel_, cut_weight_}};
        report_ = std::make_unique<QAHisto2D>(arr, histo);
      }
    }
  }

  void Write(const std::string &name) {
    if (report_) report_->Write((name + std::string(report_->Name())).data());
    report_.release();
  }

 private:
  int nchannels_ = 0;
  double *var_values_ = nullptr;
  Variable cut_i_;
  Variable cut_weight_;
  Variable cut_channel_;
  std::vector<std::unique_ptr<VariableCutBase>> cuts_;
  std::unique_ptr<QAHistoBase> report_ = nullptr;

};

}

#endif //FLOW_VARIABLECUTBASE_H
