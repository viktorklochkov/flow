//
// Created by Lukas Kreis on 03.08.17.
//

#ifndef FLOW_CORRECTIONINTERFACE_H
#define FLOW_CORRECTIONINTERFACE_H
#include <QnCorrections/QnCorrectionsManager.h>
#include "DataContainer.h"
#include "DetectorGenerator.h"
#include <iostream>
namespace Qn {
namespace Internal {
void FillDataToFramework(QnCorrectionsManager &manager, std::map<int, Qn::DataContainerDataVector> &pairs);
void AddDetectorToFramework(QnCorrectionsManager &manager,
                            DetectorType type,
                            std::map<int, Qn::DataContainerDataVector> &pairs);
void GetQnFromFramework(QnCorrectionsManager &manager, std::map<int, std::unique_ptr<Qn::DataContainerQn>> &pairs);
void SaveToTree(TTree &tree, std::map<int, std::unique_ptr<Qn::DataContainerQn>> &pairs);
}
}

#endif //FLOW_CORRECTIONINTERFACE_H
