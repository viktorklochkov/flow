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

#include "EseHandler.h"
#include "CorrelationManager.h"

void Qn::EseHandler::AddESE(const std::string &name, const std::vector<std::string> &input,
                            Correlation::function_t lambda, const TH1F &histo) {
  subevents_.emplace_back(this, name, input, lambda, histo);
}

/**
 * Sets inputs. Root file is read when it is found in the path.
 * @param percentiles_name full path of the input percentiles tree.
 * @param calib_filename full path of the calibration file.
 */
void Qn::EseHandler::SetInput(std::string percentiles_name, std::string calib_filename) {
  // gSystem->AccessPathName() has bizarre convention returns false if file exists.
  if (!gSystem->AccessPathName(calib_filename.data(), kFileExists)) {
    input_file_ = std::make_shared<TFile>(calib_filename.data(), "READ");
    if (input_file_->IsZombie()) input_file_ = nullptr;
  }
  if (!gSystem->AccessPathName(percentiles_name.data(), kFileExists)) {
    input_treefile_ = std::make_shared<TFile>(percentiles_name.data(), "READ");
    manager_->AddFriend("ESE", input_treefile_.get());
    if (input_treefile_->IsZombie()) input_treefile_ = nullptr;
  }
}

Qn::Correlation *Qn::EseHandler::RequestCorrelation(const Qn::SubEventPrototype &prototype) {
  return manager_->RegisterCorrelation(prototype.name, prototype.input, prototype.lambda, prototype.weights);
}

void Qn::EseHandler::RequestEventAxis(const Qn::Axis &axis) { manager_->EventAxis(axis); }