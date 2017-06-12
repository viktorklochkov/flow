#ifndef QNDATACONTAINER_H
#define QNDATACONTAINER_H

#include <vector>
#include <iostream>
#include "TObject.h"
#include "TAxis.h"
#include "AliQnCorrectionsQnVector.h"

template <class T> class QnDataContainer : public TObject {
  public:
    typedef typename std::vector<T>::const_iterator iterator;

    QnDataContainer() {}

    QnDataContainer(const char *name) :
      TObject(),
      name_(),
      dimension_(0),
      data_(),
      axis_(),
      stride_() {
        name_ = new TString(name);
      }

    ~QnDataContainer() {}

    iterator cbegin() {return data_.cbegin();}

    iterator cend() {return data_.cend();}

    void AddAxis(const char *name, Double_t *bins, Int_t nbins)
    {
      TAxis *axis = new TAxis(nbins,bins);
      axis->SetName(name);
      axis_.push_back(axis);
      dimension_++;
      Int_t totalbins = 1;
      for (auto axis : axis_)
      {
        totalbins *= axis->GetNbins();
      }
      std::cout << totalbins << std::endl;
      data_.resize(totalbins);
      stride_.resize(dimension_+1);
      std::cout << "calculate stride" << std::endl;
      CalculateStride();
    }

    void AddVector(T vect, std::vector<Float_t> &vars, const char *corrstep = "latest")
    {
      std::vector<Int_t> index;
      Int_t axisindex = 0;
      for (auto axis : axis_)
      {
        Int_t bin = axis->FindBin(vars.at(axisindex));
        if (bin > axis->GetNbins() || bin == 0)
        {
          return;
        }
        index.push_back(bin);
        axisindex++;
      }
      data_.at(GetBinLinearized(index)) = vect;
    }

    T GetVector(std::vector<Int_t> &bins, const char *corrstep = "latest")
    {
      return data_.at(GetBinLinearized(bins));
    }

    TAxis* GetAxis(const char *name)
    {
      TString string(name);
      for (auto axis: axis_)
      {
        if(string.EqualTo(axis->GetName())) return axis;
      }
      return NULL;
    }
    std::vector<Long64_t> GetStride() const {return stride_;}

    std::vector<Int_t> GetIndex(Long64_t offset)
    {
      std::vector<Int_t> indices;
      indices.reserve(dimension_);
      for (Int_t i = 0; i < dimension_-1; ++i)
      {
        indices.push_back(offset % axis_[i]->GetNbins());
        // std::cout << offset << " % " << axis_[i]->GetNbins() << " = " << offset << std::endl;
        offset = offset / axis_[i]->GetNbins();
        // std::cout << " / " << axis_[i]->GetNbins() << offset << std::endl;
      }
      indices.push_back(offset);
      return indices;
    }

  private:
    TString *name_;
    Int_t dimension_;
    std::vector<T> data_;
    std::vector<TAxis*> axis_;
    std::vector<Long64_t> stride_;

    void CalculateStride()
    {
      stride_[dimension_] = 1;
      for (Int_t i = 0; i < dimension_; ++i)
      {
        stride_[dimension_ - i - 1] = stride_[dimension_ - i] * axis_[dimension_ - i - 1]->GetNbins();
      }
      // stride_.insert(stride_.begin() + i, product);
      for (Int_t i = 0; i < dimension_; ++i) {
        std::cout <<"stride "<< i << " : " << stride_[i] << std::endl;
      }
    }

    Long64_t GetBinLinearized(std::vector<Int_t> &index)
    {
      Long64_t offset = (index[dimension_-1] - 1);
      for (Int_t i = 0; i < dimension_-1; ++i)
      {
        offset += stride_[i+1] * (index[i] - 1);
      }
      return offset;
    }
    // QnDataContainer(const QnDataContainer &);
    // QnDataContainer& operator= (const QnDataContainer &);

    ClassDef(QnDataContainer,1);
};

typedef QnDataContainer<AliQnCorrectionsQnVector*> QnDataContainerQn;

#endif
