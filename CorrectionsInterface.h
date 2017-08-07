//
// Created by Lukas Kreis on 18.07.17.
//

#ifndef FLOW_CORRECTIONSINTERFACE_H
#define FLOW_CORRECTIONSINTERFACE_H

#include <iostream>
#include <array>
#include <QnCorrections/QnCorrectionsCutAbove.h>
#include <QnCorrections/QnCorrectionsCutBelow.h>
#include <QnCorrections/QnCorrectionsDetectorConfigurationTracks.h>
#include <QnCorrections/QnCorrectionsProfile3DCorrelations.h>
#include <QnCorrections/QnCorrectionsQnVectorRecentering.h>
#include <QnCorrections/QnCorrectionsQnVectorTwistAndRescale.h>
#include <QnCorrections/QnCorrectionsCutWithin.h>

#include "AliReducedVarManager.h"
#include "QnCorrectionsManager.h"
#include "AliReducedEventInfo.h"
#include "AliReducedBaseTrack.h"
#include "AliReducedTrackInfo.h"
#include "AliReducedFMDInfo.h"
#include "DataContainer.h"

#define VAR AliReducedVarManager

namespace Qn {
/**
 * Internal functions
 */
namespace Internal {
/**
 * Set detector configuration for QnCorrectionsFramework
 * @param name name of detector[bin]
 * @return Returns the detector configuration
 */
QnCorrectionsDetectorConfigurationTracks *SetDetectorConfiguration(std::string name);
}

enum class DetectorId : int {
  TPC,
  VZERO,
  TZERO,
  ZDC,
  FMD
};

/**
 * Activate variables in AliReducedTree
 * @param variables variables to be used in analysis
 */
void SetVariables(std::array<VAR::Variables, VAR::kNVars> variables);
/**
 * Fills TZERO reduced tree information to the QnCorrectionsManager
 * @param manager Manager to fill information to
 * @param event Information to be filled in the manager
 */
void FillTZERO(QnCorrectionsManager &manager, AliReducedEventInfo &event);
/**
 * Fills FMD reduced tree information to the QnCorrectionsManager
 * @param manager Manager to fill information to
 * @param event Information to be filled in the manager
 */
void FillFMD(QnCorrectionsManager &manager, AliReducedEventInfo &event);
/**
 * Fills ZDC reduced tree information to the QnCorrectionsManager
 * @param manager Manager to fill information to
 * @param event Information to be filled in the manager
 */
void FillZDC(QnCorrectionsManager &manager, AliReducedEventInfo &event);
/**
 * Fills TPC reduced tree information to the QnCorrectionsManager
 * @param manager Manager to fill information to
 * @param event Information to be filled in the manager
 */
void FillTPC(QnCorrectionsManager &manager, AliReducedEventInfo &event);
/**
 * Fills VZERO reduced tree information to the QnCorrectionsManager
 * @param manager Manager to fill information to
 * @param event Information to be filled in the manager
 */
void FillVZERO(QnCorrectionsManager &manager, AliReducedEventInfo &event);
/**
 * Configures bins of a given detector
 * @param manager Manager used for correction
 * @param data binning of data
 * @param id Id of detector
 * @param name name of detector
 * @param cutvariables variables used for the definition of a data bin
 */
void ConfigureBins(QnCorrectionsManager &manager,
                   DataContainerQn &data,
                   DetectorId id, std::string name, std::vector<int> cutvariables);
/**
 * Fills corrected qn vectors of a given detector to the corresponding bins in the data container. Only fills good qn.
 * @param manager Manager which contains the corrected qn vectors
 * @param data Datacontainer which saves the data
 * @param id Id of detector
 */
void FillTree(QnCorrectionsManager &manager, DataContainerQn &data,
              DetectorId id);
/**
 * Fills data for the used detectors to the QnCorrectionsManager
 * @param manager manager used for the corrections
 * @param event Event information to be corrected
 */
void FillData(QnCorrectionsManager &manager, AliReducedEventInfo &event);


//};
}
#endif //FLOW_CORRECTIONSINTERFACE_H
