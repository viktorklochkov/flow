//
// Created by Lukas Kreis on 29.06.17.
//

#ifndef FLOW_QNAXIS_H
#define FLOW_QNAXIS_H

#include <vector>
#include <string>

/**
 * @brief Parameter axis with variable bin widths
 *
 * Basic axis implementation
 */
class QnAxis {
 public:
  QnAxis() = default;
  QnAxis(const std::string name, const std::vector<float> &bin_edges) : name_(name), bin_edges_(bin_edges) {}
  ~QnAxis() = default;
  typedef typename std::vector<float>::const_iterator iterator;
  iterator cbegin() { return bin_edges_.cbegin(); } ///< iterator for external use
  iterator cend() { return bin_edges_.cend(); } ///< iterator for external use
  inline const std::string Name() const { return name_; }
  /**
   * Finds bin index for a given value
   * @param value for finding corresponding bin
   * @return bin index
   */
  inline const long FindBin(float value) const {
    return std::distance(bin_edges_.begin(),
                         std::lower_bound(bin_edges_.begin(),
                                          bin_edges_.end(), value));
  }
  /**
   * Returns number of bins.
   * @return number of bins.
   */
  inline const long size() const { return bin_edges_.size(); }
 private:
  std::string name_;
  std::vector<float> bin_edges_;
};

#endif //FLOW_QNAXIS_H
