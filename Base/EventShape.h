//
// Created by Lukas Kreis on 12.07.18.
//

#ifndef FLOW_EVENTSHAPE_H
#define FLOW_EVENTSHAPE_H

#include <TH1F.h>
#include <TSpline.h>
#include <TCanvas.h>
#include <TFile.h>

namespace Qn {
class EventShape : public TObject {
 public:

  EventShape() {}

  EventShape(std::string name) :
      name_(name) {}

  void SetHisto(TH1F* histo) {
    histo_ = (TH1F*) histo->Clone("histo");
  }

  void IntegrateHist() {
    integral_ = (TH1F *) histo_->Clone("integral");
    for (int i = 0; i < histo_->GetNbinsX()+1; ++i) {
      double inte = histo_->Integral(0, i)/histo_->Integral();
      integral_->SetBinContent(i, inte);
    }
  }

  bool IsReady() const {return ready_;}
  void SetReady() {ready_ = true;}


  void FitWithSpline(TH1F hist) {
    IntegrateHist();
    spline_ = new TSpline3(integral_, "sp3");
    spline_->SetName("spline");
  }

  float GetPercentile(float q) {
    return static_cast<float>(spline_->Eval(q));
  }

  void WriteSpline(TFile *file) {
    file->WriteObject(spline_, name_.data());
  }

  void SetName(const std::string &name) {name_ = name;}
  std::string Name() const {return name_;}

  ~EventShape() { delete spline_; }

  bool ready_ = false;
  std::string name_;
  TSpline3 *spline_ = nullptr;
  TH1F *histo_ = nullptr;
  TH1F *integral_ = nullptr;

  /// \cond CLASSIMP
  ClassDef(EventShape,1);
  /// \endcond
};
}

#endif //FLOW_EVENTSHAPE_H
