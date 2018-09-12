//
// Created by Lukas Kreis on 20.11.17.
//

#ifndef FLOW_DATACONTAINERHELPER_H
#define FLOW_DATACONTAINERHELPER_H

#include "TGraphErrors.h"
#include "TMultiGraph.h"

#include "Profile.h"
#include "Sample.h"
#include "EventShape.h"

namespace Qn {
//Forward declaration of DataContainer
template<typename T>
class DataContainer;
namespace Internal {
/**
 * Helper class for DataContainer used for visualization.
 */
struct DataContainerHelper {
/**
 * Create a profile graph from a one dimensional DataContainerVF, which containing a distribution of floating point numbers in each bin.
 * @param data Datacontainer with one Axis to plot as a TGraphErrors.
 * @return A graph with errors corresponding to the standard error of the mean.
 */
  static TGraphErrors *DataToProfileGraph(const Qn::DataContainer<Qn::Sample> &data);
/**
 * Create a TMultiGraph containing profile graphs for each bin of the specified axis.
 * @param data datacontainer to be graphed.
 * @param axisname name of the axis which is used for the multigraph.
 * @return graph containing a profile graph for each bin.
 */
  static TMultiGraph *DataToMultiGraph(const Qn::DataContainer<Qn::Sample> &data, const std::string &axisname);

  static void SampleBrowse(Qn::DataContainer<Qn::Sample> &data, TBrowser *b);

  static void EventShapeBrowse(Qn::DataContainer<Qn::EventShape> &data, TBrowser *b);

  static void NDraw(Qn::DataContainer<Qn::Sample> &data, Option_t *option, std::string axis_name);

};
}
}

#endif //FLOW_DATACONTAINERHELPER_H
