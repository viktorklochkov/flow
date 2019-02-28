// Flow Vector Correction Framework
//
// Copyright (C) 2018  Lukas Kreis, Ilya Selyuzhenkov
// Contact: l.kreis@gsi.de; ilya.selyuzhenkov@gmail.com
// For a full list of contributors please see docs/Credits
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef QNDATACONTAINER_H
#define QNDATACONTAINER_H

#include <vector>
#include <string>
#include <stdexcept>

#include "TEnv.h"
#include "TObject.h"
#include "TMath.h"
#include "Rtypes.h"
#include "TH1F.h"
#include "TBrowser.h"
#include "TCollection.h"

#include "Axis.h"
#include "DataVector.h"
#include "QVector.h"
#include "EventShape.h"
#include "Product.h"
#include "Stats.h"

#include "DataContainerHelper.h"

/**
 * QnCorrectionsframework
 */
namespace Qn {
//
//template<typename T>
//T Merge(const T&a, const T&b) {return Merge(a,b);}

/**
 * @brief      Template container class for Q-vectors and correlations
 * @param T    Type of object inside of container
 */
template<typename T>
class DataContainer : public TObject {
 public:
  enum Settings {
    IsMergable = BIT(14),
    NCentralSettings = BIT(15)
  };

/**
 * Default constructor
 * Axes can be added later. If no axes are added the DataContainer is integrated with only one entry.
 */
  DataContainer() : integrated_(true) {
    axes_.push_back({"integrated", 1, 0, 1});
    dimension_ = 1;
    data_.resize(1);
    stride_.resize(dimension_+1);
    CalculateStride();
  };
/**
 * Constructor
 * @param axes vector of axes of the datacontainer.
 */
  explicit DataContainer(std::vector<Axis> axes) {
    AddAxes(axes);
  }
  virtual ~DataContainer() {
     delete list_;
  };

