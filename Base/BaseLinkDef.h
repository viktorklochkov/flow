//
// Created by Lukas Kreis on 12.06.17.
//
#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Qn::Axis+;
#pragma link C++ class Qn::DataVector+;
#pragma link C++ class vector<Qn::DataVector >+;
#pragma link C++ class vector<vector <Qn::DataVector> >+;
#pragma link C++ class vector<QnCorrectionsQnVector >+;
#pragma link C++ class vector<Qn::Axis >+;
#pragma link C++ class Qn::QVec+;
#pragma link C++ class Qn::QVector+;
#pragma link C++ class Qn::Profile+;
#pragma link C++ class Qn::StatisticMean+;
#pragma link C++ class Qn::Sample+;
#pragma link C++ class Qn::EventShape+;
#pragma link C++ class Qn::DataContainer<vector<Qn::DataVector> >+;
#pragma link C++ class Qn::DataContainer<Qn::EventShape>+;
#pragma link C++ class Qn::DataContainer<float >+;
#pragma link C++ class Qn::DataContainer<std::pair<bool,float> >+;
#pragma link C++ class Qn::DataContainer<Qn::Profile>+;
#pragma link C++ class Qn::DataContainer<Qn::Sample>+;
#pragma link C++ class Qn::DataContainer<Qn::QVector >+;
#pragma link C++ class Qn::DataContainer<TH1F>+;
#pragma link C++ class Qn::DataContainerHelper+;
#pragma link C++ typedef Qn::DataContainerProfile;
#pragma link C++ typedef Qn::DataContainerQVector;
#pragma link C++ typedef Qn::DataContainerDataVector;
#pragma link C++ typedef Qn::DataContainerF;
#pragma link C++ typedef Qn::DataContainerFB;
#pragma link C++ typedef Qn::DataContainerSample;
#pragma link C++ typedef Qn::DataContainerTH1F;
#pragma link C++ typedef Qn::DataContainerEventShape;


#pragma link C++ function Qn::DataToProfileGraph;
#pragma link C++ function Qn::DataToMultiGraph;
#pragma link C++ function Qn::Sqrt;

#endif
