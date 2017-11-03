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

  /*
   * Constructor
   * @param name Name of data container.
   */
  DataContainer() = default;
  ~DataContainer() = default;

  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;
  const_iterator cbegin() const { return data_.cbegin(); } ///< iterator for external use
  const_iterator cend() const { return data_.cend(); } ///< iterator for external use
  iterator begin() { return data_.begin(); } ///< iterator for external use
  iterator end() { return data_.end(); } ///< iterator for external use

  /*
   * Size of data container
   * @return number of entries in the container
   */
  std::vector<QnCorrectionsQnVector>::size_type size() const { return data_.size(); }

  /*
   * Adds existing axis for storing the data with variable binning
   * @param Axis
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

  /*
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

  /*
  * Adds a element by the variables no bounds checking
  * @param vect  element added into container
  * @param index  linear index position
  */
  void SetElement(T vect, const long index) {
    data_[index] = std::move(vect);
  }
  /**
   * Get element in the specified bin
   * @param bins Vector of bin indices of the desired element
   * @return     Element
   */
  T &GetElement(const std::vector<long> &bins) const {
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
  /*
  * Get element with the specified value
  * @param bins Vector of value to search for desired element
  * @return     Element
  */
  T const &GetElement(const std::vector<float> &values) {
    std::vector<long> index;
    std::vector<int>::size_type axisindex = 0;
    for (auto axis : axes_) {
      auto bin = axis.FindBin(values.at(axisindex));
      if (bin >= axis.size() || bin < 0)
        throw std::out_of_range("bin out of specified range");
      index.push_back(bin);
      axisindex++;
    }
    return data_.at(GetLinearIndex(index));
  }
/*
* Get element with the specified value to be able to modify it.
* @param bins Vector of value to search for desired element
* @return     Element
*/
  T &ModifyElement(const std::vector<float> &values) {
    std::vector<long> index;
    std::vector<int>::size_type axisindex = 0;
    for (auto axis : axes_) {
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
  inline std::vector<Axis> const &GetAxes() const { return axes_; }
  /**
   * Get Axis with the given name.
   *
   * Throws exception when not found.
   * @param name  Name of the desired axis
   * @return      Axis
   */
  Axis GetAxis(std::string name) const {
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
  std::vector<int> GetIndex(const long offset) const {
    long temp = offset;
    std::vector<int> indices = {};
    if (offset >= data_.size()) return indices;
    indices.resize((std::vector<int>::size_type) dimension_);
    for (int i = 0; i < dimension_ - 1; ++i) {
      indices[i] = (int) (temp % axes_[i].size());
      temp = temp / axes_[i].size();
    }
    indices[dimension_ - 1] = (int) temp;
    return indices;
  }
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
  /*
   * Clears data to be filled. To be called after one event.
   */
  void ClearData() {
    auto size = data_.size();
    data_.clear();
    data_.resize(size);
  }

 private:
  int dimension_ = 0; ///< dimensionality of data
  std::vector<T> data_; ///< linearized vector of data
  std::vector<Axis> axes_; ///< Vector of axes
  std::vector<long> stride_; ///< Offset for conversion into one dimensional vector.

  /*
   * Calculates offset for transformation into one dimensional vector.
   */
  void CalculateStride() {
    stride_[dimension_] = 1;
    for (int i = 0; i < dimension_; ++i) {
      stride_[dimension_ - i - 1] = stride_[dimension_ - i] * axes_[dimension_ - i - 1].size();
    }
  }

  /*
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
  /// \cond CLASSIMP
 ClassDef(DataContainer, 1);
  /// \endcond
};

using DataContainerQn = DataContainer<QnCorrectionsQnVector>;
using DataContainerDataVector = DataContainer<std::vector<DataVector>>;
}

#endif
