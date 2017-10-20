//
// Created by Lukas Kreis on 20.10.17.
//

#ifndef FLOW_RESOLUTION_H
#define FLOW_RESOLUTION_H

#include <TTreeReader.h>
#include <TList.h>
#include "DataContainer.h"
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

  void AddDetector(std::string a, std::string b, std::string c, RESAXIS axis);
  void Process();
  void PostProcess();
  TList *GetHistograms() const { return histograms_; }
  TTreeReader *Reader() const { return reader_; }

 private:
  TTreeReader *reader_;
  TList *histograms_;
  std::vector<ResolutionDetector> detectors_;

};

class ResolutionDetector {
  using RESAXIS = std::tuple<std::string, int, float, float>;

 public:
  ResolutionDetector() = default;
  ResolutionDetector(Resolution &res, std::string a, std::string b, std::string c, RESAXIS axis);
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
    for (int i = 0; i < hist.GetNbinsX(); ++i) {
      hist.SetBinContent(i, TMath::Sqrt(hist.GetBinContent(i)));
    }
  }
  std::string name_;
  TTreeReaderValue<Qn::DataContainerQn> aqn_;
  TTreeReaderValue<Qn::DataContainerQn> bqn_;
  TTreeReaderValue<Qn::DataContainerQn> cqn_;
  TTreeReaderValue<float> axisqn_;
  TProfile *psiab_;
  TProfile *psiac_;
  TProfile *psibc_;
  int harm_ = 2;

};

}

#endif //FLOW_RESOLUTION_H
