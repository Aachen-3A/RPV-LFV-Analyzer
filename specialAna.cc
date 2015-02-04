#include "specialAna.hh"
#include "HistClass.hh"
#include "Tools/Tools.hh"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include "boost/format.hpp"
#pragma GCC diagnostic pop

specialAna::specialAna(const Tools::MConfig &cfg) :
    runOnData(cfg.GetItem< bool >("General.RunOnData")),
    doTriggerStudies(cfg.GetItem< bool >("General.DoTriggerStudies")),
    m_JetAlgo(cfg.GetItem< std::string >("Jet.Type.Rec")),
    m_BJets_algo(cfg.GetItem< std::string >("Jet.BJets.Algo")),
    m_METType(cfg.GetItem< std::string >("MET.Type.Rec")),
    m_TauType(cfg.GetItem< std::string >("Tau.Type.Rec")),

    m_trigger_string(Tools::splitString< std::string >(cfg.GetItem< std::string >("RPV.trigger_list" , true)),
    d_mydiscmu({"isPFMuon", "isGlobalMuon", "isTrackerMuon", "isStandAloneMuon", "isTightMuon", "isHighPtMuon"}),
    m_dataPeriod(cfg.GetItem< std::string >("General.DataPeriod")),
    m_channel(cfg.GetItem< std::string >("RPV.channel"),
    config_(cfg) {
    b_14TeV = m_dataPeriod == "14TeV" ? true : false;
    b_13TeV = m_dataPeriod == "13TeV" ? true : false;
    b_8TeV = m_dataPeriod == "8TeV" ? true : false;

    b_emu = m_channel.find("emu") != std::string::npos ? true : false;
    b_etau = m_channel.find("etau") != std::string::npos ? true : false;
    b_mutau = m_channel.find("mutau") != std::string::npos ? true : false;
    b_etaue = m_channel.find("etaue") != std::string::npos ? true : false;
    b_etaumu = m_channel.find("etaumu") != std::string::npos ? true : false;
    b_mutaue = m_channel.find("mutaue") != std::string::npos ? true : false;
    b_mutaumu = m_channel.find("mutaumu") != std::string::npos ? true : false;

    std::string safeFileName = "SpecialHistos.root";
    file1 = new TFile(safeFileName.c_str(), "RECREATE");
    events_ = 0;

    // number of events, saved in a histogram
    HistClass::CreateHistoUnchangedName("h_counters", 10, 0, 11, "N_{events}");

    if (doTriggerStudies) {
        Create_trigger_effs();
    }

    mkeep_resonance_mass["emu"] = 0;
    mkeep_resonance_mass["etau"] = 0;
    mkeep_resonance_mass["mutau"] = 0;
    mkeep_resonance_mass["etaue"] = 0;
    mkeep_resonance_mass["etaumu"] = 0;
    mkeep_resonance_mass["mutaue"] = 0;
    mkeep_resonance_mass["mutaumu"] = 0;
    mkeep_resonance_mass["run"] = 0;
    mkeep_resonance_mass["ls"] = 0;
    mkeep_resonance_mass["event"] = 0;

    resonance_mass["emu"] = 0;
    resonance_mass_gen["emu"] = 0;
    resonance_mass["etau"] = 0;
    resonance_mass_gen["etau"] = 0;
    resonance_mass["mutau"] = 0;
    resonance_mass_gen["mutau"] = 0;
    resonance_mass["etaue"] = 0;
    resonance_mass_gen["etaue"] = 0;
    resonance_mass["etaumu"] = 0;
    resonance_mass_gen["etaumu"] = 0;
    resonance_mass["mutaue"] = 0;
    resonance_mass_gen["mutaue"] = 0;
    resonance_mass["mutaumu"] = 0;
    resonance_mass_gen["mutaumu"] = 0;

    HistClass::CreateTree(mkeep_resonance_mass, "data_events");

    for (unsigned int i = 0; i < 4; i++) {
        // str(boost::format("N_{%s}")%particleLatex[i] )
        HistClass::CreateHisto("num", particles[i].c_str(), 40, 0, 39,                          TString::Format("N_{%s}", particleSymbols[i].c_str()));
        HistClass::CreateHisto(3, "pt", particles[i].c_str(), 5000, 0, 5000,                    TString::Format("p_{T}^{%s} (GeV)", particleSymbols[i].c_str()));
        HistClass::CreateHisto("pt_resolution_0_500", particles[i].c_str(), 1000, -10, 10,      TString::Format("(p_{T}^{reco} - p_{T}^{gen})/p_{T}^{gen}(%s)", particleSymbols[i].c_str()));
        HistClass::CreateHisto("pt_resolution_500_1000", particles[i].c_str(), 1000, -10, 10,   TString::Format("(p_{T}^{reco} - p_{T}^{gen})/p_{T}^{gen}(%s)", particleSymbols[i].c_str()));
        HistClass::CreateHisto("pt_resolution_1000_1500", particles[i].c_str(), 1000, -10, 10,  TString::Format("(p_{T}^{reco} - p_{T}^{gen})/p_{T}^{gen}(%s)", particleSymbols[i].c_str()));
        HistClass::CreateHisto("pt_resolution_1500_2000", particles[i].c_str(), 1000, -10, 10,  TString::Format("(p_{T}^{reco} - p_{T}^{gen})/p_{T}^{gen}(%s)", particleSymbols[i].c_str()));
        HistClass::CreateHisto("pt_resolution_2000", particles[i].c_str(), 1000, -10, 10,       TString::Format("(p_{T}^{reco} - p_{T}^{gen})/p_{T}^{gen}(%s)", particleSymbols[i].c_str()));
        HistClass::CreateHisto(3, "eta", particles[i].c_str(), 80, -4, 4,                       TString::Format("#eta_{%s}", particleSymbols[i].c_str()));
        HistClass::CreateHisto(3, "phi", particles[i].c_str(), 40, -3.2, 3.2,                   TString::Format("#phi_{%s} (rad)", particleSymbols[i].c_str()));

        if (not runOnData) {
            HistClass::CreateHisto(1, "num_Gen", particles[i].c_str(), 40, 0, 39,        TString::Format("N_{%s}", particleSymbols[i].c_str()));
            HistClass::CreateHisto(1, "pt_Gen",  particles[i].c_str(), 5000, 0, 5000,    TString::Format("p_{T}^{%s} (GeV)", particleSymbols[i].c_str()));
            HistClass::CreateHisto(1, "eta_Gen", particles[i].c_str(), 80, -4, 4,        TString::Format("#eta_{%s}", particleSymbols[i].c_str()));
            HistClass::CreateHisto(1, "phi_Gen", particles[i].c_str(), 40, -3.2, 3.2,    TString::Format("#phi_{%s} (rad)", particleSymbols[i].c_str()));
        }
    }

    HistClass::CreateHisto("LLE_Gen", 100, 0, 1, "LLE");
    HistClass::CreateHisto("LQD_Gen", 100, 0, 0.001, "LQD");
    HistClass::CreateHisto("MSnl_Gen", 4000, 0, 4000, "MSnl");

    HistClass::CreateHisto("Ctr_Vtx_unweighted", 100, 0, 100, "N_{vtx}");
    HistClass::CreateHisto("Ctr_Vtx_weighted", 100, 0, 100, "N_{vtx}");

    HistClass::CreateHisto("Ctr_Vtx_emu_unweighted", 100, 0, 100, "N_{vtx}");
    HistClass::CreateHisto("Ctr_Vtx_emu_weighted", 100, 0, 100, "N_{vtx}");

    HistClass::CreateHisto("Ctr_pT_hat", 5000, 0, 5000, "#hat{p_{T}} (GeV)");
    HistClass::CreateHisto("Ctr_HT", 5000, 0, 5000, "H_{T} (GeV)");

    if (not runOnData) {
        Create_Gen_histograms("emu", "ele", "muo");
        Create_Gen_histograms("etau", "ele", "tau");
        Create_Gen_histograms("mutau", "muo", "tau");
        Create_Gen_histograms("etaue", "ele", "tau_ele");
        Create_Gen_histograms("etaumu", "ele", "tau_muo");
        Create_Gen_histograms("mutaue", "muo", "tau_ele");
        Create_Gen_histograms("mutaumu", "muo", "tau_muo");
    }

    ///-----------------------------------------------------------------
    /// Init for the e-mu channel
    channel_stages["emu"] = 4;
    Create_Resonance_histograms(channel_stages["emu"], "emu", "ele", "muo");
    Create_Resonance_histograms(channel_stages["emu"], "emu", "ele", "muo", "_Ele_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["emu"], "emu", "ele", "muo", "_Ele_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["emu"], "emu", "ele", "muo", "_Tau_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["emu"], "emu", "ele", "muo", "_Tau_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["emu"], "emu", "ele", "muo", "_Muon_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["emu"], "emu", "ele", "muo", "_Muon_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["emu"], "emu", "ele", "muo", "_Muon_syst_ResolutionUp");
    Create_Resonance_histograms(channel_stages["emu"], "emu", "ele", "muo", "_Muon_syst_ResolutionDown");
    Init_emu_cuts();
    Create_N1_histos("emu", emu_cut_cfgs);
    Create_N1_histos("emu", emu_cut_cfgs, "_Ele_syst_ScaleUp");
    Create_N1_histos("emu", emu_cut_cfgs, "_Ele_syst_ScaleDown");
    Create_N1_histos("emu", emu_cut_cfgs, "_Tau_syst_ScaleUp");
    Create_N1_histos("emu", emu_cut_cfgs, "_Tau_syst_ScaleDown");
    Create_N1_histos("emu", emu_cut_cfgs, "_Muon_syst_ScaleUp");
    Create_N1_histos("emu", emu_cut_cfgs, "_Muon_syst_ScaleDown");
    Create_N1_histos("emu", emu_cut_cfgs, "_Muon_syst_ResolutionUp");
    Create_N1_histos("emu", emu_cut_cfgs, "_Muon_syst_ResolutionDown");

    ///-----------------------------------------------------------------
    /// Init for the e-tau_h channel
    channel_stages["etau"] = 1;
    Create_Resonance_histograms(channel_stages["etau"], "etau", "ele", "tau");
    Create_Resonance_histograms(channel_stages["etau"], "etau", "ele", "tau", "_Ele_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["etau"], "etau", "ele", "tau", "_Ele_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["etau"], "etau", "ele", "tau", "_Tau_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["etau"], "etau", "ele", "tau", "_Tau_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["etau"], "etau", "ele", "tau", "_Muon_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["etau"], "etau", "ele", "tau", "_Muon_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["etau"], "etau", "ele", "tau", "_Muon_syst_ResolutionUp");
    Create_Resonance_histograms(channel_stages["etau"], "etau", "ele", "tau", "_Muon_syst_ResolutionDown");
    Init_etau_cuts();
    Create_N1_histos("etau", etau_cut_cfgs);
    Create_N1_histos("etau", etau_cut_cfgs, "_Ele_syst_ScaleUp");
    Create_N1_histos("etau", etau_cut_cfgs, "_Ele_syst_ScaleDown");
    Create_N1_histos("etau", etau_cut_cfgs, "_Tau_syst_ScaleUp");
    Create_N1_histos("etau", etau_cut_cfgs, "_Tau_syst_ScaleDown");
    Create_N1_histos("etau", etau_cut_cfgs, "_Muon_syst_ScaleUp");
    Create_N1_histos("etau", etau_cut_cfgs, "_Muon_syst_ScaleDown");
    Create_N1_histos("etau", etau_cut_cfgs, "_Muon_syst_ResolutionUp");
    Create_N1_histos("etau", etau_cut_cfgs, "_Muon_syst_ResolutionDown");

    ///-----------------------------------------------------------------
    /// Init for the mu-tau_h channel
    channel_stages["mutau"] = 7;
    Create_Resonance_histograms(channel_stages["mutau"], "mutau", "muo", "tau");
    Create_Resonance_histograms(channel_stages["mutau"], "mutau", "muo", "tau", "_Ele_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["mutau"], "mutau", "muo", "tau", "_Ele_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["mutau"], "mutau", "muo", "tau", "_Tau_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["mutau"], "mutau", "muo", "tau", "_Tau_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["mutau"], "mutau", "muo", "tau", "_Muon_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["mutau"], "mutau", "muo", "tau", "_Muon_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["mutau"], "mutau", "muo", "tau", "_Muon_syst_ResolutionUp");
    Create_Resonance_histograms(channel_stages["mutau"], "mutau", "muo", "tau", "_Muon_syst_ResolutionDown");
    Init_mutau_cuts();
    Create_N1_histos("mutau", mutau_cut_cfgs);
    Create_N1_histos("mutau", mutau_cut_cfgs, "_Ele_syst_ScaleUp");
    Create_N1_histos("mutau", mutau_cut_cfgs, "_Ele_syst_ScaleDown");
    Create_N1_histos("mutau", mutau_cut_cfgs, "_Tau_syst_ScaleUp");
    Create_N1_histos("mutau", mutau_cut_cfgs, "_Tau_syst_ScaleDown");
    Create_N1_histos("mutau", mutau_cut_cfgs, "_Muon_syst_ScaleUp");
    Create_N1_histos("mutau", mutau_cut_cfgs, "_Muon_syst_ScaleDown");
    Create_N1_histos("mutau", mutau_cut_cfgs, "_Muon_syst_ResolutionUp");
    Create_N1_histos("mutau", mutau_cut_cfgs, "_Muon_syst_ResolutionDown");

    ///-----------------------------------------------------------------
    /// Init for the e-tau_e channel
    channel_stages["etaue"] = 1;
    Create_Resonance_histograms(channel_stages["etaue"], "etaue", "ele", "tau_ele");
    Create_Resonance_histograms(channel_stages["etaue"], "etaue", "ele", "tau_ele", "_Ele_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["etaue"], "etaue", "ele", "tau_ele", "_Ele_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["etaue"], "etaue", "ele", "tau_ele", "_Tau_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["etaue"], "etaue", "ele", "tau_ele", "_Tau_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["etaue"], "etaue", "ele", "tau_ele", "_Muon_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["etaue"], "etaue", "ele", "tau_ele", "_Muon_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["etaue"], "etaue", "ele", "tau_ele", "_Muon_syst_ResolutionUp");
    Create_Resonance_histograms(channel_stages["etaue"], "etaue", "ele", "tau_ele", "_Muon_syst_ResolutionDown");
    Init_etaue_cuts();
    Create_N1_histos("etaue", etaue_cut_cfgs);
    Create_N1_histos("etaue", etaue_cut_cfgs, "_Ele_syst_ScaleUp");
    Create_N1_histos("etaue", etaue_cut_cfgs, "_Ele_syst_ScaleDown");
    Create_N1_histos("etaue", etaue_cut_cfgs, "_Tau_syst_ScaleUp");
    Create_N1_histos("etaue", etaue_cut_cfgs, "_Tau_syst_ScaleDown");
    Create_N1_histos("etaue", etaue_cut_cfgs, "_Muon_syst_ScaleUp");
    Create_N1_histos("etaue", etaue_cut_cfgs, "_Muon_syst_ScaleDown");
    Create_N1_histos("etaue", etaue_cut_cfgs, "_Muon_syst_ResolutionUp");
    Create_N1_histos("etaue", etaue_cut_cfgs, "_Muon_syst_ResolutionDown");

    ///-----------------------------------------------------------------
    /// Init for the e-tau_mu channel
    channel_stages["etaumu"] = 1;
    Create_Resonance_histograms(channel_stages["etaumu"], "etaumu", "ele", "tau_muo");
    Create_Resonance_histograms(channel_stages["etaumu"], "etaumu", "ele", "tau_muo", "_Ele_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["etaumu"], "etaumu", "ele", "tau_muo", "_Ele_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["etaumu"], "etaumu", "ele", "tau_muo", "_Tau_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["etaumu"], "etaumu", "ele", "tau_muo", "_Tau_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["etaumu"], "etaumu", "ele", "tau_muo", "_Muon_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["etaumu"], "etaumu", "ele", "tau_muo", "_Muon_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["etaumu"], "etaumu", "ele", "tau_muo", "_Muon_syst_ResolutionUp");
    Create_Resonance_histograms(channel_stages["etaumu"], "etaumu", "ele", "tau_muo", "_Muon_syst_ResolutionDown");
    Init_etaumu_cuts();
    Create_N1_histos("etaumu", etaumu_cut_cfgs);
    Create_N1_histos("etaumu", etaumu_cut_cfgs, "_Ele_syst_ScaleUp");
    Create_N1_histos("etaumu", etaumu_cut_cfgs, "_Ele_syst_ScaleDown");
    Create_N1_histos("etaumu", etaumu_cut_cfgs, "_Tau_syst_ScaleUp");
    Create_N1_histos("etaumu", etaumu_cut_cfgs, "_Tau_syst_ScaleDown");
    Create_N1_histos("etaumu", etaumu_cut_cfgs, "_Muon_syst_ScaleUp");
    Create_N1_histos("etaumu", etaumu_cut_cfgs, "_Muon_syst_ScaleDown");
    Create_N1_histos("etaumu", etaumu_cut_cfgs, "_Muon_syst_ResolutionUp");
    Create_N1_histos("etaumu", etaumu_cut_cfgs, "_Muon_syst_ResolutionDown");

    ///-----------------------------------------------------------------
    /// Init for the mu-tau_e channel
    channel_stages["mutaue"] = 6;
    Create_Resonance_histograms(channel_stages["mutaue"], "mutaue", "muo", "tau_ele");
    Create_Resonance_histograms(channel_stages["mutaue"], "mutaue", "muo", "tau_ele", "_Ele_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["mutaue"], "mutaue", "muo", "tau_ele", "_Ele_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["mutaue"], "mutaue", "muo", "tau_ele", "_Tau_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["mutaue"], "mutaue", "muo", "tau_ele", "_Tau_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["mutaue"], "mutaue", "muo", "tau_ele", "_Muon_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["mutaue"], "mutaue", "muo", "tau_ele", "_Muon_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["mutaue"], "mutaue", "muo", "tau_ele", "_Muon_syst_ResolutionUp");
    Create_Resonance_histograms(channel_stages["mutaue"], "mutaue", "muo", "tau_ele", "_Muon_syst_ResolutionDown");
    Init_mutaue_cuts();
    Create_N1_histos("mutaue", mutaue_cut_cfgs);
    Create_N1_histos("mutaue", mutaue_cut_cfgs, "_Ele_syst_ScaleUp");
    Create_N1_histos("mutaue", mutaue_cut_cfgs, "_Ele_syst_ScaleDown");
    Create_N1_histos("mutaue", mutaue_cut_cfgs, "_Tau_syst_ScaleUp");
    Create_N1_histos("mutaue", mutaue_cut_cfgs, "_Tau_syst_ScaleDown");
    Create_N1_histos("mutaue", mutaue_cut_cfgs, "_Muon_syst_ScaleUp");
    Create_N1_histos("mutaue", mutaue_cut_cfgs, "_Muon_syst_ScaleDown");
    Create_N1_histos("mutaue", mutaue_cut_cfgs, "_Muon_syst_ResolutionUp");
    Create_N1_histos("mutaue", mutaue_cut_cfgs, "_Muon_syst_ResolutionDown");

    ///-----------------------------------------------------------------
    /// Init for the mu-tau_mu channel
    channel_stages["mutaumu"] = 1;
    Create_Resonance_histograms(channel_stages["mutaumu"], "mutaumu", "muo", "tau_muo");
    Create_Resonance_histograms(channel_stages["mutaumu"], "mutaumu", "muo", "tau_muo", "_Ele_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["mutaumu"], "mutaumu", "muo", "tau_muo", "_Ele_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["mutaumu"], "mutaumu", "muo", "tau_muo", "_Tau_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["mutaumu"], "mutaumu", "muo", "tau_muo", "_Tau_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["mutaumu"], "mutaumu", "muo", "tau_muo", "_Muon_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["mutaumu"], "mutaumu", "muo", "tau_muo", "_Muon_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["mutaumu"], "mutaumu", "muo", "tau_muo", "_Muon_syst_ResolutionUp");
    Create_Resonance_histograms(channel_stages["mutaumu"], "mutaumu", "muo", "tau_muo", "_Muon_syst_ResolutionDown");
    Init_mutaumu_cuts();
    Create_N1_histos("mutaumu", mutaumu_cut_cfgs);
    Create_N1_histos("mutaumu", mutaumu_cut_cfgs, "_Ele_syst_ScaleUp");
    Create_N1_histos("mutaumu", mutaumu_cut_cfgs, "_Ele_syst_ScaleDown");
    Create_N1_histos("mutaumu", mutaumu_cut_cfgs, "_Tau_syst_ScaleUp");
    Create_N1_histos("mutaumu", mutaumu_cut_cfgs, "_Tau_syst_ScaleDown");
    Create_N1_histos("mutaumu", mutaumu_cut_cfgs, "_Muon_syst_ScaleUp");
    Create_N1_histos("mutaumu", mutaumu_cut_cfgs, "_Muon_syst_ScaleDown");
    Create_N1_histos("mutaumu", mutaumu_cut_cfgs, "_Muon_syst_ResolutionUp");
    Create_N1_histos("mutaumu", mutaumu_cut_cfgs, "_Muon_syst_ResolutionDown");
}

specialAna::~specialAna() {
}

void specialAna::analyseEvent(const pxl::Event* event) {
    initEvent(event);

    if (tail_selector(event)) return;

    if (not runOnData) {
        Fill_Gen_Controll_histo();
    }

    GenSelector();

    fo r(uint i = 0; i < MuonList->size(); i++) {
        if (MuonList->at(i)->getPt() < 25 or TMath::Abs(MuonList->at(i)->getEta()) > 2.1) continue;
        Fill_Particle_histos(0, MuonList->at(i));
    }
    HistClass::Fill("Muon_num", MuonList->size(), weight);

    for (uint i = 0; i < EleList->size(); i++) {
        if (EleList->at(i)->getPt() < 25 or TMath::Abs(EleList->at(i)->getEta()) > 2.5 or (TMath::Abs(EleList->at(i)->getEta()) > 1.442 and TMath::Abs(EleList->at(i)->getEta()) < 1.56)) continue;
        Fill_Particle_histos(0, EleList->at(i));
    }
    HistClass::Fill("Ele_num", EleList->size(), weight);

    for (uint i = 0; i < TauList->size(); i++) {
        Fill_Particle_histos(0, TauList->at(i));
    }
    HistClass::Fill("Tau_num", TauList->size(), weight);

    for (uint i = 0; i < METList->size(); i++) {
        Fill_Particle_histos(0, METList->at(i));
    }
    HistClass::Fill("MET_num", METList->size(), weight);

    if (doTriggerStudies) {
        Fill_trigger_effs();
    }

    if (TriggerSelector(event)) {
        FillControllHistos();

        for (uint i = 0; i < MuonList->size(); i++) {
            if (MuonList->at(i)->getPt() < 25 or TMath::Abs(MuonList->at(i)->getEta()) > 2.1) continue;
            Fill_Particle_histos(1, MuonList->at(i));
        }

        for (uint i = 0; i < EleList->size(); i++) {
            if (EleList->at(i)->getPt() < 25 or TMath::Abs(EleList->at(i)->getEta()) > 2.5 or (TMath::Abs(EleList->at(i)->getEta()) > 1.442 and TMath::Abs(EleList->at(i)->getEta()) < 1.56)) continue;
            Fill_Particle_histos(1, EleList->at(i));
        }

        for (uint i = 0; i < TauList->size(); i++) {
            Fill_Particle_histos(1, TauList->at(i));
        }

        for (uint i = 0; i < METList->size(); i++) {
            Fill_Particle_histos(1, METList->at(i));
        }

        for (uint i = 0; i < MuonList->size(); i++) {
            if (MuonList->at(i)->getPt() < 25 or TMath::Abs(MuonList->at(i)->getEta()) > 2.1) continue;
            if (Check_Muo_ID(MuonList->at(i))) {
                Fill_Particle_histos(2, MuonList->at(i));
            }
        }

        for (uint i = 0; i < EleList->size(); i++) {
            if (EleList->at(i)->getPt() < 25 or TMath::Abs(EleList->at(i)->getEta()) > 2.5 or (TMath::Abs(EleList->at(i)->getEta()) > 1.442 and TMath::Abs(EleList->at(i)->getEta()) < 1.56)) continue;
            if (EleList->at(i)->getUserRecord("IDpassed").asBool()) {
                Fill_Particle_histos(2, EleList->at(i));
            }
        }

        for (uint i = 0; i < TauList->size(); i++) {
            if (Check_Tau_ID(TauList->at(i))) {
                Fill_Particle_histos(2, TauList->at(i));
            }
        }

        for (uint i = 0; i < METList->size(); i++) {
            Fill_Particle_histos(2, METList->at(i));
        }

        KinematicsSelector();

        if (not runOnData) {
            FillSystematics(event, "Ele");

            FillSystematics(event, "Muon");
            FillSystematicsUpDown(event, "Muon", "Up", "Resolution");
            FillSystematicsUpDown(event, "Muon", "Down", "Resolution");

            FillSystematics(event, "Tau");
        }
    }
    endEvent(event);
}

bool specialAna::tail_selector(const pxl::Event* event) {
    std::string datastream = event->getUserRecord("Dataset").asString();
    TString Datastream = datastream;

    double cut_w_mass = 0;
    double cut_w_pt = 0;
    if (Datastream.Contains("WJetsToLNu") or Datastream.Contains("WTo")) {
        if (b_8TeV) {
            for (uint i = 0; i < S3ListGen->size(); i++) {
                if (TMath::Abs(S3ListGen->at(i) -> getUserRecord("id").asInt32()) == 24) {
                    if (S3ListGen->at(i)->getMass() > cut_w_mass) {
                        cut_w_mass = S3ListGen->at(i)->getMass();
                        cut_w_pt = S3ListGen->at(i)->getPt();
                    }
                }
            }
        } else if (b_13TeV) {
            for (uint i = 0; i < S3ListGen->size(); i++) {
                if (TMath::Abs(S3ListGen->at(i) -> getPdgNumber()) == 24) {
                    if (S3ListGen->at(i)->getMass() > cut_w_mass) {
                        cut_w_mass = S3ListGen->at(i)->getMass();
                        cut_w_pt = S3ListGen->at(i)->getPt();
                    }
                }
            }
        }
    }

    if (Datastream.Contains("WJetsToLNu_TuneZ2Star_8TeV")) {
        if (cut_w_pt > 55) return true;
    }

    if (Datastream.Contains("WJetsToLNu_PtW")) {
        if (cut_w_pt <= 55) return true;
    }

    if (Datastream.Contains("WJetsToLNu")) {
        if (cut_w_mass > 300) return true;
    }

    if (Datastream.Contains("WTo")) {
        if (cut_w_mass <= 300) return true;
    }

    /// Diboson tail fitting
    if (Datastream.Contains("WW_") or Datastream.Contains("WZ_") or Datastream.Contains("ZZ_")) {
        for (uint i = 0; i < S3ListGen->size(); i++) {
            int part_id = 0;
            if (b_8TeV) {
                part_id = TMath::Abs(S3ListGen->at(i) -> getUserRecord("id").asInt32());
            } else if (b_13TeV) {
                part_id = TMath::Abs(S3ListGen->at(i) -> getPdgNumber());
            }
            if (part_id == 23 or part_id == 22) {
                if (S3ListGen->at(i)->getPt() > 500) return true;
            }
        }
    }

    /// ttbar 8TeV tail fitting
    if (b_8TeV and Datastream.Contains("TT_CT10_TuneZ2star_8TeV-powheg")) {
        for (uint i = 0; i < S3ListGen->size(); i++) {
            if (S3ListGen->at(i) -> getUserRecord("id").asInt32() == 6) {
                for (uint j = 0; j < S3ListGen->size(); j++) {
                    if (S3ListGen->at(j) -> getUserRecord("id").asInt32() == -6) {
                        double mass = (S3ListGen->at(j)->getE() + S3ListGen->at(i)->getE())  *(S3ListGen->at(j)->getE() + S3ListGen->at(i)->getE())
                                                    - (S3ListGen->at(j)->getPx() + S3ListGen->at(i)->getPx())*(S3ListGen->at(j)->getPx() + S3ListGen->at(i)->getPx())
                                                    - (S3ListGen->at(j)->getPy() + S3ListGen->at(i)->getPy())*(S3ListGen->at(j)->getPy() + S3ListGen->at(i)->getPy())
                                                    - (S3ListGen->at(j)->getPz() + S3ListGen->at(i)->getPz())*(S3ListGen->at(j)->getPz() + S3ListGen->at(i)->getPz());
                        if (!Datastream.Contains("Mtt") and sqrt(mass) > 700) {
                            return true;
                        } else if (Datastream.Contains("Mtt") and sqrt(mass) <= 700) {
                            return true;
                        }
                    }
                }
            }
        }
    }

    /// ttbar 13TeV tail fitting
    if (b_13TeV and Datastream.Contains("TT_CT10_TuneZ2star_8TeV-powheg")) {
        for (uint i = 0; i < S3ListGen->size(); i++) {
            if (S3ListGen->at(i) -> getPdgNumber() == 6) {
                for (uint j = 0; j < S3ListGen->size(); j++) {
                    if (S3ListGen->at(j) -> getPdgNumber() == -6) {
                        double mass = (S3ListGen->at(j)->getE() + S3ListGen->at(i)->getE())  *(S3ListGen->at(j)->getE() + S3ListGen->at(i)->getE())
                                                    - (S3ListGen->at(j)->getPx() + S3ListGen->at(i)->getPx())*(S3ListGen->at(j)->getPx() + S3ListGen->at(i)->getPx())
                                                    - (S3ListGen->at(j)->getPy() + S3ListGen->at(i)->getPy())*(S3ListGen->at(j)->getPy() + S3ListGen->at(i)->getPy())
                                                    - (S3ListGen->at(j)->getPz() + S3ListGen->at(i)->getPz())*(S3ListGen->at(j)->getPz() + S3ListGen->at(i)->getPz());
                        if (!Datastream.Contains("Mtt") and sqrt(mass) > 700) {
                            return true;
                        } else if (Datastream.Contains("Mtt") and sqrt(mass) <= 700) {
                            return true;
                        }
                    }
                }
            }
        }
    }

    /// Signal parameter selection
    if (Datastream.Contains("RPVresonantToMuTau")) {
        HistClass::Fill("LLE_Gen", m_GenEvtView->getUserRecord("MC_LLE").asDouble(), weight);
        HistClass::Fill("LQD_Gen", m_GenEvtView->getUserRecord("MC_LQD").asDouble(), weight);
        HistClass::Fill("MSnl_Gen", m_GenEvtView->getUserRecord("MC_MSnl").asDouble(), weight);
        double gen_mass = m_GenEvtView->getUserRecord("MC_MSnl").asDouble();
        double gen_coupling = m_GenEvtView->getUserRecord("MC_LLE").asDouble();
        if (!(gen_mass > config_.GetItem< double >("rpv_mass.min") && gen_mass < config_.GetItem< double >("rpv_mass.max")))return true;
        if (!(gen_coupling > config_.GetItem< double >("rpv_coupling.min") && gen_coupling < config_.GetItem< double >("rpv_coupling.max")))return true;
    }

    return false;
}

void specialAna::FillSystematics(const pxl::Event* event, std::string const particleName) {
    FillSystematicsUpDown(event, particleName, "Up", "Scale");
    FillSystematicsUpDown(event, particleName, "Down", "Scale");
    // FillSystematicsUpDown(event, particleName, "Up", "Resolution");
    // FillSystematicsUpDown(event, particleName, "Down", "Resolution");
}

void specialAna::FillSystematicsUpDown(const pxl::Event* event, std::string const particleName, std::string const updown, std::string const shiftType) {
    pxl::EventView *tempEventView;

    /// extract one EventView
    /// make sure the object key is the same as in Systematics.cc specified
    tempEventView = event->findObject< pxl::EventView >(particleName + "_syst" + shiftType + updown);

    if (tempEventView == 0) {
        throw std::runtime_error("specialAna.cc: no EventView '" + particleName + "_syst" + shiftType + updown + "' found!");
    }
    /// get all particles
    std::vector< pxl::Particle* > shiftedParticles;
    tempEventView->getObjectsOfType< pxl::Particle >(shiftedParticles);

    /// backup OldList
    RememberMET = METList;
    METList = new std::vector< pxl::Particle* >;
    if (particleName == "Muon") {
        RememberPart = MuonList;
        MuonList = new std::vector< pxl::Particle* >;
        for (std::vector< pxl::Particle* >::const_iterator part_it = shiftedParticles.begin(); part_it != shiftedParticles.end(); ++part_it) {
            pxl::Particle *part = *part_it;
            std::string Name = part->getName();
            if (      Name == "Muon"    ) MuonList->push_back(part);
            else if ( Name == m_METType ) METList->push_back(part);
        }
    } else if (particleName == "Ele") {
        RememberPart = EleList;
        EleList = new std::vector< pxl::Particle* >;
        for (std::vector< pxl::Particle* >::const_iterator part_it = shiftedParticles.begin(); part_it != shiftedParticles.end(); ++part_it) {
            pxl::Particle *part = *part_it;
            std::string Name = part->getName();
            if (      Name == "Ele"     ) EleList->push_back(part);
            else if ( Name == m_METType ) METList->push_back(part);
        }
    } else if (particleName == "Tau") {
        RememberPart = TauList;
        TauList = new std::vector< pxl::Particle* >;
        for (std::vector< pxl::Particle* >::const_iterator part_it = shiftedParticles.begin(); part_it != shiftedParticles.end(); ++part_it) {
            pxl::Particle *part = *part_it;
            std::string Name = part->getName();
            if (      Name == m_TauType ) TauList->push_back(part);
            else if ( Name == m_METType ) METList->push_back(part);
        }
    }  // else if(particleName=="JET"){
    // }else if(particleName==m_METType){}

    /// reset the chosen MET and lepton
    if (METList->size() > 0) {
        sel_met = METList->at(0);
    } else {
        sel_met = 0;
    }
    sel_lepton_prompt = 0;
    sel_lepton_nprompt = 0;
    sel_lepton_nprompt_corr = 0;

    resonance_mass["emu"] = 0;
    resonance_mass_gen["emu"] = 0;
    resonance_mass["etau"] = 0;
    resonance_mass_gen["etau"] = 0;
    resonance_mass["mutau"] = 0;
    resonance_mass_gen["mutau"] = 0;
    resonance_mass["etaue"] = 0;
    resonance_mass_gen["etaue"] = 0;
    resonance_mass["etaumu"] = 0;
    resonance_mass_gen["etaumu"] = 0;
    resonance_mass["mutaue"] = 0;
    resonance_mass_gen["mutaue"] = 0;
    resonance_mass["mutaumu"] = 0;
    resonance_mass_gen["mutaumu"] = 0;

    KinematicsSelector("_" + particleName + "_syst_" + shiftType + updown);

    /// return to backup
    delete METList;
    METList = RememberMET;
    if (particleName == "Muon") {
        delete MuonList;
        MuonList = RememberPart;
    } else if (particleName == "Ele") {
        delete EleList;
        EleList = RememberPart;
    } else if (particleName == "Tau") {
        delete TauList;
        TauList = RememberPart;
    }  // else if(particleName=="JET"){
    // }else if(particleName==m_METType){}
}

void specialAna::FillControllHistos() {
    HistClass::Fill("Ctr_Vtx_unweighted", m_RecEvtView->getUserRecord("NumVertices"), event_weight);
    HistClass::Fill("Ctr_Vtx_weighted", m_RecEvtView->getUserRecord("NumVertices"), event_weight * pileup_weight);

    if (not runOnData) {
        HistClass::Fill("Ctr_pT_hat", getPtHat(), weight);
        HistClass::Fill("Ctr_HT", getHT(), weight);
    }
}

void specialAna::Init_emu_cuts() {
    emu_cut_cfgs["kinematics"] = Cuts("kinematics",             500, 0, 500);
    emu_cut_cfgs["OppSign_charge"] = Cuts("OppSign_charge",     4, -2, 2);
    emu_cut_cfgs["BJet_veto"] = Cuts("BJet_veto",               10, 0, 10);
    emu_cut_cfgs["DeltaPhi_emu"] = Cuts("DeltaPhi_emu",         100, 0, 3.2);
}

void specialAna::Init_etau_cuts() {
    etau_cut_cfgs["kinematics"] = Cuts("kinematics",            500, 0, 500);
}

void specialAna::Init_mutau_cuts() {
    mutau_cut_cfgs["kinematics"] = Cuts("kinematics",           500, 0, 500);
    mutau_cut_cfgs["zeta"] = Cuts("zeta",                       500, 0, 500, 500, 0, 500, "p_{#zeta} (GeV)", "p_{#zeta}^{vis} (GeV)");
    mutau_cut_cfgs["DeltaPhi_tauMET"] = Cuts("DeltaPhi_tauMET", 100, 0, 3.2);
    mutau_cut_cfgs["DeltaPhi_mutau"] = Cuts("DeltaPhi_mutau",   100, 0, 3.2);
    mutau_cut_cfgs["BJet_veto"] = Cuts("BJet_veto",             10, 0, 10);
    mutau_cut_cfgs["OppSign_charge"] = Cuts("OppSign_charge",   4, -2, 2);
    mutau_cut_cfgs["MT_cut"] = Cuts("MT_cut",                   5000, 0, 5000);
}

void specialAna::Init_etaue_cuts() {
    etaue_cut_cfgs["kinematics"] = Cuts("kinematics",           500, 0, 500);
}

void specialAna::Init_etaumu_cuts() {
    etaumu_cut_cfgs["kinematics"] = Cuts("kinematics",          500, 0, 500);
}

void specialAna::Init_mutaue_cuts() {
    mutaue_cut_cfgs["kinematics"] = Cuts("kinematics",          500, 0, 500);
    mutaue_cut_cfgs["BJet_veto"] = Cuts("BJet_veto",            10, 0, 10);
    mutaue_cut_cfgs["DeltaPhi_emu"] = Cuts("DeltaPhi_emu",      100, 0, 3.2);
    mutaue_cut_cfgs["lep_fraction"] = Cuts("lep_fraction",      100, 0, 5);
    mutaue_cut_cfgs["pT_taumu_ratio"] = Cuts("pT_taumu_ratio",  100, 0, 10);
    mutaue_cut_cfgs["pT_elemu_ratio"] = Cuts("pT_elemu_ratio",  100, 0, 10);
}

void specialAna::Init_mutaumu_cuts() {
    mutaumu_cut_cfgs["kinematics"] = Cuts("kinematics",         500, 0, 500);
}

void specialAna::KinematicsSelector(std::string const endung) {
    /// Selection for the e-mu channel
    if (b_emu) {
        bool b_emu_success = false;
        if (FindResonance("emu", *EleList, *MuonList)) {
            Fill_Resonance_histograms(0, "emu", "ele", "muo", endung);
            b_emu_success = true;
            emu_cut_cfgs["kinematics"].SetPassed(true);
            emu_cut_cfgs["kinematics"].SetVars(resonance_mass["emu"]);
            if (endung == "") {
                keep_data_event = true;
                mkeep_resonance_mass["emu"] = resonance_mass["emu"];
                HistClass::Fill("Ctr_Vtx_emu_unweighted", m_RecEvtView->getUserRecord("NumVertices"), event_weight);
                HistClass::Fill("Ctr_Vtx_emu_weighted", m_RecEvtView->getUserRecord("NumVertices"), event_weight * pileup_weight);
            }
        } else {
            b_emu_success = false;
            emu_cut_cfgs["kinematics"].SetPassed(false);
            emu_cut_cfgs["kinematics"].SetVars(resonance_mass["emu"]);
        }
        /// Make the same-sign charge cut
        if (OppSign_charge(&emu_cut_cfgs["OppSign_charge"])) {
            if (b_emu_success) {
                Fill_Resonance_histograms(1, "emu", "ele", "muo", endung);
                b_emu_success = true;
            }
            emu_cut_cfgs["OppSign_charge"].SetPassed(true);
        } else {
            b_emu_success = false;
            emu_cut_cfgs["OppSign_charge"].SetPassed(false);
        }
        /// Make the b-jet veto
        if (Bjet_veto(&emu_cut_cfgs["BJet_veto"])) {
            if (b_emu_success) {
                Fill_Resonance_histograms(2, "emu", "ele", "muo", endung);
                b_emu_success = true;
            }
            emu_cut_cfgs["BJet_veto"].SetPassed(true);
        } else {
            b_emu_success = false;
            emu_cut_cfgs["BJet_veto"].SetPassed(false);
        }
        /// Make the cut on DeltaPhi(e,mu)
        if (Make_DeltaPhi_emu(&emu_cut_cfgs["DeltaPhi_emu"])) {
            if (b_emu_success) {
                Fill_Resonance_histograms(3, "emu", "ele", "muo", endung);
                b_emu_success = true;
            }
            emu_cut_cfgs["DeltaPhi_emu"].SetPassed(true);
        } else {
            b_emu_success = false;
            emu_cut_cfgs["DeltaPhi_emu"].SetPassed(false);
        }
        Fill_N1_histos("emu", emu_cut_cfgs, endung);
    }
    ///-----------------------------------------------------------------
    /// Selection for the e-tau_h channel
    if (b_etau) {
        bool b_etau_success = false;
        if (FindResonance("etau", *EleList, *TauList, *METList)) {
            Fill_Resonance_histograms(0, "etau", "ele", "tau", endung);
            b_etau_success = true;
            etau_cut_cfgs["kinematics"].SetPassed(true);
            etau_cut_cfgs["kinematics"].SetVars(resonance_mass["etau"]);
        } else {
            b_etau_success = false;
            etau_cut_cfgs["kinematics"].SetPassed(false);
            etau_cut_cfgs["kinematics"].SetVars(resonance_mass["etau"]);
        }
        Fill_N1_histos("etau", etau_cut_cfgs, endung);
    }
    ///-----------------------------------------------------------------
    /// Selection for the muo-tau_h channel
    if (b_mutau) {
        bool b_mutau_success = false;
        /// Find the actual resonance
        if (FindResonance("mutau", *MuonList, *TauList, *METList)) {
            Fill_Resonance_histograms(0, "mutau", "muo", "tau", endung);
            b_mutau_success = true;
            mutau_cut_cfgs["kinematics"].SetPassed(true);
            mutau_cut_cfgs["kinematics"].SetVars(resonance_mass["mutau"]);
        } else {
            b_mutau_success = false;
            mutau_cut_cfgs["kinematics"].SetPassed(false);
            mutau_cut_cfgs["kinematics"].SetVars(resonance_mass["mutau"]);
        }
        /// Make the cut on zeta
        if (Make_zeta_cut(&mutau_cut_cfgs["zeta"])) {
            if (b_mutau_success) {
                Fill_Resonance_histograms(1, "mutau", "muo", "tau", endung);
                b_mutau_success = true;
            }
            mutau_cut_cfgs["zeta"].SetPassed(true);
        } else {
            b_mutau_success = false;
            mutau_cut_cfgs["zeta"].SetPassed(false);
        }
        /// Make the cut on DeltaPhi(tau,MET)
        if (Make_DeltaPhi_tauMET(&mutau_cut_cfgs["DeltaPhi_tauMET"])) {
            if (b_mutau_success) {
                Fill_Resonance_histograms(2, "mutau", "muo", "tau", endung);
                b_mutau_success = true;
            }
            mutau_cut_cfgs["DeltaPhi_tauMET"].SetPassed(true);
        } else {
            b_mutau_success = false;
            mutau_cut_cfgs["DeltaPhi_tauMET"].SetPassed(false);
        }
        /// Make the cut on DeltaPhi(mu,tau)
        if (Make_DeltaPhi_mutau(&mutau_cut_cfgs["DeltaPhi_mutau"])) {
            if (b_mutau_success) {
                Fill_Resonance_histograms(3, "mutau", "muo", "tau", endung);
                b_mutau_success = true;
            }
            mutau_cut_cfgs["DeltaPhi_mutau"].SetPassed(true);
        } else {
            b_mutau_success = false;
            mutau_cut_cfgs["DeltaPhi_mutau"].SetPassed(false);
        }
        /// Make the b-jet veto
        if (Bjet_veto(&mutau_cut_cfgs["BJet_veto"])) {
            if (b_mutau_success) {
                Fill_Resonance_histograms(4, "mutau", "muo", "tau", endung);
                b_mutau_success = true;
            }
            mutau_cut_cfgs["BJet_veto"].SetPassed(true);
        } else {
            b_mutau_success = false;
            mutau_cut_cfgs["BJet_veto"].SetPassed(false);
        }
        /// Make the same-sign charge cut
        if (OppSign_charge(&mutau_cut_cfgs["OppSign_charge"])) {
            if (b_mutau_success) {
                Fill_Resonance_histograms(5, "mutau", "muo", "tau", endung);
                b_mutau_success = true;
            }
            mutau_cut_cfgs["OppSign_charge"].SetPassed(true);
        } else {
            b_mutau_success = false;
            mutau_cut_cfgs["OppSign_charge"].SetPassed(false);
        }
        /// Make the M_T cut
        if (MT_cut(&mutau_cut_cfgs["MT_cut"])) {
            if (b_mutau_success) {
                Fill_Resonance_histograms(6, "mutau", "muo", "tau", endung);
                b_mutau_success = true;
                keep_data_event = true;
                mkeep_resonance_mass["mutau"] = resonance_mass["mutau"];
            }
            mutau_cut_cfgs["MT_cut"].SetPassed(true);
        } else {
            b_mutau_success = false;
            mutau_cut_cfgs["MT_cut"].SetPassed(false);
        }
        /// Fill the N-1 histograms
        Fill_N1_histos("mutau", mutau_cut_cfgs, endung);
    }
    ///-----------------------------------------------------------------
    /// Selection for the e-tau_e channel
    if (b_etaue) {
        bool b_etaue_success = false;
        if (FindResonance("etaue", *EleList, *EleList, *METList)) {
            Fill_Resonance_histograms(0, "etaue", "ele", "tau_ele", endung);
            b_etaue_success = true;
            etaue_cut_cfgs["kinematics"].SetPassed(true);
            etaue_cut_cfgs["kinematics"].SetVars(resonance_mass["etaue"]);
        }e lse {
            b_etaue_success = false;
            etaue_cut_cfgs["kinematics"].SetPassed(false);
            etaue_cut_cfgs["kinematics"].SetVars(resonance_mass["etaue"]);
        }
        Fill_N1_histos("etaue", etaue_cut_cfgs, endung);
    }
    ///-----------------------------------------------------------------
    /// Selection for the e-tau_muo channel
    if (b_etaumu) {
        bool b_etaumu_success = false;
        if (FindResonance("etaumu", *EleList, *MuonList, *METList)) {
            Fill_Resonance_histograms(0, "etaumu", "ele", "tau_muo", endung);
            b_etaumu_success = true;
            etaumu_cut_cfgs["kinematics"].SetPassed(true);
            etaumu_cut_cfgs["kinematics"].SetVars(resonance_mass["etaumu"]);
        } else {
            b_etaumu_success = false;
            etaumu_cut_cfgs["kinematics"].SetPassed(false);
            etaumu_cut_cfgs["kinematics"].SetVars(resonance_mass["etaumu"]);
        }
        Fill_N1_histos("etaumu", etaumu_cut_cfgs, endung);
    }
    ///-----------------------------------------------------------------
    /// Selection for the muo-tau_e channel
    if (b_mutaue) {
        bool b_mutaue_success = false;
        /// Find the actual resonance
        if (FindResonance("mutaue", *MuonList, *EleList, *METList)) {
            Fill_Resonance_histograms(0, "mutaue", "muo", "tau_ele", endung);
            b_mutaue_success = true;
            mutaue_cut_cfgs["kinematics"].SetPassed(true);
            mutaue_cut_cfgs["kinematics"].SetVars(resonance_mass["mutaue"]);
        } else {
            b_mutaue_success = false;
            mutaue_cut_cfgs["kinematics"].SetPassed(false);
            mutaue_cut_cfgs["kinematics"].SetVars(resonance_mass["mutaue"]);
        }
        /// Make the b-jet veto
        if (Bjet_veto(&mutaue_cut_cfgs["BJet_veto"])) {
            if (b_mutaue_success) {
                Fill_Resonance_histograms(1, "mutaue", "muo", "tau_ele", endung);
                b_mutaue_success = true;
            }
            mutaue_cut_cfgs["BJet_veto"].SetPassed(true);
        } else {
            b_mutaue_success = false;
            mutaue_cut_cfgs["BJet_veto"].SetPassed(false);
        }
        /// Make the cut on DeltaPhi(e,mu)
        if (Make_DeltaPhi_tauemu(&mutaue_cut_cfgs["DeltaPhi_emu"])) {
            if (b_mutaue_success) {
                Fill_Resonance_histograms(2, "mutaue", "muo", "tau_ele", endung);
                b_mutaue_success = true;
            }
            mutaue_cut_cfgs["DeltaPhi_emu"].SetPassed(true);
        } else {
            b_mutaue_success = false;
            mutaue_cut_cfgs["DeltaPhi_emu"].SetPassed(false);
        }
        /// Make the cut on the leptonic pT fraction
        if (Leptonic_fraction_cut(&mutaue_cut_cfgs["lep_fraction"])) {
            if (b_mutaue_success) {
                Fill_Resonance_histograms(3, "mutaue", "muo", "tau_ele", endung);
                b_mutaue_success = true;
            }
            mutaue_cut_cfgs["lep_fraction"].SetPassed(true);
        } else {
            b_mutaue_success = false;
            mutaue_cut_cfgs["lep_fraction"].SetPassed(false);
        }
        /// Make the cut on the pT ratio of mu and tau
        if (pT_mutau_ratio_cut(&mutaue_cut_cfgs["pT_taumu_ratio"])) {
            if (b_mutaue_success) {
                Fill_Resonance_histograms(4, "mutaue", "muo", "tau_ele", endung);
                b_mutaue_success = true;
            }
            mutaue_cut_cfgs["pT_taumu_ratio"].SetPassed(true);
        } else {
            b_mutaue_success = false;
            mutaue_cut_cfgs["pT_taumu_ratio"].SetPassed(false);
        }
        /// Make the cut on the pT ratio of mu and ele
        if (pT_muele_ratio_cut(&mutaue_cut_cfgs["pT_elemu_ratio"])) {
            if (b_mutaue_success) {
                Fill_Resonance_histograms(5, "mutaue", "muo", "tau_ele", endung);
                b_mutaue_success = true;
                keep_data_event = true;
                mkeep_resonance_mass["mutaue"] = resonance_mass["mutaue"];
            }
            mutaue_cut_cfgs["pT_elemu_ratio"].SetPassed(true);
        } else {
            b_mutaue_success = false;
            mutaue_cut_cfgs["pT_elemu_ratio"].SetPassed(false);
        }
        Fill_N1_histos("mutaue", mutaue_cut_cfgs, endung);
    }
    ///-----------------------------------------------------------------
    /// Selection for the muo-tau_muo channel
    if (b_mutaumu) {
        bool b_mutaumu_success = false;
        if (FindResonance("mutaumu", *MuonList, *MuonList, *METList)) {
            Fill_Resonance_histograms(0, "mutaumu", "muo", "tau_muo", endung);
            b_mutaumu_success = true;
            mutaumu_cut_cfgs["kinematics"].SetPassed(true);
            mutaumu_cut_cfgs["kinematics"].SetVars(resonance_mass["mutaumu"]);
        } else {
            b_mutaumu_success = false;
            mutaumu_cut_cfgs["kinematics"].SetPassed(false);
            mutaumu_cut_cfgs["kinematics"].SetVars(resonance_mass["mutaumu"]);
        }
        Fill_N1_histos("mutaumu", mutaumu_cut_cfgs, endung);
    }
}

void specialAna::Create_trigger_effs() {
    for (std::vector< std::string >::const_iterator it=m_trigger_string.begin(); it!= m_trigger_string.end(); it++) {
        const char* temp_trigger_name = (*it).c_str();
        HistClass::CreateEff(temp_trigger_name, 20, 0, 200, "p_{T}^{#mu} (GeV)");
    }
}

void specialAna::Fill_trigger_effs() {
    for (std::vector< std::string >::const_iterator it=m_trigger_string.begin(); it!= m_trigger_string.end(); it++) {
        const char* temp_trigger_name = (*it).c_str();
        bool trigger_decision = false;
        double trigger_pt_val = 0;
        HistClass::FillEff(temp_trigger_name, trigger_pt_val, trigger_decision);
    }
}

pxl::Particle* specialAna::Get_Trigger_match(std::string name, pxl::Particle* lepton) {
    // double part_temp_eta = lepton->getEta();
    // double part_temp_phi = lepton->getPhi();
    // int part_temp_id = 0;
    // if(name == "Tau"){
        // part_temp_id = 15;
    // }else if(name == "MET"){
        // part_temp_id = 12;
    // }else if(name == "Muon"){
        // part_temp_id = 13;
    // }else if(name == "Ele"){
        // part_temp_id = 11;
    // }
    // double temp_delta_r = 10;
    pxl::Particle* gen_match = 0;
    // for( vector< pxl::Particle* >::const_iterator part_it = S3ListGen->begin(); part_it != S3ListGen->end(); ++part_it ) {
        // pxl::Particle *part_i = *part_it;
        // int part_temp_truth_id = 0;
        // if(b_8TeV) {
            // part_temp_truth_id = TMath::Abs(part_i->getUserRecord("id").asInt32());
        // }else if(b_13TeV) {
            // part_temp_truth_id = TMath::Abs(part_i->getPdgNumber());
        // }
        // if(part_temp_id != part_temp_truth_id) continue;
        // double test_delta_r = sqrt(pow(part_temp_eta - part_i->getEta(),2) + pow(part_temp_phi - part_i->getPhi(),2));
        // if(test_delta_r < temp_delta_r){
            // temp_delta_r = test_delta_r;
            // gen_match = part_i;
        // }
    // }
    return gen_match;
}

void specialAna::Create_N1_histos(const char* channel, const std::map< std::string, Cuts > &m_cfg, std::string const endung) {
    for (auto iterator = m_cfg.begin(); iterator != m_cfg.end(); iterator++) {
        std::string dummy_key = iterator->first;
        if (iterator->second.dim() == 1) {
            HistClass::CreateHisto(2, TString::Format("N-1_%s_", channel) + dummy_key + endung, iterator->second.bx(), iterator->second.xmi(), iterator->second.xma(), iterator->second.xt() );
        } else if (iterator->second.dim() == 2) {
            HistClass::CreateHisto(TString::Format("0_N-1_%s_", channel) + dummy_key + endung, iterator->second.bx(), iterator->second.xmi(), iterator->second.xma(), iterator->second.by(), iterator->second.ymi(), iterator->second.yma(), iterator->second.xt(), iterator->second.yt() );
            HistClass::CreateHisto(TString::Format("1_N-1_%s_", channel) + dummy_key + endung, iterator->second.bx(), iterator->second.xmi(), iterator->second.xma(), iterator->second.by(), iterator->second.ymi(), iterator->second.yma(), iterator->second.xt(), iterator->second.yt() );
        } else {
            std::cerr << "At the moment only one and two dimensional N-1 distributions are supported!" << std::endl;
        }
    }
}

void specialAna::Fill_N1_histos(const char* channel, const std::map< std::string, Cuts > &m_cfg, std::string const endung) {
    for (auto iterator = m_cfg.begin(); iterator != m_cfg.end(); iterator++) {
        std::string dummy_key = iterator->first;
        bool do_n_plot = true;
        for (auto jterator = m_cfg.begin(); jterator != m_cfg.end(); jterator++) {
            if (dummy_key == jterator->first) continue;
            if (not jterator->second.pass()) {
                do_n_plot = false;
                break;
            }
        }
        if (iterator->second.dim() == 1) {
            HistClass::Fill(0, TString::Format("N-1_%s_", channel) + dummy_key + endung, iterator->second.v1(), weight);
            if (do_n_plot) {
                HistClass::Fill(1, TString::Format("N-1_%s_", channel) + dummy_key + endung, iterator->second.v1(), weight);
            }
        } else if (iterator->second.dim() == 2) {
            HistClass::Fill(TString::Format("0_N-1_%s_", channel) + dummy_key + endung, iterator->second.v1(), iterator->second.v2(), weight);
            if (do_n_plot) {
                HistClass::Fill(TString::Format("1_N-1_%s_", channel) + dummy_key + endung, iterator->second.v1(), iterator->second.v2(), weight);
            }
        } else {
            std::cerr << "At the moment only one and two dimensional N-1 distributions are supported!" << std::endl;
            return;
        }
    }
}

void specialAna::GenSelector() {
    if (b_emu) {
        if (FindResonance("emu", *S3ListGen)) {
            Fill_Gen_histograms("emu", "ele", "muo");
        }
    }
    if (b_etau) {
        if (FindResonance("etau", *S3ListGen)) {
            Fill_Gen_histograms("etau", "ele", "tau");
        }
    }
    if (b_mutau) {
        if (FindResonance("mutau", *S3ListGen)) {
            Fill_Gen_histograms("mutau", "muo", "tau");
        }
    }
    if (b_etaue) {
        if (FindResonance("etaue", *S3ListGen)) {
            Fill_Gen_histograms("etaue", "ele", "tau_ele");
        }
    }
    if (b_etaumu) {
        if (FindResonance("etaumu", *S3ListGen)) {
            Fill_Gen_histograms("etaumu", "ele", "tau_muo");
        }
    }
    if (b_mutaue) {
        if (FindResonance("mutaue", *S3ListGen)) {
            Fill_Gen_histograms("mutaue", "muo", "tau_ele");
        }
    }
    if (b_mutaumu) {
        if (FindResonance("mutaumu", *S3ListGen)) {
            Fill_Gen_histograms("mutaumu", "muo", "tau_muo");
        }
    }
}

void specialAna::Create_Gen_histograms(const char* channel, const char* part1, const char* part2) {
    /// Resonant mass histogram
    HistClass::CreateHisto(TString::Format("%s_Mass_Gen",            channel),               5000, 0, 5000, TString::Format("M_{%s,%s}(gen) (GeV)",                    part1, part2));
    /// First particle histograms
    HistClass::CreateHisto(TString::Format("%s_pT_%s_Gen",           channel, part1),        5000, 0, 5000, TString::Format("p_{T}^{%s,gen} (GeV)",                    part1));
    HistClass::CreateHisto(TString::Format("%s_eta_%s_Gen",          channel, part1),        80, -4, 4,     TString::Format("#eta^{%s,gen}",                           part1));
    HistClass::CreateHisto(TString::Format("%s_phi_%s_Gen",          channel, part1),        40, -3.2, 3.2, TString::Format("#phi^{%s,gen} (rad)",                     part1));
    /// Second particle histograms
    HistClass::CreateHisto(TString::Format("%s_pT_%s_Gen",           channel, part2),        5000, 0, 5000, TString::Format("p_{T}^{%s,gen} (GeV)",                    part2));
    HistClass::CreateHisto(TString::Format("%s_eta_%s_Gen",          channel, part2),        80, -4, 4,     TString::Format("#eta^{%s,gen}",                           part2));
    HistClass::CreateHisto(TString::Format("%s_phi_%s_Gen",          channel, part2),        40, -3.2, 3.2, TString::Format("#phi^{%s,gen} (rad)",                     part2));
    /// Delta phi between the two particles
    HistClass::CreateHisto(TString::Format("%s_Delta_phi_%s_%s_Gen", channel, part1, part2), 40, 0, 3.2,    TString::Format("#Delta#phi(%s(gen),%s(gen)) (rad)",       part1, part2));
    /// pT ratio of the two particles
    HistClass::CreateHisto(TString::Format("%s_pT_ratio_%s_%s_Gen",  channel, part1, part2), 50, 0, 10,     TString::Format("#frac{p_{T}^{%s(gen)}}{p_{T}^{%s(gen)}}", part1, part2));
}

void specialAna::Fill_Gen_histograms(const char* channel, const char* part1, const char* part2) {
    /// Resonant mass histogram
    HistClass::Fill(TString::Format("%s_Mass_Gen",            channel),               resonance_mass_gen[channel],                         weight);
    /// First particle histograms
    HistClass::Fill(TString::Format("%s_pT_%s_Gen",           channel, part1),        sel_part1_gen -> getPt(),                            weight);
    HistClass::Fill(TString::Format("%s_eta_%s_Gen",          channel, part1),        sel_part1_gen -> getEta(),                           weight);
    HistClass::Fill(TString::Format("%s_phi_%s_Gen",          channel, part1),        sel_part1_gen -> getPhi(),                           weight);
    /// Second particle histograms
    HistClass::Fill(TString::Format("%s_pT_%s_Gen",           channel, part2),        sel_part2_gen -> getPt(),                            weight);
    HistClass::Fill(TString::Format("%s_eta_%s_Gen",          channel, part2),        sel_part2_gen -> getEta(),                           weight);
    HistClass::Fill(TString::Format("%s_phi_%s_Gen",          channel, part2),        sel_part2_gen -> getPhi(),                           weight);
    /// Delta phi between the two particles
    HistClass::Fill(TString::Format("%s_Delta_phi_%s_%s_Gen", channel, part1, part2), DeltaPhi(sel_part1_gen, sel_part2_gen),              weight);
    /// pT ratio of the two particles
    HistClass::Fill(TString::Format("%s_pT_ratio_%s_%s_Gen",  channel, part1, part2), sel_part1_gen -> getPt() / sel_part2_gen -> getPt(), weight);
}

void specialAna::Create_Resonance_histograms(int n_histos, const char* channel, const char* part1, const char* part2, std::string const endung) {
    /// Cutflow histogram
    HistClass::CreateHisto(TString::Format("%s_Cutflow",                         channel) + endung,               n_histos, 0, n_histos, "Cut stage");
    /// Resonant mass histogram
    HistClass::CreateHisto(n_histos, TString::Format("%s_Mass",                  channel) + endung,               5000, 0, 5000, TString::Format("M_{%s,%s} (GeV)",                         part1, part2) );
    /// Resonant mass resolution histogram
    HistClass::CreateHisto(n_histos, TString::Format("%s_Mass_resolution",       channel) + endung, 500, 0, 5000, 1000, -10, 10, TString::Format("M^{gen}_{%s,%s} (GeV)",                 part1, part2), TString::Format("M-M_{gen}/M_{gen}(%s,%s)", part1, part2));
    HistClass::CreateHisto(n_histos, TString::Format("%s_Mass_resolution",       channel) + endung,               1000, -10, 10, TString::Format("M-M_{gen}/M_{gen}(%s,%s)",                part1, part2) );
    /// First particle histograms
    HistClass::CreateHisto(n_histos, TString::Format("%s_pT_%s",                 channel, part1) + endung,        5000, 0, 5000, TString::Format("p_{T}^{%s} (GeV)",                        part1) );
    HistClass::CreateHisto(n_histos, TString::Format("%s_eta_%s",                channel, part1) + endung,        80, -4, 4,     TString::Format("#eta^{%s}",                               part1) );
    HistClass::CreateHisto(n_histos, TString::Format("%s_phi_%s",                channel, part1) + endung,        40, -3.2, 3.2, TString::Format("#phi^{%s} (rad)",                         part1) );
    /// Second particle histograms
    HistClass::CreateHisto(n_histos, TString::Format("%s_pT_%s",                 channel, part2) + endung,        5000, 0, 5000, TString::Format("p_{T}^{%s} (GeV)",                        part2) );
    HistClass::CreateHisto(n_histos, TString::Format("%s_eta_%s",                channel, part2) + endung,        80, -4, 4,     TString::Format("#eta^{%s}",                               part2) );
    HistClass::CreateHisto(n_histos, TString::Format("%s_phi_%s",                channel, part2) + endung,        40, -3.2, 3.2, TString::Format("#phi^{%s} (rad)",                         part2) );
    /// Delta phi between the two particles
    HistClass::CreateHisto(n_histos, TString::Format("%s_Delta_phi_%s_%s",       channel, part1, part2) + endung, 40, 0, 3.2,    TString::Format("#Delta#phi(%s,%s) (rad)",                 part1, part2) );
    /// pT ratio of the two particles
    HistClass::CreateHisto(n_histos, TString::Format("%s_pT_ratio_%s_%s",        channel, part1, part2) + endung, 50, 0, 10,     TString::Format("#frac{p_{T}^{%s}}{p_{T}^{%s}}",           part1, part2) );
    /// Create histograms for channels with MET
    if (channel != const_cast<char*>("emu")) {
        /// MET histograms
        HistClass::CreateHisto(n_histos, TString::Format("%s_MET",                   channel) + endung,              5000, 0, 5000, "E_{T}^{miss} (GeV)");
        HistClass::CreateHisto(n_histos, TString::Format("%s_phi_MET",               channel) + endung,              40, -3.2, 3.2, "#phi^{E_{T}^{miss}} (rad)");
        /// Corrected second particle histogram
        HistClass::CreateHisto(n_histos, TString::Format("%s_pT_%s_corr",            channel, part2) + endung,        5000, 0, 5000, TString::Format("p_{T}^{%s(corr)} (GeV)",                  part2) );
        HistClass::CreateHisto(n_histos, TString::Format("%s_eta_%s_corr",           channel, part2) + endung,        80, -4, 4,     TString::Format("#eta^{%s(corr)}",                         part2) );
        HistClass::CreateHisto(n_histos, TString::Format("%s_phi_%s_corr",           channel, part2) + endung,        40, -3.2, 3.2, TString::Format("#phi^{%s(corr)} (rad)",                   part2) );
        /// Delta phi between the other particles
        HistClass::CreateHisto(n_histos, TString::Format("%s_Delta_phi_%s_MET",      channel, part1) + endung,        40, 0, 3.2,    TString::Format("#Delta#phi(%s,E_{T}^{miss}) (rad)",       part1) );
        HistClass::CreateHisto(n_histos, TString::Format("%s_Delta_phi_%s_MET",      channel, part2) + endung,        40, 0, 3.2,    TString::Format("#Delta#phi(%s,E_{T}^{miss}) (rad)",       part2) );
        HistClass::CreateHisto(n_histos, TString::Format("%s_Delta_phi_%s_MET_corr", channel, part2) + endung,        40, 0, 3.2,    TString::Format("#Delta#phi(%s(corr),E_{T}^{miss}) (rad)", part2) );
        HistClass::CreateHisto(n_histos, TString::Format("%s_Delta_phi_%s_%s_corr",  channel, part1, part2) + endung, 40, 0, 3.2,    TString::Format("#Delta#phi(%s,%s(corr)) (rad)",           part1, part2) );
        /// pT ratio of the other particles
        HistClass::CreateHisto(n_histos, TString::Format("%s_pT_ratio_%s_MET",       channel, part1) + endung,        50, 0, 10,     TString::Format("#frac{p_{T}^{%s}}{E_{T}^{miss}}",         part1) );
        HistClass::CreateHisto(n_histos, TString::Format("%s_pT_ratio_%s_MET",       channel, part2) + endung,        50, 0, 10,     TString::Format("#frac{p_{T}^{%s}}{E_{T}^{miss}}",         part2) );
        HistClass::CreateHisto(n_histos, TString::Format("%s_pT_ratio_%s_MET_corr",  channel, part2) + endung,        50, 0, 10,     TString::Format("#frac{p_{T}^{%s(corr)}}{E_{T}^{miss}}",   part2) );
        HistClass::CreateHisto(n_histos, TString::Format("%s_pT_ratio_%s_%s_corr",   channel, part1, part2) + endung, 50, 0, 10,     TString::Format("#frac{p_{T}^{%s}}{p_{T}^{%s(corr)}}",     part1, part2) );
    }
}

void specialAna::Fill_Resonance_histograms(int n_histos, const char* channel, const char* part1, const char* part2, std::string const endung) {
    /// Cutflow histogram
    HistClass::Fill(TString::Format("%s_Cutflow",                         channel) + endung,               n_histos,                                                          weight);
    /// Resonant mass histogram
    HistClass::Fill(n_histos, TString::Format("%s_Mass",                  channel) + endung,               resonance_mass[channel],                                           weight);
    /// Resonant mass resolution histogram
    double dummy_resolution = (resonance_mass[channel] - resonance_mass_gen[channel])/resonance_mass_gen[channel];
    HistClass::Fill(n_histos, TString::Format("%s_Mass_resolution",       channel) + endung,               dummy_resolution,                                                  weight);
    HistClass::Fill(n_histos, TString::Format("%s_Mass_resolution",       channel) + endung,               resonance_mass_gen[channel], dummy_resolution,                     weight);
    /// First particle histograms
    HistClass::Fill(n_histos, TString::Format("%s_pT_%s",                 channel, part1) + endung,        sel_lepton_prompt -> getPt(),                                      weight);
    HistClass::Fill(n_histos, TString::Format("%s_eta_%s",                channel, part1) + endung,        sel_lepton_prompt -> getEta(),                                     weight);
    HistClass::Fill(n_histos, TString::Format("%s_phi_%s",                channel, part1) + endung,        sel_lepton_prompt -> getPhi(),                                     weight);
    /// Second particle histograms
    HistClass::Fill(n_histos, TString::Format("%s_pT_%s",                 channel, part2) + endung,        sel_lepton_nprompt -> getPt(),                                     weight);
    HistClass::Fill(n_histos, TString::Format("%s_eta_%s",                channel, part2) + endung,        sel_lepton_nprompt -> getEta(),                                    weight);
    HistClass::Fill(n_histos, TString::Format("%s_phi_%s",                channel, part2) + endung,        sel_lepton_nprompt -> getPhi(),                                    weight);
    /// Delta phi between the two particles
    HistClass::Fill(n_histos, TString::Format("%s_Delta_phi_%s_%s",       channel, part1, part2) + endung, DeltaPhi(sel_lepton_prompt, sel_lepton_nprompt),                   weight);
    /// pT ratio of the two particles
    HistClass::Fill(n_histos, TString::Format("%s_pT_ratio_%s_%s",        channel, part1, part2) + endung, sel_lepton_prompt -> getPt() / sel_lepton_nprompt -> getPt(),      weight);
    /// Create histograms for channels with MET
    if (channel != const_cast<char*>("emu")) {
        /// MET histograms
        HistClass::Fill(n_histos, TString::Format("%s_MET",                   channel) + endung,               sel_met -> getPt(),                                                weight);
        HistClass::Fill(n_histos, TString::Format("%s_phi_MET",               channel) + endung,               sel_met -> getPhi(),                                               weight);
        /// Corrected second particle histogram
        HistClass::Fill(n_histos, TString::Format("%s_pT_%s_corr",            channel, part2) + endung,        sel_lepton_nprompt_corr -> getPt(),                                weight);
        HistClass::Fill(n_histos, TString::Format("%s_eta_%s_corr",           channel, part2) + endung,        sel_lepton_nprompt_corr -> getEta(),                               weight);
        HistClass::Fill(n_histos, TString::Format("%s_phi_%s_corr",           channel, part2) + endung,        sel_lepton_nprompt_corr -> getPhi(),                               weight);
        /// Delta phi between the other particles
        HistClass::Fill(n_histos, TString::Format("%s_Delta_phi_%s_MET",      channel, part1) + endung,        DeltaPhi(sel_lepton_prompt, sel_met),                              weight);
        HistClass::Fill(n_histos, TString::Format("%s_Delta_phi_%s_MET",      channel, part2) + endung,        DeltaPhi(sel_lepton_nprompt, sel_met),                             weight);
        HistClass::Fill(n_histos, TString::Format("%s_Delta_phi_%s_MET_corr", channel, part2) + endung,        DeltaPhi(sel_lepton_nprompt_corr, sel_met),                        weight);
        HistClass::Fill(n_histos, TString::Format("%s_Delta_phi_%s_%s_corr",  channel, part1, part2) + endung, DeltaPhi(sel_lepton_prompt, sel_lepton_nprompt_corr),              weight);
        /// pT ratio of the other particles
        HistClass::Fill(n_histos, TString::Format("%s_pT_ratio_%s_MET",       channel, part1) + endung,        sel_lepton_prompt -> getPt() / sel_met -> getPt(),                 weight);
        HistClass::Fill(n_histos, TString::Format("%s_pT_ratio_%s_MET",       channel, part2) + endung,        sel_lepton_nprompt -> getPt() / sel_met -> getPt(),                weight);
        HistClass::Fill(n_histos, TString::Format("%s_pT_ratio_%s_MET_corr",  channel, part2) + endung,        sel_lepton_nprompt_corr -> getPt() / sel_met -> getPt(),           weight);
        HistClass::Fill(n_histos, TString::Format("%s_pT_ratio_%s_%s_corr",   channel, part1, part2) + endung, sel_lepton_prompt -> getPt() / sel_lepton_nprompt_corr -> getPt(), weight);
    }
}

bool specialAna::FindResonance(const char* channel, std::vector< pxl::Particle* > gen_list) {
    int id_1, id_2;
    if (channel == const_cast<char*>("emu")) {
        id_1 = 11;
        id_2 = 13;
    } else if (channel == const_cast<char*>("etau") or channel == const_cast<char*>("etaue") or channel == const_cast<char*>("etaumu")) {
        id_1 = 11;
        id_2 = 15;
    } else if (channel == const_cast<char*>("mutau") or channel == const_cast<char*>("mutaue") or channel == const_cast<char*>("mutaumu")) {
        id_1 = 13;
        id_2 = 15;
    } else {
        return false;
    }

    resonance_mass_gen[channel] = 0;
    if (b_13TeV) {
        for (std::vector< pxl::Particle* >::const_iterator part_it = gen_list.begin(); part_it != gen_list.end(); ++part_it) {
            pxl::Particle *part_i = *part_it;
            if (TMath::Abs(part_i -> getPdgNumber()) == id_1) {
                for (std::vector< pxl::Particle* >::const_iterator part_jt = gen_list.begin(); part_jt != gen_list.end(); ++part_jt) {
                    pxl::Particle *part_j = *part_jt;
                    if (TMath::Abs(part_j -> getPdgNumber()) != id_2) continue;
                    pxl::Particle *part_sum = (pxl::Particle*) part_i->clone();
                    part_sum -> addP4(part_j);
                    if (part_sum -> getMass() > resonance_mass_gen[channel]) {
                        resonance_mass_gen[channel] = part_sum -> getMass();
                        sel_part1_gen = (pxl::Particle*) part_i->clone();
                        sel_part2_gen = (pxl::Particle*) part_j->clone();
                    }
                }
            }
        }
    } else if (b_8TeV) {
        for (std::vector< pxl::Particle* >::const_iterator part_it = gen_list.begin(); part_it != gen_list.end(); ++part_it) {
            pxl::Particle *part_i = *part_it;
            if (TMath::Abs(part_i -> getUserRecord("id").asInt32()) == id_1) {
                for (std::vector< pxl::Particle* >::const_iterator part_jt = gen_list.begin(); part_jt != gen_list.end(); ++part_jt) {
                    pxl::Particle *part_j = *part_jt;
                    if (TMath::Abs(part_j -> getUserRecord("id").asInt32()) != id_2) continue;
                    pxl::Particle *part_sum = (pxl::Particle*) part_i->clone();
                    part_sum -> addP4(part_j);
                    if (part_sum -> getMass() > resonance_mass_gen[channel]) {
                        resonance_mass_gen[channel] = part_sum -> getMass();
                        sel_part1_gen = (pxl::Particle*) part_i->clone();
                        sel_part2_gen = (pxl::Particle*) part_j->clone();
                    }
                }
            }
        }
    }
    if (resonance_mass_gen[channel] > 0) {
        return true;
    } else {
        return false;
    }
}

bool specialAna::FindResonance(const char* channel, std::vector< pxl::Particle* > part1_list, std::vector< pxl::Particle* > part2_list) {
    resonance_mass[channel] = 0;
    for (std::vector< pxl::Particle* >::const_iterator part_it = part1_list.begin(); part_it != part1_list.end(); ++part_it) {
        pxl::Particle *part_i = *part_it;
        if (Check_Par_ID(part_i)) {
            for (std::vector< pxl::Particle* >::const_iterator part_jt = part2_list.begin(); part_jt != part2_list.end(); ++part_jt) {
                pxl::Particle *part_j = *part_jt;
                if (not Check_Par_ID(part_j)) continue;
                pxl::Particle *part_sum = (pxl::Particle*) part_i->clone();
                part_sum -> addP4(part_j);
                if (part_sum -> getMass() > resonance_mass[channel]) {
                    resonance_mass[channel] = part_sum -> getMass();
                    sel_lepton_prompt = (pxl::Particle*) part_i->clone();
                    sel_lepton_nprompt = (pxl::Particle*) part_j->clone();
                }
            }
        }
    }
    if (resonance_mass[channel] > 0) {
        return true;
    } else {
        return false;
    }
}

bool specialAna::FindResonance(const char* channel, std::vector< pxl::Particle* > part1_list, std::vector< pxl::Particle* > part2_list, std::vector< pxl::Particle* > met_list) {
    resonance_mass[channel] = 0;
    if (not sel_met) return false;
    for (std::vector< pxl::Particle* >::const_iterator part_it = part1_list.begin(); part_it != part1_list.end(); ++part_it) {
        pxl::Particle *part_i = *part_it;
        if (Check_Par_ID(part_i)) {
            for (std::vector< pxl::Particle* >::const_iterator part_jt = part2_list.begin(); part_jt != part2_list.end(); ++part_jt) {
                pxl::Particle *part_j = *part_jt;
                if (not Check_Par_ID(part_j)) continue;
                pxl::Particle* dummy_taumu = (pxl::Particle*) part_i->clone();
                pxl::Particle* dummy_taumu_uncorr = (pxl::Particle*) part_i->clone();
                dummy_taumu->addP4(part_j);
                dummy_taumu_uncorr->addP4(part_j);
                pxl::Particle* dummy_met = new pxl::Particle();
                /// use tau eta to project MET
                TLorentzVector* calc_met = new TLorentzVector();
                calc_met -> SetPtEtaPhiM(sel_met->getPt(), part_j->getEta(), sel_met->getPhi(), 0);
                dummy_met->setP4(calc_met->Px(), calc_met->Py(), calc_met->Pz(), calc_met->E());
                dummy_taumu->addP4(dummy_met);
                delete calc_met;

                /// rotate MET to tau direction
                // TLorentzVector* calc_met = new TLorentzVector();
                // calc_met -> SetPtEtaPhiM(METList->at(0)->getPt(),0,METList->at(0)->getPhi(),0);
                // TVector3* tau_direction = new TVector3(TauList->at(i)->getPx(),TauList->at(i)->getPy(),TauList->at(i)->getPz());
                // *tau_direction = tau_direction -> Unit();
                // calc_met -> RotateUz(*tau_direction);
                // dummy_met->setP4(calc_met->Px(),calc_met->Py(),calc_met->Pz(),calc_met->E());
                // dummy_taumu->addP4(dummy_met);
                // delete tau_direction;
                // delete calc_met;

                /// project MET to tau direction
                // double dummy_p1 = METList->at(0)->getPx()/(TMath::Sin(TauList->at(i)->getTheta()) * TMath::Cos(TauList->at(i)->getPhi()));
                // double dummy_p2 = METList->at(0)->getPy()/(TMath::Sin(TauList->at(i)->getTheta()) * TMath::Sin(TauList->at(i)->getPhi()));
                // double dummy_p = (dummy_p1 + dummy_p2) / 2.;
                // dummy_met->setP4(dummy_p*TMath::Sin(TauList->at(i)->getTheta()) * TMath::Cos(TauList->at(i)->getPhi()),dummy_p*TMath::Sin(TauList->at(i)->getTheta()) * TMath::Sin(TauList->at(i)->getPhi()),dummy_p*TMath::Cos(TauList->at(i)->getTheta()),dummy_p);
                // dummy_taumu->addP4(dummy_met);

                /// project MET parallel to tau direction
                // double value = (METList->at(0)->getPx() * TauList->at(i)->getPx() + METList->at(0)->getPy() * TauList->at(i)->getPy()) / sqrt(pow(TauList->at(i)->getPx(),2) + pow(TauList->at(i)->getPy(),2));
                // TLorentzVector* calc_met = new TLorentzVector();
                // calc_met -> SetPtEtaPhiM(value,TauList->at(i)->getEta(),TauList->at(i)->getPhi(),0);
                // dummy_taumu->addP4(dummy_met);

                if (dummy_taumu->getMass() > resonance_mass[channel]) {
                    resonance_mass[channel] = dummy_taumu->getMass();
                    sel_lepton_prompt = (pxl::Particle*) part_i -> clone();
                    sel_lepton_nprompt = (pxl::Particle*) part_j -> clone();
                    sel_lepton_nprompt_corr = (pxl::Particle*) part_j -> clone();
                    sel_lepton_nprompt_corr -> addP4(dummy_met);
                }
            }
        }
    }
    if (resonance_mass[channel] > 0) {
        return true;
    } else {
        return false;
    }
}

bool specialAna::Check_Par_ID(pxl::Particle* part) {
    std::string name = part -> getName();
    if (name == m_TauType) {
        bool tau_id = Check_Tau_ID(part);
        return tau_id;
    } else if (name == "Ele") {
        bool ele_id = Check_Ele_ID(part);
        return ele_id;
    } else if (name == "Muon") {
        bool muo_id = Check_Muo_ID(part);
        return muo_id;
    } else {
        return false;
    }
}

bool specialAna::Check_Tau_ID(pxl::Particle* tau) {
    // bool passed = false;
    if (b_8TeV) {
        bool tau_ID = tau->getUserRecord("decayModeFindingOldDMs").asFloat() >= 1 ? true : false;
        bool tau_ISO = tau->getUserRecord("byLooseIsolationMVA3oldDMwLT").asFloat() >= 1 ? true : false;
        bool tau_ELE = tau->getUserRecord("againstElectronLooseMVA5").asFloat() >= 1 ? true : false;
        bool tau_MUO = tau->getUserRecord("againstMuonTight3").asFloat() >= 1 ? true : false;
        if (tau_ID && tau_ISO && tau_ELE && tau_MUO) return true;
    } else if (b_13TeV) {
        bool tau_ID = tau->getUserRecord("decayModeFinding").asFloat() >= 1 ? true : false;
        bool tau_ISO = tau->getUserRecord("byLooseIsolationMVA3oldDMwLT").asFloat() >= 1 ? true : false;
        bool tau_ELE = tau->getUserRecord("againstElectronLooseMVA5").asFloat() >= 1 ? true : false;
        bool tau_MUO = tau->getUserRecord("againstMuonTight3").asFloat() >= 1 ? true : false;
        if (tau_ID && tau_ISO && tau_ELE && tau_MUO) return true;
    }
    return false;
}

bool specialAna::Check_Muo_ID(pxl::Particle* muon) {
    // bool passed = false;
    bool muon_ID = muon->getUserRecord("isHighPtMuon").asBool() ? true : false;
    bool muon_ISO = false;
    if (b_8TeV) {
        muon_ISO = muon -> getUserRecord("IsoR3SumPt").asDouble() / muon -> getPt() < 0.1 ? true : false;
    } else if (b_13TeV) {
        muon_ISO = muon -> getUserRecord("IsoR3SumPt").asFloat() / muon -> getPt() < 0.1 ? true : false;
    }
    bool muon_eta = TMath::Abs(muon -> getEta()) < 2.1 ? true : false;
    bool muon_pt = muon -> getPt() > 45. ? true : false;
    if (muon_ID && muon_ISO && muon_eta && muon_pt) return true;
    return false;
}

bool specialAna::Check_Ele_ID(pxl::Particle* ele) {
    // bool passed = false;
    bool ele_ID = ele->getUserRecord("IDpassed").asBool() ? true : false;
    return ele_ID;
}

std::vector<double> specialAna::Make_zeta_stuff(pxl::Particle* muon, pxl::Particle* tau, pxl::Particle* met) {
    double p_zeta_vis = 0;
    double p_zeta = 0;

    if (met and muon and tau) {
        TVector3* vec_mu = new TVector3();
        TVector3* vec_tau = new TVector3();

        vec_mu -> SetXYZ(muon->getPx(), muon->getPy(), 0);
        vec_tau -> SetXYZ(tau->getPx(), tau->getPy(), 0);

        TVector3 bisec = vec_mu->Unit() + vec_tau->Unit();
        TVector3 bisec_norm = bisec.Unit();

        p_zeta_vis = (tau->getPx() * bisec_norm.X() + tau->getPy() * bisec_norm.Y()) + (muon->getPx() * bisec_norm.X() + muon->getPy() * bisec_norm.Y());
        p_zeta = p_zeta_vis + (met->getPx() * bisec_norm.X() + met->getPy() * bisec_norm.Y());
        delete vec_mu;
        delete vec_tau;
    }

    std::vector<double> out;
    out.push_back(p_zeta_vis);
    out.push_back(p_zeta);

    return out;
}

bool specialAna::Make_zeta_cut(Cuts* cuts) {
    std::vector<double> zeta_vals = Make_zeta_stuff(sel_lepton_prompt, sel_lepton_nprompt, sel_met);
    double zeta_steepnes_cut_value = -1.21;
    double zeta_offset_cut_value   = -24.1;
    cuts->SetVars(zeta_vals[0], zeta_vals[1]);
    if ((zeta_vals[0] + zeta_steepnes_cut_value * zeta_vals[1]) > zeta_offset_cut_value) {
        return true;
    } else {
        return false;
    }
}

bool specialAna::Make_DeltaPhi_tauMET(Cuts* cuts) {
    double delta_phi = 10.;
    if (sel_met and sel_lepton_nprompt) {
        delta_phi = DeltaPhi(sel_lepton_nprompt, sel_met);
    }
    double delta_phi_tau_met_cut_value = 1.3;
    cuts->SetVars(delta_phi);
    if (delta_phi < delta_phi_tau_met_cut_value) {
        return true;
    } else {
        return false;
    }
}

bool specialAna::Make_DeltaPhi_mutau(Cuts* cuts) {
    double delta_phi = 0.;
    if (sel_lepton_prompt and sel_lepton_nprompt) {
        delta_phi = DeltaPhi(sel_lepton_nprompt, sel_lepton_prompt);
    }
    double delta_phi_mu_tau_cut_value = 2.3;
    cuts->SetVars(delta_phi);
    if (delta_phi > delta_phi_mu_tau_cut_value) {
        return true;
    } else {
        return false;
    }
}

bool specialAna::Make_DeltaPhi_tauemu(Cuts* cuts) {
    double delta_phi = 0.;
    if (sel_lepton_prompt and sel_lepton_nprompt) {
        delta_phi = DeltaPhi(sel_lepton_nprompt, sel_lepton_prompt);
    }
    double delta_phi_mu_tau_cut_value = 2.7;
    cuts->SetVars(delta_phi);
    if (delta_phi > delta_phi_mu_tau_cut_value) {
        return true;
    } else {
        return false;
    }
}

bool specialAna::Make_DeltaPhi_emu(Cuts* cuts) {
    double delta_phi = 0.;
    if (sel_lepton_prompt and sel_lepton_nprompt) {
        delta_phi = DeltaPhi(sel_lepton_nprompt, sel_lepton_prompt);
    }
    double delta_phi_e_mu_cut_value = 2.7;
    cuts->SetVars(delta_phi);
    if (delta_phi > delta_phi_e_mu_cut_value) {
        return true;
    } else {
        return false;
    }
}

bool specialAna::Bjet_veto(Cuts* cuts) {
    cuts->SetVars(numBJet);
    if (numBJet < 1) {
        return true;
    } else {
        return false;
    }
}

bool specialAna::OppSign_charge(Cuts* cuts) {
    double charge_product = 0;
    if (sel_lepton_prompt and sel_lepton_nprompt) {
        charge_product = sel_lepton_prompt->getCharge() * sel_lepton_nprompt->getCharge();
    }
    cuts->SetVars(charge_product);
    if (charge_product < 0) {
        return true;
    } else {
        return false;
    }
}

bool specialAna::MT_cut(Cuts* cuts) {
    double mt = 0;
    if (sel_lepton_prompt and sel_met) {
        mt = MT(sel_lepton_prompt, sel_met);
    }
    cuts->SetVars(mt);
    double mt_min_cut_value = 180;
    if (mt > mt_min_cut_value) {
        return true;
    } else {
        return false;
    }
}

double specialAna::calc_lep_fraction() {
    double pT_sum_lep = sel_lepton_prompt->getPt() + sel_lepton_nprompt->getPt();
    double pT_sum_had = 0;
    for (std::vector< pxl::Particle* >::const_iterator part_it = JetList->begin(); part_it != JetList->end(); ++part_it) {
        pxl::Particle *part_i = *part_it;
        pT_sum_had += part_i->getPt();
    }
    double pT_sum_all = pT_sum_lep + pT_sum_had;
    return pT_sum_lep / pT_sum_all;
}

bool specialAna::Leptonic_fraction_cut(Cuts* cuts) {
    double lep_fraction = 0;
    if (sel_lepton_nprompt and sel_lepton_prompt) {
        lep_fraction = calc_lep_fraction();
    }
    cuts->SetVars(lep_fraction);
    double lep_fraction_cut_value = 0.8;
    if (lep_fraction > lep_fraction_cut_value) {
        return true;
    } else {
        return false;
    }
}

bool specialAna::pT_mutau_ratio_cut(Cuts* cuts) {
    double pT_ratio = 0;
    if (sel_lepton_nprompt_corr and sel_lepton_prompt) {
        pT_ratio = sel_lepton_nprompt_corr->getPt() / sel_lepton_prompt->getPt();
    }
    cuts->SetVars(pT_ratio);
    double pT_mutau_ratio_cut_min_val = 0.6;
    double pT_mutau_ratio_cut_max_val = 1.4;
    if (pT_ratio > pT_mutau_ratio_cut_min_val and pT_ratio < pT_mutau_ratio_cut_max_val) {
        return true;
    } else {
        return false;
    }
}

bool specialAna::pT_muele_ratio_cut(Cuts* cuts) {
    double pT_ratio = 0;
    if (sel_lepton_nprompt and sel_lepton_prompt) {
        pT_ratio = sel_lepton_prompt->getPt() / sel_lepton_nprompt->getEt();
    }
    cuts->SetVars(pT_ratio);
    double pT_muele_ratio_cut_min_val = 1;
    if (pT_ratio > pT_muele_ratio_cut_min_val) {
        return true;
    } else {
        return false;
    }
}

bool specialAna::TriggerSelector(const pxl::Event* event) {
    bool triggered = false;

    if (b_8TeV) {
        pxl::UserRecords::const_iterator us = m_TrigEvtView->getUserRecords().begin();
        for ( ; us != m_TrigEvtView->getUserRecords().end(); ++us ) {
            for (std::vector< std::string >::const_iterator it=m_trigger_string.begin(); it!= m_trigger_string.end(); it++) {
                if (std::string::npos != (*us).first.find(*it)) {
                    triggered = (*us).second;
                    triggers.insert(us->first);
                }
            }
        }
    } else if (b_13TeV) {
        for (std::vector< std::string >::const_iterator it=m_trigger_string.begin(); it!= m_trigger_string.end(); it++) {
            try {
                triggered = m_TrigEvtView->getUserRecord(*it);
            } catch ( std::runtime_error &exc ) {
                continue;
            }
            pxl::UserRecords::const_iterator us = m_TrigEvtView->getUserRecords().begin();
            for ( ; us != m_TrigEvtView->getUserRecords().end(); ++us ) {
                if (std::string::npos != (*us).first.find(*it)) {
                    triggers.insert(us->first);
                }
            }
        }
    }
     return (triggered);
}

void specialAna::Fill_Gen_Controll_histo() {
    int muon_gen_num = 0;
    int ele_gen_num = 0;
    int tau_gen_num = 0;
    for (uint i = 0; i < S3ListGen->size(); i++) {
        if (S3ListGen->at(i)->getPt() < 10 and not(TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 24)) {
            continue;
        }
        if (S3ListGen->at(i)->getPdgNumber() == 0) {
            if (S3ListGen->at(i)->hasUserRecord("id")) {
                S3ListGen->at(i)->setPdgNumber(S3ListGen->at(i)->getUserRecord("id"));
            }
        }
        if (TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 13) {
            muon_gen_num++;
            HistClass::Fill(0, "Muon_pt_Gen", S3ListGen->at(i)->getPt(), m_GenEvtView->getUserRecord("Weight"));
            HistClass::Fill(0, "Muon_eta_Gen", S3ListGen->at(i)->getEta(), m_GenEvtView->getUserRecord("Weight"));
            HistClass::Fill(0, "Muon_phi_Gen", S3ListGen->at(i)->getPhi(), m_GenEvtView->getUserRecord("Weight"));
        } else if (TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 15) {
            tau_gen_num++;
            HistClass::Fill(0, "Tau_pt_Gen", S3ListGen->at(i)->getPt(), m_GenEvtView->getUserRecord("Weight"));
            HistClass::Fill(0, "Tau_eta_Gen", S3ListGen->at(i)->getEta(), m_GenEvtView->getUserRecord("Weight"));
            HistClass::Fill(0, "Tau_phi_Gen", S3ListGen->at(i)->getPhi(), m_GenEvtView->getUserRecord("Weight"));
        } else if (TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 11) {
            ele_gen_num++;
            HistClass::Fill(0, "Ele_pt_Gen", S3ListGen->at(i)->getPt(), m_GenEvtView->getUserRecord("Weight"));
            HistClass::Fill(0, "Ele_eta_Gen", S3ListGen->at(i)->getEta(), m_GenEvtView->getUserRecord("Weight"));
            HistClass::Fill(0, "Ele_phi_Gen", S3ListGen->at(i)->getPhi(), m_GenEvtView->getUserRecord("Weight"));
        }
    }

    HistClass::Fill(0, "Tau_num_Gen", tau_gen_num, m_GenEvtView->getUserRecord("Weight"));
    HistClass::Fill(0, "Muon_num_Gen", muon_gen_num, m_GenEvtView->getUserRecord("Weight"));
    HistClass::Fill(0, "Ele_num_Gen", ele_gen_num, m_GenEvtView->getUserRecord("Weight"));
}

void specialAna::Fill_Particle_histos(int hist_number, pxl::Particle* lepton) {
    std::string name = lepton->getName();
    if (lepton->getName() == m_TauType) {
        name = "Tau";
    }
    if (lepton->getName() == m_METType) {
        name = "MET";
    }
    HistClass::Fill(hist_number, str(boost::format("%s_pt")%name), lepton->getPt(), weight);
    HistClass::Fill(hist_number, str(boost::format("%s_eta")%name), lepton->getEta(), weight);
    HistClass::Fill(hist_number, str(boost::format("%s_phi")%name), lepton->getPhi(), weight);
    if (hist_number == 2) {
        pxl::Particle* match = Get_Truth_match(name, lepton);
        if (match != 0) {
            if (match->getPt() < 500) {
                HistClass::Fill(name + TString::Format("_pt_resolution_0_500"), (lepton->getPt() - match->getPt())/match->getPt(), weight);
            } else if (match->getPt() < 1000) {
                HistClass::Fill(name + TString::Format("_pt_resolution_500_1000"), (lepton->getPt() - match->getPt())/match->getPt(), weight);
            } else if (match->getPt() < 1500) {
                HistClass::Fill(name + TString::Format("_pt_resolution_1000_1500"), (lepton->getPt() - match->getPt())/match->getPt(), weight);
            } else if (match->getPt() < 2000) {
                HistClass::Fill(name + TString::Format("_pt_resolution_1500_2000"), (lepton->getPt() - match->getPt())/match->getPt(), weight);
            } else {
                HistClass::Fill(name + TString::Format("_pt_resolution_2000"), (lepton->getPt() - match->getPt())/match->getPt(), weight);
            }
        }
    }
}

pxl::Particle* specialAna::Get_Truth_match(std::string name, pxl::Particle* lepton) {
    double part_temp_eta = lepton->getEta();
    double part_temp_phi = lepton->getPhi();
    int part_temp_id = 0;
    if (name == "Tau") {
        part_temp_id = 15;
    } else if (name == "MET") {
        part_temp_id = 12;
    } else if (name == "Muon") {
        part_temp_id = 13;
    } else if (name == "Ele") {
        part_temp_id = 11;
    }
    double temp_delta_r = 10;
    pxl::Particle* gen_match = 0;
    for (std::vector< pxl::Particle* >::const_iterator part_it = S3ListGen->begin(); part_it != S3ListGen->end(); ++part_it) {
        pxl::Particle *part_i = *part_it;
        int part_temp_truth_id = 0;
        if (b_8TeV) {
            part_temp_truth_id = TMath::Abs(part_i->getUserRecord("id").asInt32());
        } else if (b_13TeV) {
            part_temp_truth_id = TMath::Abs(part_i->getPdgNumber());
        }
        if (part_temp_id != part_temp_truth_id) continue;
        double test_delta_r = sqrt(pow(part_temp_eta - part_i->getEta(), 2) + pow(part_temp_phi - part_i->getPhi(), 2));
        if (test_delta_r < temp_delta_r) {
            temp_delta_r = test_delta_r;
            gen_match = part_i;
        }
    }
    return gen_match;
}

double specialAna::DeltaPhi(double a, double b) {
    double temp = fabs(a-b);
    if (temp <= TMath::Pi()) {
        return temp;
    } else {
        return  2.*TMath::Pi() - temp;
    }
}

double specialAna::DeltaPhi(pxl::Particle* lepton, pxl::Particle* met) {
    double a = lepton->getPhi();
    double b = met->getPhi();
    double temp = fabs(a-b);
    if (temp <= TMath::Pi()) {
        return temp;
    } else {
        return  2.*TMath::Pi() - temp;
    }
}

double specialAna::MT(pxl::Particle* lepton, pxl::Particle* met) {
    double mm = 2 * lepton->getPt() * met->getPt() *  1. - cos(lepton->getPhi() - met->getPhi());
    return sqrt(mm);
}

double specialAna::getPtHat() {
    double pthat = 0;
    pxl::Particle* w = 0;
    pxl::Particle* lepton = 0;

    if (b_13TeV) {
        for (uint i = 0; i < S3ListGen->size(); i++) {
            if (TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 24) {
                w = S3ListGen->at(i);
            }
            // take the neutrio to avoid showering and so on!!
            if ((TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 12 ||
                 TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 14 ||
                 TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 16) &&
                 lepton == 0) {
                lepton = S3ListGen->at(i);
            }
            if (w != 0 && lepton != 0) {
                break;
            }
        }
    } else if (b_8TeV) {
        for (uint i = 0; i < S3ListGen->size(); i++) {
            if (TMath::Abs(S3ListGen->at(i)->getUserRecord("id").asInt32()) == 24) {
                w = S3ListGen->at(i);
            }
            // take the neutrio to avoid showering and so on!!
            if ((TMath::Abs(S3ListGen->at(i)->getUserRecord("id").asInt32()) == 12 ||
                 TMath::Abs(S3ListGen->at(i)->getUserRecord("id").asInt32()) == 14 ||
                 TMath::Abs(S3ListGen->at(i)->getUserRecord("id").asInt32()) == 16) &&
                 lepton == 0) {
                lepton = S3ListGen->at(i);
            }
            if (w != 0 && lepton != 0) {
                break;
            }
        }
    }

    if (w != 0 && lepton != 0) {
        // boost in the w restframe
        lepton->boost(-(w->getBoostVector()));
        pthat = lepton->getPt();
    } else {
        pthat = -1;
    }
    return pthat;
}

double specialAna::getHT() {
    double ht_val = 0;
    for (uint i = 0; i < JetList->size(); i++) {
        ht_val += JetList->at(i)->getPt();
    }
    for (uint i = 0; i < BJetList->size(); i++) {
        ht_val += BJetList->at(i)->getPt();
    }
    return ht_val;
}

void specialAna::channel_writer(TFile* file, const char* channel) {
    file1->cd();
    file1->mkdir(channel);
    file1->cd(TString::Format("%s/", channel));
    HistClass::WriteAll(TString::Format("_%s_", channel), TString::Format("%s:_Cutflow", channel), TString::Format("sys:N-1"));
    file1->mkdir(TString::Format("%s/sys", channel));
    file1->cd(TString::Format("%s/sys/", channel));
    HistClass::WriteAll(TString::Format("_%s_", channel), TString::Format("%s:_Cutflow:sys", channel));
    file1->cd();
    for (int i = 0; i < channel_stages[channel]; i++) {
        char n_satge = static_cast<char>((static_cast<int>('0'))+i);
        file1->mkdir(TString::Format("%s/Stage_%c", channel, n_satge));
        file1->cd(TString::Format("%s/Stage_%c/", channel, n_satge));
        HistClass::WriteAll(TString::Format("_%s_", channel), TString::Format("%s:_%c_", channel, n_satge), TString::Format("sys:N-1"));
        HistClass::WriteAll2(TString::Format("_%s_", channel), TString::Format("%s:_%c_", channel, n_satge), TString::Format("sys:N-1"));
        file1->cd();
        file1->mkdir(TString::Format("%s/Stage_%c/sys", channel, n_satge));
        file1->cd(TString::Format("%s/Stage_%c/sys/", channel, n_satge));
        HistClass::WriteAll(TString::Format("_%s_", channel), TString::Format("_%c_:sys", n_satge), TString::Format("N-1"));
        HistClass::WriteAll2(TString::Format("_%s_", channel), TString::Format("_%c_:sys", n_satge), TString::Format("N-1"));
    }
    file1->cd();
    file1->mkdir(TString::Format("%s/N-1", channel));
    file1->cd(TString::Format("%s/N-1/", channel));
    HistClass::WriteAll(TString::Format("_%s_", channel), TString::Format("N-1"), TString::Format("sys"));
    HistClass::WriteAll2(TString::Format("_%s_", channel), TString::Format("N-1"), TString::Format("sys"));
    file1->cd();
    file1->mkdir(TString::Format("%s/N-1/sys", channel));
    file1->cd(TString::Format("%s/N-1/sys/", channel));
    HistClass::WriteAll(TString::Format("_%s_", channel), TString::Format("N-1:sys"));
    HistClass::WriteAll2(TString::Format("_%s_", channel), TString::Format("N-1:sys"));
    file1->cd();
}

void specialAna::endJob(const Serializable*) {
    std::cout << "Triggers that fired in this sample:" << std::endl;
    for (auto itr = triggers.begin(); itr != triggers.end(); ++itr) {
        std::cout << *itr << std::endl;
    }

    file1->cd();
    HistClass::WriteAll("counters");
    if (not runOnData) {
        file1->mkdir("MC");
        file1->cd("MC/");
        HistClass::WriteAll("_Gen");
    }
    if (doTriggerStudies) {
        file1->cd();
        file1->mkdir("Effs");
        file1->cd("Effs/");
        HistClass::WriteAllEff();
    }
    file1->cd();
    file1->mkdir("Ctr");
    file1->cd("Ctr/");
    HistClass::WriteAll("_Ctr_");
    file1->cd();
    file1->mkdir("Taus");
    file1->cd("Taus/");
    HistClass::WriteAll("_Tau_", "_Tau_", "sys:N-1:emu:etau:mutau:etaue:etaumu:mutaue:mutaumu");
    file1->cd();
    file1->mkdir("Muons");
    file1->cd("Muons/");
    HistClass::WriteAll("_Muon_", "_Muon_", "sys:N-1:emu:etau:mutau:etaue:etaumu:mutaue:mutaumu");
    file1->cd();
    file1->mkdir("METs");
    file1->cd("METs/");
    HistClass::WriteAll("_MET_", "_MET_", "sys:N-1:emu:etau:mutau:etaue:etaumu:mutaue:mutaumu");
    file1->cd();
    file1->mkdir("Eles");
    file1->cd("Eles/");
    HistClass::WriteAll("_Ele_", "_Ele_", "sys:N-1:emu:etau:mutau:etaue:etaumu:mutaue:mutaumu");
    file1->cd();
    HistClass::WriteAllTrees("data_events");
    channel_writer(file1, "emu");
    channel_writer(file1, "etau");
    channel_writer(file1, "mutau");
    channel_writer(file1, "etaue");
    channel_writer(file1, "etaumu");
    channel_writer(file1, "mutaue");
    channel_writer(file1, "mutaumu");

    file1->Close();

    delete file1;
}

void specialAna::initEvent(const pxl::Event* event) {
    HistClass::Fill("h_counters", 1, 1);  // increment number of events
    events_++;

    keep_data_event = false;
    mkeep_resonance_mass["emu"] = 0;
    mkeep_resonance_mass["etau"] = 0;
    mkeep_resonance_mass["mutau"] = 0;
    mkeep_resonance_mass["etaue"] = 0;
    mkeep_resonance_mass["etaumu"] = 0;
    mkeep_resonance_mass["mutaue"] = 0;
    mkeep_resonance_mass["mutaumu"] = 0;

    m_RecEvtView = event->getObjectOwner().findObject< pxl::EventView >("Rec");
    m_GenEvtView = event->getObjectOwner().findObject< pxl::EventView >("Gen");
    if (event->getObjectOwner().findObject< pxl::EventView >("Trig")) {
        m_TrigEvtView = event->getObjectOwner().findObject< pxl::EventView >("Trig");
    } else {
        m_TrigEvtView = event->getObjectOwner().findObject< pxl::EventView >("Rec");
    }

    temp_run = event->getUserRecord("Run");
    temp_ls = event->getUserRecord("LumiSection");
    temp_event = event->getUserRecord("EventNum");

    mkeep_resonance_mass["run"] = temp_run;
    mkeep_resonance_mass["ls"] = temp_ls;
    mkeep_resonance_mass["event"] = temp_event;

    numMuon  = m_RecEvtView->getUserRecord("NumMuon");
    numEle   = m_RecEvtView->getUserRecord("NumEle");
    numGamma = m_RecEvtView->getUserRecord("NumGamma");
    numTau   = m_RecEvtView->getUserRecord("Num" + m_TauType);
    numMET   = m_RecEvtView->getUserRecord("Num" + m_METType);
    numJet   = m_RecEvtView->getUserRecord("Num" + m_JetAlgo);

    EleList   = new std::vector< pxl::Particle* >;
    MuonList  = new std::vector< pxl::Particle* >;
    GammaList = new std::vector< pxl::Particle* >;
    METList   = new std::vector< pxl::Particle* >;
    JetList   = new std::vector< pxl::Particle* >;
    BJetList  = new std::vector< pxl::Particle* >;
    TauList   = new std::vector< pxl::Particle* >;

    // get all particles
    std::vector< pxl::Particle* > AllParticles;
    m_RecEvtView->getObjectsOfType< pxl::Particle >(AllParticles);
    pxl::sortParticles(AllParticles);
    numBJet = 0;
    // push them into the corresponding vectors
    for (std::vector< pxl::Particle* >::const_iterator part_it = AllParticles.begin(); part_it != AllParticles.end(); ++part_it) {
        pxl::Particle *part = *part_it;
        std::string Name = part->getName();
        part->setP4(part->getPx() * 1.05, part->getPy() * 1.05, part->getPz(), part->getE() * 0.95);
        // Only fill the collection if we want to use the particle!
        if (Name == "Muon") {
            MuonList->push_back(part);
        } else if (Name == "Ele") {
            EleList->push_back(part);
        } else if (Name == "Gamma") {
            GammaList->push_back(part);
        } else if (Name == m_TauType) {
            TauList->push_back(part);
        } else if (Name == m_METType) {
            METList->push_back(part);
        } else if (Name == m_JetAlgo) {
            if (part->getUserRecord(m_BJets_algo).toDouble() > 0.679) {
                BJetList->push_back(part);
                numBJet++;
            } else {
                JetList->push_back(part);
            }
        }
    }

    if (METList->size() > 0) {
        sel_met = METList->at(0);
    } else {
        sel_met = 0;
    }
    sel_lepton_prompt = 0;
    sel_lepton_nprompt = 0;
    sel_lepton_nprompt_corr = 0;

    resonance_mass["emu"] = 0;
    resonance_mass_gen["emu"] = 0;
    resonance_mass["etau"] = 0;
    resonance_mass_gen["etau"] = 0;
    resonance_mass["mutau"] = 0;
    resonance_mass_gen["mutau"] = 0;
    resonance_mass["etaue"] = 0;
    resonance_mass_gen["etaue"] = 0;
    resonance_mass["etaumu"] = 0;
    resonance_mass_gen["etaumu"] = 0;
    resonance_mass["mutaue"] = 0;
    resonance_mass_gen["mutaue"] = 0;
    resonance_mass["mutaumu"] = 0;
    resonance_mass_gen["mutaumu"] = 0;

    EleListGen     = new std::vector< pxl::Particle* >;
    MuonListGen    = new std::vector< pxl::Particle* >;
    GammaListGen   = new std::vector< pxl::Particle* >;
    METListGen     = new std::vector< pxl::Particle* >;
    JetListGen     = new std::vector< pxl::Particle* >;
    TauListGen     = new std::vector< pxl::Particle* >;
    S3ListGen      = new std::vector< pxl::Particle* >;

    weight = 1.;

    event_weight = 1;
    pileup_weight = 1;

    if (not runOnData) {
        event_weight = m_GenEvtView->getUserRecord("Weight");
        event_weight = 1;
        // double varKfactor_weight = m_GenEvtView->getUserRecord_def( "kfacWeight",1. );
        pileup_weight = m_GenEvtView->getUserRecord_def("PUWeight", 1.);

        if (b_13TeV) {
            weight = event_weight;
        } else if (b_8TeV) {
            weight = event_weight * pileup_weight;
        } else {
            std::stringstream error;
            error << "The data period " << m_dataPeriod << " is not supported by this analysis!\n";
            throw Tools::config_error(error.str());
        }

        // get all particles
        std::vector< pxl::Particle* > AllParticlesGen;
        m_GenEvtView->getObjectsOfType< pxl::Particle >(AllParticlesGen);
        pxl::sortParticles(AllParticlesGen);
        // push them into the corresponding vectors
        std::string genCollection = "gen";
        if (b_8TeV) {
            genCollection = "S3";
        }
        for (std::vector< pxl::Particle* >::const_iterator part_it = AllParticlesGen.begin(); part_it != AllParticlesGen.end(); ++part_it) {
            pxl::Particle *part = *part_it;
            std::string Name = part->getName();
            // Only fill the collection if we want to use the particle!
            if (     Name == "Muon"    ) MuonListGen->push_back( part );
            else if (Name == "Ele"     ) EleListGen->push_back( part );
            else if (Name == "Gamma"   ) GammaListGen->push_back( part );
            else if (Name == "Tau"     ) TauListGen->push_back( part );
            else if (Name == (m_METType+"_gen") ) METListGen->push_back( part );
            else if (Name == m_JetAlgo ) JetListGen->push_back( part );
            else if (Name == genCollection) S3ListGen->push_back( part );
        }
    }
}

void specialAna::endEvent(const pxl::Event* event) {
    delete EleList;
    delete MuonList;
    delete GammaList;
    delete TauList;
    delete METList;
    delete JetList;

    EleList = 0;
    MuonList = 0;
    GammaList = 0;
    METList = 0;
    JetList = 0;
    TauList = 0;

    if (not runOnData) {
        delete EleListGen;
        delete MuonListGen;
        delete GammaListGen;
        delete METListGen;
        delete JetListGen;
        delete TauListGen;

        EleListGen = 0;
        MuonListGen = 0;
        GammaListGen = 0;
        METListGen = 0;
        JetListGen = 0;
        TauListGen = 0;
    }

    if (runOnData and keep_data_event) {
        HistClass::FillTree("data_events");
    }
}
