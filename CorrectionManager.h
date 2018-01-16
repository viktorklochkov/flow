//
// Created by Lukas Kreis on 16.01.18.
//

#ifndef FLOW_CORRECTIONMANAGER_H
#define FLOW_CORRECTIONMANAGER_H

#include <string>
#include <map>
#include <QnCorrections/QnCorrectionsManager.h>
#include "Detector.h"
#include "VariableManager.h"
#include "Axis.h"
namespace Qn {
class CorrectionManager {

  void AddVariable(const std::string &name,const int number) {var_manager_.AddVariable(name, number);}

  void AddCorrectionAxis(Qn::Axis variable) {qncorrections_axis_.push_back(variable);}
  

  void AddDetector();



 private:
  std::vector<Qn::Axis> qncorrections_axis_;
  QnCorrectionsManager qncorrections_manager;
  VariableManager var_manager_;
  std::map<std::string, Detector> detectors_;
};
}

#endif //FLOW_CORRECTIONMANAGER_H
