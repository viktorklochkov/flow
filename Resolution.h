//
// Created by Lukas Kreis on 20.10.17.
//

#ifndef FLOW_RESOLUTION_H
#define FLOW_RESOLUTION_H

#include <TTreeReader.h>
#include <TList.h>
#include "DataContainer.h"
#include "TH1D.h"
#include "TProfile.h"
namespace Qn {

class ResolutionDetector;

class Resolution {
  using RESAXIS = std::tuple<std::string, int, float, float>;
 public:
  Resolution() = default;
  explicit Resolution(TTreeReader *reader) :
    reader_(reader),
    histograms_(new TList()) {
    histograms_->SetOwner(true);
  }

  void AddDetector(std::string a, std::string b, std::string c, Qn::Axis axis);
  void Process();
  void PostProcess();
  TList *GetHistograms() const { return histograms_; }
  TTreeReader *Reader() const { return reader_; }

 private:
  TTreeReader *reader_ = nullptr;
  TList *histograms_ = nullptr;
  std::vector<ResolutionDetector> detectors_;

};

class ResolutionDetector {

 public:
  ResolutionDetector() = default;
  ResolutionDetector(Resolution &res, std::string a, std::string b, std::string c, Qn::Axis axis);
  std::tuple<TProfile *, TProfile *, TProfile *> GetHistograms() const {
    return std::make_tuple(psiab_,
                           psiac_,
                           psibc_);
  }
  void Fill(int abin, int bbin, int cbin);
  TH1D *CalcRes();
 private:
  double PsiN(const double n, const double qx, const double qy) const { return 1 / n * TMath::ATan2(qy, qx); }
  double CosN(const int n, const double a, const double b) const { return TMath::Cos(n * (a - b)); }
  void SqrtHist(TH1D &hist) {
    for (int i = 1; i <= hist.GetNbinsX(); ++i) {
      hist.SetBinContent(i, TMath::Sign(1, hist.GetBinContent(i)) * TMath::Sqrt(TMath::Abs(hist.GetBinContent(i))));
    }
  }
  std::string name_;
  TTreeReaderValue<Qn::DataContainerQVector> aqn_;
  TTreeReaderValue<Qn::DataContainerQVector> bqn_;
  TTreeReaderValue<Qn::DataContainerQVector> cqn_;
  TTreeReaderValue<float> axisqn_;
  TProfile *psiab_ = nullptr;
  TProfile *psiac_ = nullptr;
  TProfile *psibc_ = nullptr;
  int harm_ = 2;

};

}

#endif //FLOW_RESOLUTION_H
