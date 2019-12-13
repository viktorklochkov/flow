// Flow Vector Correction Framework
//
// Copyright (C) 2019  Lukas Kreis Ilya Selyuzhenkov
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
#ifndef FLOW_RECENTERACTION_H_
#define FLOW_RECENTERACTION_H_

#include <algorithm>
#include <string>
#include <vector>

#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TFile.h"
#include "ROOT/RDataFrame.hxx"

#include "DataContainer.h"
#include "QVector.h"
#include "Common/TemplateMagic.h"

namespace Qn {
namespace Correction {

/**
 * predeclaration of Recentering class. Used to make variadic templates from a passed tuple.
 * @tparam AxesConfig
 * @tparam EventParameters
 */
template<typename AxesConfig, typename EventParameters>
class RecenterAction;

/**
 * Recentering class to be used with the AverageHelper class in a DataFrame analysis
 * to implement the recentering procedure. Recentering works using the formula:
 * \f{eqnarray*}{
 * x' &=& x - \langle x \rangle \\
 * y' &=& y - \langle y \rangle
 * \f}
 * where the averages are built in the subevents specified in the Qn::AxesConfiguration.
 * It is possible to enable width equalization.
 * \f{eqnarray*}{
 * x' &=& \frac{x - \langle x \rangle}{\sigma_{x}} \\
 * y' &=& \frac{y - \langle y \rangle}{\sigma{y}}
 * \f}
 * @tparam AxesConfig
 * @tparam EventParameters
 */
template<typename AxesConfig, typename ...EventParameters>
class RecenterAction<AxesConfig, std::tuple<EventParameters...>> {
 public:
  /**
   * Number of Input Q vectors going into the correction step.
   * It is used in the AverageHelper class.
   */
  static constexpr unsigned int NumberOfInputs = 1;
  /**
   * typename of the tuple of types of the axes' bin edges.
   * It is used in the AverageHelper class.
   */
  using EventParameterTuple = typename AxesConfig::AxisValueTypeTuple;

  /**
   * Constructor
   * @param correction_name name of the correction step
   * @param sub_event_name name of the input Q vector in the input TTree.
   * @param axes_configuration Qn::AxesConfiguration determining the sub samples used for corrections.
   */
  RecenterAction(std::string correction_name, std::string sub_event_name, AxesConfig axes_configuration) :
      correction_name_(std::move(correction_name)),
      sub_event_name_(std::move(sub_event_name)),
      event_axes_(axes_configuration) {}

  /**
   * Enables the width equalization.
   * @return returns it self.
   */
  RecenterAction EnableWidthEqualization() &&{
    use_width_equalization_ = true;
    return std::move(*this);
  }

  /**
   * Sets the minumum number of contributors required for the correction to be applied in this bin.
   * @return returns it self.
   */
  RecenterAction SetMinimumNumberOfEntries(const unsigned int minimum_entries) &&{
    min_entries_ = minimum_entries;
    return std::move(*this);
  }

  /**
   * Returns the name of the columns used in the correction step. This includes both the input Q-vector and
   * the name of the axes parameters. This function is required by the AverageHelper.
   * @return returns a vector of the column names.
   */
  std::vector<std::string> GetColumnNames() const {
    std::vector<std::string> columns;
    columns.emplace_back(sub_event_name_);
    const auto event_axes_names = event_axes_.GetNames();
    std::copy(std::begin(event_axes_names), std::end(event_axes_names), std::back_inserter(columns));
    return columns;
  }

  /**
   * Returns the name of the output Q-vector.
   * @return name of the output Q-vector
   */
  std::string GetName() const { return sub_event_name_ + "_" + correction_name_; }

