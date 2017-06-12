#ifndef QNCORRELATIONMATRIX_H
#define QNCORRELATIONMATRIX_H

#include <vector>

class TAxis;
class AliQnCorrectionsQnVector;

class QnCorrelationMatrix : public TObject {

  typedef std::vector<Float_t> harmonicsvector_;

  public:
    QnCorrelationMatrix();
    QnCorrelationMatrix(const char *name);
    ~QnCorrelationMatrix();

    AliQnCorrectionsQnVector* GetCorrelation(Int_t i, Int_t j, std::vector<Int_t> &bins);
    TAxis* GetAxis(const char *name);
    static void Correlation(const char* var, const char* step, AliQnCorrectionsQnVector *q1, AliQnCorrectionsQnVector *q2);
    static void Correlation(const char* var, const char* step, AliQnCorrectionsQnVector *q1, AliQnCorrectionsQnVector *q2, AliQnCorrectionsQnVector *q3);
    static Float_t C2(Float_t q1, Float_t q2);

  private:

    const char* name_;
    Int_t dimension_;
    std::vector<TAxis*> axis_;
    std::vector<harmonicsvector_> matrix_;

    Long64_t GetBinLinearized(std::vector<Int_t> &index);
    QnCorrelationMatrix(const QnCorrelationMatrix &);
    QnCorrelationMatrix& operator= (const QnCorrelationMatrix &);
    ClassDef(QnCorrelationMatrix,1);
};

#endif
