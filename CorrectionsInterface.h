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

namespace Internal {
QnCorrectionsDetectorConfigurationTracks *SetDetectorConfiguration(std::string name,
                                                                   QnCorrectionsDetector *detector);
}

enum class DetectorId : int {
  TPC,
  VZERO,
  TZERO,
  ZDC,
  FMD
};

void SetVariables(std::array<VAR::Variables, VAR::kNVars> variables);

void FillTZERO(QnCorrectionsManager &manager, AliReducedEventInfo &event);

void FillFMD(QnCorrectionsManager &manager, AliReducedEventInfo &event);

void FillZDC(QnCorrectionsManager &manager, AliReducedEventInfo &event);

void FillTPC(QnCorrectionsManager &manager, AliReducedEventInfo &event);

void FillVZERO(QnCorrectionsManager &manager, AliReducedEventInfo &event);

void ConfigureBins(QnCorrectionsManager &manager,
                   std::unique_ptr<DataContainerQn> const &data,
                   DetectorId id);

void FillTree(QnCorrectionsManager &manager, std::unique_ptr<DataContainerQn> const &data,
              DetectorId id);

void FillData(QnCorrectionsManager &manager, AliReducedEventInfo &event);


//};
}
#endif //FLOW_CORRECTIONSINTERFACE_H
