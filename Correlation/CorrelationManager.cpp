//
// Created by Lukas Kreis on 15.12.17.
//

#include "CorrelationManager.h"
#include <TFile.h>

namespace Qn {

void CorrelationManager::SaveToFile(std::string name) {
  auto outputfile = TFile::Open(name.data(), "RECREATE");
  for (const auto &correlation : correlations_) {
    (*correlation.second.GetResult()).Write(correlation.first.data());
  }
  outputfile->Close();
}


}