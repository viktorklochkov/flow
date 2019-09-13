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

#ifndef FLOW_EVENTSHAPE_H
#define FLOW_EVENTSHAPE_H

#include <iostream>

#include "TH1F.h"
#include "TSpline.h"
#include "TCanvas.h"
#include "TFile.h"
#include "CorrelationResult.h"

namespace Qn {
/**
 * @class Holds event shape information. Can be saved to a file.
 */
class EventShape : public TObject {
 public:

  /**
   * holds the information of the state
   */
  enum class State {
    Uninitialized,
    ReadyForCollecting,
    ReadyForCalculation
  };

  /**
   * default constructor
   */
  EventShape() = default;
  /**
   * constructor
   * @param name name of the sub event
   * @param histo histogram with the correct binning
   */
  EventShape(std::string name, TH1F histo) : name_(name) {
    auto nbins = histo.GetNbinsX();
    auto lower = histo.GetXaxis()->GetBinLowEdge(1);
    auto upper = histo.GetXaxis()->GetBinUpEdge(nbins);
    histo_ = new TH1F((name_ + "histo").data(), ";ese;counts", nbins, lower, upper);
    integral_ = new TH1F((name_ + "integral").data(), ";ese;counts", nbins, lower, upper);
  }

  virtual ~EventShape() {}

  /**
   * Sets the name and binning
   * @param histo histogram with binning information
   * @param name name of the subevent
   */
  void SetHisto(TH1F *histo, std::string name) {
    auto nbins = histo->GetNbinsX();
    auto lower = histo->GetXaxis()->GetBinLowEdge(1);
    auto upper = histo->GetXaxis()->GetBinUpEdge(nbins);
    histo_ = new TH1F((std::string("histo") + name).data(), ";ese;counts", nbins, lower, upper);
    integral_ = new TH1F((std::string("integral") + name).data(), ";ese;counts", nbins, lower, upper);
  }

  /**
   * Gets the name
   * @return the name of the subevent.
   */
  std::string Name() const { return name_; }

  /**
   * Gets the percentile of the given q vector magnitude
   * @param q magnitude of the q vector.
   * @return percentile of the current event.
   */
  inline float GetPercentile(float q) { return static_cast<float>(spline_->Eval(q)); }

  /**
   * Calculate the integrated histogram of the distribution.
   */
  void IntegrateHist();

  /**
   * Fit the histogram with a spline to calculate the percentiles.
   */
  void FitWithSpline();

  /**
   * Fill the current subevent information to the histogram.
   * @param product
   */
  void Fill(const CorrelationResult &product) { if (product.validity) histo_->Fill(product.result); }

  /**
   * Get the histogram.
   * @return returns the distribution of all events.
   */
  TH1F *GetHist() const { return histo_; }

  friend Qn::EventShape operator+(const Qn::EventShape &a, const Qn::EventShape &b);
  friend Qn::EventShape Merge(const Qn::EventShape &a, const Qn::EventShape &b);

  std::string name_;
  TSpline3 *spline_ = nullptr;
  TH1F *histo_ = nullptr;
  TH1F *integral_ = nullptr;

  /// \cond CLASSIMP
 ClassDef(EventShape, 5);
  /// \endcond
};

inline Qn::EventShape operator+(const Qn::EventShape &a, const Qn::EventShape &b) {
  Qn::EventShape c(a.name_, *a.histo_);
  c.histo_->Add(b.histo_);
  c.FitWithSpline();
  return c;
}

inline Qn::EventShape Merge(const Qn::EventShape &a, const Qn::EventShape &b) {
  Qn::EventShape c(a.name_, *a.histo_);
  c.histo_->Add(b.histo_);
  c.FitWithSpline();
  return c;
}

}

#endif //FLOW_EVENTSHAPE_H
