//
// Created by Lukas Kreis on 29.06.17.
//

#ifndef FLOW_QNTestTask_H
#define FLOW_QNTestTask_H

#include <vector>
#include <array>
#include <random>
#include "QnCorrections/QnCorrectionsProfile3DCorrelations.h"
#include "QnCorrections/QnCorrectionsProfileCorrelationComponents.h"
#include "QnCorrections/QnCorrectionsDetectorConfigurationChannels.h"
#include "QnCorrections/QnCorrectionsDetectorConfigurationBase.h"
#include <QnCorrections/QnCorrectionsDetectorConfigurationTracks.h>
#include <QnCorrections/QnCorrectionsQnVectorRecentering.h>
#include <QnCorrections/QnCorrectionsQnVectorTwistAndRescale.h>
#include <QnCorrections/QnCorrectionsCutSetBit.h>
#include <QnCorrections/QnCorrectionsCutWithin.h>
#include <QnCorrections/QnCorrectionsInputGainEqualization.h>
#include <QnCorrections/QnCorrectionsQnVectorAlignment.h>
#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"

#include "QnCorrections/QnCorrectionsManager.h"
#include "Base/DataContainer.h"
#include "DifferentialCorrection/EventInfo.h"
#include "ReducedEvent/AliReducedEventInfo.h"
#include "ReducedEvent/AliReducedVarManager.h"
#include "ReducedEvent/AliReducedTrackInfo.h"
#include "ReducedEvent/AliReducedBaseTrack.h"
#include "DifferentialCorrection/CorrectionManager.h"

#define VAR AliReducedVarManager

namespace Qn {
/**
 * Qn vector analysis TestTask. It is to be configured by the user.
 * @brief TestTask for analysing qn vectors
 */
class CorrectionTask {
 public:
  CorrectionTask() = default;
  CorrectionTask(std::string filelist, std::string incalib, std::string treename);
  CorrectionTask(std::array<std::shared_ptr<TFile>, 4> files);
  ~CorrectionTask() = default;
  void Run();

 private:
  /**
   * Initializes TestTask;
   */
  virtual void Initialize();
  /**
   * Processes one event;
   */
  virtual void Process();
  /**
   * Finalizes TestTask. Called after processing is done.
   */
  virtual void Finalize();
  /**
   * Make TChain from file list
   * @param filename name of file containing paths to root files containing the input trees
   * @return Pointer to the TChain
   */
  virtual std::unique_ptr<TChain> MakeChain(std::string filename, std::string treename);

  void SetVariables(std::vector<VAR::Variables> vars) {
    for (auto var : vars) {
      AliReducedVarManager::SetUseVariable(var);
    }
  }
 protected:
  std::shared_ptr<TFile> out_file_;
  std::shared_ptr<TFile> in_calibration_file_;
  std::shared_ptr<TFile> out_calibration_file_;
  std::unique_ptr<TTree> in_tree_;
  TTree *out_tree_;
  TTreeReader tree_reader_;
  TTreeReaderValue<AliReducedEventInfo> event_;
  Qn::CorrectionManager manager_;
  bool write_tree_;

};
}
#endif //FLOW_QNTASK_H
