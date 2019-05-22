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

#pragma link C++ class Qn::Axis+;
#pragma link C++ class Qn::DataVector+;
#pragma link C++ class vector<Qn::DataVector >+;
#pragma link C++ class vector<vector <Qn::DataVector> >+;
#pragma link C++ class vector<Qn::CorrectionQnVector >+;
#pragma link C++ class vector<Qn::Axis >+;
#pragma link C++ class Qn::QVec+;
#pragma link C++ class Qn::QVector+;
#pragma link C++ class Qn::Profile+;
#pragma link C++ class Qn::Product+;
#pragma link C++ class Qn::Sample+;
#pragma link C++ class Qn::SubSamples+;
#pragma link C++ class Qn::Stats+;
#pragma link C++ class Qn::EventShape+;
#pragma link C++ class Qn::DataVectorHolder+;
#pragma link C++ class Qn::DataContainer<vector<Qn::DataVector> >+;
#pragma link C++ class Qn::DataContainer<Qn::EventShape>+;
#pragma link C++ class Qn::DataContainer<Qn::Product>+;
#pragma link C++ class Qn::DataContainer<Qn::Stats>+;
#pragma link C++ class Qn::DataContainer<Qn::QVector>+;
#pragma link C++ class Qn::DataContainer<Qn::DataVectorHolder>+;
#pragma link C++ class Qn::DataContainer<float>+;
#pragma link C++ class Qn::DataContainer<TH1F>+;
#pragma link C++ class Qn::DataContainerHelper+;

#pragma link C++ typedef Qn::DataContainerProduct;
#pragma link C++ typedef Qn::DataContainerStats;
#pragma link C++ typedef Qn::DataContainerQVector;
#pragma link C++ typedef Qn::DataContainerDataVector;
#pragma link C++ typedef Qn::DataContainerEventShape;
#pragma link C++ typedef Qn::Errors;

#pragma link C++ function Qn::ToTGraph;
#pragma link C++ function Qn::ToTMultiGraph;
#pragma link C++ function Qn::Sqrt<DataContainer<Qn::Stats>>;

#endif
