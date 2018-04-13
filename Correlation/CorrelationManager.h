//
// Created by Lukas Kreis on 15.12.17.
//

#ifndef FLOW_CORRELATIONMANAGER_H
#define FLOW_CORRELATIONMANAGER_H

#include <TTreeReader.h>

#include <utility>
#include <Base/ResultContainer.h>
#include "Correlation.h"
#include "BootstrapSampler.h"
#include "CorrelationHolder.h"

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

  ResultContainer GetResult(const std::string &name) const {
    return *(correlations_.at(name).GetResult());
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
    BuildCorrelation();
  }

  void BuildCorrelation() {
    int nevents = reader_->GetEntries(true);
    std::vector<Qn::DataContainerQVector> qvectors;
    for (auto &corr : correlations_) {
      qvectors.reserve(corr.second.GetInputNames().size());
      for (auto &cname : corr.second.GetInputNames()) {
        qvectors.push_back(qvectors_.at(cname));
      }
      corr.second.Initialize(nevents, qvectors, event_axes_);
    }
  }

  void AddCorrelation(std::string name,
                      const std::string &containernames,
                      FUNCTION &&lambda,
                      int nsamples,
                      BootstrapSampler::Method method) {
    std::vector<std::string> containernamelist;
    tokenize(containernames, containernamelist, ", ", true);
    Qn::CorrelationHolder holder(name, containernamelist, lambda, nsamples, method);
    correlations_.emplace(name, std::move(holder));
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
    int i = 0;
    for (auto &value : tree_event_values_) {
      event_values_.at(i) = *value.second.Get();
      i++;
    }
  }

  bool CheckEvent() {
    u_long ie = 0;
    for (const auto &ae : event_axes_) {
      eventbin_.at(ie) = (ae.FindBin(event_values_[ie]));
      ie++;
    }
    for (auto bin : eventbin_) {
      if (bin==-1) return false;
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
  std::map<std::string, Qn::CorrelationHolder> correlations_;
  std::map<std::string, TTreeReaderValue<Qn::DataContainerQVector>> tree_values_;
  std::map<std::string, TTreeReaderValue<float>> tree_event_values_;
  std::map<std::string, Qn::DataContainerQVector> qvectors_;
  std::vector<float> event_values_;
  std::vector<long> eventbin_;
  std::vector<Qn::Axis> event_axes_;

};
}

#endif //FLOW_CORRELATIONMANAGER_H
