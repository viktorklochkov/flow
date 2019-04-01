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
#include "Product.h"

namespace Qn {
class EventShape : public TObject {
 public:

  enum class State {
    Uninitialized,
    ReadyForCollecting,
    ReadyForCalculation
  };

  EventShape() = default;
  EventShape(std::string name, TH1F histo) : name_(name) {
    auto nbins = histo.GetNbinsX();
    auto lower = histo.GetXaxis()->GetBinLowEdge(1);
    auto upper = histo.GetXaxis()->GetBinUpEdge(nbins);
    histo_ = new TH1F((name_+"histo").data(),";ese;counts",nbins,lower, upper);
    integral_ = new TH1F((name_+"integral").data(),";ese;counts",nbins,lower, upper);
  }

  virtual ~EventShape() {
    delete spline_;
    delete histo_;
    delete integral_;
  }

  void SetHisto(const TH1F &histo) {
    auto nbins = histo.GetNbinsX();
    auto lower = histo.GetXaxis()->GetBinLowEdge(1);
    auto upper = histo.GetXaxis()->GetBinUpEdge(nbins);
    histo_ = new TH1F((name_+"histo").data(),";ese;counts",nbins,lower, upper);
    integral_ = new TH1F((name_+"integral").data(),";ese;counts",nbins,lower, upper);
  }


  std::string Name() const { return name_; }

  inline float GetPercentile(float q) { return static_cast<float>(spline_->Eval(q)); }

//  inline float GetPercentile(double q) { return static_cast<float>(spline_->Eval(q)); }

  void IntegrateHist();

  void FitWithSpline();

  void Fill( const Product & product) {if (product.validity) histo_->Fill(product.result);}

  TH1F GetHist() const {return *histo_;}

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
