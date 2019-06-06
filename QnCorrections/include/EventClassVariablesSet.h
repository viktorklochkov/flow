#ifndef QNCORRECTIONS_EVENTCLASSVARSET_H
#define QNCORRECTIONS_EVENTCLASSVARSET_H
/***************************************************************************
 * Package:       FlowVectorCorrections                                    *
 * Authors:       Jaap Onderwaater, GSI, jacobus.onderwaater@cern.ch       *
 *                Ilya Selyuzhenkov, GSI, ilya.selyuzhenkov@gmail.com      *
 *                Víctor González, UCM, victor.gonzalez@cern.ch            *
 *                Contributors are mentioned in the code where appropriate.*
 * Development:   2012-2016                                                *
 * See cxx source for GPL licence et. al.                                  *
 ***************************************************************************/

/// \file QnCorrectionsEventClassVariablesSet.h
/// \brief Class that models the set of variables that define an event class for the Q vector correction framework

#include "EventClassVariable.h"
namespace Qn {
/// \class QnCorrectionsEventClassVariablesSet
/// \brief The set of variables which define an event class
///
/// Array of EventClassVariables that fully define the different
/// event classes considered within the Q vector correction framework.
/// The objects of this class are associated to concrete
/// detectors or detector configurations in such a way that
/// all Q vector corrections are performed according to the
/// event class the involved event is allocated.
///
/// Inherits all the methods of TObjArray specially the
/// subscript [] operator and Add method that allows
/// the array to expand.
///
/// The event class variables objects are not own by the array so,
/// they are not destroyed when the the set is destroyed. This allows
/// to create several sets with the same event class variables.
/// Pay attention to this when you create your event class variables,
/// they should live at least the same time you expect the sets to
/// live.
///
/// \author Jaap Onderwaater <jacobus.onderwaater@cern.ch>, GSI
/// \author Ilya Selyuzhenkov <ilya.selyuzhenkov@gmail.com>, GSI
/// \author Víctor González <victor.gonzalez@cern.ch>, UCM
/// \date Jan 4, 2016


class EventClassVariablesSet {
 public:
  EventClassVariablesSet() = default;
  /// Normal constructor
  /// \param n number of variables in the set
  EventClassVariablesSet(int initialsize) : fVariables(initialsize) {}
  /// Copy constructor
  /// \param tocopy the object instance to be copied
  EventClassVariablesSet(const EventClassVariablesSet &tocopy) = default;
  /// Default destructor
  virtual ~EventClassVariablesSet() = default;
  std::vector<EventClassVariable>::iterator begin() { return fVariables.begin(); }
  std::vector<EventClassVariable>::iterator end() { return fVariables.end(); }
  std::vector<EventClassVariable>::const_iterator begin() const { return fVariables.begin(); }
  std::vector<EventClassVariable>::const_iterator end() const { return fVariables.end(); }
  EventClassVariable &At(unsigned int i) { return fVariables.at(i); }
  const EventClassVariable &At(unsigned int i) const { return fVariables.at(i); }
  void Add(EventClassVariable variable) { return fVariables.push_back(variable); }
  template<typename... ARGS>
  void Add(ARGS &&... args) { return fVariables.emplace_back(std::forward<ARGS>(args)...); }
  std::vector<EventClassVariable>::size_type size() const { return fVariables.size(); }
  void GetMultidimensionalConfiguration(Int_t *nbins, Double_t *minvals, Double_t *maxvals) const;
 private:
  std::vector<EventClassVariable> fVariables;
/// \cond CLASSIMP
 ClassDef(EventClassVariablesSet, 1);
/// \endcond
};
}
#endif /* QNCORRECTIONS_EVENTCLASSVARSET_H */
