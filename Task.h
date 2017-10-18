//
// Created by Lukas Kreis on 29.06.17.
//

#ifndef FLOW_QNTASK_H
#define FLOW_QNTASK_H

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
#include "DataContainer.h"
#include "EventInfo.h"
#include "ReducedEvent/AliReducedEventInfo.h"
#include "ReducedEvent/AliReducedVarManager.h"
#include "ReducedEvent/AliReducedTrackInfo.h"
#include "ReducedEvent/AliReducedBaseTrack.h"
#include "DetectorMap.h"

#define VAR AliReducedVarManager

namespace Qn {
/**
 * Qn vector analysis task. It is to be configured by the user.
 * @brief Task for analysing qn vectors
 */
class Task {
 public:
  Task() = default;
  Task(std::string filelist, std::string incalib, std::string treename);
  Task(std::array<std::shared_ptr<TFile>, 4> files);
  ~Task() = default;
  void Run();

 private:
  /**
   * Initializes task;
   */
  virtual void Initialize();
  /**
   * Processes one event;
   */
  virtual void Process();
  /**
   * Finalizes task. Called after processing is done.
   */
  virtual void Finalize();
  /**
   * Make TChain from file list
   * @param filename name of file containing paths to root files containing the input trees
   * @return Pointer to the TChain
   */
  virtual std::unique_ptr<TChain> MakeChain(std::string filename, std::string treename);

 protected:
  std::shared_ptr<TFile> out_file_;
  std::shared_ptr<TFile> in_calibration_file_;
  std::shared_ptr<TFile> out_calibration_file_;
  std::unique_ptr<TTree> in_tree_;
  std::unique_ptr<TTree> out_tree_;
  std::unique_ptr<TTree> out_tree_raw;
  TTreeReader tree_reader_;
  TTreeReaderValue<AliReducedEventInfo> event_;
  Qn::Internal::DetectorMap raw_data_;
  std::map<int, std::unique_ptr<Qn::DataContainerQn>> qn_data_;
  std::unique_ptr<Qn::EventInfoF> qn_eventinfo_f_;
  QnCorrectionsManager qn_manager_;
  std::mt19937 eng;
  std::uniform_real_distribution<float> rnd;
  bool write_tree_;

};
}
#endif //FLOW_QNTASK_H
