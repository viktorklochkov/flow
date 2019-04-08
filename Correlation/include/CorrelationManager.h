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

#include "StatsResult.h"
#include "Sampler.h"
#include "EventShape.h"
#include "DataContainer.h"
#include "EseHandler.h"
#include "EventAxes.h"

namespace Qn {
using QVectors = const std::vector<QVectorPtr> &;
class CorrelationManager {
  using function_type = Correlation::function_type;
  using function_p = Correlation::function_p;
  using size_type = std::size_t;
 public:

  enum class ESEState {
    ESEDisabled,
    ESEEnabled
  };

  enum class ResamplingStatus {
    ResamplingDisabled,
    ResamplingEnabled
  };

  explicit CorrelationManager(std::shared_ptr<TTreeReader> reader) :
      ese_handler_(this),
      event_axes_(this),
      reader_(std::move(reader)),
      qvectors_(new std::map<std::string, DataContainerQVector *>()) {
    num_events_ = reader_->GetEntries(true);

  }

  void AddProjection(const std::string &name, const std::string &input, const std::vector<std::string> &axes);
  void AddEventAxis(const Axis &eventaxis);
  void AddCorrelation(std::string name, const std::vector<std::string> &input, function_p lambda,
                      const std::vector<Weight> &use_weights, Sampler::Resample resample = Sampler::Resample::ON);
  void AddESE(const std::string &name, const std::vector<std::string> &input, function_p lambda, const TH1F &histo);
  void ConfigureResampling(Sampler::Method method, size_type nsamples, unsigned long seed = time(0));
  void Run();

  void SetESEInputFile(const std::string &ese_name, const std::string &tree_file_name) {
    ese_calib_in_ = std::make_unique<TFile>(ese_name.data());
    ese_handler_.ConnectInput(tree_file_name, ese_calib_in_.get());
  }

  void SetOutputFile(const std::string &output_name) { correlation_file_name_ = output_name; }

  void SetESEOutputFile(const std::string &ese_name, const std::string &tree_file_name) {
    ese_handler_.ConnectOutput(tree_file_name, ese_name, &ese_treefile_out_, &ese_calib_out_);
  }

  DataContainerStats GetResult(const std::string &name) const { return stats_results_.at(name).GetResult(); }

 private:

  friend class Qn::EseSubEvent;
  friend class Qn::EventAxes;

  void AddDataContainer(const std::string &name);

  void AddQVectors(const std::vector<std::string> &qvectors);

  void Initialize();

  void Finalize();

  void MakeProjections();

  void ConfigureCorrelations();

  void UpdateEvent();

  Qn::Correlation *AddCorrelationOnly(const std::string &name, const std::vector<std::string> &inputs,
                                      CorrelationManager::function_p lambda);

 private:
  ESEState ese_state_ = ESEState::ESEDisabled;
  ResamplingStatus resampling_status_ = ResamplingStatus::ResamplingDisabled;

  size_type num_events_ = 0;

  std::unique_ptr<Qn::Sampler> sampler_ = nullptr;
  Qn::EseHandler ese_handler_;
  Qn::EventAxes event_axes_;

  std::string correlation_file_name_;
  std::unique_ptr<TFile> ese_calib_in_;
  std::shared_ptr<TFile> ese_treefile_out_;
  std::shared_ptr<TFile> ese_calib_out_;

  std::shared_ptr<TTreeReader> reader_;
  std::map<std::string, std::unique_ptr<Correlation>> correlations_;
  std::map<std::string, Qn::StatsResult> stats_results_;
  std::map<std::string, std::tuple<std::string, std::vector<std::string>>> projections_;
  std::map<std::string, TTreeReaderValue<Qn::DataContainerQVector>> tree_values_;
  std::unique_ptr<std::map<std::string, DataContainerQVector *>> qvectors_;
  std::map<std::string, DataContainerQVector> qvectors_proj_;


};
}

#endif //FLOW_CORRELATIONMANAGER_H
