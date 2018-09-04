//
// Created by Lukas Kreis on 15.12.17.
//

#ifndef FLOW_CORRELATIONMANAGER_H
#define FLOW_CORRELATIONMANAGER_H

#include <TTreeReader.h>

#include <utility>
#include <TFile.h>
#include "Correlation.h"
#include "Sampler.h"
#include "Correlator.h"
#include "Base/EventShape.h"

namespace Qn {

class CorrelationManager {
  using FUNCTION = std::function<double(std::vector<Qn::QVector> &)>;
 public:
  explicit CorrelationManager(std::shared_ptr<TTreeReader> reader) : reader_(std::move(reader)) {}
  CorrelationManager(std::shared_ptr<TTreeReader> reader, long num_events)
      : reader_(std::move(reader)), num_events_(num_events) {}

  /**
   * Adds a new DataContainer to the correlation manager.
   * Actual value is retrieved when the tree is read from the file.
   * @param name
   */
  void AddDataContainer(const std::string &name);

  /**
   * Adds a list of detectors to the correlation manager.
   * The actual values are retrieved when the tree is read from the file.
   * @param name
   */
  void AddQVectors(const std::string &namelist) {
    std::vector<std::string> names;
    tokenize(namelist, names, ", ", true);
    for (const auto & name : names) {
      AddDataContainer(name);
    }
  }

  /**
   * Adds new Projection to the correlation manager.
   * Projects the DataContainer on the specified axes and creates a new Datacontainer with a new name.
   * @param input_name Name of the input datacontainer.
   * @param output_name  Name of the output projection
   * @param axis_names Names of axes to be projected upon.
   */
  void AddProjection(const std::string &input_name, const std::string &output_name, const std::string axis_names);

  /**
   * Adds a new event variable to the correlation manager.
   * Actual value is retrieved when the tree is read from the file.
   * @param eventaxis Event variable defined as a Axis, which is used in the correlations.
   */

  void AddEventVariable(const Qn::Axis &eventaxis);
  /**
   * Adds a correlation to the output.
   * @param name Name of the correlation under which it is saved to the file
   * @param input_names Names of the input datacontainers.
   * @param lambda Function which is used to calculate the correlation.
   * @param nsamples number of samples used in the subsampling
   * @param method method which is used for the subsampling
   */
  void AddCorrelation(std::string name,
                      const std::string &input_names,
                      FUNCTION &&lambda,
                      int nsamples,
                      Sampler::Method method);
  /**
   * Adds a correlation to the output.
   * @param name Name of the correlation under which it is saved to the file
   * @param containernames Names of the input datacontainers.
   * @param lambda Function which is used to calculate the correlation.
   */
  void AddCorrelation(std::string name,
                      const std::string &containernames,
                      FUNCTION &&lambda);

  void AddInversion(std::string name, std::string input_name) {
  }

  /**
   * Adds a ESE axis to all correlations
   * @param name Name of the datacontainer used to calculate the Q-vector magnitude.
   * @param harmonic harmonic used for calculation
   */
  void AddESE(const std::string &name, int harmonic, float qmax);

  /**
   * Initializes the Correlation task.
   */
  void Initialize();

  /**
   * Process the correlation. Needs to be called in the event loop.
   */
  void Process();
  /**
   * Finalizes the correlation task
   * @param correlation_file name of the correlation file.
   * @param ese_file name of the ese q-vector magnitude calibration file.
   */
  void Finalize();

  /**
   * Set Name of correlation output file.
   * @param output_name
   */
  void SetOutputFile(const std::string output_name) {
    correlation_file_name_ = output_name;
  }
  /**
   * Set name of ese calibration file.
   * @param ese_name
   */
  void SetESECalibrationFile(const std::string ese_name) {
    ese_file_name_ = ese_name;
  }

 private:
  void MakeProjections() {
    for (const auto &projection : projections_) {
      qvectors_.at(projection.first) =
          qvectors_.at(std::get<0>(projection.second)).Projection(std::get<1>(projection.second),
                                                                  [](const Qn::QVector &a, const Qn::QVector &b) {
                                                                    auto norm = b.GetNorm();
                                                                    return (a + b).Normal(norm);
                                                                  });
    }
  }

