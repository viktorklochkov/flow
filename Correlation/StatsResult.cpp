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

#include "StatsResult.h"

namespace Qn {

void StatsResult::Fill(const size_type event_id) {
  // Fill result to the event average statistic DataContainer.
     const auto &current_event_result = correlation_current_event->GetResult();
    if (use_resampling_) {
      unsigned int ibin = 0;
      for (auto &bin : result_) {
        bin.Fill(current_event_result.At(ibin), resampler_->GetFillVector(event_id));
        ++ibin;
      }
    } else {
      unsigned int ibin = 0;
      for (auto &bin : result_) {
        bin.Fill(current_event_result.At(ibin), {});
        ++ibin;
      }
    }
  }

void StatsResult::ConfigureStats(Qn::Sampler *sampler) {
  const auto &current_event_result = correlation_current_event->GetResult();
  // configure the result datacontainer
  result_.AddAxes(current_event_result.GetAxes());
  // configure weights
  if (correlation_current_event->UsingWeights()) {
    std::for_each(result_.begin(), result_.end(), [](Qn::Stats &stats) { stats.SetWeights(Stats::Weights::OBSERVABLE); });
  } else {
    std::for_each(result_.begin(), result_.end(), [](Qn::Stats &stats) { stats.SetWeights(Stats::Weights::REFERENCE); });
  }
  // configure sampler
  if (use_resampling_) {
    if (sampler) {
      resampler_ = sampler;
      for (auto &bin :result_) {
        bin.SetNumberOfReSamples(resampler_->GetNumSamples());
      }
    }
    else {
      use_resampling_ = false;
      throw NoResamplerException();
    }
  }
}

}