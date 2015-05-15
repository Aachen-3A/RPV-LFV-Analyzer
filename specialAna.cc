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

    m_trigger_string(Tools::splitString< std::string >(cfg.GetItem< std::string >("RPV.trigger_list"))),
    d_mydiscmu({"isPFMuon", "isGlobalMuon", "isTrackerMuon", "isStandAloneMuon", "isTightMuon", "isHighPtMuon"}),
    m_dataPeriod(cfg.GetItem< std::string >("General.DataPeriod")),
    m_channel(cfg.GetItem< std::string >("RPV.channel")),
    config_(cfg)
{
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

    Create_RECO_effs();
    Create_ID_effs();

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

    HistClass::CreateTree(&mkeep_resonance_mass, "data_events");

    for (unsigned int i = 0; i < 4; i++) {
        // str(boost::format("N_{%s}")%particleLatex[i] )
        HistClass::CreateHisto("num", particles[i].c_str(), 40, 0, 39,                          TString::Format("N_{%s}", particleSymbols[i].c_str()));
        HistClass::CreateHisto(3, "pt", particles[i].c_str(), 5000, 0, 5000,                    TString::Format("p_{T}^{%s} (GeV)", particleSymbols[i].c_str()));
        if (particles[i] == "Muon") {
            HistClass::CreateHisto("pt_resolution_0_500", particles[i].c_str(), 1000, -10, 10,      TString::Format("(1/p_{T}^{reco} - 1/p_{T}^{gen})/1/p_{T}^{gen}(%s)", particleSymbols[i].c_str()));
            HistClass::CreateHisto("pt_resolution_500_1000", particles[i].c_str(), 1000, -10, 10,   TString::Format("(1/p_{T}^{reco} - 1/p_{T}^{gen})/1/p_{T}^{gen}(%s)", particleSymbols[i].c_str()));
            HistClass::CreateHisto("pt_resolution_1000_1500", particles[i].c_str(), 1000, -10, 10,  TString::Format("(1/p_{T}^{reco} - 1/p_{T}^{gen})/1/p_{T}^{gen}(%s)", particleSymbols[i].c_str()));
            HistClass::CreateHisto("pt_resolution_1500_2000", particles[i].c_str(), 1000, -10, 10,  TString::Format("(1/p_{T}^{reco} - 1/p_{T}^{gen})/1/p_{T}^{gen}(%s)", particleSymbols[i].c_str()));
            HistClass::CreateHisto("pt_resolution_2000", particles[i].c_str(), 1000, -10, 10,       TString::Format("(1/p_{T}^{reco} - 1/p_{T}^{gen})/1/p_{T}^{gen}(%s)", particleSymbols[i].c_str()));
        } else {
            HistClass::CreateHisto("pt_resolution_0_500", particles[i].c_str(), 1000, -10, 10,      TString::Format("(p_{T}^{reco} - p_{T}^{gen})/p_{T}^{gen}(%s)", particleSymbols[i].c_str()));
            HistClass::CreateHisto("pt_resolution_500_1000", particles[i].c_str(), 1000, -10, 10,   TString::Format("(p_{T}^{reco} - p_{T}^{gen})/p_{T}^{gen}(%s)", particleSymbols[i].c_str()));
            HistClass::CreateHisto("pt_resolution_1000_1500", particles[i].c_str(), 1000, -10, 10,  TString::Format("(p_{T}^{reco} - p_{T}^{gen})/p_{T}^{gen}(%s)", particleSymbols[i].c_str()));
            HistClass::CreateHisto("pt_resolution_1500_2000", particles[i].c_str(), 1000, -10, 10,  TString::Format("(p_{T}^{reco} - p_{T}^{gen})/p_{T}^{gen}(%s)", particleSymbols[i].c_str()));
            HistClass::CreateHisto("pt_resolution_2000", particles[i].c_str(), 1000, -10, 10,       TString::Format("(p_{T}^{reco} - p_{T}^{gen})/p_{T}^{gen}(%s)", particleSymbols[i].c_str()));
        }
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
    channel_stages["emu"] = 5;
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

    for (uint i = 0; i < MuonList->size(); i++) {
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

    Fill_RECO_effs();
    Fill_ID_effs();

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
        sel_met["emu"] = METList->at(0);
        sel_met["etau"] = METList->at(0);
        sel_met["mutau"] = METList->at(0);
        sel_met["etaue"] = METList->at(0);
        sel_met["etaumu"] = METList->at(0);
        sel_met["mutaue"] = METList->at(0);
        sel_met["mutaumu"] = METList->at(0);
    } else {
        sel_met["emu"] = 0;
        sel_met["etau"] = 0;
        sel_met["mutau"] = 0;
        sel_met["etaue"] = 0;
        sel_met["etaumu"] = 0;
        sel_met["mutaue"] = 0;
        sel_met["mutaumu"] = 0;
    }

    sel_lepton_prompt["emu"] = 0;
    sel_lepton_prompt["etau"] = 0;
    sel_lepton_prompt["mutau"] = 0;
    sel_lepton_prompt["etaue"] = 0;
    sel_lepton_prompt["etaumu"] = 0;
    sel_lepton_prompt["mutaue"] = 0;
    sel_lepton_prompt["mutaumu"] = 0;

    sel_lepton_nprompt["emu"] = 0;
    sel_lepton_nprompt["etau"] = 0;
    sel_lepton_nprompt["mutau"] = 0;
    sel_lepton_nprompt["etaue"] = 0;
    sel_lepton_nprompt["etaumu"] = 0;
    sel_lepton_nprompt["mutaue"] = 0;
    sel_lepton_nprompt["mutaumu"] = 0;

    sel_lepton_nprompt_corr["emu"] = 0;
    sel_lepton_nprompt_corr["etau"] = 0;
    sel_lepton_nprompt_corr["mutau"] = 0;
    sel_lepton_nprompt_corr["etaue"] = 0;
    sel_lepton_nprompt_corr["etaumu"] = 0;
    sel_lepton_nprompt_corr["mutaue"] = 0;
    sel_lepton_nprompt_corr["mutaumu"] = 0;

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

    // if (sel_lepton_prompt != 0) {
        // delete sel_lepton_prompt;
        // sel_lepton_prompt = 0;
    // }
    // if (sel_lepton_nprompt != 0) {
        // delete sel_lepton_nprompt;
        // sel_lepton_nprompt = 0;
    // }
    // if (sel_lepton_nprompt_corr != 0) {
        // delete sel_lepton_nprompt_corr;
        // sel_lepton_nprompt_corr = 0;
    // }
    // if (sel_lepton_nprompt_corr != 0) {
        // delete sel_lepton_nprompt_corr;
        // sel_lepton_nprompt_corr = 0;
    // }
// 
    // if (sel_part1_gen != 0) {
        // delete sel_part1_gen;
        // sel_part1_gen = 0;
    // }
    // if (sel_part2_gen != 0) {
        // delete sel_part2_gen;
        // sel_part2_gen = 0;
    // }

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

    if (resonance_mass_gen["emu"] != 0 and Check_Gen_Par_Acc(sel_part1_gen["emu"]) and Check_Gen_Par_Acc(sel_part2_gen["emu"])) {
        HistClass::Fill("emu_RECO_vs_Mass_Pass", resonance_mass_gen["emu"], 1);
        HistClass::Fill("emu_RECO_vs_Nvtx_Pass", m_RecEvtView->getUserRecord("NumVertices"), 1);
    }

    if (resonance_mass_gen["etau"] != 0 and Check_Gen_Par_Acc(sel_part1_gen["etau"]) and Check_Gen_Par_Acc(sel_part2_gen["etau"])) {
        HistClass::Fill("etau_RECO_vs_Mass_Pass", resonance_mass_gen["etau"], 1);
        HistClass::Fill("etau_RECO_vs_Nvtx_Pass", m_RecEvtView->getUserRecord("NumVertices"), 1);
    }

    if (resonance_mass_gen["mutau"] != 0 and Check_Gen_Par_Acc(sel_part1_gen["mutau"]) and Check_Gen_Par_Acc(sel_part2_gen["mutau"])) {
        HistClass::Fill("mutau_RECO_vs_Mass_Pass", resonance_mass_gen["mutau"], 1);
        HistClass::Fill("mutau_RECO_vs_Nvtx_Pass", m_RecEvtView->getUserRecord("NumVertices"), 1);
    }

    if (resonance_mass_gen["etaue"] != 0 and Check_Gen_Par_Acc(sel_part1_gen["etaue"]) and Check_Gen_Par_Acc(sel_part2_gen["etaue"])) {
        HistClass::Fill("etaue_RECO_vs_Mass_Pass", resonance_mass_gen["etaue"], 1);
        HistClass::Fill("etaue_RECO_vs_Nvtx_Pass", m_RecEvtView->getUserRecord("NumVertices"), 1);
    }

    if (resonance_mass_gen["etaumu"] != 0 and Check_Gen_Par_Acc(sel_part1_gen["etaumu"]) and Check_Gen_Par_Acc(sel_part2_gen["etaumu"])) {
        HistClass::Fill("etaumu_RECO_vs_Mass_Pass", resonance_mass_gen["etaumu"], 1);
        HistClass::Fill("etaumu_RECO_vs_Nvtx_Pass", m_RecEvtView->getUserRecord("NumVertices"), 1);
    }

    if (resonance_mass_gen["mutaue"] != 0 and Check_Gen_Par_Acc(sel_part1_gen["mutaue"]) and Check_Gen_Par_Acc(sel_part2_gen["mutaue"])) {
        HistClass::Fill("mutaue_RECO_vs_Mass_Pass", resonance_mass_gen["mutaue"], 1);
        HistClass::Fill("mutaue_RECO_vs_Nvtx_Pass", m_RecEvtView->getUserRecord("NumVertices"), 1);
    }
}

void specialAna::Init_emu_cuts() {
    emu_cut_cfgs["kinematics"] = Cuts("kinematics",             500, 0, 500);
    emu_cut_cfgs["OppSign_charge"] = Cuts("OppSign_charge",     4, -2, 2);
    emu_cut_cfgs["BJet_veto"] = Cuts("BJet_veto",               10, 0, 10);
    emu_cut_cfgs["DeltaPhi_emu"] = Cuts("DeltaPhi_emu",         100, 0, 3.2);
    emu_cut_cfgs["DeltaR_emu"] = Cuts("DeltaR_emu",             100, 0, 10);
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
        if (OppSign_charge(&emu_cut_cfgs["OppSign_charge"], "emu")) {
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
        /// Make the DeltaR Cut
        if (Make_DeltaR_emu(&emu_cut_cfgs["DeltaR_emu"])) {
            if (b_emu_success) {
                Fill_Resonance_histograms(4, "emu", "ele", "muo", endung);
                b_emu_success = true;
            }
            emu_cut_cfgs["DeltaR_emu"].SetPassed(true);
        } else {
            b_emu_success = false;
            emu_cut_cfgs["DeltaR_emu"].SetPassed(false);
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
        if (b_etau_success) {
            std::cout << "found etau resonance" << std::endl;
        }
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
        if (OppSign_charge(&mutau_cut_cfgs["OppSign_charge"], "mutau")) {
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
        } else {
            b_etaue_success = false;
            etaue_cut_cfgs["kinematics"].SetPassed(false);
            etaue_cut_cfgs["kinematics"].SetVars(resonance_mass["etaue"]);
        }
        Fill_N1_histos("etaue", etaue_cut_cfgs, endung);
        if (b_etaue_success) {
            std::cout << "found etaue resonance" << std::endl;
        }
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
        if (b_etaumu_success) {
            std::cout << "found etaumu resonance" << std::endl;
        }
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
        if (b_mutaumu_success) {
            std::cout << "found mutaumu resonance" << std::endl;
        }
    }
}

void specialAna::Create_ID_effs() {
    Create_ID_object_effs("Muon");
    Create_ID_object_effs("Ele");
    Create_ID_object_effs("Tau");
}

void specialAna::Create_ID_object_effs(std::string object) {
    HistClass::CreateEff(TString::Format("%s_ID_vs_pT", object.c_str()),         300, 0, 3000,
                         TString::Format("p_{T}^{%s(reco)} (GeV)", object.c_str()));
    HistClass::CreateEff(TString::Format("%s_ID_vs_Nvtx", object.c_str()),       70, 0, 70,
                         "n_{vtx}");
    HistClass::CreateEff(TString::Format("%s_ID_vs_eta_vs_phi", object.c_str()), 150, -3, 3, 150, -3.2, 3.2,
                         TString::Format("#eta(%s(reco))", object.c_str()), TString::Format("#phi(%s(reco)) (rad)", object.c_str()));

    HistClass::CreateEff(TString::Format("%s_ID_vs_pT_in_Acc", object.c_str()),         300, 0, 3000,
                         TString::Format("p_{T}^{%s(reco)} (GeV)", object.c_str()));
    HistClass::CreateEff(TString::Format("%s_ID_vs_Nvtx_in_Acc", object.c_str()),       70, 0, 70,
                         "n_{vtx}");
    HistClass::CreateEff(TString::Format("%s_ID_vs_eta_vs_phi_in_Acc", object.c_str()), 150, -3, 3, 150, -3.2, 3.2,
                         TString::Format("#eta(%s(reco))", object.c_str()), TString::Format("#phi(%s(reco)) (rad)", object.c_str()));

    HistClass::CreateEff(TString::Format("%s_ID_vs_pT_gen", object.c_str()),         300, 0, 3000,
                         TString::Format("p_{T}^{%s(gen)} (GeV)", object.c_str()));
    HistClass::CreateEff(TString::Format("%s_ID_vs_eta_vs_phi_gen", object.c_str()), 150, -3, 3, 150, -3.2, 3.2,
                         TString::Format("#eta(%s(gen))", object.c_str()), TString::Format("#phi(%s(gen)) (rad)", object.c_str()));

    HistClass::CreateEff(TString::Format("%s_ID_vs_pT_in_Acc_gen", object.c_str()),         300, 0, 3000,
                         TString::Format("p_{T}^{%s(gen)} (GeV)", object.c_str()));
    HistClass::CreateEff(TString::Format("%s_ID_vs_eta_vs_phi_in_Acc_gen", object.c_str()), 150, -3, 3, 150, -3.2, 3.2,
                         TString::Format("#eta(%s(gen))", object.c_str()), TString::Format("#phi(%s(gen)) (rad)", object.c_str()));
}

void specialAna::Fill_ID_effs() {
    Fill_ID_object_effs("Muon", 13, *MuonList);
    Fill_ID_object_effs("Ele", 11, *EleList);
    Fill_ID_object_effs("Tau", 15, *TauList);
}

void specialAna::Fill_ID_object_effs(std::string object, int id, std::vector< pxl::Particle* > part_list) {
    for (std::vector< pxl::Particle* >::const_iterator part_it = S3ListGen->begin(); part_it != S3ListGen->end(); ++part_it) {
        pxl::Particle *part_i = *part_it;
        pxl::Particle* matched_reco_particle = 0;
        if (TMath::Abs(part_i->getPdgNumber()) != id) continue;
        double delta_r_max = 0.25;
        for (std::vector< pxl::Particle* >::const_iterator part_jt = part_list.begin(); part_jt != part_list.end(); ++part_jt) {
            pxl::Particle *part_j = *part_jt;
            if (DeltaPhi(part_j, part_i) < delta_r_max) {
                delta_r_max = DeltaPhi(part_j, part_i);
                matched_reco_particle = (pxl::Particle*) part_j->clone();
            }
        }
        if (matched_reco_particle != 0) {
            if (Check_Par_ID(matched_reco_particle, false, false)) {
                HistClass::FillEff(TString::Format("%s_ID_vs_pT", object.c_str()), matched_reco_particle->getPt(), true);
                HistClass::FillEff(TString::Format("%s_ID_vs_Nvtx", object.c_str()), m_RecEvtView->getUserRecord("NumVertices"), true);
                HistClass::FillEff(TString::Format("%s_ID_vs_eta_vs_phi", object.c_str()), matched_reco_particle->getEta(), matched_reco_particle->getPhi(), true);

                HistClass::FillEff(TString::Format("%s_ID_vs_pT_gen", object.c_str()), part_i->getPt(), true);
                HistClass::FillEff(TString::Format("%s_ID_vs_eta_vs_phi_gen", object.c_str()), part_i->getEta(), part_i->getPhi(), true);
            } else {
                HistClass::FillEff(TString::Format("%s_ID_vs_pT", object.c_str()), matched_reco_particle->getPt(), false);
                HistClass::FillEff(TString::Format("%s_ID_vs_Nvtx", object.c_str()), m_RecEvtView->getUserRecord("NumVertices"), false);
                HistClass::FillEff(TString::Format("%s_ID_vs_eta_vs_phi", object.c_str()), matched_reco_particle->getEta(), matched_reco_particle->getPhi(), false);

                HistClass::FillEff(TString::Format("%s_ID_vs_pT_gen", object.c_str()), part_i->getPt(), false);
                HistClass::FillEff(TString::Format("%s_ID_vs_eta_vs_phi_gen", object.c_str()), part_i->getEta(), part_i->getPhi(), false);
            }
            if (Check_Par_Acc(matched_reco_particle)) {
                if (Check_Par_ID(matched_reco_particle, false, false)) {
                    HistClass::FillEff(TString::Format("%s_ID_vs_pT_in_Acc", object.c_str()), matched_reco_particle->getPt(), true);
                    HistClass::FillEff(TString::Format("%s_ID_vs_Nvtx_in_Acc", object.c_str()), m_RecEvtView->getUserRecord("NumVertices"), true);
                    HistClass::FillEff(TString::Format("%s_ID_vs_eta_vs_phi_in_Acc", object.c_str()), matched_reco_particle->getEta(), matched_reco_particle->getPhi(), true);

                    HistClass::FillEff(TString::Format("%s_ID_vs_pT_in_Acc_gen", object.c_str()), part_i->getPt(), true);
                    HistClass::FillEff(TString::Format("%s_ID_vs_eta_vs_phi_in_Acc_gen", object.c_str()), part_i->getEta(), part_i->getPhi(), true);
                } else {
                    HistClass::FillEff(TString::Format("%s_ID_vs_pT_in_Acc", object.c_str()), matched_reco_particle->getPt(), false);
                    HistClass::FillEff(TString::Format("%s_ID_vs_Nvtx_in_Acc", object.c_str()), m_RecEvtView->getUserRecord("NumVertices"), false);
                    HistClass::FillEff(TString::Format("%s_ID_vs_eta_vs_phi_in_Acc", object.c_str()), matched_reco_particle->getEta(), matched_reco_particle->getPhi(), false);

                    HistClass::FillEff(TString::Format("%s_ID_vs_pT_in_Acc_gen", object.c_str()), part_i->getPt(), false);
                    HistClass::FillEff(TString::Format("%s_ID_vs_eta_vs_phi_in_Acc_gen", object.c_str()), part_i->getEta(), part_i->getPhi(), false);
                }
            }
        }
        delete matched_reco_particle;
    }
}

void specialAna::Create_RECO_effs() {
    Create_RECO_object_effs("Muon");
    Create_RECO_object_effs("Ele");
    Create_RECO_object_effs("Tau");
    TString x_bins_names[13] = {"1Pi0Pi0",
                                "1Pi1Pi0",
                                "1Pi2Pi0",
                                "1Pi>2Pi0",
                                "3Pi0Pi0",
                                "3Pi1Pi0",
                                "3Pi2Pi0",
                                "3Pi>2Pi0",
                                ">3Pi0Pi0",
                                ">3Pi1Pi0",
                                ">3Pi2Pi0",
                                ">3Pi>2Pi0",
                                "else"};
    TString y_bins_names[16] = {"tauDecay1ChargedPion0PiZero",
                                "tauDecay1ChargedPion1PiZero",
                                "tauDecay1ChargedPion2PiZero",
                                "tauDecay1ChargedPion3PiZero",
                                "tauDecay1ChargedPion4PiZero",
                                "tauDecay2ChargedPion0PiZero",
                                "tauDecay2ChargedPion1PiZero",
                                "tauDecay2ChargedPion2PiZero",
                                "tauDecay2ChargedPion3PiZero",
                                "tauDecay2ChargedPion4PiZero",
                                "tauDecay3ChargedPion0PiZero",
                                "tauDecay3ChargedPion1PiZero",
                                "tauDecay3ChargedPion2PiZero",
                                "tauDecay3ChargedPion3PiZero",
                                "tauDecay3ChargedPion4PiZero",
                                "tauDecayOther"};

    HistClass::CreateHisto("Tau_RECO_vs_gendm_vs_recodm_0_500", 13, 0, 13, 16, 0, 16, "DM(gen)", "DM(reco)");
    HistClass::NameBins("Tau_RECO_vs_gendm_vs_recodm_0_500", 13, x_bins_names, 16, y_bins_names);
    HistClass::CreateHisto("Tau_RECO_vs_gendm_vs_recodm_500_1000", 13, 0, 13, 16, 0, 16, "DM(gen)", "DM(reco)");
    HistClass::NameBins("Tau_RECO_vs_gendm_vs_recodm_500_1000", 13, x_bins_names, 16, y_bins_names);
    HistClass::CreateHisto("Tau_RECO_vs_gendm_vs_recodm_1000_1500", 13, 0, 13, 16, 0, 16, "DM(gen)", "DM(reco)");
    HistClass::NameBins("Tau_RECO_vs_gendm_vs_recodm_1000_1500", 13, x_bins_names, 16, y_bins_names);
    HistClass::CreateHisto("Tau_RECO_vs_gendm_vs_recodm_1500_2000", 13, 0, 13, 16, 0, 16, "DM(gen)", "DM(reco)");
    HistClass::NameBins("Tau_RECO_vs_gendm_vs_recodm_1500_2000", 13, x_bins_names, 16, y_bins_names);
    HistClass::CreateHisto("Tau_RECO_vs_gendm_vs_recodm_2000", 13, 0, 13, 16, 0, 16, "DM(gen)", "DM(reco)");
    HistClass::NameBins("Tau_RECO_vs_gendm_vs_recodm_2000", 13, x_bins_names, 16, y_bins_names);
    Create_RECO_object_effs("MET");
}

void specialAna::Create_RECO_object_effs(std::string object) {
    HistClass::CreateHisto(TString::Format("%s_RECO_DeltaR", object.c_str()), 320, 0, 3.2, "#DeltaR(RECO, gen)");
    HistClass::CreateHisto(TString::Format("%s_RECO_DeltaEta", object.c_str()), 320, 0, 3.2, "#Delta#eta(RECO, gen)");
    HistClass::CreateHisto(TString::Format("%s_RECO_DeltaPhi", object.c_str()), 320, 0, 3.2, "#Delta#phi(RECO, gen)");

    HistClass::CreateEff(TString::Format("%s_RECO_vs_pT", object.c_str()),         300, 0, 3000,
                         TString::Format("p_{T}^{%s(gen)} (GeV)", object.c_str()));
    HistClass::CreateEff(TString::Format("%s_RECO_vs_Nvtx", object.c_str()),       70, 0, 70,
                         "n_{vtx}");
    HistClass::CreateEff(TString::Format("%s_RECO_vs_eta_vs_phi", object.c_str()), 150, -3, 3, 150, -3.2, 3.2,
                         TString::Format("#eta(%s(gen))", object.c_str()), TString::Format("#phi(%s(gen)) (rad)", object.c_str()));
    if (object != "MET") {
        HistClass::CreateEff(TString::Format("%s_RECO_vs_pT_in_Acc", object.c_str()),         300, 0, 3000,
                             TString::Format("p_{T}^{%s(gen)} (GeV)", object.c_str()));
        HistClass::CreateEff(TString::Format("%s_RECO_vs_Nvtx_in_Acc", object.c_str()),       70, 0, 70,
                             "n_{vtx}");
        HistClass::CreateEff(TString::Format("%s_RECO_vs_eta_vs_phi_in_Acc", object.c_str()), 150, -3, 3, 150, -3.2, 3.2,
                             TString::Format("#eta(%s(gen))", object.c_str()), TString::Format("#phi(%s(gen)) (rad)", object.c_str()));
    }
}

void specialAna::Fill_RECO_effs() {
    Fill_RECO_object_effs("Muon", 13, *MuonList);
    Fill_RECO_object_effs("Ele", 11, *EleList);
    Fill_RECO_object_effs("Tau", 15, *TauList);
    Fill_RECO_object_effs("MET", 12, *METList);
}

void specialAna::Fill_RECO_object_effs(std::string object, int id, std::vector< pxl::Particle* > part_list) {
    if (object == "MET") {
        pxl::Particle* gen_met = 0;
        for (std::vector< pxl::Particle* >::const_iterator part_it = S3ListGen->begin(); part_it != S3ListGen->end(); ++part_it) {
            pxl::Particle *part_i = *part_it;
            if (TMath::Abs(part_i->getPdgNumber()) != 12 and TMath::Abs(part_i->getPdgNumber()) != 14 and TMath::Abs(part_i->getPdgNumber()) != 16 ) continue;
            if (gen_met == 0) {
                gen_met = (pxl::Particle*) part_i->clone();
            } else {
                gen_met->addP4(part_i);
            }
        }
        if (gen_met != 0) {
            if (part_list.size() > 0 and DeltaPhi(part_list[0], gen_met) < 0.25) {
                HistClass::FillEff(TString::Format("%s_RECO_vs_pT", object.c_str()), gen_met->getPt(), true);
                HistClass::FillEff(TString::Format("%s_RECO_vs_Nvtx", object.c_str()), m_RecEvtView->getUserRecord("NumVertices"), true);
                HistClass::FillEff(TString::Format("%s_RECO_vs_eta_vs_phi", object.c_str()), gen_met->getEta(), gen_met->getPhi(), true);
            } else {
                HistClass::FillEff(TString::Format("%s_RECO_vs_pT", object.c_str()), gen_met->getPt(), false);
                HistClass::FillEff(TString::Format("%s_RECO_vs_Nvtx", object.c_str()), m_RecEvtView->getUserRecord("NumVertices"), false);
                HistClass::FillEff(TString::Format("%s_RECO_vs_eta_vs_phi", object.c_str()), gen_met->getEta(), gen_met->getPhi(), false);
            }
        }
        delete gen_met;
    } else if (object == "Tau") {
        for (std::vector< pxl::Particle* >::const_iterator part_it = TauVisListGen->begin(); part_it != TauVisListGen->end(); ++part_it) {
            pxl::Particle *part_i = *part_it;
            pxl::Particle* matched_reco_particle = 0;
            if (part_i->getUserRecord("decay_mode_id") == 0 or part_i->getUserRecord("decay_mode_id") == 1) continue;
            double delta_r_max = 0.25;
            double delta_r_min = 3;
            double delta_eta_min = 3;
            double delta_phi_min = 3;
            for (std::vector< pxl::Particle* >::const_iterator part_jt = part_list.begin(); part_jt != part_list.end(); ++part_jt) {
                pxl::Particle *part_j = *part_jt;
                double temp_delta_r = DeltaPhi(part_j, part_i);
                if (temp_delta_r < delta_r_min) {
                    delta_r_min = temp_delta_r;
                    delta_phi_min = DeltaPhi(part_j, part_i);
                    delta_eta_min = TMath::Abs(part_j->getEta() - part_i->getEta());
                    if (temp_delta_r < delta_r_max) {
                        matched_reco_particle = (pxl::Particle*) part_j->clone();
                    }
                }
            }
            HistClass::Fill(TString::Format("%s_RECO_DeltaR", object.c_str()), delta_r_min, 1.);
            HistClass::Fill(TString::Format("%s_RECO_DeltaEta", object.c_str()), delta_eta_min, 1.);
            HistClass::Fill(TString::Format("%s_RECO_DeltaPhi", object.c_str()), delta_phi_min, 1.);
            if (matched_reco_particle != 0) {
                HistClass::FillEff(TString::Format("%s_RECO_vs_pT", object.c_str()), part_i->getPt(), true);
                HistClass::FillEff(TString::Format("%s_RECO_vs_Nvtx", object.c_str()), m_RecEvtView->getUserRecord("NumVertices"), true);
                HistClass::FillEff(TString::Format("%s_RECO_vs_eta_vs_phi", object.c_str()), part_i->getEta(), part_i->getPhi(), true);

                if (matched_reco_particle->getUserRecord("decayMode") == 17) {
                    if (part_i->getPt() < 500) {
                        HistClass::Fill("Tau_RECO_vs_gendm_vs_recodm_0_500",
                                         static_cast<double>(part_i->getUserRecord("decay_mode_id")) - 1.5,
                                         15.5,
                                         1.);
                    } else if (part_i->getPt() > 500 and part_i->getPt() < 1000) {
                        HistClass::Fill("Tau_RECO_vs_gendm_vs_recodm_500_1000",
                                         static_cast<double>(part_i->getUserRecord("decay_mode_id")) - 1.5,
                                         15.5,
                                         1.);
                    } else if (part_i->getPt() > 1000 and part_i->getPt() < 1500) {
                        HistClass::Fill("Tau_RECO_vs_gendm_vs_recodm_1000_1500",
                                         static_cast<double>(part_i->getUserRecord("decay_mode_id")) - 1.5,
                                         15.5,
                                         1.);
                    } else if (part_i->getPt() > 1500 and part_i->getPt() < 2000) {
                        HistClass::Fill("Tau_RECO_vs_gendm_vs_recodm_1500_2000",
                                         static_cast<double>(part_i->getUserRecord("decay_mode_id")) - 1.5,
                                         15.5,
                                         1.);
                    } else if (part_i->getPt() > 2000) {
                        HistClass::Fill("Tau_RECO_vs_gendm_vs_recodm_2000",
                                         static_cast<double>(part_i->getUserRecord("decay_mode_id")) - 1.5,
                                         15.5,
                                         1.);
                    }
                } else {
                    if (part_i->getPt() < 500) {
                        HistClass::Fill("Tau_RECO_vs_gendm_vs_recodm_0_500",
                                         static_cast<double>(part_i->getUserRecord("decay_mode_id")) - 1.5,
                                         static_cast<double>(matched_reco_particle->getUserRecord("decayMode")) + 0.5,
                                         1.);
                    } else if (part_i->getPt() > 500 and part_i->getPt() < 1000) {
                        HistClass::Fill("Tau_RECO_vs_gendm_vs_recodm_500_1000",
                                         static_cast<double>(part_i->getUserRecord("decay_mode_id")) - 1.5,
                                         static_cast<double>(matched_reco_particle->getUserRecord("decayMode")) + 0.5,
                                         1.);
                    } else if (part_i->getPt() > 1000 and part_i->getPt() < 1500) {
                        HistClass::Fill("Tau_RECO_vs_gendm_vs_recodm_1000_1500",
                                         static_cast<double>(part_i->getUserRecord("decay_mode_id")) - 1.5,
                                         static_cast<double>(matched_reco_particle->getUserRecord("decayMode")) + 0.5,
                                         1.);
                    } else if (part_i->getPt() > 1500 and part_i->getPt() < 2000) {
                        HistClass::Fill("Tau_RECO_vs_gendm_vs_recodm_1500_2000",
                                         static_cast<double>(part_i->getUserRecord("decay_mode_id")) - 1.5,
                                         static_cast<double>(matched_reco_particle->getUserRecord("decayMode")) + 0.5,
                                         1.);
                    } else if (part_i->getPt() > 2000) {
                        HistClass::Fill("Tau_RECO_vs_gendm_vs_recodm_2000",
                                         static_cast<double>(part_i->getUserRecord("decay_mode_id")) - 1.5,
                                         static_cast<double>(matched_reco_particle->getUserRecord("decayMode")) + 0.5,
                                         1.);
                    }
                }
            } else {
                HistClass::FillEff(TString::Format("%s_RECO_vs_pT", object.c_str()), part_i->getPt(), false);
                HistClass::FillEff(TString::Format("%s_RECO_vs_Nvtx", object.c_str()), m_RecEvtView->getUserRecord("NumVertices"), false);
                HistClass::FillEff(TString::Format("%s_RECO_vs_eta_vs_phi", object.c_str()), part_i->getEta(), part_i->getPhi(), false);
            }
            if (Check_Gen_Par_Acc(part_i) and resonance_mass_gen["emu"] != 0) {
                if (matched_reco_particle != 0) {
                    HistClass::FillEff(TString::Format("%s_RECO_vs_pT_in_Acc", object.c_str()), part_i->getPt(), true);
                    HistClass::FillEff(TString::Format("%s_RECO_vs_Nvtx_in_Acc", object.c_str()), m_RecEvtView->getUserRecord("NumVertices"), true);
                    HistClass::FillEff(TString::Format("%s_RECO_vs_eta_vs_phi_in_Acc", object.c_str()), part_i->getEta(), part_i->getPhi(), true);
                } else {
                    HistClass::FillEff(TString::Format("%s_RECO_vs_pT_in_Acc", object.c_str()), part_i->getPt(), false);
                    HistClass::FillEff(TString::Format("%s_RECO_vs_Nvtx_in_Acc", object.c_str()), m_RecEvtView->getUserRecord("NumVertices"), false);
                    HistClass::FillEff(TString::Format("%s_RECO_vs_eta_vs_phi_in_Acc", object.c_str()), part_i->getEta(), part_i->getPhi(), false);
                }
            }
            delete matched_reco_particle;
        }
    } else {
        for (std::vector< pxl::Particle* >::const_iterator part_it = S3ListGen->begin(); part_it != S3ListGen->end(); ++part_it) {
            pxl::Particle *part_i = *part_it;
            pxl::Particle* matched_reco_particle = 0;
            if (TMath::Abs(part_i->getPdgNumber()) != id) continue;
            double delta_r_max = 0.25;
            double delta_r_min = 3;
            double delta_phi_min = 3;
            double delta_eta_min = 3;
            for (std::vector< pxl::Particle* >::const_iterator part_jt = part_list.begin(); part_jt != part_list.end(); ++part_jt) {
                pxl::Particle *part_j = *part_jt;
                double temp_delta_r = DeltaPhi(part_j, part_i);
                if (temp_delta_r < delta_r_min) {
                    delta_r_min = temp_delta_r;
                    delta_phi_min = DeltaPhi(part_j, part_i);
                    delta_eta_min = TMath::Abs(part_j->getEta() - part_i->getEta());
                    if (delta_r_min < delta_r_max) {
                        matched_reco_particle = (pxl::Particle*) part_j->clone();
                    }
                }
            }
            HistClass::Fill(TString::Format("%s_RECO_DeltaR", object.c_str()), delta_r_min, 1.);
            HistClass::Fill(TString::Format("%s_RECO_DeltaEta", object.c_str()), delta_eta_min, 1.);
            HistClass::Fill(TString::Format("%s_RECO_DeltaPhi", object.c_str()), delta_phi_min, 1.);
            if (matched_reco_particle != 0) {
                HistClass::FillEff(TString::Format("%s_RECO_vs_pT", object.c_str()), part_i->getPt(), true);
                HistClass::FillEff(TString::Format("%s_RECO_vs_Nvtx", object.c_str()), m_RecEvtView->getUserRecord("NumVertices"), true);
                HistClass::FillEff(TString::Format("%s_RECO_vs_eta_vs_phi", object.c_str()), part_i->getEta(), part_i->getPhi(), true);
            } else {
                HistClass::FillEff(TString::Format("%s_RECO_vs_pT", object.c_str()), part_i->getPt(), false);
                HistClass::FillEff(TString::Format("%s_RECO_vs_Nvtx", object.c_str()), m_RecEvtView->getUserRecord("NumVertices"), false);
                HistClass::FillEff(TString::Format("%s_RECO_vs_eta_vs_phi", object.c_str()), part_i->getEta(), part_i->getPhi(), false);
            }
            if (Check_Gen_Par_Acc(part_i) and resonance_mass_gen["emu"] != 0) {
                if (matched_reco_particle != 0) {
                    HistClass::FillEff(TString::Format("%s_RECO_vs_pT_in_Acc", object.c_str()), part_i->getPt(), true);
                    HistClass::FillEff(TString::Format("%s_RECO_vs_Nvtx_in_Acc", object.c_str()), m_RecEvtView->getUserRecord("NumVertices"), true);
                    HistClass::FillEff(TString::Format("%s_RECO_vs_eta_vs_phi_in_Acc", object.c_str()), part_i->getEta(), part_i->getPhi(), true);
                } else {
                    HistClass::FillEff(TString::Format("%s_RECO_vs_pT_in_Acc", object.c_str()), part_i->getPt(), false);
                    HistClass::FillEff(TString::Format("%s_RECO_vs_Nvtx_in_Acc", object.c_str()), m_RecEvtView->getUserRecord("NumVertices"), false);
                    HistClass::FillEff(TString::Format("%s_RECO_vs_eta_vs_phi_in_Acc", object.c_str()), part_i->getEta(), part_i->getPhi(), false);
                }
            }
            delete matched_reco_particle;
        }
    }
}

void specialAna::Create_trigger_effs() {
    for (std::vector< std::string >::const_iterator it=m_trigger_string.begin(); it!= m_trigger_string.end(); it++) {
        const char* temp_trigger_name = (*it).c_str();
        trigger_defs[temp_trigger_name] = new Trigger(*it);
        if (trigger_defs[temp_trigger_name]->GetDimension()) {
            HistClass::CreateEff(TString::Format("%s_vs_pT(%s,%s)", temp_trigger_name, trigger_defs[temp_trigger_name]->GetPart1Name().c_str(), trigger_defs[temp_trigger_name]->GetPart2Name().c_str()),
                                 300, 0, 3000, 300, 0, 3000,
                                 TString::Format("p_{T}^{%s,%s} (GeV)", trigger_defs[temp_trigger_name]->GetPart1Name().c_str(), trigger_defs[temp_trigger_name]->GetPart2Name().c_str()));
            HistClass::CreateEff(TString::Format("%s_vs_Nvtx", temp_trigger_name), 70, 0, 70, "n_{vtx}");
            HistClass::CreateEff(TString::Format("%s_vs_eta_vs_phi(%s)", temp_trigger_name, trigger_defs[temp_trigger_name]->GetPart1Name().c_str()),
                                 150, -3, 3, 150, -3.2, 3.2,
                                 TString::Format("#eta(%s)", trigger_defs[temp_trigger_name]->GetPart1Name().c_str()),
                                 TString::Format("#phi(%s) (rad)", trigger_defs[temp_trigger_name]->GetPart1Name().c_str()));
            HistClass::CreateEff(TString::Format("%s_vs_eta_vs_phi(%s)", temp_trigger_name, trigger_defs[temp_trigger_name]->GetPart2Name().c_str()),
                                 150, -3, 3, 150, -3.2, 3.2,
                                 TString::Format("#eta(%s)", trigger_defs[temp_trigger_name]->GetPart2Name().c_str()),
                                 TString::Format("#phi(%s) (rad)", trigger_defs[temp_trigger_name]->GetPart2Name().c_str()));
        } else {
            HistClass::CreateEff(TString::Format("%s_vs_pT(%s)", temp_trigger_name, trigger_defs[temp_trigger_name]->GetPart1Name().c_str()),
                                 300, 0, 3000,
                                 TString::Format("p_{T}^{%s} (GeV)", trigger_defs[temp_trigger_name]->GetPart1Name().c_str()));
            HistClass::CreateEff(TString::Format("%s_vs_Nvtx", temp_trigger_name), 70, 0, 70, "n_{vtx}");
            HistClass::CreateEff(TString::Format("%s_vs_eta_vs_phi(%s)", temp_trigger_name, trigger_defs[temp_trigger_name]->GetPart1Name().c_str()),
                                 150, -3, 3, 150, -3.2, 3.2,
                                 TString::Format("#eta(%s)", trigger_defs[temp_trigger_name]->GetPart1Name().c_str()),
                                 TString::Format("#phi(%s) (rad)", trigger_defs[temp_trigger_name]->GetPart1Name().c_str()));
        }
    }
}

void specialAna::Fill_trigger_effs() {
    for (std::vector< std::string >::const_iterator it=m_trigger_string.begin(); it!= m_trigger_string.end(); it++) {
        if (not trigger_defs[(*it).c_str()]->GetDimension()) {
            Get_Trigger_match_1(*it);
        } else {
            Get_Trigger_match_2(*it);
        }
    }
}

void specialAna::Get_Trigger_match_2(std::string trigger_name) {
    std::vector< pxl::Particle* > * particles_1;
    std::vector< pxl::Particle* > * particles_2;

    if (trigger_defs[trigger_name.c_str()]->GetPart1Name() == "Mu") {
        particles_1 = MuonList;
    } else if (trigger_defs[trigger_name.c_str()]->GetPart1Name() == "Ele") {
        particles_1 = EleList;
    } else if (trigger_defs[trigger_name.c_str()]->GetPart1Name() == "Tau") {
        particles_1 = TauList;
    }

    if (trigger_defs[trigger_name.c_str()]->GetPart2Name() == "Mu") {
        particles_2 = MuonList;
    } else if (trigger_defs[trigger_name.c_str()]->GetPart2Name() == "Ele") {
        particles_2 = EleList;
    } else if (trigger_defs[trigger_name.c_str()]->GetPart2Name() == "Tau") {
        particles_2 = TauList;
    }

    std::vector< pxl::Particle* > AllTriggers;
    m_TrigEvtView->getObjectsOfType< pxl::Particle >(AllTriggers);

    for (std::vector< pxl::Particle* >::const_iterator part_jt = particles_1->begin(); part_jt != particles_1->end(); ++part_jt) {
        pxl::Particle *part = *part_jt;
        if (not Check_Par_ID(part, false, false)) continue;

        bool match_found = false;
        for (std::vector< pxl::Particle* >::const_iterator part_it = AllTriggers.begin(); part_it != AllTriggers.end(); ++part_it) {
            pxl::Particle *trig = *part_it;
            if (trigger_name.find(trig->getName()) != std::string::npos) {
                match_found = true;
                break;
            }
        }

        pxl::Particle* part_2;
        bool second_match_found = false;
        for (std::vector< pxl::Particle* >::const_iterator part_kt = particles_2->begin(); part_kt != particles_2->end(); ++part_kt) {
            pxl::Particle *part_k = *part_kt;
            if (not Check_Par_ID(part_k, false, false)) continue;
            if (part->getId() == part_k->getId()) continue;
            part_2 = (pxl::Particle*)part_k->clone();
            second_match_found = true;
        }

        if (match_found and second_match_found) {
            if (Check_Par_ID(part, false, true) and Check_Par_ID(part_2, false, true)) {
                HistClass::FillEff(TString::Format("%s_vs_pT(%s,%s)", trigger_name.c_str(), trigger_defs[trigger_name.c_str()]->GetPart1Name().c_str(), trigger_defs[trigger_name.c_str()]->GetPart2Name().c_str()),
                                   part->getPt(), part_2->getPt(), true);
            }
            if (Check_Par_ID(part, true, true) and Check_Par_ID(part_2, true, true)) {
                HistClass::FillEff(TString::Format("%s_vs_Nvtx", trigger_name.c_str()), m_RecEvtView->getUserRecord("NumVertices"), true);
            }
            if (Check_Par_ID(part, true, false) and Check_Par_ID(part_2, true, true)) {
                HistClass::FillEff(TString::Format("%s_vs_eta_vs_phi(%s)", trigger_name.c_str(), trigger_defs[trigger_name.c_str()]->GetPart1Name().c_str()),
                                   part->getEta(), part->getPhi(), true);
            }
            if (Check_Par_ID(part, true, true) and Check_Par_ID(part_2, true, false)) {
                HistClass::FillEff(TString::Format("%s_vs_eta_vs_phi(%s)", trigger_name.c_str(), trigger_defs[trigger_name.c_str()]->GetPart2Name().c_str()),
                                   part_2->getEta(), part_2->getPhi(), true);
            }
        } else if (second_match_found) {
            if (Check_Par_ID(part, false, true) and Check_Par_ID(part_2, false, true)) {
                HistClass::FillEff(TString::Format("%s_vs_pT(%s,%s)", trigger_name.c_str(), trigger_defs[trigger_name.c_str()]->GetPart1Name().c_str(), trigger_defs[trigger_name.c_str()]->GetPart2Name().c_str()),
                                   part->getPt(), part_2->getPt(), false);
            }
            if (Check_Par_ID(part, true, true) and Check_Par_ID(part_2, true, true)) {
                HistClass::FillEff(TString::Format("%s_vs_Nvtx", trigger_name.c_str()), m_RecEvtView->getUserRecord("NumVertices"), false);
            }
            if (Check_Par_ID(part, true, false) and Check_Par_ID(part_2, true, true)) {
                HistClass::FillEff(TString::Format("%s_vs_eta_vs_phi(%s)", trigger_name.c_str(), trigger_defs[trigger_name.c_str()]->GetPart1Name().c_str()),
                                   part->getEta(), part->getPhi(), false);
            }
            if (Check_Par_ID(part, true, true) and Check_Par_ID(part_2, true, false)) {
                HistClass::FillEff(TString::Format("%s_vs_eta_vs_phi(%s)", trigger_name.c_str(), trigger_defs[trigger_name.c_str()]->GetPart2Name().c_str()),
                                   part_2->getEta(), part_2->getPhi(), false);
            }
        }
        if (second_match_found) delete part_2;
    }
}

void specialAna::Get_Trigger_match_1(std::string trigger_name) {
    std::vector< pxl::Particle* > * particles;

    if (trigger_defs[trigger_name.c_str()]->GetPart1Name() == "Mu") {
        particles = MuonList;
    } else if (trigger_defs[trigger_name.c_str()]->GetPart1Name() == "Ele") {
        particles = EleList;
    } else if (trigger_defs[trigger_name.c_str()]->GetPart1Name() == "Tau") {
        particles = TauList;
    }

    std::vector< pxl::Particle* > AllTriggers;
    m_TrigEvtView->getObjectsOfType< pxl::Particle >(AllTriggers);

    for (std::vector< pxl::Particle* >::const_iterator part_jt = particles->begin(); part_jt != particles->end(); ++part_jt) {
        pxl::Particle *part = *part_jt;
        if (not Check_Par_ID(part, false, false)) continue;

        bool match_found = false;
        double trig_match_dr = 10;
        pxl::Particle* trig_cand;
        for (std::vector< pxl::Particle* >::const_iterator part_it = AllTriggers.begin(); part_it != AllTriggers.end(); ++part_it) {
            pxl::Particle *trig = *part_it;
            if (trigger_name.find(trig->getName()) != std::string::npos) {
                if (match_found and trig_cand->getE() == trig->getE()) continue;
                double dummy_dr = DeltaR(trig, part);
                if (dummy_dr < trig_match_dr) {
                    trig_match_dr = dummy_dr;
                    match_found = true;
                    trig_cand = (pxl::Particle*)trig->clone();
                }
            }
        }

        if (match_found) {
            if (Check_Par_ID(part, false, true)) {
                HistClass::FillEff(TString::Format("%s_vs_pT(%s)", trigger_name.c_str(), trigger_defs[trigger_name.c_str()]->GetPart1Name().c_str()),
                                   part->getPt(), true);
            }
            if (Check_Par_ID(part, true, false)) {
                HistClass::FillEff(TString::Format("%s_vs_eta_vs_phi(%s)", trigger_name.c_str(), trigger_defs[trigger_name.c_str()]->GetPart1Name().c_str()),
                                   part->getEta(), part->getPhi(), true);
            }
            if (Check_Par_ID(part)) {
                HistClass::FillEff(TString::Format("%s_vs_Nvtx", trigger_name.c_str()), m_RecEvtView->getUserRecord("NumVertices"), true);
            }
        } else {
            if (Check_Par_ID(part, false, true)) {
                HistClass::FillEff(TString::Format("%s_vs_pT(%s)", trigger_name.c_str(), trigger_defs[trigger_name.c_str()]->GetPart1Name().c_str()),
                                   part->getPt(), false);
            }
            if (Check_Par_ID(part, true, false)) {
                HistClass::FillEff(TString::Format("%s_vs_eta_vs_phi(%s)", trigger_name.c_str(), trigger_defs[trigger_name.c_str()]->GetPart1Name().c_str()),
                                   part->getEta(), part->getPhi(), false);
            }
            if (Check_Par_ID(part)) {
                HistClass::FillEff(TString::Format("%s_vs_Nvtx", trigger_name.c_str()), m_RecEvtView->getUserRecord("NumVertices"), false);
            }
        }
        if (match_found) delete trig_cand;
    }
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
    HistClass::Fill(TString::Format("%s_pT_%s_Gen",           channel, part1),        sel_part1_gen[channel] -> getPt(),                            weight);
    HistClass::Fill(TString::Format("%s_eta_%s_Gen",          channel, part1),        sel_part1_gen[channel] -> getEta(),                           weight);
    HistClass::Fill(TString::Format("%s_phi_%s_Gen",          channel, part1),        sel_part1_gen[channel] -> getPhi(),                           weight);
    /// Second particle histograms
    HistClass::Fill(TString::Format("%s_pT_%s_Gen",           channel, part2),        sel_part2_gen[channel] -> getPt(),                            weight);
    HistClass::Fill(TString::Format("%s_eta_%s_Gen",          channel, part2),        sel_part2_gen[channel] -> getEta(),                           weight);
    HistClass::Fill(TString::Format("%s_phi_%s_Gen",          channel, part2),        sel_part2_gen[channel] -> getPhi(),                           weight);
    /// Delta phi between the two particles
    HistClass::Fill(TString::Format("%s_Delta_phi_%s_%s_Gen", channel, part1, part2), DeltaPhi(sel_part1_gen[channel], sel_part2_gen[channel]),              weight);
    /// pT ratio of the two particles
    HistClass::Fill(TString::Format("%s_pT_ratio_%s_%s_Gen",  channel, part1, part2), sel_part1_gen[channel] -> getPt() / sel_part2_gen[channel] -> getPt(), weight);

    if (Check_Gen_Par_Acc(sel_part1_gen[channel]) and Check_Gen_Par_Acc(sel_part2_gen[channel])) {
        HistClass::FillEff(TString::Format("%s_Acc_vs_Mass", channel), resonance_mass_gen[channel], true);
        HistClass::FillEff(TString::Format("%s_Acc_vs_Nvtx", channel), m_RecEvtView->getUserRecord("NumVertices"), true);
    } else {
        HistClass::FillEff(TString::Format("%s_Acc_vs_Mass", channel), resonance_mass_gen[channel], false);
        HistClass::FillEff(TString::Format("%s_Acc_vs_Nvtx", channel), m_RecEvtView->getUserRecord("NumVertices"), false);
    }

    if (resonance_mass_gen[channel] != 0) {
        HistClass::Fill(TString::Format("%s_RECO_vs_Mass_All", channel), resonance_mass_gen[channel], 1);
        HistClass::Fill(TString::Format("%s_RECO_vs_Nvtx_All", channel), m_RecEvtView->getUserRecord("NumVertices"), 1);
    
        HistClass::Fill(TString::Format("%s_Eff_vs_Mass_All", channel), resonance_mass_gen[channel], 1);
        HistClass::Fill(TString::Format("%s_Eff_vs_Nvtx_All", channel), m_RecEvtView->getUserRecord("NumVertices"), 1);
    }
}

void specialAna::Create_Resonance_histograms(int n_histos, const char* channel, const char* part1, const char* part2, std::string const endung) {
    /// Cutflow histogram
    HistClass::CreateHisto(TString::Format("%s_Cutflow",                         channel) + endung,               n_histos, 0, n_histos, "Cut stage");
    /// Resonant mass histogram
    HistClass::CreateHisto(n_histos, TString::Format("%s_Mass",                  channel) + endung,               5000, 0, 5000, TString::Format("M_{%s,%s} (GeV)",                         part1, part2) );
    /// Delta R Histogram between muon and electron
    HistClass::CreateHisto(n_histos, TString::Format("%s_dR",                    channel) + endung,               500, 0, 10, TString::Format("#Delta R_{%s,%s}",                           part1, part2) );
    /// Resonant mass resolution histogram
    HistClass::CreateHisto(n_histos, TString::Format("%s_Mass_resolution",       channel) + endung, 500, 0, 5000, 1000, -10, 10, TString::Format("M^{gen}_{%s,%s} (GeV)",                   part1, part2), TString::Format("M-M_{gen}/M_{gen}(%s,%s)", part1, part2));
    HistClass::CreateHisto(n_histos, TString::Format("%s_Mass_resolution",       channel) + endung,               1000, -10, 10, TString::Format("M-M_{gen}/M_{gen}(%s,%s)",                part1, part2) );
    /// Particle resolutions
    HistClass::CreateHisto(n_histos, TString::Format("%s_pT_resolution_%s",      channel, part1) + endung,        1000, -10, 10, TString::Format("p_{T}-p_{T,gen}/p_{T,gen}(%s)",           part1) );
    HistClass::CreateHisto(n_histos, TString::Format("%s_deltaR_match_%s",       channel, part1) + endung,        1000,   0, 10, TString::Format("#DeltaR(gen,RECO)(%s)",                   part1) );
    HistClass::CreateHisto(n_histos, TString::Format("%s_pT_resolution_%s",      channel, part2) + endung,        1000, -10, 10, TString::Format("p_{T}-p_{T,gen}/p_{T,gen}(%s)",           part2) );
    HistClass::CreateHisto(n_histos, TString::Format("%s_deltaR_match_%s",       channel, part2) + endung,        1000,   0, 10, TString::Format("#DeltaR(gen,RECO)(%s)",                   part2) );
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

    if (endung == const_cast<char*>("")) {
        HistClass::CreateEff(TString::Format("%s_Acc_vs_Mass", channel), 600, 0, 6000,
                             TString::Format("M_{%s}^{(gen)} (GeV)", channel));
        HistClass::CreateEff(TString::Format("%s_Acc_vs_Nvtx", channel),       70, 0, 70,
                             "n_{vtx}");

        HistClass::CreateHisto(TString::Format("%s_RECO_vs_Mass_All", channel), 600, 0, 6000);
        HistClass::CreateHisto(TString::Format("%s_RECO_vs_Mass_Pass", channel), 600, 0, 6000);
        HistClass::CreateHisto(TString::Format("%s_RECO_vs_Nvtx_All", channel), 70, 0, 70);
        HistClass::CreateHisto(TString::Format("%s_RECO_vs_Nvtx_Pass", channel), 70, 0, 70);

        HistClass::CreateHisto(TString::Format("%s_Eff_vs_Mass_All", channel), 600, 0, 6000);
        HistClass::CreateHisto(TString::Format("%s_Eff_vs_Mass_Pass", channel), 600, 0, 6000);
        HistClass::CreateHisto(TString::Format("%s_Eff_vs_Nvtx_All", channel), 70, 0, 70);
        HistClass::CreateHisto(TString::Format("%s_Eff_vs_Nvtx_Pass", channel), 70, 0, 70);
    }
}

void specialAna::Fill_Resonance_histograms(int n_histos, const char* channel, const char* part1, const char* part2, std::string const endung) {
    /// Cutflow histogram
    HistClass::Fill(TString::Format("%s_Cutflow",                         channel) + endung,               n_histos,                                                          weight);
    /// Resonant mass histogram
    HistClass::Fill(n_histos, TString::Format("%s_Mass",                  channel) + endung,               resonance_mass[channel],                                           weight);
    /// Delta R between electron and muon
    HistClass::Fill(n_histos, TString::Format("%s_dR",                    channel) + endung,               DeltaR(sel_lepton_prompt[channel], sel_lepton_nprompt[channel]),                     weight);
    /// Resonant mass resolution histogram
    double dummy_resolution = (resonance_mass[channel] - resonance_mass_gen[channel])/resonance_mass_gen[channel];
    HistClass::Fill(n_histos, TString::Format("%s_Mass_resolution",       channel) + endung,               dummy_resolution,                                                  weight);
    HistClass::Fill(n_histos, TString::Format("%s_Mass_resolution",       channel) + endung,               resonance_mass_gen[channel], dummy_resolution,                     weight);
    /// Particle resolution histograms
    double part1_resolution = (sel_lepton_prompt[channel]->getPt() - sel_part1_gen[channel]->getPt()) / sel_part1_gen[channel]->getPt();
    double part1_dr = DeltaR(sel_lepton_prompt[channel], sel_part1_gen[channel]);
    HistClass::Fill(n_histos, TString::Format("%s_pT_resolution_%s",      channel, part1) + endung,        part1_resolution,                                                  weight);
    HistClass::Fill(n_histos, TString::Format("%s_deltaR_match_%s",       channel, part1) + endung,        part1_dr,                                                          weight);
    double part2_resolution = 0;
    double part2_dr = 0;
    if (sel_lepton_nprompt_corr[channel] != 0) {
        part2_resolution = (sel_lepton_nprompt_corr[channel]->getPt() - sel_part2_gen[channel]->getPt()) / sel_part2_gen[channel]->getPt();
        part2_dr = DeltaR(sel_lepton_nprompt_corr[channel], sel_part1_gen[channel]);
    } else {
        part2_resolution = (sel_lepton_nprompt[channel]->getPt() - sel_part2_gen[channel]->getPt()) / sel_part2_gen[channel]->getPt();
        part2_dr = DeltaR(sel_lepton_nprompt[channel], sel_part2_gen[channel]);
    }
    HistClass::Fill(n_histos, TString::Format("%s_pT_resolution_%s",      channel, part2) + endung,        part2_resolution,                                                  weight);
    HistClass::Fill(n_histos, TString::Format("%s_deltaR_match_%s",       channel, part2) + endung,        part2_dr,                                                          weight);
    /// First particle histograms
    HistClass::Fill(n_histos, TString::Format("%s_pT_%s",                 channel, part1) + endung,        sel_lepton_prompt[channel] -> getPt(),                                      weight);
    HistClass::Fill(n_histos, TString::Format("%s_eta_%s",                channel, part1) + endung,        sel_lepton_prompt[channel] -> getEta(),                                     weight);
    HistClass::Fill(n_histos, TString::Format("%s_phi_%s",                channel, part1) + endung,        sel_lepton_prompt[channel] -> getPhi(),                                     weight);
    /// Second particle histograms
    HistClass::Fill(n_histos, TString::Format("%s_pT_%s",                 channel, part2) + endung,        sel_lepton_nprompt[channel] -> getPt(),                                     weight);
    HistClass::Fill(n_histos, TString::Format("%s_eta_%s",                channel, part2) + endung,        sel_lepton_nprompt[channel] -> getEta(),                                    weight);
    HistClass::Fill(n_histos, TString::Format("%s_phi_%s",                channel, part2) + endung,        sel_lepton_nprompt[channel] -> getPhi(),                                    weight);
    /// Delta phi between the two particles
    HistClass::Fill(n_histos, TString::Format("%s_Delta_phi_%s_%s",       channel, part1, part2) + endung, DeltaPhi(sel_lepton_prompt[channel], sel_lepton_nprompt[channel]),                   weight);
    /// pT ratio of the two particles
    HistClass::Fill(n_histos, TString::Format("%s_pT_ratio_%s_%s",        channel, part1, part2) + endung, sel_lepton_prompt[channel] -> getPt() / sel_lepton_nprompt[channel] -> getPt(),      weight);
    /// Create histograms for channels with MET
    if (channel != const_cast<char*>("emu")) {
        /// MET histograms
        HistClass::Fill(n_histos, TString::Format("%s_MET",                   channel) + endung,               sel_met[channel] -> getPt(),                                                weight);
        HistClass::Fill(n_histos, TString::Format("%s_phi_MET",               channel) + endung,               sel_met[channel] -> getPhi(),                                               weight);
        /// Corrected second particle histogram
        HistClass::Fill(n_histos, TString::Format("%s_pT_%s_corr",            channel, part2) + endung,        sel_lepton_nprompt_corr[channel] -> getPt(),                                weight);
        HistClass::Fill(n_histos, TString::Format("%s_eta_%s_corr",           channel, part2) + endung,        sel_lepton_nprompt_corr[channel] -> getEta(),                               weight);
        HistClass::Fill(n_histos, TString::Format("%s_phi_%s_corr",           channel, part2) + endung,        sel_lepton_nprompt_corr[channel] -> getPhi(),                               weight);
        /// Delta phi between the other particles
        HistClass::Fill(n_histos, TString::Format("%s_Delta_phi_%s_MET",      channel, part1) + endung,        DeltaPhi(sel_lepton_prompt[channel], sel_met[channel]),                              weight);
        HistClass::Fill(n_histos, TString::Format("%s_Delta_phi_%s_MET",      channel, part2) + endung,        DeltaPhi(sel_lepton_nprompt[channel], sel_met[channel]),                             weight);
        HistClass::Fill(n_histos, TString::Format("%s_Delta_phi_%s_MET_corr", channel, part2) + endung,        DeltaPhi(sel_lepton_nprompt_corr[channel], sel_met[channel]),                        weight);
        HistClass::Fill(n_histos, TString::Format("%s_Delta_phi_%s_%s_corr",  channel, part1, part2) + endung, DeltaPhi(sel_lepton_prompt[channel], sel_lepton_nprompt_corr[channel]),              weight);
        /// pT ratio of the other particles
        HistClass::Fill(n_histos, TString::Format("%s_pT_ratio_%s_MET",       channel, part1) + endung,        sel_lepton_prompt[channel] -> getPt() / sel_met[channel] -> getPt(),                 weight);
        HistClass::Fill(n_histos, TString::Format("%s_pT_ratio_%s_MET",       channel, part2) + endung,        sel_lepton_nprompt[channel] -> getPt() / sel_met[channel] -> getPt(),                weight);
        HistClass::Fill(n_histos, TString::Format("%s_pT_ratio_%s_MET_corr",  channel, part2) + endung,        sel_lepton_nprompt_corr[channel] -> getPt() / sel_met[channel] -> getPt(),           weight);
        HistClass::Fill(n_histos, TString::Format("%s_pT_ratio_%s_%s_corr",   channel, part1, part2) + endung, sel_lepton_prompt[channel] -> getPt() / sel_lepton_nprompt_corr[channel] -> getPt(), weight);
    }

    if (n_histos == 0 and resonance_mass_gen[channel] != 0) {
        HistClass::Fill(TString::Format("%s_Eff_vs_Mass_Pass", channel), resonance_mass_gen[channel], 1);
        HistClass::Fill(TString::Format("%s_Eff_vs_Nvtx_Pass", channel), m_RecEvtView->getUserRecord("NumVertices"), 1);
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
                        sel_part1_gen[channel] = (pxl::Particle*) part_i->clone();
                        sel_part2_gen[channel] = (pxl::Particle*) part_j->clone();
                    }
                    delete part_sum;
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
                        sel_part1_gen[channel] = (pxl::Particle*) part_i->clone();
                        sel_part2_gen[channel] = (pxl::Particle*) part_j->clone();
                    }
                    delete part_sum;
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
                    sel_lepton_prompt[channel] = (pxl::Particle*) part_i->clone();
                    sel_lepton_nprompt[channel] = (pxl::Particle*) part_j->clone();
                }
                delete part_sum;
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
    if (not sel_met[channel]) return false;
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
                calc_met -> SetPtEtaPhiM(sel_met[channel]->getPt(), part_j->getEta(), sel_met[channel]->getPhi(), 0);
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
                    sel_lepton_prompt[channel] = (pxl::Particle*) part_i -> clone();
                    sel_lepton_nprompt[channel] = (pxl::Particle*) part_j -> clone();
                    sel_lepton_nprompt_corr[channel] = (pxl::Particle*) part_j -> clone();
                    sel_lepton_nprompt_corr[channel] -> addP4(dummy_met);
                }
                delete dummy_taumu;
                delete dummy_taumu_uncorr;
                delete dummy_met;
            }
        }
    }
    if (resonance_mass[channel] > 0) {
        return true;
    } else {
        return false;
    }
}

