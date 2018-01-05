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
  using DATAFUNCTION = std::function<Qn::DataContainerQVector (Qn::DataContainerQVector &)>;
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

  void AddFunction(const std::string &name, DATAFUNCTION &&lambda) {
    apply_function_qvectors.emplace(std::make_pair(name, lambda));
  }

  void ApplyFunction() {
    for (const auto &data : apply_function_qvectors) {
      qvectors_.at(data.first) = data.second(qvectors_.at(data.first));
    }
  }

  /** Adds a new projection to the manager. Actual projection is created after the data have been read from file.
   * @param originname Original data container.
   * @param newname Name of the projection.
   * @param axesnames Names of the axes.
   */
  void AddProjection(const std::string &originname, const std::string &newname, const std::string &axesnames) {
    auto toproject = qvectors_.at(originname);
    std::vector<Axis> axes;
    DataContainerQVector projection;
    projections_.emplace(originname, std::make_tuple(newname, axesnames, axes));
    qvectors_.emplace(newname, projection);
  }

  /**
   * Adds a new correlation to the manager. Actual correlations is created when data is read from the file.
   * @param name Name of the correlation.
   * @param containernames Names of the correlated data containers.
   * @param lambda Function used for the correlation.
   */
  void AddCorrelation(const std::string &name,
                      const std::string &containernames,
                      FUNCTION &&lambda) {
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
  std::map<std::string, DATAFUNCTION> apply_function_qvectors;
  std::map<std::string, Qn::DataContainerQVector> qvectors_;
  std::map<std::string, std::tuple<std::string, std::string, std::vector<Qn::Axis>>> projections_;
  std::vector<float> event_values_;
  std::vector<long> eventbin_;
  std::vector<Qn::Axis> event_axes_;

};
}

#endif //FLOW_CORRELATIONMANAGER_H
