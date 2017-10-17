//
// Created by Lukas Kreis on 17.10.17.
//

#ifndef FLOW_CORRELATION_H
#define FLOW_CORRELATION_H

#include <TProfile.h>
#include "EventInfo.h"
#include "DataContainer.h"
namespace Correlation {

//class Correlator {
// public:
//  Correlator(Qn::EventInfoF eventinfo) : eventinfo_(eventinfo) {
//
//  }
//  ~Correlator();
//
//  float operator()(float x, float y) {
//    return x * y;
//  };
// private:
//  Qn::EventInfoF eventinfo_;
//};
enum class Component {
  X,
  Y
};

class Correlation {
 public:
  Correlation(std::vector<Qn::Axis> axes,
              std::tuple<Qn::DataContainerQn, Component, int> A,
              std::tuple<Qn::DataContainerQn, Component, int> B) :
      histograms_(),
      component_a_(A),
      component_b_(B) {
    for (const auto &axis : axes) {
      histograms_.AddAxis(axis);
    }
  }
  void Correlate(Qn::EventInfoF eventinfo) {
    for (unsigned i = 0; i < qvectorsA_.size(); ++i) {
      QnCorrectionsQnVector qA = qvectorsA_.GetElement(i);
      QnCorrectionsQnVector qB = qvectorsB_.GetElement(i);
      float correlation;
      if (std::get<1>(component_a_) == Component::X && std::get<1>(component_b_) == Component::Y) {
        correlation = qA.Qx(std::get<2>(component_a_)) * qB.Qy(std::get<2>(component_b_));
      }
      if (std::get<1>(component_a_) == Component::X && std::get<1>(component_b_) == Component::X) {
        correlation = qA.Qx(std::get<2>(component_a_)) * qB.Qx(std::get<2>(component_b_));
      }
      if (std::get<1>(component_a_) == Component::Y && std::get<1>(component_b_) == Component::Y) {
        correlation = qA.Qy(std::get<2>(component_a_)) * qB.Qy(std::get<2>(component_b_));
      }
      if (std::get<1>(component_a_) == Component::Y && std::get<1>(component_b_) == Component::X) {
        correlation = qA.Qy(std::get<2>(component_a_)) * qB.Qx(std::get<2>(component_b_));
      }
    }

//    histograms_.ModifyElement(eventinfo.GetVector())
  }
 private:
  Qn::DataContainerQn qvectorsA_;
  Qn::DataContainerQn qvectorsB_;
  Qn::DataContainer<TProfile> histograms_;
  std::tuple<Qn::DataContainerQn, Component, int> component_a_;
  std::tuple<Qn::DataContainerQn, Component, int> component_b_;

};

}

#endif //FLOW_CORRELATION_H
