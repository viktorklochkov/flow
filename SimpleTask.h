//
// Created by Lukas Kreis on 13.11.17.
//

#ifndef FLOW_SIMPLETASK_H
#define FLOW_SIMPLETASK_H

#include <TChain.h>
#include <TTreeReader.h>
#include "DataContainer.h"
class SimpleTask {
 public:
  SimpleTask() = default;
  SimpleTask(std::string filelist, std::string treename);

  void AddDataContainer(std::string name);
  void AddEventVariable(std::string name);
  void Run();

 private:
  void Initialize();
  void Process();
  std::unique_ptr<TTree> in_tree_;
  TTreeReader reader_;
  std::map<std::string, std::shared_ptr<TTreeReaderValue<Qn::DataContainerQVector>>> values_;
  std::map<std::string, TTreeReaderValue<float>> eventvalues_;
  std::map<std::string,
           std::tuple<Qn::DataContainerVF,
                      std::shared_ptr<TTreeReaderValue<Qn::DataContainerQVector>>,
                      std::shared_ptr<TTreeReaderValue<Qn::DataContainerQVector>>
           >
  > correlations_;
  std::vector<Qn::Axis> eventaxes_;
  /**
   * Make TChain from file list
   * @param filename name of file containing paths to root files containing the input trees
   * @return Pointer to the TChain
   */
  std::unique_ptr<TChain> MakeChain(std::string filename, std::string treename);
};

#endif //FLOW_SIMPLETASK_H
