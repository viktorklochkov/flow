//
// Created by Lukas Kreis on 13.10.17.
//

#include "QuickTask.h"
#include "DataInterface.h"
#include "CorrectionInterface.h"

namespace Qn {

QuickTask::QuickTask(std::string filelist, std::string incalib, std::string treename) :
  Task(filelist, incalib, treename) {

}

void QuickTask::Process() {

}

void QuickTask::Initialize() {
//  using Axes = std::vector<Qn::Axis>;
//  Qn::Interface::SetVariables({VAR::Variables::kVtxZ, VAR::Variables::kPt, VAR::Variables::kEta, VAR::Variables::kP,
//                               VAR::Variables::kPhi});
//
//  Axis ptaxis("Pt", 1, 0, 3, VAR::kPt);
//  Axis etaaxis("Eta", 1, -0.8, 0.8, VAR::kEta);
//  Axis vzerorings("EtaRings", {-3.7, -3.2, -2.7, -2.2, -1.7, 2.8, 3.4, 3.9, 4.5, 5.1}, VAR::kEta);
//  Axis vzeroringsA("EtaRings", {2.8, 3.4, 3.9, 4.5, 5.1}, VAR::kEta);
//  Axis vzeroringsC("EtaRings", {-3.7, -3.2, -2.7, -2.2, -1.7}, VAR::kEta);
//
//  Axes tpcaxes = {ptaxis, etaaxis};
//  Axes vzeroaxes = {vzerorings};
//  Axes vzeroaxesA = {vzeroringsA};
//  Axes vzeroaxesC = {vzeroringsC};
//
//  Qn::Internal::AddDetectorToMap(raw_data_,
//                                 Configuration::DetectorId::TPC_reference,
//                                 new Configuration::TPC(),
//                                 Configuration::DetectorType::Track
//  );
//  Qn::Internal::AddDetectorToMap(raw_data_,
//                                 Configuration::DetectorId::TPC,
//                                 new Configuration::TPC(),
//                                 Configuration::DetectorType::Track,
//                                 tpcaxes
//  );
//  Qn::Internal::AddDetectorToMap(raw_data_,
//                                 Configuration::DetectorId::VZEROA,
//                                 new Configuration::VZEROA(),
//                                 Configuration::DetectorType::Channel,
//                                 vzeroaxesA,
//                                 64);
//  Qn::Internal::AddDetectorToMap(raw_data_,
//                                 Configuration::DetectorId::VZEROC,
//                                 new Configuration::VZEROC(),
//                                 Configuration::DetectorType::Channel,
//                                 vzeroaxesC,
//                                 64);
//  Qn::Internal::AddDetectorToMap(raw_data_,
//                                 Configuration::DetectorId::VZEROA_reference,
//                                 new Configuration::VZEROA_reference(),
//                                 Configuration::DetectorType::Channel,
//                                 64);
//  Qn::Internal::AddDetectorToMap(raw_data_,
//                                 Configuration::DetectorId::VZEROC_reference,
//                                 new Configuration::VZEROC_reference(),
//                                 Configuration::DetectorType::Channel,
//                                 64);
//  Qn::Internal::AddDetectorToMap(raw_data_,
//                                 Configuration::DetectorId::FMDC_reference,
//                                 new Configuration::FMDC_reference(),
//                                 Configuration::DetectorType::Channel,
//                                 2000);
//  Qn::Internal::AddDetectorToMap(raw_data_,
//                                 Configuration::DetectorId::FMDA_reference,
//                                 new Configuration::FMDA_reference(),
//                                 Configuration::DetectorType::Channel,
//                                 2000);
//  Qn::Internal::AddDetectorToMap(raw_data_,
//                                 Configuration::DetectorId::ZDCA_reference,
//                                 new Configuration::ZDCA_reference(),
//                                 Configuration::DetectorType::Channel,
//                                 5);
//  Qn::Internal::AddDetectorToMap(raw_data_,
//                                 Configuration::DetectorId::ZDCC_reference,
//                                 new Configuration::ZDCC_reference(),
//                                 Configuration::DetectorType::Channel,
//                                 5);
//
//  qn_data_ = Qn::Internal::MakeQnDataContainer(raw_data_);
//
//  auto eventset = new QnCorrectionsEventClassVariablesSet(2);
//  double centbins[][2] = {{0.0, 2}, {100.0, 100}};
//  double vtxbins[][2] = {{-10.0, 4}, {-7.0, 1}, {7.0, 8}, {10.0, 1}};
//  eventset->Add(new QnCorrectionsEventClassVariable(VAR::kCentVZERO, "Centrality", centbins));
//  eventset->Add(new QnCorrectionsEventClassVariable(VAR::kVtxZ, "z vertex", vtxbins));
//
//  Qn::Internal::AddDetectorsToFramework(qn_manager_, raw_data_, *eventset);
//  Qn::Internal::SaveToTree(*out_tree_, qn_data_);
//  qn_eventinfo_f_->AddVariable("Centrality");
//  qn_eventinfo_f_->AddVariable("VtxZ");
//  Qn::Internal::SaveToTree(*out_tree_, qn_eventinfo_f_);
//  in_tree_->SetImplicitMT(true);
//  qn_manager_.SetCalibrationHistogramsList(in_calibration_file_.get());
//  qn_manager_.SetShouldFillQAHistograms();
//  qn_manager_.SetShouldFillOutputHistograms();
//  qn_manager_.InitializeQnCorrectionsFramework();
//  qn_manager_.SetCurrentProcessListName("test");
}

//QuickTask


}