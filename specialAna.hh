#ifndef specialAna_hh
#define specialAna_hh

#include <iostream>
#include <string>
#include <unordered_set>
#include <fstream>

///clean up the header!!!
#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"
#include "Tools/PXL/Sort.hh"
//#include "Tools/Tools.hh"
#include "Tools/MConfig.hh"
#include "TH1F.h"
#include "TH2F.h"
#include "TString.h"
#include "TLorentzVector.h"
#include <TFile.h>

#include "Main/Systematics.hh"

//----------------------------------------------------------------------
using namespace std;

//class Systematics;

class specialAna : public pxl::AnalysisProcess  {
public:
    specialAna( const Tools::MConfig &config );
    virtual ~specialAna();

    virtual void endJob(const Serializable*);
    virtual void analyseEvent( const pxl::Event* event );

    void channel_writer(TFile* file, const char* channel);

    TFile* file1;

    bool tail_selector(const pxl::Event* event);

    void Create_Gen_histograms(const char* channel, const char* part1, const char* part2);
    void Fill_Gen_histograms(const char* channel, const char* part1, const char* part2);

    void Create_Resonance_histograms(int n_histos, const char* channel, const char* part1, const char* part2, std::string const endung = "");
    void Fill_Resonance_histograms(int n_histos, const char* channel, const char* part1, const char* part2, std::string const endung = "");

    void KinematicsSelector(std::string const endung = "");

    bool FindResonance(const char* channel, vector< pxl::Particle* > gen_list);
    bool FindResonance(vector< pxl::Particle* > part1_list, vector< pxl::Particle* > part2_list);
    bool FindResonance(vector< pxl::Particle* > part1_list, vector< pxl::Particle* > part2_list, vector< pxl::Particle* > met_list);

    void GenSelector();

    void Fill_Gen_Controll_histo( );

    void Fill_Particle_histos(int hist_number, pxl::Particle* lepton);
    void Fill_Gen_histograms(int n_histos, const char* channel, const char* part1, const char* part2);

    void FillSystematics(const pxl::Event* event, std::string const particleName);
    void FillSystematicsUpDown(const pxl::Event* event, std::string const particleName, std::string const updown, std::string const shiftType);

    void initEvent( const pxl::Event* event );
    void endEvent( const pxl::Event* event );

    bool Check_Par_ID(pxl::Particle* part);
    bool Check_Muo_ID(pxl::Particle* muon);
    bool Check_Tau_ID(pxl::Particle* tau);
    bool Check_Ele_ID(pxl::Particle* ele);

    vector<double> Make_zeta_stuff(pxl::Particle* muon, pxl::Particle* tau, pxl::Particle* met);
    bool Make_zeta_cut();

    bool TriggerSelector(const pxl::Event* event);
    double DeltaPhi(double a, double b);
    double DeltaPhi(pxl::Particle* lepton, pxl::Particle* met);
    double MT(pxl::Particle* lepton, pxl::Particle* met);
    double getPtHat();


    pxl::EventView *m_RecEvtView;
    pxl::EventView *m_GenEvtView;
    pxl::EventView *m_TrigEvtView;

    bool runOnData;
    string const m_JetAlgo, m_BJets_algo, m_METType, m_TauType;

    const std::string particles[4] = {"Ele", "Muon", "Tau", "MET"};
    const std::string particleSymbols[4] = {"e", "#mu", "#tau", "E_{T}^{miss}"};


    TString d_mydisc[66];

    bool isOldPXLFile;

    const std::string m_cutdatafile;
    const vector< string >  m_trigger_string;
    TString d_mydiscmu[6];
    const std::string m_dataPeriod;
    const std::string m_channel;
    const Tools::MConfig config_;

    double temp_run;
    double temp_ls;
    double temp_event;

    double weight;

    unsigned int numMuon;
    unsigned int numEle;
    unsigned int numTau;
    unsigned int numGamma;
    unsigned int numMET;
    unsigned int numJet;
    unsigned int numBJet;

    int events_;
    unsigned int n_lepton;

    vector< pxl::Particle* > * EleList;
    vector< pxl::Particle* > * MuonList;
    vector< pxl::Particle* > * TauList;
    vector< pxl::Particle* > * GammaList;
    vector< pxl::Particle* > * METList;
    vector< pxl::Particle* > * JetList;

    vector< pxl::Particle* > * RememberPart;
    vector< pxl::Particle* > * RememberMET;

    vector< pxl::Particle* > * EleListGen;
    vector< pxl::Particle* > * MuonListGen;
    vector< pxl::Particle* > * TauListGen;
    vector< pxl::Particle* > * GammaListGen;
    vector< pxl::Particle* > * METListGen;
    vector< pxl::Particle* > * JetListGen;
    vector< pxl::Particle* > * S3ListGen;

    bool b_14TeV;
    bool b_13TeV;
    bool b_8TeV;

    bool b_emu;
    bool b_etau;
    bool b_mutau;
    bool b_etaue;
    bool b_etaumu;
    bool b_mutaue;
    bool b_mutaumu;

    map< string, bool > emu_cuts;
    map< string, bool > etau_cuts;
    map< string, bool > mutau_cuts;
    map< string, bool > etaue_cuts;
    map< string, bool > etaumu_cuts;
    map< string, bool > mutaue_cuts;
    map< string, bool > mutaumu_cuts;

    pxl::Particle* sel_part1_gen;
    pxl::Particle* sel_part2_gen;

    pxl::Particle* sel_lepton_prompt;
    pxl::Particle* sel_lepton_nprompt;
    pxl::Particle* sel_met;
    pxl::Particle* sel_lepton_nprompt_corr;

    double resonance_mass;
    double resonance_mass_gen;

    unordered_set< string > triggers;


    map< string,float > mLeptonTree;


};

#endif