  using QnAxes = std::vector<Axis>;
  using size_type = std::size_t;
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;
  const_iterator begin() const { return data_.cbegin(); } ///< iterator for external use
  const_iterator end() const { return data_.cend(); } ///< iterator for external use
  iterator begin() { return data_.begin(); } ///< iterator for external use
  iterator end() { return data_.end(); } ///< iterator for external use

/**
 * Size of data container
 * @return number of entries in the container
 */
  size_type size() const noexcept { return data_.size(); }

/**
 * Adds axes for storing data
 * @param axes vector of axes
 */
  void AddAxes(const QnAxes &axes) {
    if (!axes.empty()) {
      if (integrated_) this->Reset();
      for (const auto &axis : axes) {
        AddAxis(axis);
      }
    }
  }

/**
 * Adds existing axis for storing the data with variable binning
 * @param axis Axis to be added.
 */
  void AddAxis(const Axis &axis) {
    if (integrated_) this->Reset();
    if (std::find_if(axes_.begin(), axes_.end(), [axis](const Axis &axisc) { return axisc.Name()==axis.Name(); })
        !=axes_.end())
      throw std::runtime_error("Axis already defined in vector.");
    axes_.push_back(axis);
    dimension_++;
    size_type totalbins = 1;
    for (const auto &iaxis : axes_) {
      totalbins *= iaxis.size();
    }
    data_.resize(totalbins);
    stride_.resize((size_type) dimension_ + 1);
    CalculateStride();
  }
/**
 * Initialized the object in the histogram to a given object
 * @param obj Object to be intialized to
 */
  void InitializeEntries(const T obj) {
    for (auto &bin : data_) {
      bin = obj;
    }
  }

/**
 * Calculates the diagonal indices of a given number of axes.
 * @param axes vector of axes.
 * @return vector of linear indices in the diagonal.
 */
  std::vector<size_type> GetDiagonal(const QnAxes &axes) {
    std::vector<size_type> diagonal;
    size_type stride_sum = 0;
    size_type nbins_diagonal = 1;
    size_type iaxis = 0;
    for (const auto &ref_axis : axes_) {
      bool found = false;
      for (const auto &axis : axes) {
        if (ref_axis==axis) {
          stride_sum += stride_[iaxis];
          found = true;
        }
      }
      if (!found) nbins_diagonal *= ref_axis.size();
      ++iaxis;
    }
    for (size_type ibin = 0; ibin < axes[0].size(); ++ibin) {
      for (size_type idiag = 0; idiag < nbins_diagonal; ++idiag) {
        diagonal.push_back(ibin*stride_sum + idiag);
      }
    }
    return diagonal;
  }

/**
 * Get element in the specified bin
 * @param bins Vector of bin indices of the desired element
 * @return     Element
 */
  T const &At(const typename std::vector<size_type> &bins) const { return data_.at(GetLinearIndex(bins)); }

/**
 * Get element in the specified bin
 * @param bins Vector of bin indices of the desired element
 * @return     Element
 */
  T &At(const typename std::vector<size_type> &bins) { return data_.at(GetLinearIndex(bins)); }

/**
 * Get element in the specified bin
 * @param index index of element
 * @return      Element
 */
  T &At(size_type index) noexcept { return data_.at(index); }


/**
 * Get element in the specified bin
 * @param index index of element
 * @return      Element
 */
  T const &At(size_type index) const noexcept { return data_.at(index); }


/**
 * Calls function on element specified by indices.
 * @tparam Function type of function to be called on the object
 * @param indices indicies multidimensional indices of the element.
 * @param lambda function to be called on the element. Takes element of type T as an argument.
 */
  template<typename Function>
  void CallOnElement(const typename std::vector<size_type> &indices, Function &&lambda) {
    lambda(data_[GetLinearIndex(indices)]);
  }

/**
 * Calls function on element for integrated datacontainer.
 * @tparam Function type of function to be called on the object
 * @param coordinates coordinates of element to be modified
 * @param lambda function to be called on the element. Takes element of type T as an argument.
 */
  template<typename Function>
  void CallOnElement(Function &&lambda) {
    lambda(data_[0]);
  }

/**
 * Calls function on element specified by indices.
 * @tparam Function type of function to be called on the object
 * @param coordinates coordinates of element to be modified
 * @param lambda function to be called on the element. Takes element of type T as an argument.
 */
  template<typename Function>
  void CallOnElement(const std::vector<float> &coordinates, Function &&lambda) {
    lambda(data_[GetLinearIndex(GetIndex(coordinates))]);
  }

/**
 * Calls function on element specified by indices.
 * @tparam Function type of function to be called on the object
 * @param index linear index of element to be modified
 * @param lambda function to be called on the element. Takes element of type T as an argument.
 */
  template<typename Function>
  inline void CallOnElement(const long index, Function &&lambda) {
    lambda(data_[index]);
  }

/**
 * Get vector of axes
 * @return Vector of axes
 */
  inline const QnAxes &GetAxes() const { return axes_; }

