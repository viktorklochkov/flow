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
#include "EventVariables.h"

#include "ROOT/RMakeUnique.hxx"

namespace Qn {
using QVectors = const std::vector<QVectorPtr> &;
class CorrelationManager {
  using function_t = Qn::Correlation::function_t;
  using size_type = std::size_t;

 public:
  explicit CorrelationManager(TTree *tree) :
      ese_handler_(this),
      event_axes_(this),
      tree_(tree),
      reader_(new TTreeReader(tree)),
      qvectors_(new std::map<std::string, DataContainerQVector *>()) {
    num_events_ = reader_->GetEntries(true);
  }

  void AddProjection(const std::string &name, const std::string &input, const std::vector<std::string> &axes);
  void AddEventAxis(const Axis &eventaxis);
  void AddCorrelation(std::string name, const std::vector<std::string> &input, function_t lambda,
                      const std::vector<Weight> &use_weights, Sampler::Resample resample = Sampler::Resample::ON);
  void AddEventShape(const std::string &name,
                     const std::vector<std::string> &input,
                     function_t lambda,
                     const TH1F &histo);
  void SetResampling(Sampler::Method method, size_type nsamples, unsigned long seed = time(0));

  void SetOutputFile(const std::string &output_name) { correlation_file_name_ = output_name; }

  void SetESEInputFile(const std::string &ese_name, const std::string &tree_file_name) {
    ese_handler_.SetInput(tree_file_name, ese_name);
  }
  void SetESEOutputFile(const std::string &ese_name, const std::string &tree_file_name) {
    ese_handler_.SetOutput(tree_file_name, ese_name);
  }

  void Run();

  DataContainerStats GetResult(const std::string &name) const { return stats_results_.at(name).GetResult(); }

 private:

  friend class Qn::EventVariables;
  friend class Qn::EseHandler;

  void AddDataContainer(const std::string &name);

  void Initialize();

  void Finalize();

  void MakeProjections();

  void ConfigureCorrelations();

  void UpdateEvent();

  Qn::Correlation *RegisterCorrelation(const std::string &name,
                                       const std::vector<std::string> &inputs,
                                       function_t lambda,
                                       std::vector<Qn::Weight> use_weights);

  void AddFriend(const std::string &treename, TFile *file) { tree_->AddFriend(treename.data(), file); }

  void SetRunEventId(const std::string &run, const std::string &event) {
    //event_axes_.AddRunEventId();
  }

  std::shared_ptr<TTreeReader> &GetReader() { return reader_; }

 private:
  size_type num_events_ = 0;

  std::unique_ptr<Qn::Sampler> sampler_ = nullptr;
  Qn::EseHandler ese_handler_;
  Qn::EventVariables event_axes_;

  std::string correlation_file_name_;

  TTree *tree_;
  std::shared_ptr<TTreeReader> reader_;
  std::map<std::string, std::unique_ptr<Qn::Correlation>> correlations_;
  std::map<std::string, Qn::StatsResult> stats_results_;
  std::map<std::string, std::tuple<std::string, std::vector<std::string>>> projections_;
  std::map<std::string, TTreeReaderValue<Qn::DataContainerQVector>> tree_values_;
  std::unique_ptr<std::map<std::string, Qn::DataContainerQVector *>> qvectors_;
  std::map<std::string, Qn::DataContainerQVector> qvectors_proj_;

};
}

#endif //FLOW_CORRELATIONMANAGER_H
