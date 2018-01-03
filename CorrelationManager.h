//
// Created by Lukas Kreis on 15.12.17.
//

#ifndef FLOW_CORRELATIONMANAGER_H
#define FLOW_CORRELATIONMANAGER_H

#include <TTreeReader.h>

#include <utility>
#include "Correlation.h"

namespace Qn {

class CorrelationManager {
  using FUNCTION = std::function<double(std::vector<Qn::QVector> &)>;
  using PROJECTION = std::function<std::vector<Qn::QVector>(std::vector<Qn::QVector> &)>;
 public:
  explicit CorrelationManager(std::shared_ptr<TTreeReader> reader) :
      reader_(std::move(reader)) {

  }

  void AddDataContainer(std::string name) {
    TTreeReaderValue<Qn::DataContainerQVector>
        value(*reader_, name.data());
    tree_values_.emplace(name, value);
    DataContainerQVector a;
    qvectors_.emplace(name, a);
  }

  void AddEventVariable(const Qn::Axis &eventaxis) {
    TTreeReaderValue<float> value(*reader_, eventaxis.Name().data());
    tree_event_values_.emplace(eventaxis.Name(), value);
    event_values_.emplace_back(-999);
    eventbin_.emplace_back(-1);
    event_axes_.push_back(eventaxis);
  }

  void AddProjection(const std::string &originname, const std::string &newname, const std::string &axesnames) {
    auto toproject = qvectors_.at(originname);
    std::vector<Axis> axes;
    DataContainerQVector projection;
    projections_.emplace(originname, std::make_tuple(newname, axesnames, axes));
    qvectors_.emplace(newname, projection);
  }

  void AddCorrelation(std::string name,
                      std::string containernames,
                      FUNCTION lambda) {
    std::list<std::string> containernamelist;
    tokenize(containernames, containernamelist, ", ", true);
    build_correlations_.emplace(name, std::make_pair(containernamelist, lambda));
  }

  void SaveToFile(std::string name);

  void MakeProjections();

  void Initialize();

  void UpdateEvent();

  bool CheckEvent();

  void BuildCorrelation();

  void FillCorrelations();

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
  std::map<std::string, TTreeReaderValue<Qn::DataContainerQVector>> tree_values_;
  std::map<std::string, TTreeReaderValue<float>> tree_event_values_;
  std::map<std::string, std::pair<std::list<std::string>, FUNCTION>> build_correlations_;
  std::map<std::string, std::pair<std::list<std::string>, Qn::Correlation>> correlations_;
  std::map<std::string, Qn::DataContainerQVector> qvectors_;
  std::map<std::string, std::tuple<std::string, std::string, std::vector<Qn::Axis>>> projections_;
  std::vector<float> event_values_;
  std::vector<long> eventbin_;
  std::vector<Qn::Axis> event_axes_;

};
}

#endif //FLOW_CORRELATIONMANAGER_H
