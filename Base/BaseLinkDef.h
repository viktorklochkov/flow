// Flow Vector Correction Framework
//
// Copyright (C) 2018  Lukas Kreis, Ilya Selyuzhenkov
// Contact: l.kreis@gsi.de; ilya.selyuzhenkov@gmail.com
// For a full list of contributors please see docs/Credits
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ namespace Qn;
#pragma link C++ nestedclass;
#pragma link C++ nestedtypedef;

#pragma link C++ class Qn::Axis<float>+;
#pragma link C++ class Qn::Axis<double>+;
#pragma link C++ class vector<Qn::Axis >+;
#pragma link C++ class Qn::QVec+;
#pragma link C++ class Qn::QVector+;
#pragma link C++ class Qn::Profile+;
#pragma link C++ class Qn::Product+;
#pragma link C++ class Qn::Sample+;
#pragma link C++ class Qn::SubSamples+;
#pragma link C++ class Qn::Stats+;
#pragma link C++ class Qn::EventShape+;
#pragma link C++ class Qn::SubEventHolder+;
#pragma link C++ class Qn::DataContainer<Qn::EventShape,float>+;
#pragma link C++ class Qn::DataContainer<Qn::Product,float>+;
#pragma link C++ class Qn::DataContainer<Qn::Stats,float>+;
#pragma link C++ class Qn::DataContainer<Qn::QVector,float>+;
#pragma link C++ class Qn::DataContainer<std::shared_ptr<Qn::SubEvent>,float>+;
#pragma link C++ class Qn::DataContainer<float,float>+;
#pragma link C++ class Qn::DataContainer<TH1F, float>+;
#pragma link C++ class Qn::DataContainerHelper+;

#pragma link C++ typedef Qn::DataContainerProduct;
#pragma link C++ typedef Qn::AxisF;
#pragma link C++ typedef Qn::AxisD;
#pragma link C++ typedef Qn::DataContainerStats;
#pragma link C++ typedef Qn::DataContainerQVector;
#pragma link C++ typedef Qn::DataContainerEventShape;
#pragma link C++ typedef Qn::Errors;

#pragma link C++ function Qn::ToTGraph;
#pragma link C++ function Qn::ToTMultiGraph;
#pragma link C++ function Qn::Sqrt<DataContainer<Qn::Stats>>;

#pragma link C++ class Qn::CorrectionOnInputData+;
#pragma link C++ class Qn::CorrectionOnQvector+;
#pragma link C++ class Qn::CorrectionsSetOnInputData+;
#pragma link C++ class Qn::CorrectionsSetOnQvector+;
#pragma link C++ class Qn::CorrectionStep+;
#pragma link C++ class Qn::SubEvent+;
#pragma link C++ class Qn::SubEventChannels+;
#pragma link C++ class Qn::SubEventTracks+;
#pragma link C++ class Qn::EventClassVariable+;
#pragma link C++ class Qn::EventClassVariablesSet+;
#pragma link C++ class Qn::CorrectionHistogram+;
#pragma link C++ class Qn::CorrectionHistogramBase+;
#pragma link C++ class Qn::CorrectionHistogramChannelized+;
#pragma link C++ class Qn::CorrectionHistogramChannelizedSparse+;
#pragma link C++ class Qn::CorrectionHistogramSparse+;
#pragma link C++ class Qn::GainEqualization+;
#pragma link C++ class Qn::CorrectionCalculator+;
#pragma link C++ class Qn::CorrectionProfile+;
#pragma link C++ class Qn::CorrectionProfile3DCorrelations+;
#pragma link C++ class Qn::CorrectionProfileChannelized+;
#pragma link C++ class Qn::CorrectionProfileChannelizedIngress+;
#pragma link C++ class Qn::CorrectionProfileComponents+;
#pragma link C++ class Qn::CorrectionProfileCorrelationComponents+;
#pragma link C++ class Qn::CorrectionProfileCorrelationComponentsHarmonics+;
#pragma link C++ class Qn::CorrectionQnVector+;
#pragma link C++ class Qn::Alignment+;
#pragma link C++ class Qn::CorrectionQnVectorBuild+;
#pragma link C++ class Qn::Recentering+;
#pragma link C++ class Qn::TwistAndRescale+;

#endif
