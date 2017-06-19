#ifndef QNDATACONTAINER_H
#define QNDATACONTAINER_H

#include <vector>
#include <stdexcept>
#include <memory>
#include "TAxis.h"
#include "QnCorrectionsQnVector.h"

template <class T> class QnDataContainer {
  public:
    typedef typename std::vector<T>::const_iterator iterator;

    QnDataContainer(std::string &name) :
      name_(name),
      dimension_(0)
      {
      }

    ~QnDataContainer() = default;

    iterator cbegin() {return data_.cbegin();}

    iterator cend() {return data_.cend();}

    void AddAxis(const char *name, const std::vector<double> &bins, const int nbins)
    {
      axis_.emplace_back(std::shared_ptr<TAxis>(new TAxis(nbins, &bins[0])));
      axis_.back()->SetName(name);
      dimension_++;
      int totalbins = 1;
      for (const auto &axis : axis_)
      {
        totalbins *= axis->GetNbins();
      }
      data_.resize(totalbins);
      stride_.resize(dimension_+1);
      CalculateStride();
    }

    void AddVector(const T &vect, const std::vector<float> &vars, const char *corrstep = "latest")
    {
      std::vector<int> index;
      int axisindex = 0;
      for (auto axis : axis_)
      {
        Int_t bin = axis->FindBin(vars.at(axisindex));
        if (bin > axis->GetNbins() || bin == 0)
        {
          throw std::out_of_range("bin out of specified range");
          return;
        }
        index.push_back(bin);
        axisindex++;
      }
      data_.at(GetBinLinearized(index)) = vect;
    }

    T GetVector(const std::vector<int> &bins, const char *corrstep = "latest")
    {
      return data_.at(GetBinLinearized(bins));
    }

    std::shared_ptr<TAxis> GetAxis(const char *name)
    {
      std::string string(name);
      for (auto axis: axis_)
      {
        std::string axisname(axis->GetName());
        if(string == axisname) return axis;
      }
      return nullptr;
    }
    std::vector<long long> GetStride() const {return stride_;}

    std::vector<int> GetIndex(const long long offset)
    {
      long long temp = offset;
      std::vector<int> indices;
      indices.reserve(dimension_);
      for (Int_t i = 0; i < dimension_-1; ++i)
      {
        indices.push_back(offset % axis_[i]->GetNbins());
        temp = temp / axis_[i]->GetNbins();
      }
      indices.push_back(temp);
      return indices;
    }

  private:
    std::string name_;
    int dimension_;
    std::vector<T> data_;
    std::vector<std::shared_ptr<TAxis>> axis_;
    std::vector<long long> stride_;

    void CalculateStride()
    {
      stride_[dimension_] = 1;
      for (Int_t i = 0; i < dimension_; ++i)
      {
        stride_[dimension_ - i - 1] = stride_[dimension_ - i] * axis_[dimension_ - i - 1]->GetNbins();
      }
    }

    long long GetBinLinearized(const std::vector<int> &index)
    {
      long long offset = (index[dimension_-1] - 1);
      for (Int_t i = 0; i < dimension_-1; ++i)
      {
        offset += stride_[i+1] * (index[i] - 1);
      }
      return offset;
    }
};

typedef QnDataContainer<QnCorrectionsQnVector*> QnDataContainerQn;

#endif
