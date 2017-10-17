//
// Created by Lukas Kreis on 13.10.17.
//

#ifndef FLOW_QUICKTASK_H
#define FLOW_QUICKTASK_H

#include "Task.h"

namespace Qn {
class QuickTask : private Task {
 public:
 private:
  QuickTask(std::string filelist, std::string incalib, std::string treename);
  /**
    * Initializes task;
    */
  void Initialize() override;
  /**
   * Processes one event;
   */
  void Process() override;

};
};

#endif //FLOW_QUICKTASK_H
