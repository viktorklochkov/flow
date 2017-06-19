#include "TString.h"
#include "QnAnalysisVector.h"
#include <iostream>

ClassImp(QnAnalysisVector)

QnAnalysisVector::QnAnalysisVector() :
  TObject(),
  name_(0x0),
  dimension_(0),
  qnvectors_(),
  axis_(),
  stride_()
{
}

QnAnalysisVector::QnAnalysisVector(const char *name) :
  TObject(),
  name_(TString(name)),
  dimension_(0),
  qnvectors_(),
  axis_(),
  stride_()
{
}

QnAnalysisVector::~QnAnalysisVector()
{
}

TAxis* QnAnalysisVector::GetAxis(const char *name)
{
  TString string(name);
  for (auto &axis: axis_)
  {
    if(string.EqualTo(axis.GetName())) return &axis;
  }
  return NULL;
}

void QnAnalysisVector::AddAxis(const char *name, Double_t *bins, Int_t nbins)
{
  TAxis axis(nbins,bins);
  axis.SetName(name);
  axis_.push_back(axis);
  dimension_++;
  Int_t totalbins = 1;
  for (auto &axis : axis_)
  {
    totalbins *= axis.GetNbins();
  }
  qnvectors_.resize(totalbins);
  stride_.resize(dimension_);
  CalculateStride();
}

void QnAnalysisVector::CalculateStride() {
  for (Int_t i = 0; i < dimension_; ++i)
  {
    Int_t product = 1;
    for (Int_t j = i+1; j < dimension_; ++j)
    {
      product *= axis_[j].GetNbins();
    }
    stride_.insert(stride_.begin() + i, product);
  }
}

QnCorrectionsQnVector* QnAnalysisVector::GetQnVector(std::vector<Int_t> &bins, const char *corrstep)
{
  return qnvectors_.at(GetBinLinearized(bins));
}

void QnAnalysisVector::AddQnVector(QnCorrectionsQnVector *qn, std::vector<Float_t> &vars, const char *corrstep)
{
  std::vector<Int_t> index;
  Int_t axisindex = 0;
  for (auto &axis : axis_)
  {
    Int_t bin = axis.FindBin(vars.at(axisindex));
    if (bin > axis.GetNbins() || bin == 0)
    {
      printf("Qn in over or underflow bin! Not added");
      return;
    }
    index.push_back(bin);
    axisindex++;
  }
  std::cout << GetBinLinearized(index) << std::endl;
  qnvectors_.at(GetBinLinearized(index)) = qn;
  // Insert crashed when inserting element after elements have been added to a later position why?
  // qnvectors_.insert(qnvectors_.begin() + GetBinLinearized(index), qn);
}

std::vector<Int_t> QnAnalysisVector::GetIndex(Long64_t offset) {
  std::vector<Int_t> indices;
  indices.reserve(dimension_);
  for (Int_t i = 0; i < dimension_; ++i)
  {
    indices.push_back(offset % axis_[i].GetNbins());
    offset = offset / axis_[i].GetNbins();
  }
  return indices;
}

Long64_t QnAnalysisVector::GetBinLinearized(std::vector<Int_t> &index)
{
  Long64_t offset = 0;
  for (Int_t i = 0; i < dimension_; ++i)
  {
    Int_t product = 1;
    offset += stride_[i] * (index[i]-1);
  }
  return offset;
}
