/**
 * @file QnDataContainer.h
 * @author Lukas Kreis
 */
#ifndef QNDATACONTAINER_H
#define QNDATACONTAINER_H

#include "Axis.h"
#include "QnCorrections/QnCorrectionsQnVector.h"
#include "Rtypes.h"
#include "DataVector.h"
#include "QVector.h"
#include "Stats.h"

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>

/**
 * QnCorrectionsframework
 */
namespace Qn {
/**
 * @brief      Template container class for Q-vectors and correlations
 * @param T    Type of object inside of container
 */
template<typename T>
class DataContainer : public TObject {
 public:
/**
 * Constructor
 * @param name Name of data container.
 */
  DataContainer() = default;
  virtual ~DataContainer() = default;

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
  std::vector<QnCorrectionsQnVector>::size_type size() const { return data_.size(); }

/**
 * Adds axes for storing data
 * @param axes vector of axes
 */
  void AddAxes(const std::vector<Axis> &axes) {
    for (const auto &axis : axes) {
      AddAxis(axis);
    }
  }
/**
 * Adds existing axis for storing the data with variable binning
 * @param axis Axis to be added.
 */
  void AddAxis(const Axis &axis) {
    if (std::find_if(axes_.begin(), axes_.end(), [axis](const Axis &axisc) { return axisc.Name() == axis.Name(); })
        != axes_.end())
      throw std::runtime_error("axis already defined in vector");
    axes_.push_back(axis);
    dimension_++;
    std::vector<float>::size_type totalbins = 1;
    for (const auto &iaxis : axes_) {
      totalbins *= iaxis.size();
    }
    data_.resize(totalbins);
    stride_.resize((std::vector<long>::size_type) dimension_ + 1);
    CalculateStride();
  }

/**
 * Adds a element by the variables
 * @param vect  element added into container
 * @param vars  Vector of Variables used to determine position in the container
 *              e.g. [p_t,eta] = [5 GeV, 0.5]
 */
  void SetElement(T &vect, const std::vector<float> &values) {
    std::vector<long> index;
    std::vector<int>::size_type axisindex = 0;
    for (const auto &axis : axes_) {
      index.push_back(axis.FindBin(values.at(axisindex)));
      axisindex++;
    }
    data_[GetLinearIndex(index)] = std::move(vect);
  }

/**
 * Adds a element by the variables no bounds checking
 * @param vect  element added into container
 * @param index  linear index position
 */
  void SetElement(T &vect, const long index) {
    data_[index] = std::move(vect);
  }

/**
 * Get element in the specified bin
 * @param bins Vector of bin indices of the desired element
 * @return     Element
 */
  T const &GetElement(const std::vector<long> &bins) const {
    return data_.at(GetLinearIndex(bins));
  }

/**
 * Get element in the specified bin
 * @param linear_index index of element
 * @return     Element
 */
  T const &GetElement(int linear_index) const {
    return data_.at(linear_index);
  }

/**
  * Get element with the specified value
  * @param bins Vector of value to search for desired element
  * @return     Element
  */
  T const &GetElement(const std::vector<float> &values) const {
    std::vector<long> index;
    std::vector<int>::size_type axisindex = 0;
    for (const auto &axis : axes_) {
      auto bin = axis.FindBin(values.at(axisindex));
      if (bin >= axis.size() || bin < 0)
        throw std::out_of_range("bin out of specified range");
      index.push_back(bin);
      axisindex++;
    }
    return data_.at(GetLinearIndex(index));
  }

/**
 * Get vector of axes
 * @return Vector of axes
 */
  inline const std::vector<Axis> &GetAxes() const { return axes_; }

/**
 * Get Axis with the given name.
 * Throws exception when not found.
 * @param name  Name of the desired axis
 * @return      Axis
 */
  Axis GetAxis(const std::string name) const {
    for (auto axis: axes_) {
      if (name == axis.Name()) return axis;
    }
    throw std::out_of_range("axis not found aborting");
  }

/**
 * Calculates indices in multiple dimensions from linearized index
 * @param offset Index of linearized vector
 * @return       Vector of indices. Empty for invalid offset.
 */
  std::vector<long> GetIndex(const long offset) const {
    long temp = offset;
    std::vector<long> indices;
    if ((u_long) offset >= data_.size()) return indices;
    indices.resize(dimension_);
    for (int i = 0; i < dimension_ - 1; ++i) {
      indices[dimension_ - i - 1] = (int) (temp % axes_[dimension_ - i - 1].size());
      temp = temp / axes_[dimension_ - i - 1].size();
    }
    indices[0] = (int) temp;
    return indices;
  }

/**
 * Gives description by concenating axis names with coordinates
 * @param offset linear index
 * @return string with bin description
 */
  std::string GetBinDescription(const long offset) const {
    auto indices = GetIndex(offset);
    if (indices.empty()) return "invalid offset";
    std::string outstring;
    int i = 0;
    for (auto it = axes_.cbegin(); it != axes_.cend(); ++it) {
      const auto &axis = *it;
      outstring += axis.Name();
      outstring += "(" + std::to_string(axis.GetLowerBinEdge(indices[i])) + ", "
          + std::to_string(axis.GetUpperBinEdge(indices[i])) + ")";
      if (it + 1 != axes_.cend()) outstring += "; ";
      ++i;
    }
    return outstring;
  }
/**
 * Projects datacontainer on a subset of axes
 * @tparam Function typename of function.
 * @param axes subset of axes used for the projection.
 * @param lambda Function used to add two entries.
 * @return projected datacontainer.
 */
  template<typename Function>
  DataContainer<T> Projection(const std::vector<Axis> &axes, Function &&lambda) const {
    DataContainer<T> projection;
    int linearindex = 0;
    std::vector<bool> isprojected;
    auto originalaxes = this->GetAxes();
    for (const auto &originalaxis : originalaxes) {
      for (const auto &axis : axes) {
        isprojected.push_back((originalaxis.Name() == axis.Name()) == 0);
      }
    }
    if (axes.empty()) {
      Qn::Axis integrated("integrated", 1, 0, 1, -1);
      projection.AddAxis(integrated);
      for (const auto &bin : data_) {
        long index = 0;
        projection.AddElement(index, lambda, bin);
      }
    } else {
      projection.AddAxes(axes);
      for (const auto &bin : data_) {
        auto indices = this->GetIndex(linearindex);
        for (auto index = indices.begin(); index < indices.end(); ++index) {
          if (isprojected[std::distance(indices.begin(), index)]) indices.erase(index);
        }
        projection.AddElement(indices, lambda, bin);
        ++linearindex;
      }
    }
    return projection;
  }

/**
 * "Integrates" container
 * @tparam Function typename of function.
 * @param axes subset of axes used for the projection.
 * @param lambda Function used to add two entries.
 * @return integrated container
 */
  template<typename Function>
  DataContainer<T> Projection(Function &&lambda) const {
    DataContainer<T> projection;
    Qn::Axis integrated("integrated", 1, 0, 1, -1);
    projection.AddAxis(integrated);
    for (const auto &bin : data_) {
      long index = 0;
      projection.AddElement(index, lambda, bin);
    }
    return projection;
  }
/**
 * Calls function on element specified by indices.
 * @tparam Function type of function to be called on the object
 * @param indices indicies multidimensional indices of the element.
 * @param lambda function to be called on the element. Takes element of type T as an argument.
 */
  template<typename Function>
  void CallOnElement(const std::vector<long> &indices, Function &&lambda) {
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
 * Calls function on element specified by indices.
 * @tparam Function type of function to be called on the object
 * @param indices multidimensional indices of the element.
 * @param lambda function to be called on the element. Takes element of type T as an argument.
 * @param element element to be added.
 */
  template<typename Function>
  void AddElement(const std::vector<long> &indices, Function &&lambda, T element) {
    auto &oelement = data_.at(GetLinearIndex(indices));
    auto add = [&lambda](T &e1, T &e2) { e1 = lambda(e1, e2); };
    add(oelement, element);
  }

/**
 * Calls function on element specified by indices.
 * @tparam Function type of function to be called on the object
 * @param index index of the element.
 * @param lambda function to be called on the element. Takes element of type T as an argument.
 * @param element element to be added.
 */
  template<typename Function>
  void AddElement(const long index, Function &&lambda, T element) {
    auto add = [&lambda](T &e1, T &e2) { e1 = lambda(e1, e2); };
    add(data_[index], element);
  }

/**
 * Map function to datacontainer. Does not modify the original container.
 * @tparam Function function
 * @param lambda unary function to be applied to each element.
 * @return datacontainer after applying function.
 */
  template<typename Function>
  DataContainer<T> Map(Function &&lambda) {
    DataContainer<T> result(*this);
    std::transform(data_.begin(), data_.end(), result.begin(), [&lambda](T &element) { return lambda(element); });
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
  DataContainer<T> Select(const Qn::Axis &axis) const {
    DataContainer<T> selected;
    long axisposition = 0;
    long tmpaxisposition = 0;
    for (const auto &a : axes_) {
      if (a.Name() == axis.Name()) {
        selected.AddAxis(axis);
        axisposition = tmpaxisposition;
      } else {
        selected.AddAxis(a);
      }
      tmpaxisposition++;
    }
    long index = 0;
    for (const auto &bin : data_) {
      auto indices = GetIndex(index);
      auto binlow = axes_[axisposition].GetLowerBinEdge(indices[axisposition]);
      auto binhigh = axes_[axisposition].GetUpperBinEdge(indices[axisposition]);
      auto binmid = binlow + (binhigh - binlow) / 2;
      auto rebinnedindex = axis.FindBin(binmid);
      if (rebinnedindex != -1) {
        indices[axisposition] = rebinnedindex;
        selected.CallOnElement(indices, [&bin](T &element) { element = bin; });
      }
      ++index;
    }
    if (axis.size() == 1) {
      selected.axes_.erase(selected.axes_.begin()+axisposition);
      selected.stride_.resize(selected.axes_.size()+1);
      selected.dimension_ = selected.axes_.size();
      selected.CalculateStride();
    }
    return selected;
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
    std::vector<long> indices;
    long index = 0;
    if (axes_.size() > data.axes_.size()) {
      for (long iaxis = 0; data.axes_.size() - 1; ++iaxis) {
        if (axes_[iaxis].Id() != data.axes_[iaxis].Id()) {
          std::string errormsg = "Axes do not match.";
          throw std::logic_error(errormsg);
        }
      }
      result.AddAxes(axes_);
      indices.reserve((unsigned long) dimension_);
      for (const auto &bin_a : data_) {
        indices = GetIndex(index);
        auto bin_b = data.GetElement(indices);
        result.CallOnElement(indices, [&lambda, &bin_a, &bin_b](T &element) { element = lambda(bin_a, bin_b); });
        ++index;
      }
    } else {
      for (long iaxis = axes_.size() - 1; iaxis > 0; --iaxis) {
        if (axes_[iaxis].Id() != data.axes_[iaxis].Id()) {
          std::string errormsg = "Axes do not match.";
          throw std::logic_error(errormsg);
        }
      }
      result.AddAxes(data.axes_);
      indices.reserve((unsigned long) data.dimension_);
      for (const auto &bin_b : data.data_) {
        indices = data.GetIndex(index);
        auto bin_a = GetElement(indices);
        result.CallOnElement(indices, [&lambda, &bin_a, &bin_b](T &element) { element = lambda(bin_a, bin_b); });
        ++index;
      }
    }
    return result;
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
    long axisposition = -1;
    bool axisfound = false;
    /*
     * Check if axis to be rebinned is found in the datacontainer.
     */
    for (auto axis = std::begin(axes_); axis < std::end(axes_); ++axis) {
      if (axis->Name() == rebinaxis.Name()) {
        rebinned.AddAxis(rebinaxis);
        axisposition = std::distance(axes_.begin(), axis);
        axisfound = true;
      } else {
        rebinned.AddAxis(*axis);
      }
    }
    if (!axisfound) {
      std::string errormsg = "Datacontainer does not have axis of name " + rebinaxis.Name();
      throw std::logic_error(errormsg);
    }
    /*
     * Check if there is no overlap in the bin edges.
     */
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
    long ibin = 0;
    for (const auto &bin : data_) {
      auto indices = GetIndex(ibin);
      auto binlow = axes_[axisposition].GetLowerBinEdge(indices[axisposition]);
      auto binhigh = axes_[axisposition].GetUpperBinEdge(indices[axisposition]);
      auto binmid = binlow + (binhigh - binlow) / 2;
      auto rebinnedindex = rebinaxis.FindBin(binmid);
      indices[axisposition] = rebinnedindex;
      rebinned.AddElement(indices, lambda, bin);
      ++ibin;
    }
    return rebinned;
  }

  /*
   * Clears data to be filled. To be called after one event.
   */
  void ClearData() {
    auto size = data_.size();
    data_.clear();
    data_.resize(size);
  }
/**
 * Calculates one dimensional index from a vector of indices.
 * @param index vector of indices in multiple dimensions
 * @return      index in one dimension
 */
  long GetLinearIndex(const std::vector<long> &index) const {
    long offset = (index[dimension_ - 1]);
    for (int i = 0; i < dimension_ - 1; ++i) {
      offset += stride_[i + 1] * (index[i]);
    }
    return offset;
  }
/**
 * Checks if datacontainer is integrated.
 * It is integrated if first axis is the integrated axis with Id == -1;
 * @return true if integrated, else false.
 */
  inline bool IsIntegrated() const { return axes_[0].Id() == -1; }

 private:
  int dimension_ = 0; ///< dimensionality of data
  std::vector<T> data_; ///< linearized vector of data
  std::vector<Axis> axes_; ///< Vector of axes
  std::vector<long> stride_; ///< Offset for conversion into one dimensional vector.

/**
 * Calculates multidimensional index from coordinates
 * @param coordinates floating point coordinates
 * @return index belonging to coordinates
 */
  std::vector<long> GetIndex(const std::vector<float> &coordinates) const {
    std::vector<long> indices;
    u_long axisindex = 0;
    for (const auto &axis : axes_) {
      auto bin = axis.FindBin(coordinates[axisindex]);
      if (bin < 0)
        throw std::out_of_range("bin out of specified range");
      indices.push_back(bin);
      axisindex++;
    }
    return indices;
  }

/**
 * Calculates offset for transformation into one dimensional vector.
 */
  void CalculateStride() {
    stride_[dimension_] = 1;
    for (int i = 0; i < dimension_; ++i) {
      stride_[dimension_ - i - 1] = stride_[dimension_ - i] * axes_[dimension_ - i - 1].size();
    }
  }

  /// \cond CLASSIMP
 ClassDef(DataContainer, 6);
  /// \endcond
};

using DataContainerQn = DataContainer<QnCorrectionsQnVector>;
using DataContainerF = DataContainer<float>;
using DataContainerVF = DataContainer<std::vector<float>>;
using DataContainerStat = DataContainer<Qn::Statistics>;
using DataContainerDataVector = DataContainer<std::vector<DataVector>>;
using DataContainerQVector = DataContainer<Qn::QVector>;
}

#endif
