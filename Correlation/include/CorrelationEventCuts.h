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

#ifndef QNEVENTCUTS_H
#define QNEVENTCUTS_H

#include <string>
#include "TTreeReaderValue.h"
#include "ROOT/RMakeUnique.hxx"
#include "ROOT/RIntegerSequence.hxx"

#include "Cuts.h"

namespace Qn {
class CorrelationEventCuts {
 public:
  void AddCut(std::unique_ptr<CutBase> cut) {
    cuts_.push_back(std::move(cut));
  }
  bool CheckCuts() {
    bool passed = true;
    cut_report_->Fill(0);
    unsigned int ibin = 1;
    for (auto &cut : cuts_) {
      passed = cut->Check() && passed;
      if (passed) cut_report_->Fill(ibin);
      ++ibin;
    }
    return passed;
  }
  void CreateReport() {
    cut_report_ = new TH1D("CutReport", "cut report;cuts;number of events", cuts_.size() + 1, 0, cuts_.size() + 1);
    auto axis = cut_report_->GetXaxis();
    axis->SetBinLabel(1, "All Events");
    for (unsigned int i = 0; i < cuts_.size(); ++i) {
      axis->SetBinLabel(i+2, cuts_.at(i)->Name().data());
    }
  }
  TH1D* GetReport() {
    return cut_report_;
  }

 private:
  std::vector<std::unique_ptr<CutBase>> cuts_;
  TH1D *cut_report_;
};

}

#endif