  /**
 * Get vector of axes
 * @return Vector of axes
 */
  inline QnAxes &GetAxes() { return axes_; }

/**
 * Get Axis with the given name.
 * Throws exception when not found.
 * @param name  Name of the desired axis
 * @return      Axis
 */
  Axis GetAxis(const std::string name) const {
    for (auto axis: axes_) {
      if (name==axis.Name()) return axis;
    }
    throw std::out_of_range("axis not found aborting");
  }

/**
 * Calculates indices in multiple dimensions from linearized index
 * @param indices Outparameter for the indices
 * @param offset Index of linearized vector
 */
  void GetIndex(typename std::vector<size_type> &indices, const unsigned long offset) const {
    unsigned long temp = offset;
    if (offset < data_.size()) {
      indices.resize(dimension_);
      for (unsigned int i = 0; i < dimension_ - 1; ++i) {
        indices[dimension_ - i - 1] = temp%axes_[dimension_ - i - 1].size();
        temp = temp/axes_[dimension_ - i - 1].size();
      }
      indices[0] = temp;
    }
  }

/**
 * Gives description by concenating axis names with coordinates
 * @param offset linear index
 * @return string with bin description
 */
  std::string GetBinDescription(const unsigned long offset) const {
    std::vector<unsigned long> indices;
    GetIndex(indices, offset);
    if (indices.empty()) return "invalid offset";
    std::string outstring;
    int i = 0;
    for (auto it = axes_.cbegin(); it!=axes_.cend(); ++it) {
      const auto &axis = *it;
      outstring += axis.Name();
      outstring += "(" + std::to_string(axis.GetLowerBinEdge(indices[i])) + ", "
          + std::to_string(axis.GetUpperBinEdge(indices[i])) + ")";
      if (it + 1!=axes_.cend()) outstring += "; ";
      ++i;
    }
    return outstring;
  }

/**
 * Projects datacontainer on a subset of axes
 * @tparam Function typename of function.
 * @param axis_names subset of axes used for the projection.
 * @param lambda Function used to add two entries.
 * @return projected datacontainer.
 */
  template<typename Function>
  DataContainer<T> Projection(const std::vector<std::string> axis_names,
                              Function &&lambda) const {
    DataContainer<T> projection;
    unsigned long linearindex = 0;
    std::vector<bool> isprojected;
    isprojected.resize(axes_.size());
    for (const auto &name : axis_names) {
      bool bproj = false;
      size_type iaxis = 0;
      for (const auto &originalaxis : axes_) {
        if (originalaxis.Name()==name) {
          bproj = true;
          break;
        }
        iaxis++;
      }
      isprojected.at(iaxis) = bproj;
    }
    size_type iaxis = 0;
    for (const auto proj : isprojected) {
      if (proj) projection.AddAxis(axes_.at(iaxis));
      ++iaxis;
    }
    if (axis_names.empty()) {
      projection.At(0) = data_.at(0);
      for (auto bin = data_.begin()+1; bin < data_.end(); ++bin) {
        projection.At(0) = lambda(projection.At(0), *bin);
      }
    } else {
      std::vector<size_type> indices;
      std::vector<size_type> projindices;
      indices.reserve(dimension_);
      projindices.resize(projection.dimension_);
      // other bins
      for (auto bin : data_) {
        this->GetIndex(indices, linearindex);
        size_type iprojbin = 0;
        for (size_type i = 0; i < indices.size(); ++i) {
          if (isprojected.at(i)) {
            projindices.at(iprojbin) = indices.at(i);
            ++iprojbin;
          }
        }
        projection.At(projindices) = lambda(projection.At(projindices), bin);
        ++linearindex;
      }
    }
    return projection;
  }

/**
 * Projects datacontainer on a subset of axes
 * @param axis_names subset of axes used for the projection.
 * @return projected datacontainer.
 */
  DataContainer<T>
  Projection(const std::vector<std::string> axis_names = {}) const {
    auto lambda = [](const T &a, const T &b) { return Qn::MergeBins(a,b); };
    return Projection(axis_names, lambda);
  }

/**
 * Projects datacontainer on a subset of axes
 * @tparam Function typename of function.
 * @param axes subset of axes used for the projection.
 * @param lambda Function used to add two entries.
 * @param exindices indices excluded from the projection.
 * @return projected datacontainer.
 */
  template<typename Function>
  DataContainer<T> ProjectionExclude(const std::vector<std::string> axis_names,
                                     Function &&lambda, std::vector<int> exindices) const {
    DataContainer<T> projection;
    size_type linearindex = 0;
    std::vector<bool> isprojected;
    isprojected.resize(axes_.size());
    for (const auto &name : axis_names) {
      bool bproj = false;
      size_type iaxis = 0;
      for (const auto &originalaxis : axes_) {
        if (originalaxis.Name()==name) {
          bproj = true;
          break;
        }
        iaxis++;
      }
      isprojected.at(iaxis) = bproj;
    }
    size_type iaxis = 0;
    for (const auto proj : isprojected) {
      if (proj) projection.AddAxis(axes_.at(iaxis));
      ++iaxis;
    }
    if (axis_names.empty()) {
      for (const auto &bin : data_) {
        projection.At(0) = lambda(projection.At(0), bin);

      }
    } else {
      std::vector<size_type> indices;
      std::vector<size_type> projindices;
      indices.reserve(dimension_);
      projindices.resize(projection.dimension_);
      for (const auto &bin : data_) {
        if (std::find(exindices.begin(), exindices.end(), linearindex)!=exindices.end()) {
          ++linearindex;
          continue;
        }
        this->GetIndex(indices, linearindex);
        size_type iprojbin = 0;
        for (size_type i = 0; i < indices.size(); ++i) {
          if (isprojected.at(i)) {
            projindices.at(iprojbin) = indices.at(i);
            ++iprojbin;
          }
        }
        projection.At(projindices) = lambda(projection.At(projindices), bin);
        ++linearindex;
      }
    }
    return projection;
  }

/**
 * Map function to datacontainer. Does not modify the original container.
 * @tparam Function function
 * @param lambda unary function to be applied to each element.
 * @return datacontainer after applying function.
 */
  template<typename Function>
  DataContainer<T> Map(Function &&lambda) const {
    DataContainer<T> result(*this);
    std::transform(data_.begin(), data_.end(), result.begin(), [&lambda](const T &element) { return lambda(element); });
    return result;
  }

/**
 * Selects subrange of axis of datacontainer. If resulting axis is one dimensional it is deleted and dimension of
 * the resulting container is reduced by one.
 * @tparam Function type of function
 * @param data input data container
 * @param axis subrange of axis to perform selection
 * @return
 */
  DataContainer<T> Select(const Axis &axis) const {
    DataContainer<T> selected;
    long axisposition = 0;
    long tmpaxisposition = 0;
    for (const auto &a : axes_) {
      if (a.Name()==axis.Name()) {
        selected.AddAxis(axis);
        axisposition = tmpaxisposition;
      } else {
        selected.AddAxis(a);
      }
      tmpaxisposition++;
    }
    unsigned long index = 0;
    std::vector<unsigned long> indices;
    indices.reserve(dimension_);
    for (const auto &bin : data_) {
      GetIndex(indices, index);
      auto binlow = axes_[axisposition].GetLowerBinEdge(indices[axisposition]);
      auto binhigh = axes_[axisposition].GetUpperBinEdge(indices[axisposition]);
      auto binmid = binlow + (binhigh - binlow)/2;
      auto rebinnedindex = axis.FindBin(binmid);
      if (rebinnedindex!=-1) {
        indices[axisposition] = static_cast<unsigned long>(rebinnedindex);
        selected.CallOnElement(indices, [&bin](T &element) { element = bin; });
      }
      ++index;
    }
    if (axis.size()==1) {
      selected.axes_.erase(selected.axes_.begin() + axisposition);
      selected.stride_.resize(selected.axes_.size() + 1);
      selected.dimension_ = selected.axes_.size();
      selected.CalculateStride();
    }
    return selected;
  }

/**
 * Rebins the Datacontainer using the supplied function to calculate the new bin entries of the specified axis.
 * @tparam Function
 * @param rebinaxis axis to be rebinned.
 * @param lambda function used to calculate new bin entries.
 * @return rebinned datacontainer.
 */
  template<typename Function>
  DataContainer<T> Rebin(const Axis &rebinaxis, Function &&lambda) const {
    DataContainer<T> rebinned;
    unsigned long axisposition = 0;
    bool axisfound = false;
    // Check if axis to be rebinned is found in the datacontainer.
    for (auto axis = std::begin(axes_); axis < std::end(axes_); ++axis) {
      if (axis->Name()==rebinaxis.Name()) {
        rebinned.AddAxis(rebinaxis);
        axisposition = static_cast<unsigned long>(std::distance(axes_.begin(), axis));
        axisfound = true;
      } else {
        rebinned.AddAxis(*axis);
      }
    }
    if (!axisfound) {
      std::string errormsg = "Datacontainer does not have axis of name " + rebinaxis.Name();
      throw std::logic_error(errormsg);
    }
    //Check if there is no overlap in the bin edges.
    bool rebin_ok = true;
    for (const auto &rebinedge : rebinaxis) {
      bool found = false;
      for (const auto &binedge : (Axis) axes_.at(axisposition)) {
        float test = TMath::Abs(rebinedge - binedge);
        if (test < 10e-4) {
          found = rebin_ok;
          break;
        }
      }
      rebin_ok = rebin_ok && found;
    }
    if (!rebin_ok) {
      std::string errormsg = "Rebinned axis has overlapping bins." + rebinaxis.Name();
      throw std::logic_error(errormsg);
    }
    unsigned long ibin = 0;
    std::vector<size_type> indices;
    indices.reserve(dimension_);
    for (const auto &bin : data_) {
      GetIndex(indices, ibin);
      auto binlow = axes_[axisposition].GetLowerBinEdge(indices[axisposition]);
      auto binhigh = axes_[axisposition].GetUpperBinEdge(indices[axisposition]);
      auto binmid = binlow + (binhigh - binlow)/2;
      auto rebinnedindex = rebinaxis.FindBin(binmid);
      indices[axisposition] = static_cast<unsigned long>(rebinnedindex);
      if (rebinnedindex!=-1) rebinned.At(indices) = lambda(rebinned.At(indices), bin);
      ++ibin;
    }
    return rebinned;
  }

/**
 * Rebins the Datacontainer to the new bin entries of the specified axis.
 * Using default addition method.
 * @param rebinaxis axis to be rebinned.
 * @return rebinned datacontainer.
 */
  DataContainer<T> Rebin(const Axis &rebinaxis) const {
    auto lambda = [](const T &a, const T &b) { return Qn::MergeBins(a,b); };
    return Rebin(rebinaxis, lambda);
  }

/**
 * Apply function to two datacontainers.
 * The axes need to have the same order.
 * Elements in datacontainer with smaller dimensions are used as "integrated bins".
 * @tparam Function type of function
 * @param data Datacontainer
 * @param lambda function to be applied on both elements
 * @return resulting datacontainer.
 */
  template<typename Function>
  DataContainer<T> Apply(const DataContainer<T> &data, Function &&lambda) const {
    DataContainer<T> result;
    std::vector<size_type> indices;
    unsigned long index = 0;
    if (axes_.size() > data.axes_.size()) {
      for (unsigned long iaxis = 0; iaxis < data.axes_.size() - 1; ++iaxis) {
        if (axes_[iaxis].Name()!=data.axes_[iaxis].Name()) {
          std::string errormsg = "Axes do not match.";
          throw std::logic_error(errormsg);
        }
      }
      result.AddAxes(axes_);
      indices.reserve(dimension_);
      for (const auto &bin_a : data_) {
        GetIndex(indices, index);
        result.data_[index] = lambda(bin_a, data.At(indices));
        ++index;
      }
    } else {
      for (unsigned long iaxis = axes_.size() - 1; iaxis > 0; --iaxis) {
        if (axes_[iaxis].Name()!=data.axes_[iaxis].Name()) {
          std::string errormsg = "Axes do not match.";
          throw std::logic_error(errormsg);
        }
      }
      result.AddAxes(data.axes_);
      indices.reserve(data.dimension_);
      for (const auto &bin_b : data.data_) {
        data.GetIndex(indices, index);
        result.data_[index] = lambda(At(indices), bin_b);
        ++index;
      }
    }
    return result;
  }

/**
 * Clears data to be filled. To be called after one event.
 */
  void ClearData() {
    auto size = data_.size();
    data_.assign(size, T());
  }
/**
 * Clear data at the specified postion.
 * @param position position at which to clear the data.
 */
  void ClearDataAt(const size_type position) {
    data_.at(position) = T();
  }

/**
 * Checks if datacontainer is integrated.
 * It is integrated if first axis is the integrated axis with Id == -1;
 * @return true if integrated, else false.
 */
  inline bool IsIntegrated() const { return integrated_; }

/**
 * Merges DataContainer with DataContainers in TCollection.
 * Function used in "hadd"
 * A function with signature T Merge( T, T) needs to be implemented for merging to work.
 * @param inputlist List of datacontainers
 * @return size of datacontainer. dummyvalue
 */
  Long64_t Merge(TCollection *inputlist) {
    TIter next(inputlist);
    while (auto data = (DataContainer<T> *) next()) {
      auto lambda = [](const T &a, const T &b) -> T { return Qn::Merge(a, b); };
      *this = this->Apply(*data, lambda);
    }
    return this->size();
  }

