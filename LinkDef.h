//
// Created by Lukas Kreis on 12.06.17.
//
#ifdef __MAKECINT__

#pragma link C++ class Qn::Axis+;
#pragma link C++ class Qn::Correlation+;
#pragma link C++ class Qn::DataContainer<std::unique_ptr<QnCorrectionsQnVector> >+;
#pragma link C++ class Qn::DataContainer<std::unique_ptr<Qn::Correlation> >+;
#pragma link C++ typedef Qn::DataContainerQn;
#pragma link C++ typedef Qn::DataContainerC;
#pragma link C++ class Qn::EventInfo<int>+;
#pragma link C++ class Qn::EventInfo<float>+;
#pragma link C++ typedef Qn::EventInfoF;
#pragma link C++ typedef Qn::EventInfoI;
#pragma link C++ class Qn::CorrectionsInterface;

#endif