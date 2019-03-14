// Flow Vector Correction Framework
//
// Copyright (C) 2018  Lukas Kreis, Ilya Selyuzhenkov
// Contact: l.kreis@gsi.de; ilya.selyuzhenkov@gmail.com
// For a full list of contributors please see docs/Credits
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef FLOW_CORRELATIONMANAGER_H
#define FLOW_CORRELATIONMANAGER_H

#include <memory>
#include <utility>
#include <ctime>

#include "TTreeReader.h"
#include "TFile.h"

#include "Correlation.h"
#include "Sampler.h"
#include "Correlator.h"
#include "EventShape.h"
#include "DataContainer.h"

namespace Qn {

class CorrelationManager {
  using FUNCTION = std::function<double(std::vector<Qn::QVector> &)>;
  using size_type = std::size_t;
 public:
  explicit CorrelationManager(std::shared_ptr<TTreeReader> reader) : reader_(std::move(reader)) { num_events_ = reader_->GetEntries(true);}
  CorrelationManager(std::shared_ptr<TTreeReader> reader, size_type num_events)
      : reader_(std::move(reader)), num_events_(num_events) {}

  void AddDataContainer(const std::string &name);

  void AddQVectors(const std::string &namelist);

  void AddProjection(const std::string &input_name, const std::string &output_name, const std::string &axis_names);

  void AddEventVariable(const Qn::Axis &eventaxis);

  void AddCorrelation(std::string name, const std::string &input_names, FUNCTION &&lambda, Qn::Sampler::Resample resample = Qn::Sampler::Resample::ON);

  void SetRefQinCorrelation(const std::string &name, const std::vector<Qn::Weight> &use_weights) {
    correlations_.at(name).SetReferenceQVectors(use_weights);
  }

  void ConfigureResampling(Sampler::Method method, size_type nsamples, unsigned long seed = time(0)) { sampler_.Configure(method, nsamples, seed); }

  void AddESE(const std::string &name, FUNCTION &&lambda, float qmax);

  void Initialize();

  void Process();

  void Finalize();

  /**
   * Set Name of correlation output file.
   * @param output_name
   */
  void SetOutputFile(const std::string &output_name) { correlation_file_name_ = output_name; }
  /**
   * Set name of ese calibration file.
   * @param ese_name
   */
  void SetESECalibrationFile(const std::string &ese_name) { ese_file_name_ = ese_name; }

  DataContainerStats GetResult(const std::string &name) const { return correlations_.at(name).GetResult(); }

 private:
  void MakeProjections();

  void SaveToFile(std::string name);

  void BuildCorrelations();

  void FillESE(std::map<std::string, Qn::Correlator> &corr);

  void FillCorrelations(std::map<std::string, Qn::Correlator> &corr);

  void UpdateEvent();

  bool CheckEvent();

  bool CheckESEEvent();

  void BuildESECorrelation();

  void FitEventShape();

  void SaveEventShape(const std::string &filename);

  bool IsESE() const;

 private:
  Qn::Sampler sampler_;
  std::string correlation_file_name_;
  std::string ese_file_name_;
  std::unique_ptr<TFile> ese_file_;
  std::shared_ptr<TTreeReader> reader_;
  std::map<std::string, Qn::Correlator> correlations_;
  std::map<std::string, Qn::Correlator> ese_correlations_;
  std::map<std::string, std::tuple<std::string, std::vector<std::string>>> projections_;
  std::map<std::string, TTreeReaderValue<Qn::DataContainerQVector>> tree_values_;
  std::map<std::string, TTreeReaderValue<float>> tree_event_values_;
  std::map<std::string, Qn::DataContainerQVector> qvectors_;
  std::vector<float> event_values_;
  std::vector<unsigned long> eventbin_;
  std::vector<Qn::Axis> event_axes_;
  std::vector<Qn::Axis> eventshape_axes_;
  std::unique_ptr<Qn::DataContainerEventShape> event_shape_ = nullptr;
  bool fill_ese_ = false;
  bool use_ese_ = false;
  size_type num_events_ = 0;

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

};
}

#endif //FLOW_CORRELATIONMANAGER_H
