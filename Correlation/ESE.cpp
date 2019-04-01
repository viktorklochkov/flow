// Flow std::vector Correction Framework
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

#include <include/ESE.h>

#include "ESE.h"
#include "CorrelationManager.h"

void Qn::ESE::CheckInputQVectors() {
  std::vector<std::string> not_found_names;
  for (const auto &corr : correlations_) {
    for (const auto &name :  corr.second.GetInputNames()) {
      if (qvectors_->find(name)==qvectors_->end()) {
        main_manager_->AddQVectors({name});
      }
    }
  }
}

void Qn::ESE::AddESE(const std::string &name,
                     const std::vector<std::string> &input,
                     Qn::Correlation::function_type &&lambda,
                     const TH1F &histo) {
  std::vector<Qn::Weight> use_weights(input.size());
  for_each(use_weights.begin(),use_weights.end(),[](Weight &a){a=Qn::Weight::REFERENCE;});
  main_manager_->AddCorrelationOnly(name,input,lambda,use_weights);
  es_results.emplace(name,std::make_unique<EventShapeResult>(nullptr,histo));
//    Qn::CorrelationESE correlation(name, input, lambda, histo);
//    correlations_.emplace(name, correlation);
}
}