  /**
   * Initializes the correction step using the information inside the input tree.
   * @param reader reader wrapping the input Q-vector tree. This function is required by the AverageHelper.
   */
  void Initialize(TTreeReader &reader) {
    auto entry = reader.GetCurrentEntry();
    reader.Restart();
    TTreeReaderValue<DataContainerQVector> input_data(reader, sub_event_name_.data());
    reader.SetLocalEntry(1);
    if (input_data.GetSetupStatus() < 0) {
      auto message = std::string("The Q-Vector entry") +
          input_data.GetBranchName() + "in the tree is not valid. Cannot setup the recentering";
      throw std::runtime_error(message);
    }
    auto input_q = input_data->At(0);
    input_q.InitializeHarmonics();
    int i_harmonic = 0;
    int harmonic = input_q.GetFirstHarmonic();
    while (harmonic!=-1) {
      x_.emplace_back();
      y_.emplace_back();
      x_[i_harmonic].AddAxes(event_axes_.GetVector());
      y_[i_harmonic].AddAxes(event_axes_.GetVector());
      if (!input_data->IsIntegrated()) {
        x_[i_harmonic].AddAxes(input_data->GetAxes());
        y_[i_harmonic].AddAxes(input_data->GetAxes());
      }
      harmonics_vector_.push_back(harmonic);
      harmonic = input_q.GetNextHarmonic(harmonic);
      i_harmonic++;
    }
    stride_ = input_data->size();
    reader.Restart();
    reader.SetLocalEntry(entry);
  }

  /**
   * Calculates the corrections and saves them in the x_ and y_ member variables.
   * This function is required by the AverageHelper.
   * @param input input Q-vector, which is supposed to be corrected.
   * @param event_parameters event parameters determining the correction histogram bin.
   */
  void CalculateAction(const Qn::DataContainerQVector &input, EventParameters ...event_parameters) {
    auto event_bin = event_axes_.GetLinearIndex(event_parameters...) * stride_;
    if (event_bin < 0) return;
    for (std::size_t i = 0u; i < harmonics_vector_.size(); ++i) {
      for (std::size_t ibin = 0; ibin < stride_; ++ibin) {
        const auto output_bin = event_bin+ibin;
        if (input[ibin].sumweights() > 0.) {
          x_[i].At(output_bin).Fill(input[ibin].x(harmonics_vector_[i]), 1.);
          y_[i].At(output_bin).Fill(input[ibin].y(harmonics_vector_[i]), 1.);
        }
      }
    }
  }

  /**
   * Applies the correction on the input Q-vector and returns a corrected Q-vector.
   * Uses the information saved in a previous iteration over the data.
   * This function is used by the Define functionality of the RDataFrame.
   * @param input_q input to the correction step.
   * @param coordinates event parameters determining the correction histogram bin.
   * @return corrected Q-vector
   */
  Qn::DataContainerQVector operator()(const Qn::DataContainerQVector &input_q, EventParameters ...coordinates) {
    Qn::DataContainerQVector corrected_q(input_q);
    auto event_bin = event_axes_.GetLinearIndex(coordinates...) * stride_;
    if (event_bin < 0) return corrected_q;
    for (std::size_t ibin = 0; ibin < stride_; ++ibin) {
      auto correction_bin = event_bin+ibin;
      if (x_[0].At(correction_bin).Entries() < min_entries_) continue;
      auto x_width = 1.;
      auto y_width = 1.;
      corrected_q[ibin].UpdateCorrectionStep(Qn::QVector::CorrectionStep::RECENTERED);
      for (std::size_t i_harmonic = 0; i_harmonic < harmonics_vector_.size(); ++i_harmonic) {
        if (use_width_equalization_) {
          x_width = x_[i_harmonic][correction_bin].Sigma();
          y_width = y_[i_harmonic][correction_bin].Sigma();
        }
        auto harmonic = harmonics_vector_[i_harmonic];
        corrected_q[ibin].SetQ(harmonic, (input_q[ibin].x(harmonic) - x_[i_harmonic][correction_bin].Mean())/x_width,
                                         (input_q[ibin].y(harmonic) - y_[i_harmonic][correction_bin].Mean())/y_width);
      }
    }
    return corrected_q;
  }

