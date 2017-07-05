/**
 * @file QnDataContainer.h
 * @author Lukas Kreis
 */
#ifndef QNDATACONTAINER_H
#define QNDATACONTAINER_H

#include "Axis.h"

#include "QnCorrectionsQnVector.h"
#include "Rtypes.h"

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
class DataContainer {
 public:

  /*
   * Constructor
   * @param name Name of data container.
   */
  DataContainer() = default;
  DataContainer(std::string name) :
      name_(name),
      dimension_(0) {
  }
  ~DataContainer() = default;

  typedef typename std::vector<T>::const_iterator iterator;
  iterator cbegin() { return data_.cbegin(); } ///< iterator for external use
  iterator cend() { return data_.cend(); } ///< iterator for external use

  /*
   * Adds additional axis for storing the data.
   * @param name  Name of the axis
   * @param bins  Vector of bin edges
   */
  void AddAxis(std::string name, const std::vector<float> &bins) {
    if (std::find_if(axis_.begin(), axis_.end(), [name](const Axis &axis) { return axis.Name() == name; })
        != axis_.end())
      throw std::runtime_error("axis already defined in vector");
    axis_.emplace_back(name, bins);
    dimension_++;
    int totalbins = 1;
    for (const auto &axis : axis_) {
      totalbins *= axis.size() - 1;
    }
    data_.resize(totalbins);
    stride_.resize(dimension_ + 1);
    CalculateStride();
  }

  void AddAxis(std::string name, const int nbins, const float lowbin, const float upbin) {
    if (std::find_if(axis_.begin(), axis_.end(), [name](const Axis &axis) { return axis.Name() == name; })
        != axis_.end())
      throw std::runtime_error("axis already defined in vector");
    axis_.emplace_back(name, nbins, lowbin, upbin);
    dimension_++;
    int totalbins = 1;
    for (const auto &axis : axis_) {
      totalbins *= axis.size() - 1;
    }
    data_.resize(totalbins);
    stride_.resize(dimension_ + 1);
    CalculateStride();
  }

  /*
   * Adds a datavector by the variables
   * @param vect  Vector added into container
   * @param vars  Vector of Variables used to determine position in the container
   *              e.g. [p_t,eta] = [5 GeV, 0.5]
   */
  void SetElement(T &vect, const std::vector<float> &values) {
    std::vector<int> index;
    int axisindex = 0;
    for (auto axis : axis_) {
      int bin = (int) axis.FindBin(values.at(axisindex));
      if (bin >= axis.size() || bin <= 0)
        throw std::out_of_range("bin out of specified range");
      index.push_back(bin);
      axisindex++;
    }
    data_[GetLinearIndex(index)] = std::move(vect);
  }
  /*
   * Get element in the specified bin
   * @param bins Vector of bin indices of the desired element
   * @return     Element
   */
  T const &GetElement(const std::vector<int> &bins) {
    return data_.at(GetLinearIndex(bins));
  }

  /*
 * Get element with the specified value
 * @param bins Vector of value to search for desired element
 * @return     Element
 */
  T const &GetElement(const std::vector<float> &values) {
    std::vector<int> index;
    int axisindex = 0;
    for (auto axis : axis_) {
      int bin = (int) axis.FindBin(values.at(axisindex));
      if (bin >= axis.size() || bin <= 0)
        throw std::out_of_range("bin out of specified range");
      index.push_back(bin);
      axisindex++;
    }
    return data_.at(GetLinearIndex(index));
  }

  /*
   * Get Axis with the given name.
   *
   * Throws exception when not found.
   * @param name  Name of the desired axis
   * @return      Axis
   */
  Axis GetAxis(std::string name) {
    for (auto axis: axis_) {
      if (name == axis.Name()) return axis;
    }
    throw std::out_of_range("axis not found aborting");
  }

  /*
   * Calculates indices in multiple dimensions from linearized index
   * @param offset Index of linearized vector
   * @return       Vector of indices
   */
  std::vector<int> GetIndex(const long offset) {
    long temp = offset;
    std::vector<int> indices;
    indices.resize(dimension_);
    for (int i = 0; i < dimension_ - 1; ++i) {
      indices[i] = (int) offset % axis_[i].size() - 1;
      temp = temp / axis_[i].size() - 1;
    }
    indices[dimension_ - 1] = (int) temp;
    return indices;
  }
  /*
   * Clears data to be filled. To be called after one event.
   */
  void ClearData() {
    data_.clear();
  }

 private:
  std::string name_;  ///< name of data container
  int dimension_; ///< dimension of data container
  std::vector<T> data_; ///< Vector of data vectors
  std::vector<Axis> axis_; ///< Vector of axis
  std::vector<long> stride_; ///< Offset for conversion into one dimensional vector.

  /*
   * Calculates offset for transformation into one dimensional vector.
   */
  void CalculateStride() {
    stride_[dimension_] = 1;
    for (int i = 0; i < dimension_; ++i) {
      stride_[dimension_ - i - 1] = stride_[dimension_ - i] * axis_[dimension_ - i - 1].size() - 1;
    }
  }

  /*
   * Calculates one dimensional index from a vector of indices.
   * @param index vector of indices in multiple dimensions
   * @return      index in one dimension
   */
  long GetLinearIndex(const std::vector<int> &index) {
    long offset = (index[dimension_ - 1] - 1);
    for (int i = 0; i < dimension_ - 1; ++i) {
      offset += stride_[i + 1] * (index[i] - 1);
    }
    return offset;
  }
  /// \cond CLASSIMP
 ClassDef(DataContainer, 1);
  /// \endcond
};

typedef DataContainer<std::unique_ptr<QnCorrectionsQnVector>> DataContainerQn;
}
#endif
