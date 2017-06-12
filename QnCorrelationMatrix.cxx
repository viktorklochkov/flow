#include "TString.h"
#include "TAxis.h"
#include "TObject.h"
#include "QnCorrelationMatrix.h"
#include <iostream>

#define MAXHARMONICS 8

ClassImp(QnCorrelationMatrix)

QnCorrelationMatrix::QnCorrelationMatrix() :
  TObject(),
  name_(0x0),
  dimension_(0),
  qnvectors_(),
  axis_()
{
}

QnCorrelationMatrix::QnCorrelationMatrix(const char *name) :
  TObject(),
  name_(name),
  dimension_(0),
  qnvectors_(),
  axis_()
{

}

QnCorrelationMatrix::~QnCorrelationMatrix()
{
}

TAxis* QnCorrelationMatrix::GetAxis(const char *name)
{
  TString string(name);
  for (auto axis: axis_)
  {
    if(string.EqualTo(axis->GetName())) return axis;
  }
  return NULL;
}


static Float_t QnCorrelationMatrix::Cn2(Float_t q1, Float_t q2)
{
  return q1 * q2;
}

// static void QnCorrelationMatrix::C2(const char* step, QnAnalysisVector *q1, QnAnalysisVector *q2)
// {
//   AliQnCorrectionsQnVector *qvec1 = q1->GetQnVector(const char *corrstep = "latest");
//   AliQnCorrectionsQnVector *qvec2 = q2->GetQnVector(const char *corrstep = "latest");
//   for (Long64_t i = 0; i < q1->GetSize(); ++i) {
//
//   }
// }

//
// void QnCorrelationMatrix::AddAxis(const char *name, Double_t *bins, Int_t nbins)
// {
//   TAxis *axis = new TAxis(nbins,bins);
//   axis->SetName(name);
//   axis_.push_back(axis);
//   dimension_++;
//   Int_t totalbins = 1;
//   for (auto axis : axis_)
//   {
//     totalbins *= axis->GetNbins();
//   }
//   qnvectors_.resize(totalbins);
// }

// AliQnCorrectionsQnVector* QnCorrelationMatrix::GetQnVector(std::vector<Int_t> &bins, const char *corrstep)
// {
//   return qnvectors_.at(GetBinLinearized(bins));
// }

Long64_t QnCorrelationMatrix::GetBinLinearized(std::vector<Int_t> &index)
{
  Long64_t offset = 0;
  for (Int_t i = 0; i < dimension_; ++i) {
    Int_t product = 1;
    for (Int_t j = i+1; j < dimension_; ++j) {
      product *= axis_[j]->GetNbins();
    }
    offset += product * index[i];
  }
  return offset;
}
