/**
 * @file QnDataContainer.h
 * @author Lukas Kreis
 */
#ifndef QNDATACONTAINER_H
#define QNDATACONTAINER_H

#include "Axis.h"

#include "QnCorrections/QnCorrectionsQnVector.h"
#include "Correlation.h"
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
//      name_(name),
      dimension_(0) {
  }
  ~DataContainer() = default;

  using iterator = typename std::vector<T>::const_iterator;
  iterator cbegin() const { return data_.cbegin(); } ///< iterator for external use
  iterator cend() const { return data_.cend(); } ///< iterator for external use
  iterator begin() const { return data_.begin(); } ///< iterator for external use
  iterator end() const { return data_.end(); } ///< iterator for external use
  /**
   * Size of data container
   * @return number of entries in the container
   */
  std::vector<QnCorrectionsQnVector>::size_type size() const {return data_.size();}
  /**
   * Adds existing axis for storing the data with variable binning
   * @param Axis
   */
  void AddAxis(const Axis &axis) {
    if (std::find_if(axis_.begin(), axis_.end(), [axis](const Axis &axis_c) { return axis_c.Name() == axis.Name(); })
        != axis_.end())
      throw std::runtime_error("axis already defined in vector");
    axis_.push_back(axis);
    dimension_++;
    std::vector<float>::size_type totalbins = 1;
    for (const auto &iaxis : axis_) {
      totalbins *= iaxis.size();
    }
    data_.resize(totalbins);
    stride_.resize((std::vector<long>::size_type) dimension_ + 1);
    CalculateStride();
  }
  /**
   * Adds additional axis for storing the data with variable binning
   * @param name  Name of the axis
   * @param bins  Vector of bin edges
   */
  void AddAxis(const std::string name, const std::vector<float> &bins) {
    if (std::find_if(axis_.begin(), axis_.end(), [name](const Axis &axis) { return axis.Name() == name; })
        != axis_.end())
      throw std::runtime_error("axis already defined in vector");
    axis_.emplace_back(name, bins);
    dimension_++;
    std::vector<float>::size_type totalbins = 1;
    for (const auto &axis : axis_) {
      totalbins *= axis.size();
    }
    data_.resize(totalbins);
    stride_.resize((std::vector<long>::size_type) dimension_ + 1);
    CalculateStride();
  }
  /**
   * Adds additional axis for storing the data with fixed binning.
   * @param name Name of the axis
   * @param nbins Number of bins
   * @param lowbin
   * @param upbin
   */
  void AddAxis(const std::string name, const int nbins, const float lowbin, const float upbin) {
    if (std::find_if(axis_.begin(), axis_.end(), [name](const Axis &axis) { return axis.Name() == name; })
        != axis_.end())
      throw std::runtime_error("axis already defined in vector");
    axis_.emplace_back(name, nbins, lowbin, upbin);
    dimension_++;
    std::vector<float>::size_type totalbins = 1;
    for (const auto &axis : axis_) {
      totalbins *= axis.size();
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
    for (const auto &axis : axis_) {
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
  void SetElement(T vect, const long index) {
    data_[index] = std::move(vect);
  }
  /*
   * Get element in the specified bin
   * @param bins Vector of bin indices of the desired element
   * @return     Element
   */
  T const &GetElement(const std::vector<long> &bins) {
    return data_.at(GetLinearIndex(bins));
  }

  /*
 * Get element with the specified value
 * @param bins Vector of value to search for desired element
 * @return     Element
 */
  T const &GetElement(const std::vector<float> &values) {
    std::vector<long> index;
    std::vector<int>::size_type axisindex = 0;
    for (auto axis : axis_) {
      auto bin = axis.FindBin(values.at(axisindex));
      if (bin >= axis.size() || bin < 0)
        throw std::out_of_range("bin out of specified range");
      index.push_back(bin);
      axisindex++;
    }
    return data_.at(GetLinearIndex(index));
  }
  /**
   * Get vector of axis
   * @return Vector of axices
   */
  std::vector<Axis> GetAxices() const { return axis_;}
  /*
   * Get Axis with the given name.
   *
   * Throws exception when not found.
   * @param name  Name of the desired axis
   * @return      Axis
   */
  Axis GetAxis(std::string name) const {
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
  std::vector<int> GetIndex(const long offset) const {
    long temp = offset;
    std::vector<int> indices;
    indices.resize((std::vector<int>::size_type) dimension_);
    for (int i = 0; i < dimension_ - 1; ++i) {
      indices[i] = (int) (offset % axis_[i].size());
      temp = temp / axis_[i].size();
    }
    indices[dimension_ - 1] = (int) temp;
    return indices;
  }
  /*
   * Clears data to be filled. To be called after one event.
   */
  void ClearData() {
//    for (auto &element : data_) {
//      element.reset(nullptr);
//    }
    auto size = data_.size();
    data_.clear();
    data_.resize(size);
  }

 private:
  char dimension_; ///< dimensionality of data
  std::vector<T> data_; ///< linearized vector of data
  std::vector<Axis> axis_; ///< Vector of axis
  std::vector<long> stride_; ///< Offset for conversion into one dimensional vector.

  /*
   * Calculates offset for transformation into one dimensional vector.
   */
  void CalculateStride() {
    stride_[dimension_] = 1;
    for (int i = 0; i < dimension_; ++i) {
      stride_[dimension_ - i - 1] = stride_[dimension_ - i] * axis_[dimension_ - i - 1].size();
    }
  }

  /*
   * Calculates one dimensional index from a vector of indices.
   * @param index vector of indices in multiple dimensions
   * @return      index in one dimension
   */
  long GetLinearIndex(const std::vector<long> &index) {
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

//typedef DataContainer<std::unique_ptr<const QnCorrectionsQnVector>> DataContainerQn;
typedef DataContainer<QnCorrectionsQnVector> DataContainerTest;
//typedef DataContainer<std::unique_ptr<Correlation>> DataContainerC;
}
#endif
