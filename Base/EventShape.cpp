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

#include "EventShape.h"

void Qn::EventShape::IntegrateHist() {
  integral_ = (TH1F *) histo_->Clone("integral");
  for (int i = 0; i < histo_->GetNbinsX()+1; ++i) {
    double inte = histo_->Integral(0, i)/histo_->Integral();
    integral_->SetBinContent(i, inte);
  }
}

void Qn::EventShape::FitWithSpline(TH1F hist) {
  IntegrateHist();
  spline_ = new TSpline3(integral_, "sp3");
  spline_->SetName("spline");
}