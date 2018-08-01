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
    BuildESECorrelation();
  }

  void BuildCorrelations() {
    auto nevents = reader_->GetEntries(true);
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
    FillCorr(correlations_);
  }

  void FillESECorrelation() {
    if (event_shape_) FillESECorr(ese_correlations_);
  }

  void FillESECorr(std::map<std::string, Qn::Correlator> &corr) {
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

  void FillCorr(std::map<std::string, Qn::Correlator> &corr) {
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

  void BuildESECorrelation() {
    auto nevents = reader_->GetEntries(true);
    std::vector<Qn::DataContainerQVector> qvectors;
    for (auto &corr : ese_correlations_) {
      qvectors.clear();
      qvectors.reserve(corr.second.GetInputNames().size());
      for (const auto &cname : corr.second.GetInputNames()) {
        qvectors.push_back(qvectors_.at(cname));
      }
      corr.second.ConfigureCorrelation(qvectors, event_axes_);
      corr.second.BuildSamples(nevents);
    }
  }

  void SetESEInputFile(const std::string &name) {
    ese_file_.reset(new TFile("ese_file.root", "OPEN"));
    fill_ese_ = true;
    if (ese_file_->IsOpen()) {
      event_shape_.reset((Qn::DataContainerESE *) (ese_file_->Get("ESE"))->Clone("ESE"));
      fill_ese_ = false;
    }
  }

  void FitEventShape(const std::string &name) {
    int ibin = 0;
    for (auto bin : ese_correlations_.at("ESE_" + name).GetBinnedResult()) {
      event_shape_->At(ibin).SetName("ESE_" + name);;
      event_shape_->At(ibin).SetHisto(&bin);
      event_shape_->At(ibin).FitWithSpline(bin);
      event_shape_->At(ibin).SetReady();
      ++ibin;
    }
  }

  void SaveEventShape() {
    if (fill_ese_) {
      event_shape_.reset((Qn::DataContainerESE *) event_shape_->Clone("ESE"));
      ese_file_.reset(new TFile("ese_file.root", "RECREATE"));
      if (event_shape_ && fill_ese_) ese_file_->WriteTObject(event_shape_.get(), "ESE", "");
      ese_file_->Close();
    }
  }

  void CheckESEbin(const std::string &name) {
    auto ese = ese_correlations_.at("ESE_" + name).GetCorrelation().GetCorrelation();
    auto pair = ese.At(eventbin_);
    float value = -999.;
    if (pair.first) value = pair.second;
    event_values_.back() = event_shape_->At(eventbin_).GetPercentile(value);
  }

  void AddESE(const std::string &name, int harmonic) {
    auto Mag = [harmonic](const std::vector<Qn::QVector> &a) {
      return 1./TMath::Sqrt(a[0].sumweights())*a[0].DeNormal().mag(harmonic);
    };
    Qn::Correlator correlator({name}, Mag, TH1F("q", ";q;%;", 100, 0., 8.));
    correlator.ConfigureSampler(Sampler::Method::NONE, 0);
    ese_correlations_.emplace("ESE_" + name, std::move(correlator));
    if (event_shape_) eventbin_.push_back(-1);
    if (!event_shape_) {
      event_shape_.reset(new Qn::DataContainerESE);
      event_shape_->AddAxes(event_axes_);
    }
    if (event_shape_ && !fill_ese_) {
      eventshape_axes_.emplace_back("ESE_" + name, 10, 0., 1.);
    }
    if (event_shape_ && !fill_ese_) {
      event_values_.emplace_back(-999);
    }
  }

  bool IsESE() const {
    return std::all_of(event_shape_->begin(),
                       event_shape_->end(),
                       [](Qn::EventShape &a) { return a.IsReady(); }) && !fill_ese_;
  }

 private:
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
  bool fill_ese_;

};
}

#endif //FLOW_CORRELATIONMANAGER_H
