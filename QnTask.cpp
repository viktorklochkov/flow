//
// Created by Lukas Kreis on 29.06.17.
//

#include <iostream>
#include "QnTask.h"

QnTask::QnTask(std::unique_ptr<TTree> tree) :
    tree_(std::move(tree)),
    tree_reader_(tree_.get()),
    event_(tree_reader_, "event"),
    qn_manager_()
{
}

void QnTask::Run() {
  Initialize();
  std::cout << "Processing" << std::endl;
  std::cout << "----------" << std::endl;
  while (tree_reader_.Next()) {
    Process();
  }
  Finalize();
}

void QnTask::Initialize() {
  std::cout << "Initializing" << std::endl;
  std::cout << "------------" << std::endl;
  qn_manager_.SetShouldFillQAHistograms();
  qn_manager_.SetShouldFillOutputHistograms();
  qn_manager_.SetCurrentProcessListName("process");
  qn_manager_.InitializeQnCorrectionsFramework();
}
void QnTask::Process() {
  qn_manager_.ClearEvent();
  qn_manager_.ProcessEvent();
}
void QnTask::Finalize() {
  std::cout<< "Finalizing" << std::endl;
  std::cout << "---------" << std::endl;
}