  /**
   * Loads the correction histograms from a previous iteration from the file.
   * @param file file which contains the correction histograms.
   * @param reader reader which wraps the input tree. Needed to perform the initialization.
   */
  bool LoadCorrectionFromFile(TDirectory * dir, TTreeReader *reader) {
    if (!dir) {
      std::cout << "Rerunning the correction step." << std::endl;
      return false;
    }
    if (!dir->FindKey(GetName().data())) {
      std::cout << "Correction for " << GetName() << " not found in file " << dir->GetName() << ". ";
      std::cout << "Rerunning the correction step." << std::endl;
      return false;
    }
    Initialize(*reader);
    for (std::size_t i_harmonic = 0; i_harmonic < harmonics_vector_.size(); ++i_harmonic) {
      auto harmonic = harmonics_vector_[i_harmonic];
      auto x = dynamic_cast<Qn::DataContainerStatistic *>(dir->Get((GetName() + "/X_"
          + std::to_string(harmonic)).data()));
      auto y = dynamic_cast<Qn::DataContainerStatistic *>(dir->Get((GetName() + "/Y_"
          + std::to_string(harmonic)).data()));
      if (!(x && y)) {
        std::cout << "harmonic" << harmonic << " of Q-vector correction step "
          << GetName() << " is not not found in the file." << std::endl;
        Reset();
        return false;
      }
      auto read_axes = x->GetAxes();
      auto configured_axes = x_.at(i_harmonic).GetAxes();
      std::vector<bool> result;
      std::transform(std::begin(read_axes),
                     std::end(read_axes),
                     std::begin(configured_axes),
                     std::back_inserter(result),
                     [](const Qn::AxisD &a, const Qn::AxisD &b) { return a==b; });
      if (!std::all_of(result.begin(), result.end(), [](bool x) { return x; })) {
        std::cout << "Axes of Q-vector correction step " << GetName()
          << " are not equal to the ones found in the file." << std::endl;
        Reset();
        return false;
      }
      x_.at(i_harmonic) = *x;
      y_.at(i_harmonic) = *y;
    }
    std::cout << "Correction for " << GetName() << " found in directory " << dir->GetName() << "." << std::endl;
    return true;
  }

  /**
   * Wrapper function to easily apply the correctino on the Q-vector
   * @tparam DataFrame a RDataFrame type
   * @param df the dataframe which contains the input Q-vector.
   * @return returns a dataframe which has a column of the corrected Q-vector.
   */
  template<typename DataFrame>
  auto ApplyCorrection(DataFrame &df) const {
    std::vector<std::string> columns;
    columns.push_back(sub_event_name_);
    const auto event_axes_names = event_axes_.GetNames();
    std::copy(std::begin(event_axes_names), std::end(event_axes_names), std::back_inserter(columns));
    return df.Define(GetName(), *this, columns);
  }

  /**
   * Merges the correction histogram after the collection of statistics is complete.
   * This function is required by the AverageHelper class.
   * @param results the other results which are to be merged with this one.
   */
  void Merge(const std::vector<std::shared_ptr<RecenterAction>> &results) {
    for (std::size_t i_harmonic = 0; i_harmonic < harmonics_vector_.size(); ++i_harmonic) {
      TList xs;
      TList ys;
      for (const auto &result : results) {
        xs.Add(&result->x_[i_harmonic]);
        ys.Add(&result->y_[i_harmonic]);
      }
      x_[i_harmonic].Merge(&xs);
      y_[i_harmonic].Merge(&ys);
    }
  }

  /**
   * Writes the correction histograms to file.
   * @param directory directory in which the correction histograms are supposed to be written.
   */
  void Write(TDirectory *directory) {
    directory->cd();
    directory->mkdir(GetName().data());
    directory->cd(GetName().data());
    for (std::size_t i_harmonic = 0; i_harmonic < harmonics_vector_.size(); ++i_harmonic) {
      x_.at(i_harmonic).Write((std::string("X_") + std::to_string(harmonics_vector_[i_harmonic])).data());
      y_.at(i_harmonic).Write((std::string("Y_") + std::to_string(harmonics_vector_[i_harmonic])).data());
    }
    auto max_entries = std::max_element(x_[0].begin(), x_[0].end(),
                                        [](const Statistic &a, const Statistic &b) {
                                          return a.Entries() < b.Entries();
                                        })->Entries();
    auto min_entries = std::min_element(x_[0].begin(), x_[0].end(),
                                        [](const Statistic &a, const Statistic &b) {
                                          return a.Entries() < b.Entries();
                                        })->Entries();
    auto difference = (max_entries - min_entries) * 0.05;
    max_entries = max_entries + difference;
    min_entries = min_entries - difference;
    TH1F histo_bin_occupancy("bin_occupancy", "occupancy per bin; Counts; Entries in a bin", 100, min_entries, max_entries);
    for (auto &bin : x_[0]) {
      auto n = bin.Entries();
      histo_bin_occupancy.Fill(n);
    }
    histo_bin_occupancy.Write("EntriesPerBin");
    directory->cd("../");
  }

