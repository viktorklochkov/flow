//
// Created by Lukas Kreis on 12.06.17.
//
#ifdef __MAKECINT__

#pragma link off all typedefs;
#pragma link off all globals;
#pragma link off all functions;
#pragma link off all classes;

//#pragma link C++ class std::vector+;
#pragma link C++ class Qn::Axis+;
#pragma link C++ class Qn::DataVector+;
#pragma link C++ class vector<Qn::DataVector >+;
#pragma link C++ class vector<vector <Qn::DataVector> >+;
#pragma link C++ class vector<QnCorrectionsQnVector >+;
#pragma link C++ class vector<Qn::Axis >+;
#pragma link C++ class Qn::DataContainer<QnCorrectionsQnVector >+;
#pragma link C++ class Qn::DataContainer<vector<Qn::DataVector> >+;
#pragma link C++ class Qn::DataContainer<float >+;
#pragma link C++ class Qn::DataContainer<std::vector<float> >+;
#pragma link C++ typedef Qn::DataContainerQn;
#pragma link C++ typedef Qn::DataContainerDataVector;
#pragma link C++ typedef Qn::DataContainerD;
#pragma link C++ class Qn::Resolution+;
#pragma link C++ class Qn::ResolutionDetector+;
#pragma link C++ class QaAnalysis+;


// Not needed
//#pragma link C++ class Qn::EventInfoValue<float>+;
//#pragma link C++ class Qn::EventInfoValue<int>+;
//#pragma link C++ class Qn::EventInfo<int>+;
//#pragma link C++ class Qn::EventInfo<float>+;
//#pragma link C++ typedef Qn::EventInfoF;
//#pragma link C++ typedef Qn::EventInfoI;

#endif
