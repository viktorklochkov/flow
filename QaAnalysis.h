//
// Created by Lukas Kreis on 24.10.17.
//

#ifndef FLOW_QAANALYSIS_H
#define FLOW_QAANALYSIS_H

#include <TFile.h>
#include "DataContainer.h"
class QaAnalysis {
 public:
  QaAnalysis(TFile *file, std::string name) :
      name_(name),
      file_(file),
      trackhistograms_(new TList()),
      eventhistograms_(new TList()) {}
  void TrackQa();
  void EventQa();
 private:
  std::string name_;
  TFile *file_;
  std::unique_ptr<TList> trackhistograms_;
  std::unique_ptr<TList> eventhistograms_;


};

#endif //FLOW_QAANALYSIS_H
