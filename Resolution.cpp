//
// Created by Lukas Kreis on 20.10.17.
//

#include <TProfile.h>
#include "Resolution.h"

namespace Qn {

void Resolution::AddDetector(const std::string a, const std::string b, const std::string c, Qn::Axis axis) {



  ResolutionDetector detector(*this, a, b, c, axis);
  detectors_.push_back(detector);
  auto hists = detector.GetHistograms();
  histograms_->Add(std::get<0>(hists));
  histograms_->Add(std::get<1>(hists));
  histograms_->Add(std::get<2>(hists));

}

void Resolution::Process() {
  for (auto &detector : detectors_) {
    detector.Fill(0,0,0);
  }
}

void Resolution::PostProcess() {
  for (auto &detector : detectors_) {
    histograms_->Add(detector.CalcRes());
  }
}

ResolutionDetector::ResolutionDetector(Resolution &res, std::string a, std::string b, std::string c, Qn::Axis axis) :
    name_(a + b + c + axis.Name()),
    aqn_(*res.Reader(), a.data()),
    bqn_(*res.Reader(), b.data()),
    cqn_(*res.Reader(), c.data()),
    axisqn_(*res.Reader(), axis.Name().data()) {
  std::string axisname = axis.Name();
  int nbins = (int) axis.size();
  float min = axis.GetLowerBinEdge(0);
  float max = axis.GetUpperBinEdge(nbins-1);
  std::string nameac = "psi" + a + c + "-" + b;
  std::string nameab = "psi" + a + b + "-" + c;
  std::string namebc = "psi" + b + c + "-" + a;
  psiac_ = new TProfile(nameac.data(), nameac.data(), nbins, min, max);
  psiab_ = new TProfile(nameab.data(), nameab.data(), nbins, min, max);
  psibc_ = new TProfile(namebc.data(), namebc.data(), nbins, min, max);

}
void ResolutionDetector::Fill(const int abin, const int bbin, const int cbin) {
  double psia = PsiN(harm_,aqn_->GetElement(abin).x(harm_),aqn_->GetElement(abin).y(harm_));
  double psib = PsiN(harm_,bqn_->GetElement(bbin).x(harm_),bqn_->GetElement(bbin).y(harm_));
  double psic = PsiN(harm_,cqn_->GetElement(cbin).x(harm_),cqn_->GetElement(cbin).y(harm_));
  psiac_->Fill(*axisqn_,CosN(harm_,psia,psic));
  psiab_->Fill(*axisqn_,CosN(harm_,psia,psib));
  psibc_->Fill(*axisqn_,CosN(harm_,psib,psic));
}
TH1D *ResolutionDetector::CalcRes() {
  TH1D pac = *psiac_->ProjectionX();
  TH1D pbc = *psibc_->ProjectionX();
  TH1D pab = *psiab_->ProjectionX();

  std::string resolutionname = "r" + name_;
  auto *ra = (TH1D *) pab.Clone(resolutionname.data());
  ra->SetName(resolutionname.data());
  ra->SetTitle(resolutionname.data());
  ra->Multiply(&pac);
  ra->Divide(&pbc);
  SqrtHist(*ra);
  return ra;
}

}
