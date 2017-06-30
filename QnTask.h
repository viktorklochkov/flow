//
// Created by Lukas Kreis on 29.06.17.
//

#ifndef FLOW_QNTASK_H
#define FLOW_QNTASK_H

#include <vector>
#include <TTreeReader.h>
#include <QnCorrectionsManager.h>
#include "TTree.h"
#include "QnDataContainer.h"
#include "AliReducedEventInfo.h"

class TTreeReader;

class QnTask {
 public:
  QnTask() = default;
  QnTask(std::unique_ptr<TTree> tree);
  ~QnTask() = default;
  void Run();
 private:
  /**
   * Initializes task;
   */
  void Initialize();
  /**
   * Processes one event;
   */
  void Process();
  /**
   * Finalizes task. Called after processing is done.
   */
  void Finalize();

 private:
  std::unique_ptr<TTree> tree_;
  TTreeReader tree_reader_;
  TTreeReaderValue<AliReducedEventInfo> event_;
  QnCorrectionsManager qn_manager_;
};

#endif //FLOW_QNTASK_H