 private:
  bool integrated_ = true;      ///< Flag to show if container is integrated (only one bin)
  unsigned long dimension_ = 0; ///< dimensionality of data
  std::vector<T> data_;         ///< linearized vector of data
  QnAxes axes_;                 ///< Vector of axes
  std::vector<long> stride_;    ///< Offset for conversion into one dimensional vector.
  TList *list_ = nullptr;       ///!<! List to temporarily hold histograms when accessing with the TBrowser.
  friend Qn::DataContainerHelper;

  void Reset() {
    integrated_ = false;
    dimension_ = 0;
    data_.clear();
    axes_.clear();
    stride_.clear();
  }
/**
 * Calculates one dimensional index from a vector of indices.
 * @param index vector of indices in multiple dimensions
 * @return      index in one dimension
 */
  size_type GetLinearIndex(const std::vector<size_type> &index) const {
    size_type offset = (index[dimension_ - 1]);
    for (unsigned int i = 0; i < dimension_ - 1; ++i) {
      offset += stride_[i + 1]*(index[i]);
    }
    return offset;
  }

/**
 * Calculates linear index from coordinates
 * returns -1 if outside of range.
 * @param coordinates floating point coordinates
 * @return linear index
 */
  long GetLinearIndex(const std::vector<float> &coordinates) const {
    typename std::vector<size_type> indices;
    unsigned long axisindex = 0;
    for (const auto &axis : axes_) {
      auto bin = axis.FindBin(coordinates[axisindex]);
      if (bin >= (unsigned int) axis.size() || bin < 0) {
        return -1;
      } else {
        indices.push_back(static_cast<unsigned long &&>(bin));
        axisindex++;
      }
    }
    return GetLinearIndex(indices);
  }

/**
 * Calculates multidimensional index from coordinates
 * @param coordinates floating point coordinates
 * @return index belonging to coordinates
 */
  std::vector<size_type> GetIndex(const std::vector<float> &coordinates) const {
    std::vector<size_type> indices(dimension_);
    unsigned long axisindex = 0;
    for (const auto &axis : axes_) {
      auto bin = axis.FindBin(coordinates[axisindex]);
      if (bin < 0 || static_cast<size_type>(bin) > axis.size())
        throw std::out_of_range("bin out of specified range");
      indices[axisindex] = static_cast<size_type &&>(bin);
      axisindex++;
    }
    return indices;
  }

/**
 * Calculates offset for transformation into one dimensional vector.
 */
  void CalculateStride() {
    stride_[dimension_] = 1;
    for (unsigned int i = 0; i < dimension_; ++i) {
      stride_[dimension_ - i - 1] = stride_[dimension_ - i]*axes_[dimension_ - i - 1].size();
    }
  }

