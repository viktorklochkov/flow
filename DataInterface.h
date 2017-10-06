//
// Created by Lukas Kreis on 09.08.17.
//

#ifndef FLOW_DATAINTERFACE_H
#define FLOW_DATAINTERFACE_H

#include <map>

#include <ReducedEvent/AliReducedTrackInfo.h>
#include <ReducedEvent/AliReducedEventInfo.h>
#include <ReducedEvent/AliReducedVarManager.h>

#include "QnCorrections/QnCorrectionsManager.h"
//#include "CorrectionInterface.h"
#include "DataContainer.h"
#include "DetectorMap.h"

#define VAR AliReducedVarManager

namespace Qn {
/**
 * Function used to interface to the user data files.
 */
namespace Interface {
/**
 * Set variables used in the ALICE reduced tree interface
 * @param vars vector of variables to be used
 */
void SetVariables(std::vector<VAR::Variables> vars);
/**
 * Fills tpc data into the datacontainer
 * @param datacontainer datacontainer with configured binning
 * @param event ALICE event data
 */
void FillTpc(std::unique_ptr<Qn::DataContainerDataVector> &datacontainer, AliReducedEventInfo &event);
//
void FillVZEROA(std::unique_ptr<Qn::DataContainerDataVector> &datacontainer, AliReducedEventInfo &event);

void FillVZEROC(std::unique_ptr<Qn::DataContainerDataVector> &datacontainer, AliReducedEventInfo &event);

void FillFMDA(std::unique_ptr<Qn::DataContainerDataVector> &datacontainer, AliReducedEventInfo &event);

void FillFMDC(std::unique_ptr<Qn::DataContainerDataVector> &datacontainer, AliReducedEventInfo &event);


void FillDetectors(Qn::Internal::DetectorMap &map, AliReducedEventInfo &event);
}
}

#endif //FLOW_DATAINTERFACE_H
