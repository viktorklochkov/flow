//
// Created by Lukas Kreis on 29.06.17.
//

#ifndef FLOW_QNAXIS_H
#define FLOW_QNAXIS_H

#include <vector>
#include <string>

namespace Qn {
/**
 * @brief Parameter axis with variable bin widths
 *
 * Basic axis implementation
 */
class Axis {
 public:

  Axis() = default; ///< default constructor

  /**
   * Constructor for variable or fixed bin width.
   * @param name name of axis.
   * @param bin_edges vector of bin edges. starting with lowest bin edge and ending with uppermost bin edge.
   */
  Axis(const std::string name, const std::vector<float> &bin_edges) : name_(name), bin_edges_(bin_edges) {}

  /**
   * Constructor for fixed bin width. Calculates bin width automatically and sets bin edges.
   * @param name name of axis
   * @param nbins number of bins
   * @param lowbin lowest bin edge
   * @param upbin uppermost bin edge
   */
  Axis(const std::string name, const int nbins, const float lowbin, const float upbin) : name_(name) {
    for (int i = 0; i < nbins + 1; ++i) {
      float bin_width = (upbin - lowbin) / (float) nbins;
      bin_edges_.push_back(lowbin + i * bin_width);
    }
  }

  ~Axis() = default; ///< default destructor

  typedef typename std::vector<float>::const_iterator iterator;
  iterator cbegin() { return bin_edges_.cbegin(); } ///< iterator for external use
  iterator cend() { return bin_edges_.cend(); } ///< iterator for external use
  /**
   * Returns Name of axis.
   * @return name of axis
   */
  inline const std::string Name() const { return name_; }
  /**
   * Finds bin index for a given value
   * if value is smaller than lowest bin return -1.
   * @param value for finding corresponding bin
   * @return bin index
   */
  inline const long FindBin(float value) const {
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
    if (bin >= bin_edges_.size() - 1 || bin < 0) throw "value out of bin range";
    return bin;
  }
  /**
   * Returns number of bins.
   * @return number of bins.
   */
  inline const long size() const { return bin_edges_.size(); }
  /**
   * Gets lower bin edge
   * @param bin Index of bin of interest
   * @return lower edge of bin of interest
   */
  inline const float GetLowerBinEdge(int bin) { return bin_edges_.at(bin); }
  /**
   * Gets upper bin edge
   * @param bin Index of bin of interest
   * @return upper edge of bin of interest
   */
  inline const float GetUpperBinEdge(int bin) { return bin_edges_.at(bin + 1); }
 private:
  std::string name_;
  std::vector<float> bin_edges_;
};
}
#endif //FLOW_QNAXIS_H