 private:
  bool use_width_equalization_ = false; /// Switch for applying the width equalization procedure.
  unsigned int min_entries_ = 10; /// Number of minimum entries in a bin required to apply corrections.
  unsigned int stride_ = 1.; /// stride of the differential Q-vector.
  std::vector<unsigned int> harmonics_vector_; /// vector of enabled harmonics.
  std::string correction_name_; /// name of the correction step.
  std::string sub_event_name_; /// name of the input Q-vector.
  AxesConfig event_axes_; /// event axes used to classify the events in classes for the correction step.
  std::vector<Qn::DataContainerStatistic> x_; /// x component of correction histograms.
  std::vector<Qn::DataContainerStatistic> y_; /// y component correction histograms.

  /**
   * Resets the Correction.
   */
  void Reset() {
    x_.clear();
    y_.clear();
    harmonics_vector_.clear();
    stride_ = 1;
  }
};

/**
 * Function which creates the Recentering correction step
 * @tparam EventAxes Qn::AxesConfiguration
 * @param correction_name name of the correction step
 * @param sub_event_name name of the input Q vector in the input TTree.
 * @param axes_configuration Qn::AxesConfiguration determining the sub samples used for corrections.
 * @return RecenterAction
 */
template<typename EventAxes>
auto Recentering(const std::string &correction_name, const std::string &sub_event_name, EventAxes axes_configuration) {
  return RecenterAction<EventAxes, typename EventAxes::AxisValueTypeTuple>{correction_name, sub_event_name,
                                                                           axes_configuration};
}

/**
 * Convenience function to apply the corrections of multiple Recentering procedures.
 * Ends the recursion.
 * @tparam DataFrame type of a RDataFrame
 * @tparam Last Type of the last remaining correction
 * @param df RDataFrame which contains the input Q-vectors.
 * @param last The last remaining correction step.
 * @return RDataFrame, which contains the the corrected Q-vectors.
 */
template<typename DataFrame, typename Last>
auto ApplyCorrections(DataFrame df, Last last) {
  return last.ApplyCorrection(df);
}

/**
 * Convenience function to apply the corrections of multiple Recentering procedures.
 * Ends the recursion.
 * @tparam DataFrame type of a RDataFrame
 * @tparam First Type of the first correction step.
 * @tparam Rest Types of the remaining correction steps
 * @param df RDataFrame which contains the input Q-vectors.
 * @param first the first correction step to be applied.
 * @param rest rest of the correction steps.
 * @return RDataFrame, which contains the the corrected Q-vectors.
 */
template<typename DataFrame, typename First, typename... Rest>
inline auto ApplyCorrections(DataFrame df, First first, Rest ...rest) {
  return ApplyCorrections(first.ApplyCorrection(df), rest...);
}



/**
 * Convenience function to apply the corrections of multiple Recentering procedures.
 * @tparam DataFrame type of a RDataFrame
 * @param df RDataFrame which contains the input Q-vectors.
 * @param first the first correction step to be applied.
 * @param rest rest of the correction steps.
 * @return RDataFrame, which contains the the corrected Q-vectors.
 */
template<typename DataFrame, typename VectorOfCorrections>
inline auto ApplyCorrectionsVector(DataFrame df, VectorOfCorrections corrections) {
  ROOT::RDF::RNode corrected_df(df);
  for (auto &result : corrections) {
    corrected_df = Qn::TemplateMagic::DereferenceRResultPtr(result).ApplyCorrection(corrected_df);
  }
  return corrected_df;
}





template<typename DataFrame>
inline auto SnapshotVector(DataFrame df, std::string filename, std::string treename,
                           std::vector<std::string> qvectors, std::vector<std::string> branches) {
  std::vector<std::string> columns;
  std::transform(std::begin(qvectors),
                 std::end(qvectors),
                 std::back_inserter(columns),
                 [](auto name){ return name;});
  std::transform(std::begin(branches),
                 std::end(branches),
                 std::back_inserter(columns),
                 [](auto name){ return name;});
  return df.Snapshot(treename, filename, columns);
}


}
}
#endif //FLOW_RECENTERACTION_H_