bool specialAna::Check_Par_ID(pxl::Particle* part, bool do_pt_cut, bool do_eta_cut) {
    std::string name = part -> getName();
    if (name == m_TauType) {
        bool tau_id = Check_Tau_ID(part);
        return tau_id;
    } else if (name == "Ele") {
        bool ele_id = Check_Ele_ID(part, do_pt_cut, do_eta_cut);
        return ele_id;
    } else if (name == "Muon") {
        bool muo_id = Check_Muo_ID(part, do_pt_cut, do_eta_cut);
        return muo_id;
    } else {
        return false;
    }
}

bool specialAna::Check_Par_Acc(pxl::Particle* part, bool do_pt_cut, bool do_eta_cut) {
    std::string name = part -> getName();
    if (name == m_TauType) {
        return true;
    } else if (name == "Ele") {
        bool ele_eta = TMath::Abs(part->getEta()) < 2.5 ? true : false;
        bool ele_pt  = part->getPt() > 110. ? true : false;
        if (not do_pt_cut) ele_pt = true;
        if (not do_eta_cut) ele_eta = true;
        if (ele_eta and ele_pt) {
            return true;
        } else {
            return false;
        }
    } else if (name == "Muon") {
        bool muo_eta = TMath::Abs(part->getEta()) < 2.1 ? true : false;
        bool muo_pt  = part->getPt() > 45. ? true : false;
        if (not do_pt_cut) muo_pt = true;
        if (not do_eta_cut) muo_eta = true;
        if (muo_eta and muo_pt) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool specialAna::Check_Gen_Par_Acc(pxl::Particle* part, bool do_pt_cut, bool do_eta_cut) {
    if (TMath::Abs(part -> getPdgNumber()) == 15) {
        return true;
    } else if (TMath::Abs(part -> getPdgNumber()) == 11) {
        bool ele_eta = true;
        if (TMath::Abs(part->getEta()) > 2.5 or (TMath::Abs(part->getEta()) < 1.566 and TMath::Abs(part->getEta()) > 1.4442)) {
            ele_eta = false;
        }
        bool ele_pt  = part->getPt() > 35. ? true : false;
        if (not do_pt_cut) ele_pt = true;
        if (not do_eta_cut) ele_eta = true;
        if (ele_eta and ele_pt) {
            return true;
        } else {
            return false;
        }
    } else if (TMath::Abs(part -> getPdgNumber()) == 13) {
        bool muo_eta = TMath::Abs(part->getEta()) < 2.1 ? true : false;
        bool muo_pt  = part->getPt() > 45. ? true : false;
        if (not do_pt_cut) muo_pt = true;
        if (not do_eta_cut) muo_eta = true;
        if (muo_eta and muo_pt) {
            return true;
        } else {
            return false;
        }
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

bool specialAna::Check_Muo_ID(pxl::Particle* muon, bool do_pt_cut, bool do_eta_cut) {
    bool muon_ID = muon->getUserRecord("isHighPtMuon").asBool() ? true : false;
    bool muon_ISO = false;
    if (b_8TeV) {
        muon_ISO = muon -> getUserRecord("IsoR3SumPt").asDouble() / muon -> getPt() < 0.1 ? true : false;
    } else if (b_13TeV) {
        muon_ISO = muon -> getUserRecord("IsoR3SumPt").asFloat() / muon -> getPt() < 0.1 ? true : false;
    }
    bool muon_eta = TMath::Abs(muon -> getEta()) < 2.1 ? true : false;
    bool muon_pt = muon -> getPt() > 45. ? true : false;
    if (not do_pt_cut) {
        muon_pt = true;
    }
    if (not do_eta_cut) {
        muon_eta = true;
    }
    if (muon_ID && muon_ISO && muon_eta && muon_pt) return true;
    return false;
}

bool specialAna::Check_Ele_ID(pxl::Particle* ele, bool do_pt_cut, bool do_eta_cut) {
    bool ele_ID = ele->getUserRecord("IDpassed").asBool() ? true : false;
    bool ele_eta = TMath::Abs(ele -> getEta()) < 2.5 ? true : false;
    bool ele_pt = ele -> getPt() > 110. ? true : false;

    if (not do_pt_cut) {
        ele_pt = true;
    }
    if (not do_eta_cut) {
        ele_eta = true;
    }
    if (ele_ID && ele_eta && ele_pt) return true;
    return false;
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
    std::vector<double> zeta_vals = Make_zeta_stuff(sel_lepton_prompt["mutau"], sel_lepton_nprompt["mutau"], sel_met["mutau"]);
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
    if (sel_met["mutau"] and sel_lepton_nprompt["mutau"]) {
        delta_phi = DeltaPhi(sel_lepton_nprompt["mutau"], sel_met["mutau"]);
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
    if (sel_lepton_prompt["mutau"] and sel_lepton_nprompt["mutau"]) {
        delta_phi = DeltaPhi(sel_lepton_nprompt["mutau"], sel_lepton_prompt["mutau"]);
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
    if (sel_lepton_prompt["mutaue"] and sel_lepton_nprompt["mutaue"]) {
        delta_phi = DeltaPhi(sel_lepton_nprompt["mutaue"], sel_lepton_prompt["mutaue"]);
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
    if (sel_lepton_prompt["emu"] and sel_lepton_nprompt["emu"]) {
        delta_phi = DeltaPhi(sel_lepton_nprompt["emu"], sel_lepton_prompt["emu"]);
    }
    double delta_phi_e_mu_cut_value = 2.7;
    cuts->SetVars(delta_phi);
    if (delta_phi > delta_phi_e_mu_cut_value) {
        return true;
    } else {
        return false;
    }
}

bool specialAna::Make_DeltaR_emu(Cuts* cuts) {
    double delta_R = 0.;
    if (sel_lepton_prompt["emu"] and sel_lepton_nprompt["emu"]) {
        delta_R = DeltaR(sel_lepton_prompt["emu"], sel_lepton_nprompt["emu"]);
    }
    double delta_R_cut_value = 0.2;
    cuts->SetVars(delta_R);
    if (delta_R > delta_R_cut_value) {
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

bool specialAna::OppSign_charge(Cuts* cuts, const char* channel) {
    double charge_product = 0;
    if (sel_lepton_prompt[channel] and sel_lepton_nprompt[channel]) {
        charge_product = sel_lepton_prompt[channel]->getCharge() * sel_lepton_nprompt[channel]->getCharge();
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
    if (sel_lepton_prompt["mutau"] and sel_met["mutau"]) {
        mt = MT(sel_lepton_prompt["mutau"], sel_met["mutau"]);
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
    double pT_sum_lep = sel_lepton_prompt["mutaue"]->getPt() + sel_lepton_nprompt["mutaue"]->getPt();
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
    if (sel_lepton_nprompt["mutaue"] and sel_lepton_prompt["mutaue"]) {
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
    if (sel_lepton_nprompt_corr["mutaue"] and sel_lepton_prompt["mutaue"]) {
        pT_ratio = sel_lepton_nprompt_corr["mutaue"]->getPt() / sel_lepton_prompt["mutaue"]->getPt();
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
    if (sel_lepton_nprompt["mutaue"] and sel_lepton_prompt["mutaue"]) {
        pT_ratio = sel_lepton_prompt["mutaue"]->getPt() / sel_lepton_nprompt["mutaue"]->getEt();
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
            if (name == "Muon") {
                if (match->getPt() < 500) {
                    HistClass::Fill(name + TString::Format("_pt_resolution_0_500"), ((1./lepton->getPt()) - (1./match->getPt()))/(1./match->getPt()), weight);
                } else if (match->getPt() < 1000) {
                    HistClass::Fill(name + TString::Format("_pt_resolution_500_1000"), ((1./lepton->getPt()) - (1./match->getPt()))/(1./match->getPt()), weight);
                } else if (match->getPt() < 1500) {
                    HistClass::Fill(name + TString::Format("_pt_resolution_1000_1500"), ((1./lepton->getPt()) - (1./match->getPt()))/(1./match->getPt()), weight);
                } else if (match->getPt() < 2000) {
                    HistClass::Fill(name + TString::Format("_pt_resolution_1500_2000"), ((1./lepton->getPt()) - (1./match->getPt()))/(1./match->getPt()), weight);
                } else {
                    HistClass::Fill(name + TString::Format("_pt_resolution_2000"), ((1./lepton->getPt()) - (1./match->getPt()))/(1./match->getPt()), weight);
                }
            } else {
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

pxl::Particle* specialAna::Get_tau_truth_decay_mode(pxl::EventView& eventview, pxl::Particle* truth_tau) {
    int n_prong = 0;
    std::vector<pxl::Particle*>* final_state_part_list = new std::vector< pxl::Particle* >;
    std::vector<pxl::Particle*>* temp_part = new std::vector< pxl::Particle* >;
    temp_part->push_back(truth_tau);
    std::vector<pxl::Particle*>* new_temp_part = new std::vector< pxl::Particle* >;
    pxl::Particle* vis_tau_decay = new pxl::Particle();
    vis_tau_decay->setName("Tau_visible_decay");
    vis_tau_decay->setP4(0, 0, 0, 0);
    while (true) {
        bool continue_loop = false;
        for (uint i = 0; i < temp_part->size(); i++) {
            pxl::Particle* temp_part_dummy = temp_part->at(i);
            if (temp_part_dummy->getDaughters().size() == 0) {
                if (TMath::Abs(temp_part_dummy->getPdgNumber()) == 11) {  /// electrons
                    final_state_part_list->push_back(temp_part_dummy);
                    n_prong++;
                    vis_tau_decay->addP4(temp_part_dummy);
                } else if (TMath::Abs(temp_part_dummy->getPdgNumber()) == 13) {  /// muons
                    final_state_part_list->push_back(temp_part_dummy);
                    n_prong++;
                    vis_tau_decay->addP4(temp_part_dummy);
                } else if (TMath::Abs(temp_part_dummy->getPdgNumber()) == 111) {  /// pi 0
                    final_state_part_list->push_back(temp_part_dummy);
                    vis_tau_decay->addP4(temp_part_dummy);
                } else if (TMath::Abs(temp_part_dummy->getPdgNumber()) == 211) {  /// pi +
                    final_state_part_list->push_back(temp_part_dummy);
                    n_prong++;
                    vis_tau_decay->addP4(temp_part_dummy);
                } else if (TMath::Abs(temp_part_dummy->getPdgNumber()) == 130 or
                    TMath::Abs(temp_part_dummy->getPdgNumber()) == 310 or
                    TMath::Abs(temp_part_dummy->getPdgNumber()) == 311) {  /// K 0
                    final_state_part_list->push_back(temp_part_dummy);
                    vis_tau_decay->addP4(temp_part_dummy);
                } else if (TMath::Abs(temp_part_dummy->getPdgNumber()) == 321) {  /// K +
                    final_state_part_list->push_back(temp_part_dummy);
                    n_prong++;
                    vis_tau_decay->addP4(temp_part_dummy);
                }
            } else {
                for (std::set< pxl::Relative* >::const_iterator part_it = temp_part_dummy->getDaughters().begin(); part_it != temp_part_dummy->getDaughters().end(); ++part_it) {
                    pxl::Relative *part_i = *part_it;
                    pxl::Particle* part = (pxl::Particle*)part_i;
                    if (TMath::Abs(part->getPdgNumber()) == 16) {  /// tau neutrino
                        continue;
                    } else if (TMath::Abs(part->getPdgNumber()) == 14) {  /// muon neutrino
                        continue;
                    } else if (TMath::Abs(part->getPdgNumber()) == 12) {  /// electron neutrino
                        continue;
                    } else if (TMath::Abs(part->getPdgNumber()) == 22) {  /// photon
                        vis_tau_decay->addP4(part);
                        continue;
                    } else if (TMath::Abs(part->getPdgNumber()) == 11) {  /// electrons
                        final_state_part_list->push_back(part);
                        n_prong++;
                        vis_tau_decay->addP4(part);
                    } else if (TMath::Abs(part->getPdgNumber()) == 13) {  /// muons
                        final_state_part_list->push_back(part);
                        n_prong++;
                        vis_tau_decay->addP4(part);
                    } else if (TMath::Abs(part->getPdgNumber()) == 111) {  /// pi 0
                        final_state_part_list->push_back(part);
                        vis_tau_decay->addP4(part);
                    } else if (TMath::Abs(part->getPdgNumber()) == 211) {  /// pi +
                        final_state_part_list->push_back(part);
                        n_prong++;
                        vis_tau_decay->addP4(part);
                    } else if (TMath::Abs(part->getPdgNumber()) == 130 or
                        TMath::Abs(part->getPdgNumber()) == 310 or
                        TMath::Abs(part->getPdgNumber()) == 311) {  /// K 0
                        final_state_part_list->push_back(part);
                        vis_tau_decay->addP4(part);
                    } else if (TMath::Abs(part->getPdgNumber()) == 321) {  /// K +
                        final_state_part_list->push_back(part);
                        n_prong++;
                        vis_tau_decay->addP4(part);
                    } else {  /// others like W +
                        continue_loop = true;
                        new_temp_part->push_back(part);
                    }
                }
            }
        }
        if (not continue_loop) {
            break;
        } else {
            delete temp_part;
            temp_part = new std::vector< pxl::Particle* >;
            *temp_part = *new_temp_part;
            delete new_temp_part;
            new_temp_part = new std::vector< pxl::Particle* >;
        }
    }

    int n_piplus = 0;
    int n_pizero = 0;
    int n_Kplus = 0;
    int n_Kzero = 0;
    int n_ele = 0;
    int n_muo = 0;
    int charge = 0;
    for (std::vector< pxl::Particle* >::const_iterator part_it = final_state_part_list->begin(); part_it != final_state_part_list->end(); ++part_it) {
        pxl::Particle *part_i = *part_it;
        if (part_i->getPdgNumber() == 211) {
            n_piplus++;
            charge++;
        } else if (part_i->getPdgNumber() == -211) {
            n_piplus++;
            charge -= 1;
        } else if (part_i->getPdgNumber() == 111) {
            n_pizero++;
        } else if (part_i->getPdgNumber() == 130 or
                   part_i->getPdgNumber() == 310 or
                   part_i->getPdgNumber() == 311 or
                   part_i->getPdgNumber() == -311) {
            n_Kzero++;
        } else if (part_i->getPdgNumber() == 321) {
            n_Kplus++;
            charge++;
        } else if (part_i->getPdgNumber() == -321) {
            n_Kplus++;
            charge -= 1;
        } else if (part_i->getPdgNumber() == 11) {
            n_ele++;
            charge -= 1;
        } else if (part_i->getPdgNumber() == -11) {
            n_ele++;
            charge++;
        } else if (part_i->getPdgNumber() == 13) {
            n_muo++;
            charge -= 1;
        } else if (part_i->getPdgNumber() == -13) {
            n_muo++;
            charge++;
        } else {
            std::cerr << "Found a particle that should not be here" << std::endl;
            std::cerr << "ID: " << part_i->getPdgNumber() << std::endl;
        }
    }

    TString decay_mode = "";
    int decay_mode_id = -1;
    if (n_ele > 0 and (n_piplus > 0 or n_Kplus > 0)) {
        if (n_ele == 2) {
            n_pizero++;
            n_ele = 0;
        } else {
            decay_mode = TString::Format("%iEleX", n_ele);
            decay_mode_id = 14;
        }
    } else if (n_ele > 0 and n_muo == 0) {
        decay_mode = TString::Format("%iEle", n_ele);
        decay_mode_id = 0;
    } else if (n_muo > 0 and (n_piplus > 0 or n_Kplus > 0)) {
        decay_mode = TString::Format("%iMuoX", n_muo);
        decay_mode_id = 14;
    } else if (n_muo > 0 and n_ele == 0) {
        decay_mode = TString::Format("%iMuo", n_muo);
        decay_mode_id = 1;
    } else if (n_muo > 0 and n_ele > 0) {
        decay_mode = TString::Format("%iEle%iMuo", n_ele, n_muo);
        decay_mode_id = 14;
    } else {
        TString pi_zero_part = "";
        if (n_pizero > 0) {
            pi_zero_part = TString::Format("%iPi0", n_pizero);
        }
        TString pi_plus_part = "";
        if (n_piplus > 0) {
            pi_plus_part = TString::Format("%iPi", n_piplus);
        }
        TString K_zero_part = "";
        if (n_Kzero > 0) {
            K_zero_part = TString::Format("%iK0", n_Kzero);
        }
        TString K_plus_part = "";
        if (n_Kplus > 0) {
            K_plus_part = TString::Format("%iK", n_Kplus);
        }
        decay_mode = (pi_plus_part + pi_zero_part + K_zero_part + K_plus_part);
        if (n_piplus + n_Kplus == 1) {
            if (n_pizero + n_Kzero == 0) {
                decay_mode_id = 2;
            } else if (n_pizero + n_Kzero == 1) {
                decay_mode_id = 3;
            } else if (n_pizero + n_Kzero == 2) {
                decay_mode_id = 4;
            } else {
                decay_mode_id = 5;
            }
        } else if (n_piplus + n_Kplus == 3) {
            if (n_pizero + n_Kzero == 0) {
                decay_mode_id = 6;
            } else if (n_pizero + n_Kzero == 1) {
                decay_mode_id = 7;
            } else if (n_pizero + n_Kzero == 2) {
                decay_mode_id = 8;
            } else {
                decay_mode_id = 9;
            }
        } else {
            if (n_pizero + n_Kzero == 0) {
                decay_mode_id = 10;
            } else if (n_pizero + n_Kzero == 1) {
                decay_mode_id = 11;
            } else if (n_pizero + n_Kzero == 2) {
                decay_mode_id = 12;
            } else {
                decay_mode_id = 13;
            }
        }
    }

    delete final_state_part_list;
    delete new_temp_part;
    delete temp_part;

    vis_tau_decay->setUserRecord("n_prong",  n_prong);
    vis_tau_decay->setUserRecord("decay_mode",  (std::string)decay_mode);
    vis_tau_decay->setUserRecord("decay_mode_id",  decay_mode_id);

    return vis_tau_decay;
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

double specialAna::DeltaR(pxl::Particle* lepton, pxl::Particle* met) {
    double d_eta = TMath::Abs(lepton->getEta() - met->getEta());
    double d_phi = DeltaPhi(lepton, met);
    return sqrt(pow(d_eta, 2) + pow(d_phi, 2));
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

void specialAna::raw_input(TString question) {
  TString answer;
  std::cout << question << std::endl;
  std::cin >> answer;
}

void specialAna::channel_writer(TFile* file, const char* channel) {
    file1->cd();
    file1->mkdir(channel);
    file1->cd(TString::Format("%s/", channel));
    HistClass::WriteAll(TString::Format("_%s_", channel), TString::Format("%s:_Cutflow", channel), TString::Format("sys:N-1"));
    HistClass::WriteAllEff(TString::Format("%s_", channel));
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

void specialAna::Fill_overall_efficiencies() {
    HistClass::CreateEff("emu_RECO_vs_Mass", HistClass::ReturnHist("emu_RECO_vs_Mass_All"), HistClass::ReturnHist("emu_RECO_vs_Mass_Pass"));
    HistClass::CreateEff("emu_RECO_vs_Nvtx", HistClass::ReturnHist("emu_RECO_vs_Nvtx_All"), HistClass::ReturnHist("emu_RECO_vs_Nvtx_Pass"));
    HistClass::CreateEff("emu_Eff_vs_Mass", HistClass::ReturnHist("emu_Eff_vs_Mass_All"), HistClass::ReturnHist("emu_Eff_vs_Mass_Pass"));
    HistClass::CreateEff("emu_Eff_vs_Nvtx", HistClass::ReturnHist("emu_Eff_vs_Nvtx_All"), HistClass::ReturnHist("emu_Eff_vs_Nvtx_Pass"));

    HistClass::CreateEff("etau_RECO_vs_Mass", HistClass::ReturnHist("etau_RECO_vs_Mass_All"), HistClass::ReturnHist("etau_RECO_vs_Mass_Pass"));
    HistClass::CreateEff("etau_RECO_vs_Nvtx", HistClass::ReturnHist("etau_RECO_vs_Nvtx_All"), HistClass::ReturnHist("etau_RECO_vs_Nvtx_Pass"));
    HistClass::CreateEff("etau_Eff_vs_Mass", HistClass::ReturnHist("etau_Eff_vs_Mass_All"), HistClass::ReturnHist("etau_Eff_vs_Mass_Pass"));
    HistClass::CreateEff("etau_Eff_vs_Nvtx", HistClass::ReturnHist("etau_Eff_vs_Nvtx_All"), HistClass::ReturnHist("etau_Eff_vs_Nvtx_Pass"));

    HistClass::CreateEff("mutau_RECO_vs_Mass", HistClass::ReturnHist("mutau_RECO_vs_Mass_All"), HistClass::ReturnHist("mutau_RECO_vs_Mass_Pass"));
    HistClass::CreateEff("mutau_RECO_vs_Nvtx", HistClass::ReturnHist("mutau_RECO_vs_Nvtx_All"), HistClass::ReturnHist("mutau_RECO_vs_Nvtx_Pass"));
    HistClass::CreateEff("mutau_Eff_vs_Mass", HistClass::ReturnHist("mutau_Eff_vs_Mass_All"), HistClass::ReturnHist("mutau_Eff_vs_Mass_Pass"));
    HistClass::CreateEff("mutau_Eff_vs_Nvtx", HistClass::ReturnHist("mutau_Eff_vs_Nvtx_All"), HistClass::ReturnHist("mutau_Eff_vs_Nvtx_Pass"));

    HistClass::CreateEff("etaue_RECO_vs_Mass", HistClass::ReturnHist("etaue_RECO_vs_Mass_All"), HistClass::ReturnHist("etaue_RECO_vs_Mass_Pass"));
    HistClass::CreateEff("etaue_RECO_vs_Nvtx", HistClass::ReturnHist("etaue_RECO_vs_Nvtx_All"), HistClass::ReturnHist("etaue_RECO_vs_Nvtx_Pass"));
    HistClass::CreateEff("etaue_Eff_vs_Mass", HistClass::ReturnHist("etaue_Eff_vs_Mass_All"), HistClass::ReturnHist("etaue_Eff_vs_Mass_Pass"));
    HistClass::CreateEff("etaue_Eff_vs_Nvtx", HistClass::ReturnHist("etaue_Eff_vs_Nvtx_All"), HistClass::ReturnHist("etaue_Eff_vs_Nvtx_Pass"));

    HistClass::CreateEff("etaumu_RECO_vs_Mass", HistClass::ReturnHist("etaumu_RECO_vs_Mass_All"), HistClass::ReturnHist("etaumu_RECO_vs_Mass_Pass"));
    HistClass::CreateEff("etaumu_RECO_vs_Nvtx", HistClass::ReturnHist("etaumu_RECO_vs_Nvtx_All"), HistClass::ReturnHist("etaumu_RECO_vs_Nvtx_Pass"));
    HistClass::CreateEff("etaumu_Eff_vs_Mass", HistClass::ReturnHist("etaumu_Eff_vs_Mass_All"), HistClass::ReturnHist("etaumu_Eff_vs_Mass_Pass"));
    HistClass::CreateEff("etaumu_Eff_vs_Nvtx", HistClass::ReturnHist("etaumu_Eff_vs_Nvtx_All"), HistClass::ReturnHist("etaumu_Eff_vs_Nvtx_Pass"));

    HistClass::CreateEff("mutaue_RECO_vs_Mass", HistClass::ReturnHist("mutaue_RECO_vs_Mass_All"), HistClass::ReturnHist("mutaue_RECO_vs_Mass_Pass"));
    HistClass::CreateEff("mutaue_RECO_vs_Nvtx", HistClass::ReturnHist("mutaue_RECO_vs_Nvtx_All"), HistClass::ReturnHist("mutaue_RECO_vs_Nvtx_Pass"));
    HistClass::CreateEff("mutaue_Eff_vs_Mass", HistClass::ReturnHist("mutaue_Eff_vs_Mass_All"), HistClass::ReturnHist("mutaue_Eff_vs_Mass_Pass"));
    HistClass::CreateEff("mutaue_Eff_vs_Nvtx", HistClass::ReturnHist("mutaue_Eff_vs_Nvtx_All"), HistClass::ReturnHist("mutaue_Eff_vs_Nvtx_Pass"));

    HistClass::CreateEff("mutaumu_RECO_vs_Mass", HistClass::ReturnHist("mutaumu_RECO_vs_Mass_All"), HistClass::ReturnHist("mutaumu_RECO_vs_Mass_Pass"));
    HistClass::CreateEff("mutaumu_RECO_vs_Nvtx", HistClass::ReturnHist("mutaumu_RECO_vs_Nvtx_All"), HistClass::ReturnHist("mutaumu_RECO_vs_Nvtx_Pass"));
    HistClass::CreateEff("mutaumu_Eff_vs_Mass", HistClass::ReturnHist("mutaumu_Eff_vs_Mass_All"), HistClass::ReturnHist("mutaumu_Eff_vs_Mass_Pass"));
    HistClass::CreateEff("mutaumu_Eff_vs_Nvtx", HistClass::ReturnHist("mutaumu_Eff_vs_Nvtx_All"), HistClass::ReturnHist("mutaumu_Eff_vs_Nvtx_Pass"));

}

void specialAna::endJob(const Serializable*) {
    Fill_overall_efficiencies();

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
        file1->mkdir("HLT_Effs");
        file1->cd("HLT_Effs/");
        HistClass::WriteAllEff("HLT");
    }
    file1->cd();
    file1->mkdir("ID_Effs");
    file1->cd("ID_Effs/");
    HistClass::WriteAllEff("ID");
    HistClass::WriteAll2("ID");
    file1->cd();
    file1->mkdir("RECO_Effs");
    file1->cd("RECO_Effs/");
    HistClass::WriteAllEff("RECO");
    HistClass::WriteAll2("RECO");
    HistClass::WriteAll("RECO");
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

    HistClass::CleanUp();

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
        sel_met["emu"] = METList->at(0);
        sel_met["etau"] = METList->at(0);
        sel_met["mutau"] = METList->at(0);
        sel_met["etaue"] = METList->at(0);
        sel_met["etaumu"] = METList->at(0);
        sel_met["mutaue"] = METList->at(0);
        sel_met["mutaumu"] = METList->at(0);
    } else {
        sel_met["emu"] = 0;
        sel_met["etau"] = 0;
        sel_met["mutau"] = 0;
        sel_met["etaue"] = 0;
        sel_met["etaumu"] = 0;
        sel_met["mutaue"] = 0;
        sel_met["mutaumu"] = 0;
    }

    sel_lepton_prompt["emu"] = 0;
    sel_lepton_prompt["etau"] = 0;
    sel_lepton_prompt["mutau"] = 0;
    sel_lepton_prompt["etaue"] = 0;
    sel_lepton_prompt["etaumu"] = 0;
    sel_lepton_prompt["mutaue"] = 0;
    sel_lepton_prompt["mutaumu"] = 0;

    sel_lepton_nprompt["emu"] = 0;
    sel_lepton_nprompt["etau"] = 0;
    sel_lepton_nprompt["mutau"] = 0;
    sel_lepton_nprompt["etaue"] = 0;
    sel_lepton_nprompt["etaumu"] = 0;
    sel_lepton_nprompt["mutaue"] = 0;
    sel_lepton_nprompt["mutaumu"] = 0;

    sel_lepton_nprompt_corr["emu"] = 0;
    sel_lepton_nprompt_corr["etau"] = 0;
    sel_lepton_nprompt_corr["mutau"] = 0;
    sel_lepton_nprompt_corr["etaue"] = 0;
    sel_lepton_nprompt_corr["etaumu"] = 0;
    sel_lepton_nprompt_corr["mutaue"] = 0;
    sel_lepton_nprompt_corr["mutaumu"] = 0;

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
    TauVisListGen  = new std::vector< pxl::Particle* >;
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
            if (Name == "Muon") {
                MuonListGen->push_back(part);
            } else if (Name == "Ele") {
                EleListGen->push_back(part);
            } else if (Name == "Gamma") {
                GammaListGen->push_back(part);
            } else if (Name == "Tau") {
                TauListGen->push_back(part);
            } else if (Name == (m_METType+"_gen") ) {
                METListGen->push_back(part);
            } else if (Name == m_JetAlgo) {
                JetListGen->push_back(part);
            } else if (Name == genCollection) {
                S3ListGen->push_back(part);
                if (part->getPdgNumber() == 15 or part->getPdgNumber() == -15) {
                    TauVisListGen->push_back(Get_tau_truth_decay_mode(*m_GenEvtView, part));
                }
            }
        }
    }
}

void specialAna::endEvent(const pxl::Event* event) {
    // for(auto const &it1 : sel_lepton_prompt) {
        // if (it1.second != 0) {
            // delete it1.second;
            // it1.second = 0;
        // }
    // }

    // if (sel_lepton_prompt != 0) {
        // delete sel_lepton_prompt;
        // sel_lepton_prompt = 0;
    // }
    // if (sel_lepton_nprompt != 0) {
        // delete sel_lepton_nprompt;
        // sel_lepton_nprompt = 0;
    // }
    // if (sel_lepton_nprompt_corr != 0) {
        // delete sel_lepton_nprompt_corr;
        // sel_lepton_nprompt_corr = 0;
    // }
    // if (sel_lepton_nprompt_corr != 0) {
        // delete sel_lepton_nprompt_corr;
        // sel_lepton_nprompt_corr = 0;
    // }
// 
    // if (sel_part1_gen != 0) {
        // delete sel_part1_gen;
        // sel_part1_gen = 0;
    // }
    // if (sel_part2_gen != 0) {
        // delete sel_part2_gen;
        // sel_part2_gen = 0;
    // }

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
        delete S3ListGen;

        for (uint i = 0; i < TauVisListGen->size(); i++) {
            delete TauVisListGen->at(i);
        }
        delete TauVisListGen;

        TauVisListGen = 0;
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
