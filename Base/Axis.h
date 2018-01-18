//
// Created by Lukas Kreis on 29.06.17.
//

#ifndef FLOW_QNAXIS_H
#define FLOW_QNAXIS_H

#include <vector>
#include <string>
#include <stdexcept>
#include "Rtypes.h"

namespace Qn {
/**
 * @brief Parameter axis with variable bin widths
 *
 * Basic axis implementation
 */

class Axis {
 public:

  Axis() = default; ///< default constructor
  virtual ~Axis() = default; ///< default destructor

  /**
   * Constructor for variable or fixed bin width.
   * @param name name of axis.
   * @param bin_edges vector of bin edges. starting with lowest bin edge and ending with uppermost bin edge.
   */
  Axis(std::string name, std::vector<float> bin_edges)
      : name_(std::move(name)), bin_edges_(std::move(bin_edges)) {}

  /**
   * Constructor for fixed bin width. Calculates bin width automatically and sets bin edges.
   * @param name name of axis
   * @param nbins number of bins
   * @param lowbin lowest bin edge
   * @param upbin uppermost bin edge
   */
  Axis(std::string name, const int nbins, const float lowbin, const float upbin)
      : name_(std::move(name)) {
    for (int i = 0; i < nbins + 1; ++i) {
      float bin_width = (upbin - lowbin) / (float) nbins;
      bin_edges_.push_back(lowbin + i * bin_width);
    }
  }

  typedef typename std::vector<float>::const_iterator citerator;
  typedef typename std::vector<float>::iterator iterator;
  citerator begin() const { return bin_edges_.cbegin(); } ///< iterator for external use
  citerator end() const { return bin_edges_.cend(); } ///< iterator for external use
  iterator begin() { return bin_edges_.begin(); } ///< iterator for external use
  iterator end() { return bin_edges_.end(); } ///< iterator for external use
  /**
 * Set Name of axis.
 * @param name name of axis
 */
  inline void SetName(std::string name) { name_ = name; }
  /**
   * Returns Name of axis.
   * @return name of axis
   */
  inline std::string Name() const { return name_; }
  /**
   * Finds bin index for a given value
   * if value is smaller than lowest bin return -1.
   * @param value for finding corresponding bin
   * @return bin index
   */
  inline long FindBin(float value) const {
    long bin = 0;
    if (value < *bin_edges_.begin()) {
      bin = -1;
    } else {
      auto lb = std::lower_bound(bin_edges_.begin(), bin_edges_.end(), value);
      if (lb == bin_edges_.begin() || *lb == value)
        bin = (lb - bin_edges_.begin());
      else
        bin = (lb - bin_edges_.begin()) - 1;
    }
    if (bin >= (long) bin_edges_.size() - 1 || bin < 0)
      bin = -1;
    return bin;
  }

  /**
 * Finds bin iterator for a given value
 * if value is smaller than lowest bin returns end().
 * @param value for finding corresponding bin
 * @return bin index
 */
  inline citerator FindBinIter(float value) {
    citerator bin;
    if (value < *bin_edges_.begin()) {
      bin = end();
    } else {
      auto lb = std::lower_bound(bin_edges_.begin(), bin_edges_.end(), value);
      if (lb == bin_edges_.begin() || *lb == value)
        bin = lb;
      else
        bin = lb - 1;
    }
    if (*bin >= (long) bin_edges_.size() - 1 || *bin < 0) bin = end();
    return bin;
  }

  /**
   * Returns number of bins.
   * @return number of bins.
   */
  inline long size() const { return bin_edges_.size() - 1; }
  /**
   * Gets lower bin edge
   * @param bin Index of bin of interest
   * @return lower edge of bin of interest
   */
  inline float GetLowerBinEdge(long bin) const { return bin_edges_.at(bin); }
  /**
   * Gets upper bin edge
   * @param bin Index of bin of interest
   * @return upper edge of bin of interest
   */
  inline float GetUpperBinEdge(long bin) const { return bin_edges_.at(bin + 1); }

 private:
  std::string name_;
  std::vector<float> bin_edges_;



  /// \cond CLASSIMP
 ClassDef(Axis, 1);
  /// \endcond
};
}
#endif //FLOW_QNAXIS_H
