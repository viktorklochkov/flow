//
// Created by Lukas Kreis on 23.02.18.
//

#ifndef FLOW_RESULTCONTAINER_H
#define FLOW_RESULTCONTAINER_H

#include <utility>

#include "DataContainer.h"

namespace Qn {

class ResultContainer : public TObject {
 public:
  ResultContainer(std::string name, const std::vector<Axis> &axes, int nsamples) :
      name_(std::move(name)),
      correlation_(axes),
      correlated_error_(axes),
      errorcalc_(false),
      corr_errors_(false) {
    subsamples_.resize(nsamples);
    for (auto &sample : subsamples_) {
      sample.AddAxes(axes);
    }
//    if (nsamples > 0) corr_errors_ = true;
  }
  ResultContainer(std::string name, int nsamples) :
      name_(std::move(name)),
      correlation_(),
      correlated_error_(),
      errorcalc_(false),
      corr_errors_(false) {
    subsamples_.resize(nsamples);
//    if (nsamples > 0) corr_errors_ = true;
  }
  virtual ~ResultContainer() = default;

  void UseCorrelatedErrors(bool errors) { corr_errors_ = errors; }

  friend ResultContainer operator+(const ResultContainer &a, const ResultContainer &b);
  friend ResultContainer operator-(const ResultContainer &a, const ResultContainer &b);
  friend ResultContainer operator*(const ResultContainer &a, const ResultContainer &b);
  friend ResultContainer operator/(const ResultContainer &a, const ResultContainer &b);
  friend ResultContainer operator*(const ResultContainer &a, double b);
  friend ResultContainer Sqrt(const ResultContainer &a);

  Profile At(int i) {
    if (!errorcalc_) RecalculateError();
    if (corr_errors_) return correlated_error_.At(i);
    return correlation_.At(i);
  }

  Profile At(const std::vector<long> &bin) {
    if (!errorcalc_) RecalculateError();
    if (corr_errors_) return correlated_error_.At(bin);
    return correlation_.At(bin);
  }

  void RecalculateError() {
    int ibin = 0;
    correlated_error_.ClearData();
    for (const auto &sample : subsamples_) {
      ibin = 0;
      for (const auto &bin : sample) {
        correlated_error_.At(ibin).Update(bin.Mean());
        ++ibin;
      }
    }
    ibin = 0;
    for (auto &bin : correlated_error_) {
      bin = Profile(correlation_.At(ibin).Mean(),
                    correlation_.At(ibin).Sum(),
                    correlation_.At(ibin).Sum2(),
                    bin.Error(),
                    correlation_.At(ibin).Entries());
      ++ibin;
    }
    errorcalc_ = true;
  }

  void Fill(int ibin, double value, std::vector<u_long> isamples) {
    correlation_.At(ibin).Update(value);
    for (auto &isample : isamples) {
      subsamples_.at(isample).At(ibin).Update(value);
    }
  }

  void Fill(double value, std::vector<u_long> isamples) {
    correlation_.At(0).Update(value);
    for (auto &isample : isamples) {
      subsamples_.at(isample).At(0).Update(value);
    }
  }

  using iterator = typename std::vector<Profile>::iterator;
  using const_iterator = typename std::vector<Profile>::const_iterator;
  const_iterator begin() const {
    if (!corr_errors_) return correlation_.begin();
    return correlated_error_.begin();
  } ///< iterator for external use
  const_iterator end() const {
    if (!corr_errors_) return correlation_.end();
    return correlated_error_.end();
  } ///< iterator for external use
  iterator begin() {
    if (!corr_errors_) return correlation_.begin();
    return correlated_error_.begin();
  } ///< iterator for external use
  iterator end() {
    if (!corr_errors_) return correlation_.end();
    return correlated_error_.end();
  } ///< iterator for external use

 private:
  std::string name_;
  DataContainerProfile correlation_;
  DataContainerProfile correlated_error_;
  std::vector<DataContainerProfile> subsamples_;
  bool errorcalc_;
  bool corr_errors_;

  /// \cond CLASSIMP
 ClassDef(ResultContainer, 1);
  /// \endcond
};

inline ResultContainer operator+(const ResultContainer &a, const ResultContainer &b) {
  ResultContainer c(a);
  c.correlation_.Apply(b.correlation_, [](const Profile &a, const Profile &b) { return a + b; });
  for (auto i = 0; i < a.subsamples_.size(); ++i) {
    c.subsamples_[i].Apply(b.subsamples_[i], [](const Profile &a, const Profile &b) { return a + b; });
  }
  c.errorcalc_ = false;
  return c;
}
inline ResultContainer operator-(const ResultContainer &a, const ResultContainer &b) {
  ResultContainer c(a);
  c.correlation_.Apply(b.correlation_, [](const Profile &a, const Profile &b) { return a - b; });
  for (auto i = 0; i < a.subsamples_.size(); ++i) {
    c.subsamples_[i].Apply(b.subsamples_[i], [](const Profile &a, const Profile &b) { return a - b; });
  }
  c.errorcalc_ = false;
  return c;
}
inline ResultContainer operator*(const ResultContainer &a, const ResultContainer &b) {
  ResultContainer c(a);
  c.correlation_.Apply(b.correlation_, [](const Profile &a, const Profile &b) { return a*b; });
  for (auto i = 0; i < a.subsamples_.size(); ++i) {
    c.subsamples_[i].Apply(b.subsamples_[i], [](const Profile &a, const Profile &b) { return a*b; });
  }
  c.errorcalc_ = false;
  return c;
}
inline ResultContainer operator/(const ResultContainer &a, const ResultContainer &b) {
  ResultContainer c(a);
  c.correlation_.Apply(b.correlation_, [](const Profile &a, const Profile &b) { return a/b; });
  for (auto i = 0; i < a.subsamples_.size(); ++i) {
    c.subsamples_[i].Apply(b.subsamples_[i], [](const Profile &a, const Profile &b) { return a/b; });
  }
  c.errorcalc_ = false;
  return c;
}
inline ResultContainer operator*(const ResultContainer &a, double b) {
  ResultContainer c(a);
  c.correlation_.Map([b](Profile &a) { return a*b; });
  for (auto i = 0; i < a.subsamples_.size(); ++i) {
    c.subsamples_[i].Map([b](Profile &a) { return a*b; });
  }
  c.errorcalc_ = false;
  return c;
}

inline ResultContainer Sqrt(const ResultContainer &a) {
  ResultContainer c(a);
  c.correlation_.Map([](Profile &a) { return a.Sqrt(); });
  for (auto i = 0; i < a.subsamples_.size(); ++i) {
    c.subsamples_[i].Map([](Profile &a) { return a.Sqrt(); });
  }
  c.errorcalc_ = false;
  return c;
}

}

#endif //FLOW_RESULTCONTAINER_H
