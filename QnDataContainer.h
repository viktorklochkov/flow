/**
 * @file QnDataContainer.h
 * @author Lukas Kreis
 */
#ifndef QNDATACONTAINER_H
#define QNDATACONTAINER_H

#include <vector>
#include <stdexcept>
#include <memory>
#include "QnCorrectionsQnVector.h"
/**
 * @brief      Template container class for Q-vectors and correlations
 * @param T    Type of object inside of container
 */
template<class T>
class QnDataContainer {
 public:
  struct Axis {
    Axis() = default;
    Axis(const std::string axisname, const std::vector<float> &bins) : name(axisname), bin_edges(bins) {}
    ~Axis() = default;
    std::string name;
    std::vector<float> bin_edges;
  };

  typedef typename std::vector<T>::const_iterator iterator;

/**
 * Constructor
 * @param name Name of data container.
 */

  QnDataContainer() = default;
  QnDataContainer(std::string &name) :
      name_(name),
      dimension_(0) {
  }

  ~QnDataContainer() = default;

  iterator cbegin() { return data_.cbegin(); } ///< iterator for external use

  iterator cend() { return data_.cend(); } ///< iterator for external use

/**
 * Adds additional axis for storing the data.
 * @param name  Name of the axis
 * @param bins  Vector of bin edges
 */
  void AddAxis(std::string name, const std::vector<float> &bins) {
    if (std::find_if(axis_.begin(), axis_.end(), [name](const Axis &axis) { return axis.name == name; }) != axis_.end())
      throw std::runtime_error("axis already defined in vector");
    axis_.emplace_back(name, bins);
    dimension_++;
    u_int totalbins = 1;
    for (const auto &axis : axis_) {
      totalbins *= axis.bin_edges.size() - 1;
    }
    data_.resize(totalbins);
    stride_.resize(dimension_ + 1);
    CalculateStride();
  }

/**
 * Adds a datavector by the variables
 * @param vect  Vector added into container
 * @param vars  Vector of Variables used to determine position in the container
 *              e.g. [p_t,eta] = [5 GeV, 0.5]
 */
  void AddVector(T &vect, const std::vector<float> &vars) {
    std::vector<int> index;
    int axisindex = 0;
    for (auto axis : axis_) {
      int bin = (int) std::distance(axis.bin_edges.begin(),
                                    std::lower_bound(axis.bin_edges.begin(), axis.bin_edges.end(), vars.at(axisindex)));
      if (bin == axis.bin_edges.size() || bin == 0)
        throw std::out_of_range("bin out of specified range");
      index.push_back(bin);
      axisindex++;
    }
    data_[GetLinearIndex(index)] = std::move(vect);
  }
/**
 * Get Vector in the specified bins
 * @param bins Data vector of bin indices of the desired vector
 * @return     Data vector
 */
  T GetVector(const std::vector<int> &bins) {
    return data_.at(GetLinearIndex(bins));
  }

/**
 * Get axis with the given name. Throws exception when not found.
 * @param name  Name of the desired axis
 * @return      axis
 */
  std::vector<float> GetAxis(std::string name) {
    for (auto axis: axis_) {
      if (name == axis.name) return axis;
    }
    throw std::out_of_range("axis not found aborting");
  }

  /**
   * Calculates indices in multiple dimensions from linearized index
   * @param offset Index of linearized vector
   * @return       Vector of indices
   */
  std::vector<int> GetIndex(const long offset) {
    long temp = offset;
    std::vector<int> indices;
    indices.resize(dimension_);
    for (Int_t i = 0; i < dimension_ - 1; ++i) {
      indices[i] = (int) offset % axis_[i].bin_edges.size() - 1;
      temp = temp / axis_[i].bin_edges.size() - 1;
    }
    indices[dimension_ - 1] = (int) temp;
    return indices;
  }

 private:
  std::string name_;  ///< name of data container
  u_int dimension_; ///< dimension of data container
  std::vector<T> data_; ///< Vector of data vectors
  std::vector<Axis> axis_; ///< Vector of axis
  std::vector<long> stride_; ///< Offset for conversion into one dimensional vector.

/**
 * Calculates offset for transformation into one dimensional vector.
 */
  void CalculateStride() {
    stride_[dimension_] = 1;
    for (Int_t i = 0; i < dimension_; ++i) {
      stride_[dimension_ - i - 1] = stride_[dimension_ - i] * axis_[dimension_ - i - 1].bin_edges.size() - 1;
    }
  }

/**
 * Calculates one dimensional index from a vector of indices.
 * @param index vector of indices in multiple dimensions
 * @return      index in one dimension
 */
  long GetLinearIndex(const std::vector<int> &index) {
    long offset = (index[dimension_ - 1] - 1);
    for (Int_t i = 0; i < dimension_ - 1; ++i) {
      offset += stride_[i + 1] * (index[i] - 1);
    }
    return offset;
  }
};

typedef QnDataContainer<std::unique_ptr<QnCorrectionsQnVector>> QnDataContainerQn;

#endif
