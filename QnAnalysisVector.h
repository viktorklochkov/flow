#ifndef QNANALYSISVECTOR_H
#define QNANALYSISVECTOR_H

#include <vector>
#include "TAxis.h"
#include "QnCorrectionsQnVector.h"
#include "TObject.h"


class QnAnalysisVector : public TObject {
  public:
    QnAnalysisVector();
    QnAnalysisVector(const char *name);
    ~QnAnalysisVector();
    typedef std::vector<QnCorrectionsQnVector*>::const_iterator iterator;
    iterator cbegin() {return qnvectors_.cbegin();}
    iterator cend() {return qnvectors_.cend();}
    void AddAxis(const char *name, Double_t *bins, Int_t nbins);
    void AddQnVector(QnCorrectionsQnVector *qn, std::vector<Float_t> &vars, const char *corrstep = "latest");
    QnCorrectionsQnVector* GetQnVector(std::vector<Int_t> &bins, const char *corrstep = "latest");
    TAxis* GetAxis(const char *name);
    std::vector<Long64_t> GetStride() const {return stride_;}
    std::vector<Int_t> GetIndex(Long64_t offset);

  private:
    const char* name_;
    Int_t dimension_;
    std::vector<QnCorrectionsQnVector*> qnvectors_;
    std::vector<TAxis*> axis_;
    std::vector<Long64_t> stride_;

    void CalculateStride();
    Long64_t GetBinLinearized(std::vector<Int_t> &index);
    QnAnalysisVector(const QnAnalysisVector &);
    QnAnalysisVector& operator= (const QnAnalysisVector &);
    ClassDef(QnAnalysisVector,1);
};

#endif
