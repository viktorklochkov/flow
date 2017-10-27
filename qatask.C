void qatask(std::string filename, std::string name) {
  R__LOAD_LIBRARY(/lustre/nyx/alice/users/lkreis/analysis/flow/build/libflow.so) 
  TFile *file = TFile::Open(filename.data());
  QaAnalysis qa(file,name);
  qa.TrackQa();
  qa.EventQa();
}
