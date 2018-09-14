// Flow Vector Correction Framework
//
// Copyright (C) 2018  Lukas Kreis, Ilya Selyuzhenkov
// Contact: l.kreis@gsi.de; ilya.selyuzhenkov@gmail.com
// For a full list of contributors please see docs/Credits
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "DataFiller.h"
//void Qn::Differential::Interface::DataFiller::FillTPCR(Qn::Detector &detector, AliReducedEventInfo &event) {
//  auto values = new float[AliReducedVarManager::Variables::kNVars];
//  AliReducedTrackInfo *track = nullptr;
//  auto trackList = event.GetTracks();
//  TIter next(trackList);
//  next.Reset();
//  auto &datacontainer = detector.GetDataContainer();
//  long ntracks = trackList->GetSize();
//  std::for_each(datacontainer->begin(),
//                datacontainer->end(),
//                [ntracks](std::vector<DataVector> &vector) { vector.reserve(ntracks); });
//  while ((track = (AliReducedTrackInfo *) next())!=nullptr) {
//    if (!(track->TestQualityFlag(23) || track->TestQualityFlag(24))) continue;
////    if (!(track->TestQualityFlag(15))) continue;
//    VAR::FillTrackInfo(track, values);
////    if (values[VAR::kTPCncls] < 70.) continue;
//    if (values[VAR::kEta] > 0.8 || values[VAR::kEta] < -0.8) continue;
//    if (values[VAR::kPt] < 0.2 || values[VAR::kPt] > 10.0) continue;
//    datacontainer->CallOnElement([values](std::vector<DataVector> &vector) {
//      vector.emplace_back(values[VAR::kPhi]);
//    });
//  }
//  delete[] values;
//}
//void Qn::Differential::Interface::DataFiller::FillTPCHybrid(Qn::Detector &detector, AliReducedEventInfo &event) {
//  auto values = new float[AliReducedVarManager::Variables::kNVars];
//  AliReducedTrackInfo *track = nullptr;
//  auto trackList = event.GetTracks();
//  TIter next(trackList);
//  next.Reset();
//  auto &datacontainer = detector.GetDataContainer();
//  auto &axes = datacontainer->GetAxes();
//  std::vector<float> trackparams;
//  trackparams.reserve(axes.size());
//  long ntracks = trackList->GetSize();
//  std::for_each(datacontainer->begin(),
//                datacontainer->end(),
//                [ntracks](std::vector<DataVector> &vector) { vector.reserve(ntracks); });
//  while ((track = (AliReducedTrackInfo *) next())!=nullptr) {
//    if (!(track->TestQualityFlag(23) || track->TestQualityFlag(24))) continue;
//    VAR::FillTrackInfo(track, values);
////    if (values[VAR::kTPCncls] < 70.) continue;
//    if (values[VAR::kEta] > 0.8 || values[VAR::kEta] < -0.8) continue;
//    if (values[VAR::kPt] < 0.2 || values[VAR::kPt] > 10.0) continue;
//    for (const auto num : detector.GetEnums()) {
//      trackparams.push_back(values[num]);
//    }
//    try {
//      datacontainer->CallOnElement(trackparams, [values](std::vector<DataVector> &vector) {
//        vector.emplace_back(values[VAR::kPhi]);
//      });
//    }
//    catch (std::exception &) {
//      continue;
//    }
//    trackparams.clear();
//  }
//  delete[] values;
//}
//void Qn::Differential::Interface::DataFiller::FillTPC(Qn::Detector &detector, AliReducedEventInfo &event) {
//  auto values = new float[AliReducedVarManager::Variables::kNVars];
//  AliReducedTrackInfo *track = nullptr;
//  auto trackList = event.GetTracks();
//  TIter next(trackList);
//  next.Reset();
//  auto &datacontainer = detector.GetDataContainer();
//  auto &axes = datacontainer->GetAxes();
//  std::vector<float> trackparams;
//  trackparams.reserve(axes.size());
//  long ntracks = trackList->GetSize();
//  std::for_each(datacontainer->begin(),
//                datacontainer->end(),
//                [ntracks](std::vector<DataVector> &vector) { vector.reserve(ntracks); });
//  while ((track = (AliReducedTrackInfo *) next())!=nullptr) {
//    if (!(track->TestQualityFlag(23) || track->TestQualityFlag(24))) continue;
////    if (!(track->TestQualityFlag(15))) continue;
//    VAR::FillTrackInfo(track, values);
////    if (values[VAR::kTPCncls] < 70.) continue;
//    if (values[VAR::kEta] > 0.8 || values[VAR::kEta] < -0.8) continue;
//    if (values[VAR::kPt] < 0.2 || values[VAR::kPt] > 10.0) continue;
//    for (const auto num : detector.GetEnums()) {
//      trackparams.push_back(values[num]);
//    }
//    try {
//      datacontainer->CallOnElement(trackparams, [values](std::vector<DataVector> &vector) {
//        vector.emplace_back(values[VAR::kPhi]);
//      });
//    }
//    catch (std::exception &) {
//      continue;
//    }
//    trackparams.clear();
//  }
//  delete[] values;
//}
//void Qn::Differential::Interface::DataFiller::FillVZEROA(Qn::Detector &detector, AliReducedEventInfo &event) {
//  const std::array<double, 8> X = {{0.92388, 0.38268, -0.38268, -0.92388, -0.92388, -0.38268, 0.38268, 0.92388}};
//  const std::array<double, 8> Y = {{0.38268, 0.92388, 0.92388, 0.38268, -0.38268, -0.92388, -0.92388, -0.38268}};
//  for (int ich = 32; ich < 64; ich++) {
//    double weight = event.MultChannelVZERO(ich);
//    if (weight < 0.01) weight = 0.0;
//    detector.GetDataContainer()->CallOnElement([ich, Y, X, weight](std::vector<DataVector> &vector) {
//      vector.emplace_back(TMath::ATan2(Y[ich%8], X[ich%8]), weight);
//    });
//  }
//}
//void Qn::Differential::Interface::DataFiller::FillVZEROC(Qn::Detector &detector, AliReducedEventInfo &event) {
//  const std::array<double, 8> X = {{0.92388, 0.38268, -0.38268, -0.92388, -0.92388, -0.38268, 0.38268, 0.92388}};
//  const std::array<double, 8> Y = {{0.38268, 0.92388, 0.92388, 0.38268, -0.38268, -0.92388, -0.92388, -0.38268}};
//  for (int ich = 0; ich < 32; ich++) {
//    double weight = event.MultChannelVZERO(ich);
//    if (weight < 0.01) weight = 0.0;
//    detector.GetDataContainer()->CallOnElement([ich, Y, X, weight](std::vector<DataVector> &vector) {
//      vector.emplace_back(TMath::ATan2(Y[ich%8], X[ich%8]), weight);
//    });
//  }
//}
//void Qn::Differential::Interface::DataFiller::FillZDCC(Qn::Detector &detector, AliReducedEventInfo &event) {
//  const std::array<double, 4> X = {{-1.75, 1.75, -1.75, 1.75}};
//  const std::array<double, 4> Y = {{-1.75, -1.75, 1.75, 1.75}};
//  for (u_short ich = 0; ich < 4; ich++) {
//    double weight = event.EnergyZDCnTree(ich + 1);
//    if (weight > 100) {
//      detector.GetDataContainer()->CallOnElement([ich, Y, X, weight](std::vector<DataVector> &vector) {
//        vector.emplace_back(TMath::ATan2(Y[ich], X[ich]), weight);
//      });
//    }
//  }
//}
//void Qn::Differential::Interface::DataFiller::FillZDCA(Qn::Detector &detector, AliReducedEventInfo &event) {
//  const std::array<double, 4> X = {{1.75, -1.75, 1.75, -1.75}};
//  const std::array<double, 4> Y = {{-1.75, -1.75, 1.75, 1.75}};
//  for (u_short ich = 0; ich < 4; ich++) {
//    double weight = event.EnergyZDCnTree(ich + 6);
//    if (weight > 100) {
//      detector.GetDataContainer()->CallOnElement([ich, Y, X, weight](std::vector<DataVector> &vector) {
//        vector.emplace_back(TMath::ATan2(Y[ich], X[ich]), weight);
//      });
//    }
//  }
//}
