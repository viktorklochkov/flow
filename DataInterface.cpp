//
// Created by Lukas Kreis on 09.08.17.
//

#include <iostream>
#include "DataInterface.h"
#include <array>
#include <ReducedEvent/AliReducedFMDInfo.h>
#include <THnSparse.h>

#define VAR AliReducedVarManager

namespace Qn {
namespace Interface {
void SetVariables(std::vector<VAR::Variables> vars) {
  for (auto var : vars) {
    AliReducedVarManager::SetUseVariable(var);
  }
}

void FillTpc(std::unique_ptr<Qn::DataContainerDataVector> &datacontainer,
             AliReducedEventInfo &event,
             TList &histograms,
             Fill fillhistograms) {
  auto values = new float[AliReducedVarManager::Variables::kNVars];
  AliReducedTrackInfo *track = nullptr;
  auto trackList = event.GetTracks();
  TIter next(trackList);
  next.Reset();
  auto *h_track_qa = (THnSparseF *) histograms.FindObject("trackqa");
  while ((track = (AliReducedTrackInfo *) next()) != nullptr) {
    if (!track->TestQualityFlag(15)) continue;
    VAR::FillTrackInfo(track, values);
    if (values[VAR::kEta] > 0.8 || values[VAR::kEta] < -0.8) continue;
    if (values[VAR::kPt] < 0.2 || values[VAR::kPt] > 5.0) continue;
    if (fillhistograms == Fill::QA) {
      if (h_track_qa) {
        const int ndims = 9;
        double trackparams[ndims] =
            {values[VAR::kPt], values[VAR::kEta], values[VAR::kPhi], values[VAR::kDcaXY], values[VAR::kDcaZ],
             values[VAR::kTPCsignal], values[VAR::kCharge], values[VAR::kTPCchi2]};
        h_track_qa->Fill(trackparams);
      }
    }
    values[-1] = 0;
    auto axes = datacontainer->GetAxes();
    std::vector<float> trackparams;
    trackparams.reserve(axes.size());
    for (const auto &axis : axes) {
      trackparams.push_back(values[axis.Id()]);
    }
    try {
      auto &element = datacontainer->ModifyElement(trackparams);
      element.emplace_back(values[VAR::kPhi], values[VAR::kPt]);
      datacontainer->CallOnElement(trackparams, [values](std::vector<DataVector> &vector) {
        vector.emplace_back(values[VAR::kPhi], values[VAR::kPt]);
      });

    }
    catch (std::exception &) {
      continue;
    }
  }
  delete[] values;
}

void FillVZEROA(std::unique_ptr<Qn::DataContainerDataVector> &datacontainer, AliReducedEventInfo &event) {
  const std::array<double, 8> X = {{0.92388, 0.38268, -0.38268, -0.92388, -0.92388, -0.38268, 0.38268, 0.92388}};
  const std::array<double, 8> Y = {{0.38268, 0.92388, 0.92388, 0.38268, -0.38268, -0.92388, -0.92388, -0.38268}};
  const std::array<float, 4> etaborders = {{4.8, 4.2, 3.65, 3.1}};
  for (int ich = 32; ich < 64; ich++) {
    double weight = event.MultChannelVZERO(ich);
    if (weight < 0.01) weight = 0.0;
    auto axes = datacontainer->GetAxes();
    std::vector<float> eta;
    eta.reserve(axes.size());
    for (const auto &axis : axes) {
      float etavalue = 0;
      if (axis.IsIntegrated()) {
        datacontainer->CallOnElement(std::vector<float>{0.5},
                                     [ich, Y, X, weight](std::vector<DataVector> &vector) {
                                       vector.emplace_back(TMath::ATan2(Y[ich], X[ich]), weight);
                                     });
      } else {
        etavalue = etaborders[(ich - 32) / 8];
        eta.push_back(etavalue);
        datacontainer->CallOnElement(eta,
                                     [ich, Y, X, weight](std::vector<DataVector> &vector) {
                                       vector.emplace_back(TMath::ATan2(Y[ich], X[ich]), weight);
                                     });
      }
    }
  }
}

void FillVZEROC(std::unique_ptr<Qn::DataContainerDataVector> &datacontainer, AliReducedEventInfo &event) {
  const std::array<double, 8> X = {{0.92388, 0.38268, -0.38268, -0.92388, -0.92388, -0.38268, 0.38268, 0.92388}};
  const std::array<double, 8> Y = {{0.38268, 0.92388, 0.92388, 0.38268, -0.38268, -0.92388, -0.92388, -0.38268}};
  const std::array<float, 4> etaborders = {{-3.45, -2.95, -2.45, -1.95}};
  for (int ich = 0; ich < 32; ich++) {
    double weight = event.MultChannelVZERO(ich);
    if (weight < 0.01) weight = 0.0;
    auto axes = datacontainer->GetAxes();
    std::vector<float> eta;
    eta.reserve(axes.size());
    for (const auto &axis : axes) {
      float etavalue = 0;
      if (axis.IsIntegrated()) {
        datacontainer->CallOnElement(std::vector<float>{0.5},
                                     [ich, Y, X, weight](std::vector<DataVector> &vector) {
                                       vector.emplace_back(TMath::ATan2(Y[ich], X[ich]), weight);
                                     });
      } else {
        etavalue = etaborders[ich / 8];
        eta.push_back(etavalue);
        datacontainer->CallOnElement(eta,
                                     [ich, Y, X, weight](std::vector<DataVector> &vector) {
                                       vector.emplace_back(TMath::ATan2(Y[ich], X[ich]), weight);
                                     });
      }
    }
  }
}

void FillFMDA(std::unique_ptr<Qn::DataContainerDataVector> &datacontainer, AliReducedEventInfo &event) {
  AliReducedFMDInfo *fmd = nullptr;
  auto fmdList = event.GetFMD();
  TIter next(fmdList);
  next.Reset();
  auto axes = datacontainer->GetAxes();
  while ((fmd = (AliReducedFMDInfo *) next()) != nullptr) {
    if (fmd->Id() < 1000) continue;
    for (const auto &axis : axes) {
      if (axis.IsIntegrated()) {
        datacontainer->CallOnElement(std::vector<float>{0.5},
                                     [fmd](std::vector<DataVector> &vector) {
                                       vector.emplace_back(fmd->Phi(), fmd->Multiplicity());
                                     });
      } else {
        std::vector<float> eta = {fmd->Eta()};
        datacontainer->CallOnElement(eta,
                                     [fmd](std::vector<DataVector> &vector) {
                                       vector.emplace_back(fmd->Phi(), fmd->Multiplicity());
                                     });
      }
    }
  }
}

void FillFMDC(std::unique_ptr<Qn::DataContainerDataVector> &datacontainer, AliReducedEventInfo &event) {
  AliReducedFMDInfo *fmd = nullptr;
  TClonesArray *fmdList = event.GetFMD();
  TIter next(fmdList);
  next.Reset();
  auto axes = datacontainer->GetAxes();
  while ((fmd = (AliReducedFMDInfo *) next()) != nullptr) {
    if (fmd->Id() > 1000) continue;
    for (const auto &axis : axes) {
      if (axis.IsIntegrated()) {
        datacontainer->CallOnElement(std::vector<float>{0.5},
                                     [fmd](std::vector<DataVector> &vector) {
                                       vector.emplace_back(fmd->Phi(), fmd->Multiplicity());
                                     });
      } else {
        std::vector<float> eta = {fmd->Eta()};
        datacontainer->CallOnElement(eta,
                                     [fmd](std::vector<DataVector> &vector) {
                                       vector.emplace_back(fmd->Phi(), fmd->Multiplicity());
                                     });
      }
    }
  }
}

void FillZDCA(std::unique_ptr<Qn::DataContainerDataVector> &datacontainer, AliReducedEventInfo &event) {
  const std::array<double, 10> X = {{0.0, -1.75, 1.75, -1.75, 1.75, 0.0, 1.75, -1.75, 1.75, -1.75}};
  const std::array<double, 10> Y = {{0.0, -1.75, -1.75, 1.75, 1.75, 0.0, -1.75, -1.75, 1.75, 1.75}};
  auto axes = datacontainer->GetAxes();
  for (u_short ich = 5; ich < 10; ich++) {
    for (const auto &axis : axes) {
      if (axis.IsIntegrated()) {
        double weight = event.EnergyZDCnTree(ich);
        if (weight < 0.01) weight = 0.;
        datacontainer->CallOnElement(std::vector<float>{0.5},
                                     [ich, Y, X, weight](std::vector<DataVector> &vector) {
                                       vector.emplace_back(TMath::ATan2(Y[ich], X[ich]), weight);
                                     });
      }
    }
  }
}

void FillZDCC(std::unique_ptr<Qn::DataContainerDataVector> &datacontainer, AliReducedEventInfo &event) {
  const std::array<double, 10> X = {{/* C*/ 0.0, -1.75, 1.75, -1.75, 1.75, /* A */0.0, 1.75, -1.75, 1.75, -1.75}};
  const std::array<double, 10> Y = {{/* C*/ 0.0, -1.75, -1.75, 1.75, 1.75, /*A*/0.0, -1.75, -1.75, 1.75, 1.75}};
  auto axes = datacontainer->GetAxes();
  for (u_short ich = 0; ich < 5; ich++) {
    for (const auto &axis : axes) {
      if (axis.IsIntegrated()) {
        double weight = event.EnergyZDCnTree(ich);
        if (weight < 0.01) weight = 0.;
        datacontainer->CallOnElement(std::vector<float>{0.5},
                                     [ich, Y, X, weight](std::vector<DataVector> &vector) {
                                       vector.emplace_back(TMath::ATan2(Y[ich], X[ich]), weight);
                                     });
      }
    }
  }
}

void FillDetectors(Qn::Internal::DetectorMap &map, AliReducedEventInfo &event, TList &histograms) {
  if (map.find((int) Configuration::DetectorId::ZDCA_reference) != map.end())
    FillZDCA(std::get<1>(map[(int) Configuration::DetectorId::ZDCA_reference]), event);

  if (map.find((int) Configuration::DetectorId::ZDCC_reference) != map.end())
    FillZDCC(std::get<1>(map[(int) Configuration::DetectorId::ZDCC_reference]), event);

  if (map.find((int) Configuration::DetectorId::FMDA_reference) != map.end())
    FillFMDA(std::get<1>(map[(int) Configuration::DetectorId::FMDA_reference]), event);

  if (map.find((int) Configuration::DetectorId::FMDC_reference) != map.end())
    FillFMDC(std::get<1>(map[(int) Configuration::DetectorId::FMDC_reference]), event);

  if (map.find((int) Configuration::DetectorId::TPC) != map.end())
    FillTpc(std::get<1>(map[(int) Configuration::DetectorId::TPC]), event, histograms, Fill::NO);

  if (map.find((int) Configuration::DetectorId::TPC_reference) != map.end())
    FillTpc(std::get<1>(map[(int) Configuration::DetectorId::TPC_reference]), event, histograms, Fill::QA);

  if (map.find((int) Configuration::DetectorId::VZEROA_reference) != map.end())
    FillVZEROA(std::get<1>(map[(int) Configuration::DetectorId::VZEROA_reference]), event);

  if (map.find((int) Configuration::DetectorId::VZEROC_reference) != map.end())
    FillVZEROC(std::get<1>(map[(int) Configuration::DetectorId::VZEROC_reference]), event);

  if (map.find((int) Configuration::DetectorId::VZEROA) != map.end())
    FillVZEROA(std::get<1>(map[(int) Configuration::DetectorId::VZEROA]), event);

  if (map.find((int) Configuration::DetectorId::VZEROC) != map.end())
    FillVZEROC(std::get<1>(map[(int) Configuration::DetectorId::VZEROC]), event);
}

}
}

