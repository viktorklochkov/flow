//
// Created by Lukas Kreis on 15.12.17.
//

#ifndef FLOW_CORRELATIONMANAGER_H
#define FLOW_CORRELATIONMANAGER_H

#include <TTreeReader.h>

#include <utility>
#include "Correlation.h"
#include "Sampler.h"
#include "Correlator.h"

namespace Qn {

class CorrelationManager {
  using FUNCTION = std::function<double(std::vector<Qn::QVector> &)>;
  using DATAFUNCTION = std::function<Qn::DataContainerQVector(Qn::DataContainerQVector &)>;
  using PROJECTION = std::function<std::vector<Qn::QVector>(std::vector<Qn::QVector> &)>;
 public:
  explicit CorrelationManager(std::shared_ptr<TTreeReader> reader) :
      reader_(std::move(reader)) {

  }
  /**
   * Adds a new DataContainer to the correlation manager.
   * Actual value is retrieved when the tree is read from the file.
   * @param name
   */
  void AddDataContainer(const std::string &name) {
    TTreeReaderValue<Qn::DataContainerQVector>
        value(*reader_, name.data());
    tree_values_.emplace(name, value);
    DataContainerQVector a;
    qvectors_.emplace(name, a);
  }

  void AddProjection(const std::string &input, const std::string &name, const std::string axesstring) {
    auto toproject = qvectors_.at(input);
    DataContainerQVector projection;
    std::vector<std::string> axisnames;
    tokenize(axesstring, axisnames, ", ", true);
    projections_.emplace(name, std::make_tuple(input, axisnames));
    qvectors_.emplace(name, projection);
  }

  void MakeProjections() {
    for (const auto &projection : projections_) {
      qvectors_.at(projection.first) =
          qvectors_.at(std::get<0>(projection.second)).Projection(std::get<1>(projection.second),
                                                                  [](const Qn::QVector &a,
                                                                     const Qn::QVector &b) {
                                                                    return (a
                                                                        + b).Normal(Qn::QVector::Normalization::QOVERM);
                                                                  });
    }
  }

  /**
   * Adds a new event variable to the correlation manager.
   * Actual value is retrieved when the tree is read from the file.
   * @param eventaxis Event variable defined as a Axis, which is used in the correlations.
   */
  void AddEventVariable(const Qn::Axis &eventaxis) {
    TTreeReaderValue<float> value(*reader_, eventaxis.Name().data());
    tree_event_values_.emplace(eventaxis.Name(), value);
    event_values_.emplace_back(-999);
    eventbin_.emplace_back(-1);
    event_axes_.push_back(eventaxis);
  }

  DataContainerSample GetResult(const std::string &name) const {
    return correlations_.at(name).GetResult();
  }

  void SaveToFile(std::string name);

  void Initialize() {
    for (auto &value : tree_values_) {
      qvectors_.at(value.first) = *value.second.Get();
    }
    int i = 0;
    for (auto &value : tree_event_values_) {
      event_values_.at(i) = *value.second.Get();
      i++;
    }
    MakeProjections();
    BuildCorrelations();
  }

  void BuildCorrelations() {
    auto nevents = reader_->GetEntries(true);
    std::vector<Qn::DataContainerQVector> qvectors;
    for (auto &corr : correlations_) {
      qvectors.reserve(corr.second.GetInputNames().size());
      for (const auto &cname : corr.second.GetInputNames()) {
        qvectors.push_back(qvectors_.at(cname));
      }
      corr.second.ConfigureCorrelation(qvectors, event_axes_);
      corr.second.BuildSamples(nevents);
    }
  }

  void AddCorrelation(std::string name,
                      const std::string &containernames,
                      FUNCTION &&lambda,
                      int nsamples,
                      Sampler::Method method) {
    std::vector<std::string> containernamelist;
    tokenize(containernames, containernamelist, ", ", true);
    Qn::Correlator correlator(containernamelist, lambda);
    correlator.ConfigureSampler(method, nsamples);
    correlations_.emplace(name, std::move(correlator));
  }

  void FillCorrelations() {
    for (auto &pair : correlations_) {
      u_long i = 0;
      std::vector<DataContainerQVector> inputs;
      inputs.resize(pair.second.GetInputNames().size());
      for (const auto &name : pair.second.GetInputNames()) {
        inputs.at(i) = qvectors_.at(name);
        ++i;
      }
      pair.second.FillCorrelation(inputs, eventbin_, reader_->GetCurrentEntry());
    }
  }

  void UpdateEvent() {
    for (auto &value : tree_values_) {
      qvectors_.at(value.first) = *value.second.Get();
    }
    unsigned long i = 0;
    for (auto &value : tree_event_values_) {
      event_values_.at(i) = *value.second.Get();
      i++;
    }
    MakeProjections();
  }

  bool CheckEvent() {
    u_long ie = 0;
    for (const auto &ae : event_axes_) {
      auto bin = ae.FindBin(event_values_[ie]);
      if (bin!=-1) {
        eventbin_.at(ie) = (unsigned long) bin;
      } else {
        return false;
      }
      ie++;
    }
    return true;
  }

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

 private:
  std::shared_ptr<TTreeReader> reader_;
  std::map<std::string, Qn::Correlator> correlations_;
  std::map<std::string, std::tuple<std::string, std::vector<std::string>>> projections_;
  std::map<std::string, TTreeReaderValue<Qn::DataContainerQVector>> tree_values_;
  std::map<std::string, TTreeReaderValue<float>> tree_event_values_;
  std::map<std::string, Qn::DataContainerQVector> qvectors_;
  std::vector<float> event_values_;
  std::vector<unsigned long> eventbin_;
  std::vector<Qn::Axis> event_axes_;

};
}

#endif //FLOW_CORRELATIONMANAGER_H