  DataContainerSample GetResult(const std::string &name) const {
    return correlations_.at(name).GetResult();
  }

  void SaveToFile(std::string name);

  void BuildCorrelations() {
    std::vector<Qn::DataContainerQVector> qvectors;
    for (auto &corr : correlations_) {
      qvectors.clear();
      qvectors.reserve(corr.second.GetInputNames().size());
      for (const auto &cname : corr.second.GetInputNames()) {
        qvectors.push_back(qvectors_.at(cname));
      }
      std::vector<Qn::Axis> axes;
      axes = event_axes_;
      if (event_shape_ && !fill_ese_) {
        axes.push_back(eventshape_axes_.at(0));
      }
      corr.second.ConfigureCorrelation(qvectors, axes);
      corr.second.BuildSamples(num_events_);
    }
  }

  void FillESE(std::map<std::string, Qn::Correlator> &corr) {
    for (auto &pair : corr) {
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

  void FillCorrelations(std::map<std::string, Qn::Correlator> &corr) {
    for (auto &pair : corr) {
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

  bool CheckESEEvent() {
    if (use_ese_) {
      for (auto bin : ese_correlations_) {
        auto ese = bin.second.GetCorrelation().GetCorrelation();
        auto pair = ese.At(eventbin_);
        float value = -999.;
        if (pair.first) value = pair.second;
        event_values_.back() = event_shape_->At(eventbin_).GetPercentile(value);
      }
    }
    if (!fill_ese_) {
      u_long ie = 0;
      auto event_axis_size = event_axes_.size();
      for (const auto &ae : eventshape_axes_) {
        auto bin = ae.FindBin(event_values_.back());
        if (bin!=-1) {
          eventbin_.at(event_axis_size + ie) = (unsigned long) bin;
        } else {
          return false;
        }
        ie++;
      }
      return true;
    } else {
      return false;
    }
  }

  void BuildESECorrelation() {
    if (!use_ese_) return;
    for (const auto &ese : ese_correlations_) {
      if (!event_shape_) {
        event_shape_.reset(new Qn::DataContainerESE);
        event_shape_->AddAxes(event_axes_);
      }
      if (event_shape_ && !fill_ese_) {
        eventshape_axes_.emplace_back(ese.first, 10, 0., 1.);
      }
      if (event_shape_ && !fill_ese_) {
        event_values_.emplace_back(-999);
      }
    }
    std::vector<Qn::DataContainerQVector> qvectors;
    for (auto &corr : ese_correlations_) {
      qvectors.clear();
      qvectors.reserve(corr.second.GetInputNames().size());
      for (const auto &cname : corr.second.GetInputNames()) {
        qvectors.push_back(qvectors_.at(cname));
      }
      corr.second.ConfigureCorrelation(qvectors, event_axes_);
      corr.second.BuildSamples(num_events_);
    }
  }

  void FitEventShape() {
    if (!use_ese_ || !fill_ese_) return;
    int ibin = 0;
    for (auto correlation : ese_correlations_) {
      for (auto bin : *correlation.second.GetBinnedResult()) {
        auto name = correlation.first;
        event_shape_->At(ibin).SetName("ESE_" + name);;
        event_shape_->At(ibin).SetHisto(&bin);
        event_shape_->At(ibin).FitWithSpline(bin);
        event_shape_->At(ibin).SetReady();
        ++ibin;
      }
    }
  }

  void SaveEventShape(const std::string &filename) {
    if (fill_ese_ && use_ese_) {
      event_shape_.reset((Qn::DataContainerESE *) event_shape_->Clone("ESE"));
      ese_file_.reset(new TFile(filename.data(), "RECREATE"));
      if (event_shape_ && fill_ese_) ese_file_->WriteTObject(event_shape_.get(), "ESE", "");
      ese_file_->Close();
    }
  }

  bool IsESE() const {
    if (!use_ese_) return false;
    return std::all_of(event_shape_->begin(),
                       event_shape_->end(),
                       [](Qn::EventShape &a) { return a.IsReady(); }) && !fill_ese_;
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
  std::unique_ptr<Qn::DataContainerESE> event_shape_ = nullptr;
  bool fill_ese_ = false;
  bool use_ese_ = false;
  long num_events_ = 0;

};
}

#endif //FLOW_CORRELATIONMANAGER_H
