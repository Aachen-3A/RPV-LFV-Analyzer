#ifndef specialAna_hh
#define specialAna_hh

#include <iostream>
#include <string>
#include <unordered_set>
#include <fstream>
#include <map>
#include <vector>

/// clean up the header!!!
#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"
#include "Tools/PXL/Sort.hh"
#include "Tools/MConfig.hh"
#include "TH1F.h"
#include "TH2F.h"
#include "TString.h"
#include "TLorentzVector.h"
#include <TFile.h>

#include "Main/Systematics.hh"
#include "CutClass.hh"
#include "TriggerClass.hh"

//----------------------------------------------------------------------

class specialAna : public pxl::AnalysisProcess  {
 public:
    explicit specialAna(const Tools::MConfig &config);
    virtual ~specialAna();

    virtual void endJob(const Serializable*);
    virtual void analyseEvent(const pxl::Event* event);

    void channel_writer(TFile* file, const char* channel);

    TFile* file1;

    bool tail_selector(const pxl::Event* event);

    void Create_Gen_histograms(const char* channel, const char* part1, const char* part2);
    void Fill_Gen_histograms(const char* channel, const char* part1, const char* part2);

    void Init_emu_cuts();
    void Init_etau_cuts();
    void Init_mutau_cuts();
    void Init_etaue_cuts();
    void Init_etaumu_cuts();
    void Init_mutaue_cuts();
    void Init_mutaumu_cuts();

    void FillControllHistos();

    void Create_trigger_effs();
    void Fill_trigger_effs();

    void Create_N1_histos(const char* channel, const std::map< std::string, Cuts > &m_cfg, std::string const endung = "");
    void Fill_N1_histos(const char* channel, const std::map< std::string, Cuts > &m_cfg, std::string const endung = "");

    void Create_Resonance_histograms(int n_histos, const char* channel, const char* part1, const char* part2, std::string const endung = "");
    void Fill_Resonance_histograms(int n_histos, const char* channel, const char* part1, const char* part2, std::string const endung = "");

    void KinematicsSelector(std::string const endung = "");

    bool FindResonance(const char* channel, std::vector< pxl::Particle* > gen_list);
    bool FindResonance(const char* channel, std::vector< pxl::Particle* > part1_list, std::vector< pxl::Particle* > part2_list);
    bool FindResonance(const char* channel, std::vector< pxl::Particle* > part1_list, std::vector< pxl::Particle* > part2_list, std::vector< pxl::Particle* > met_list);

    void GenSelector();

    void Fill_Gen_Controll_histo();

    void Fill_Particle_histos(int hist_number, pxl::Particle* lepton);
    void Fill_Gen_histograms(int n_histos, const char* channel, const char* part1, const char* part2);

    void Get_Trigger_match_1(std::string trigger_name);
    void Get_Trigger_match_2(std::string trigger_name);
    pxl::Particle* Get_Truth_match(std::string name, pxl::Particle* lepton);

    pxl::Particle* Get_tau_truth_decay_mode(pxl::EventView& eventview, pxl::Particle* truth_tau);

    void Create_ID_effs();
    void Create_ID_object_effs(std::string object);
    void Fill_ID_effs();
    void Fill_ID_object_effs(std::string object, int id, std::vector< pxl::Particle* > part_list);

    void Create_RECO_effs();
    void Create_RECO_object_effs(std::string object);
    void Fill_RECO_effs();
    void Fill_RECO_object_effs(std::string object, int id, std::vector< pxl::Particle* > part_list);

    void Fill_overall_efficiencies();

    void FillSystematics(const pxl::Event* event, std::string const particleName);
    void FillSystematicsUpDown(const pxl::Event* event, std::string const particleName, std::string const updown, std::string const shiftType);

    void initEvent(const pxl::Event* event);
    void endEvent(const pxl::Event* event);

    bool Check_Par_ID(pxl::Particle* part, bool do_pt_cut = true, bool do_eta_cut = true);
    bool Check_Par_Acc(pxl::Particle* part, bool do_pt_cut = true, bool do_eta_cut = true);
    bool Check_Gen_Par_Acc(pxl::Particle* part, bool do_pt_cut = true, bool do_eta_cut = true);
    bool Check_Muo_ID(pxl::Particle* muon, bool do_pt_cut = true, bool do_eta_cut = true);
    bool Check_Tau_ID(pxl::Particle* tau);

    bool Check_Ele_ID(pxl::Particle* ele, bool do_pt_cut = true, bool do_eta_cut = true, bool forceHEEP = true);

