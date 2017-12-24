//
// Created by Lukas Kreis on 13.11.17.
//

#ifndef FLOW_SIMPLETASK_H
#define FLOW_SIMPLETASK_H

#include <TChain.h>
#include <TTreeReader.h>
#include "DataContainer.h"
#include "Correlation.h"

#define VAR AliReducedVarManager

class SimpleTask {
 public:
  SimpleTask() = default;
  SimpleTask(std::string filelist, std::string treename);

  void Run();

 private:
  std::unique_ptr<TTree> in_tree_;
  std::shared_ptr<TTreeReader> reader_;

  /**
   * Make TChain from file list
   * @param filename name of file containing paths to root files containing the input trees
   * @return Pointer to the TChain
   */
  std::unique_ptr<TChain> MakeChain(std::string filename, std::string treename);
};

#endif //FLOW_SIMPLETASK_H