 public:
  /**
   * Set Bits of the Container to configure contents
   * see contents for possible settings
   * @param bits settings bits
   */
  void SetSetting(unsigned int bits) {(void)bits;}

  void ResetSetting(unsigned int bits) {(void)bits;}

//--------------------------------//
// Visualization methods for ROOT //
// Template specialization needed //
//--------------------------------//
/**
 * Can draw the DataContainer with up to two dimensions.
 * Implementration for template specializations please see below
 * @param option draw option
 * @param axis_name name of axis used for second dimension.
 */
  void NDraw(Option_t *option, const std::string &axis_name = "") {(void)option; (void)axis_name;}

/**
 * Display contents of DataContainer in TBrowser.
 * Implementation for template specializations please see below.
 * @param b TBrowser
 */
  virtual void Browse(TBrowser *b) {(void)b;}

/// \cond CLASSIMP
 ClassDef(DataContainer, 12);
/// \endcond
};

//-----------------------------------------//
// Common alias for types of DataContainer //
// needed for ROOT IO                      //
//-----------------------------------------//

using DataContainerProduct = DataContainer<Qn::Product>;
using DataContainerStats = DataContainer<Qn::Stats>;
using DataContainerQVector = DataContainer<Qn::QVector>;
using DataContainerDataVector = DataContainer<std::vector<DataVector>>;
using DataContainerEventShape = DataContainer<Qn::EventShape>;

//--------------------------------------------//
// Template specializations for visualisation //
//--------------------------------------------//

template<>
inline void DataContainer<Stats>::Browse(TBrowser *b) {
  DataContainerHelper::StatsBrowse(this, b);
}
template<>
inline void DataContainer<EventShape>::Browse(TBrowser *b) {
  DataContainerHelper::EventShapeBrowse(this, b);
}
template<>
inline void DataContainer<Stats>::NDraw(Option_t *option, const std::string &axis_name) {
  DataContainerHelper::NDraw(*this, option, axis_name);
}

//-----------------------------------//
// Template specializations for bits //
//-----------------------------------//
template<>
inline void DataContainer<Stats>::SetSetting(const unsigned int bits) {
  auto cleanbits = 0x1FFC000 & bits; // 0x1FFC000 bitmask with only bits from 14 - 24 on.
  SetBit(cleanbits, true);
  for (auto &bin : data_) {
    bin.SetBits(cleanbits);
  }
}

template<>
inline void DataContainer<Stats>::ResetSetting(const unsigned int bits) {
  auto cleanbits = 0x1FFC000 & bits; // 0x1FFC000 bitmask with only bits from 14 - 24 on.
  ResetBit(cleanbits);
  for (auto &bin : data_) {
    bin.ResetBits(cleanbits);
  }
}



//-----------------------------------------//
// Operations for DataContainer arithmetic //
//-----------------------------------------//
template<typename T>
DataContainer<T> operator+(const DataContainer<T> &a, const DataContainer<T> &b) {
  return a.Apply(b, [](const T &a, const T &b) { return a + b; });
}
template<typename T>
DataContainer<T> operator-(const DataContainer<T> &a, const DataContainer<T> &b) {
  return a.Apply(b, [](const T &a, const T &b) { return a - b; });
}
template<typename T>
DataContainer<T> operator*(const DataContainer<T> &a, const DataContainer<T> &b) {
  return a.Apply(b, [](const T &a, const T &b) { return a*b; });
}
template<typename T>
DataContainer<T> operator/(const DataContainer<T> &a, const DataContainer<T> &b) {
  return a.Apply(b, [](const T &a, const T &b) { return a/b; });
}
template<typename T>
DataContainer<T> operator*(const DataContainer<T> &a, double b) {
  return a.Map([b](const T &a) { return a*b; });
}
template<typename T>
DataContainer<T> Sqrt(const DataContainer<T> &a) {
  return a.Map([](const T &x) { return Qn::Sqrt(x); });
}
/**
 * Transformation of a DataContainer providing the operation:
 * \f[
 *      Bin_i=\Sum_j\neqi Bin_j
 * \f]
 * A transformed copy is returned.
 * @tparam T Type of Bincontent
 * @param input DataContainer to be transformed.
 * @return Transformed DataContainer
 */
template<typename T>
DataContainer<T> ExclusiveSum(const DataContainer<T> &input) {
  DataContainer<T> Summed(input);
  Summed.ClearData();
  for (auto ibin = std::begin(input); ibin < std::end(input); ++ibin) {
    for (auto jbin = std::begin(input); jbin < std::end(input); ++jbin) {
      if (ibin!=jbin) {
        Summed.At(std::distance(std::begin(input), ibin)) = Summed.At(std::distance(std::begin(input), ibin)) + *jbin;
      }
    }
  }
  return Summed;
}

template<>
Long64_t DataContainer<std::pair<bool,float>>::Merge(TCollection *inputlist) = delete;
template<>
Long64_t DataContainer<std::vector<DataVector>>::Merge(TCollection *inputlist) = delete;
template<>
Long64_t DataContainer<Qn::Product>::Merge(TCollection *inputlist) = delete;
template<>
Long64_t DataContainer<Qn::QVector>::Merge(TCollection *inputlist) = delete;

};
#endif