    int FindJetFakeElectrons(pxl::Particle* ele);

    std::vector<double> Make_zeta_stuff(pxl::Particle* muon, pxl::Particle* tau, pxl::Particle* met);
    bool Make_zeta_cut(Cuts* cuts);
    bool Make_DeltaPhi_tauMET(Cuts* cuts);
    bool Make_DeltaPhi_mutau(Cuts* cuts);
    bool Make_DeltaPhi_tauemu(Cuts* cuts);
    bool Bjet_veto(Cuts* cuts);
    bool OppSign_charge(Cuts* cuts, const char* channel);
    bool MT_cut(Cuts* cuts);
    double calc_lep_fraction();
    bool Leptonic_fraction_cut(Cuts* cuts);
    bool pT_mutau_ratio_cut(Cuts* cuts);
    bool pT_muele_ratio_cut(Cuts* cuts);
    bool Make_DeltaPhi_emu(Cuts* cuts);
    bool Make_DeltaR_emu(Cuts* cuts);

    bool TriggerSelector(const pxl::Event* event);
    double DeltaPhi(double a, double b);
    double DeltaPhi(pxl::Particle* lepton, pxl::Particle* met);
    double DeltaR(pxl::Particle* lepton, pxl::Particle* met);
    double MT(pxl::Particle* lepton, pxl::Particle* met);
    double getPtHat();
    double getHT();

    void raw_input(TString question);

    pxl::EventView *m_RecEvtView;
    pxl::EventView *m_GenEvtView;
    pxl::EventView *m_TrigEvtView;

    bool runOnData;
    bool doTriggerStudies;
    const std::string m_JetAlgo, m_BJets_algo, m_METType, m_TauType;
    bool doFakeRate;

    const std::string particles[4] = {"Ele", "Muon", "Tau", "MET"};
    const std::string particleSymbols[4] = {"e", "#mu", "#tau", "E_{T}^{miss}"};

    TString d_mydisc[66];

    bool isOldPXLFile;

    const std::string m_cutdatafile;
    const std::vector< std::string >  m_trigger_string;
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

    std::vector< pxl::Particle* > * EleList;
    std::vector< pxl::Particle* > * MuonList;
    std::vector< pxl::Particle* > * TauList;
    std::vector< pxl::Particle* > * GammaList;
    std::vector< pxl::Particle* > * METList;
    std::vector< pxl::Particle* > * JetList;
    std::vector< pxl::Particle* > * BJetList;

    std::vector< pxl::Particle* > * RememberPart;
    std::vector< pxl::Particle* > * RememberMET;

    std::vector< pxl::Particle* > * EleListGen;
    std::vector< pxl::Particle* > * MuonListGen;
    std::vector< pxl::Particle* > * TauListGen;
    std::vector< pxl::Particle* > * TauVisListGen;
    std::vector< pxl::Particle* > * GammaListGen;
    std::vector< pxl::Particle* > * METListGen;
    std::vector< pxl::Particle* > * JetListGen;
    std::vector< pxl::Particle* > * S3ListGen;

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

    std::map< std::string, Cuts > emu_cut_cfgs;
    std::map< std::string, Cuts > etau_cut_cfgs;
    std::map< std::string, Cuts > mutau_cut_cfgs;
    std::map< std::string, Cuts > etaue_cut_cfgs;
    std::map< std::string, Cuts > etaumu_cut_cfgs;
    std::map< std::string, Cuts > mutaue_cut_cfgs;
    std::map< std::string, Cuts > mutaumu_cut_cfgs;

    std::map< std::string, Trigger * > trigger_defs;

    std::map< std::string, int > channel_stages;

    /// map to keep track of the resonance masses of the different channels
    std::map< std::string, double > resonance_mass;
    std::map< std::string, double > resonance_mass_gen;

    /// map to keep track of the selected gen level particles for each channel
    std::map< std::string, pxl::Particle*> sel_part1_gen;
    std::map< std::string, pxl::Particle*> sel_part2_gen;

    /// map to keep track of the selected reco level particles for each channel
    std::map< std::string, pxl::Particle*> sel_lepton_prompt;
    std::map< std::string, pxl::Particle*> sel_lepton_nprompt;
    std::map< std::string, pxl::Particle*> sel_met;
    std::map< std::string, pxl::Particle*> sel_lepton_nprompt_corr;

    std::unordered_set< std::string > triggers;

    std::map< std::string, float > mLeptonTree;

    bool keep_data_event;
    std::map< std::string, float > mkeep_resonance_mass;

    double event_weight;
    double pileup_weight;
};

#endif
