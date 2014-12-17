#include "specialAna.hh"
#include "HistClass.hh"
#include "Tools/Tools.hh"
#include "boost/format.hpp"

specialAna::specialAna( const Tools::MConfig &cfg ) :
    runOnData(       cfg.GetItem< bool >( "General.RunOnData" ) ),
    m_JetAlgo(       cfg.GetItem< string >( "Jet.Type.Rec" ) ),
    m_BJets_algo(    cfg.GetItem< string >( "Jet.BJets.Algo" ) ),
    m_METType(       cfg.GetItem< string >( "MET.Type.Rec" ) ),
    m_TauType(       cfg.GetItem< string >( "Tau.Type.Rec" ) ),

    m_trigger_string( Tools::splitString< string >( cfg.GetItem< string >( "RPV.trigger_list" ), true  ) ),
    d_mydiscmu(  {"isPFMuon","isGlobalMuon","isTrackerMuon","isStandAloneMuon","isTightMuon","isHighPtMuon"} ),
    m_dataPeriod(    cfg.GetItem< string >( "General.DataPeriod" )),
    m_channel(       cfg.GetItem< string >( "RPV.channel" )),
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

    string safeFileName = "SpecialHistos.root";
    file1 = new TFile(safeFileName.c_str(), "RECREATE");
    events_ = 0;

    // number of events, saved in a histogram
    HistClass::CreateHistoUnchangedName("h_counters", 10, 0, 11, "N_{events}");

    for(unsigned int i=0;i<4;i++){
        //str(boost::format("N_{%s}")%particleLatex[i] )
        HistClass::CreateHisto("num",particles[i].c_str(), 40, 0, 39,            TString::Format("N_{%s}", particleSymbols[i].c_str()) );
        HistClass::CreateHisto(3,"pt",particles[i].c_str(), 5000, 0, 5000,       TString::Format("p_{T}^{%s} (GeV)", particleSymbols[i].c_str()) );
        HistClass::CreateHisto(3,"eta",particles[i].c_str(), 80, -4, 4,          TString::Format("#eta_{%s}", particleSymbols[i].c_str()) );
        HistClass::CreateHisto(3,"phi",particles[i].c_str(), 40, -3.2, 3.2,      TString::Format("#phi_{%s} (rad)", particleSymbols[i].c_str()) );

        if(not runOnData){
            HistClass::CreateHisto(1,"num_Gen",particles[i].c_str(), 40, 0, 39,        TString::Format("N_{%s}", particleSymbols[i].c_str()) );
            HistClass::CreateHisto(1,"pt_Gen",particles[i].c_str(), 5000, 0, 5000,     TString::Format("p_{T}^{%s} (GeV)", particleSymbols[i].c_str()) );
            HistClass::CreateHisto(1,"eta_Gen",particles[i].c_str(), 80, -4, 4,        TString::Format("#eta_{%s}", particleSymbols[i].c_str()) );
            HistClass::CreateHisto(1,"phi_Gen",particles[i].c_str(), 40, -3.2, 3.2,    TString::Format("#phi_{%s} (rad)", particleSymbols[i].c_str()) );
        }
    }

    HistClass::CreateHisto("LLE_Gen",100,0,1,"LLE");
    HistClass::CreateHisto("LQD_Gen",100,0,0.001,"LQD");
    HistClass::CreateHisto("MSnl_Gen",4000,0,4000,"MSnl");

    if(not runOnData) {
        Create_Gen_histograms("emu", "ele", "muo");
        Create_Gen_histograms("etau", "ele", "tau");
        Create_Gen_histograms("mutau", "muo", "tau");
        Create_Gen_histograms("etaue", "ele", "tau_ele");
        Create_Gen_histograms("etaumu", "ele", "tau_muo");
        Create_Gen_histograms("mutaue", "muo", "tau_ele");
        Create_Gen_histograms("mutaumu", "muo", "tau_muo");
    }

    channel_stages["emu"] = 1;

    Create_Resonance_histograms(channel_stages["emu"], "emu", "ele", "muo");
    Create_Resonance_histograms(channel_stages["emu"], "emu", "ele", "muo","_Ele_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["emu"], "emu", "ele", "muo","_Ele_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["emu"], "emu", "ele", "muo","_Tau_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["emu"], "emu", "ele", "muo","_Tau_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["emu"], "emu", "ele", "muo","_Muon_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["emu"], "emu", "ele", "muo","_Muon_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["emu"], "emu", "ele", "muo","_Muon_syst_ResolutionUp");
    Create_Resonance_histograms(channel_stages["emu"], "emu", "ele", "muo","_Muon_syst_ResolutionDown");
    Init_emu_cuts();
    Create_N1_histos("emu", emu_cut_cfgs);
    Create_N1_histos("emu", emu_cut_cfgs,"_Ele_syst_ScaleUp");
    Create_N1_histos("emu", emu_cut_cfgs,"_Ele_syst_ScaleDown");
    Create_N1_histos("emu", emu_cut_cfgs,"_Tau_syst_ScaleUp");
    Create_N1_histos("emu", emu_cut_cfgs,"_Tau_syst_ScaleDown");
    Create_N1_histos("emu", emu_cut_cfgs,"_Muon_syst_ScaleUp");
    Create_N1_histos("emu", emu_cut_cfgs,"_Muon_syst_ScaleDown");
    Create_N1_histos("emu", emu_cut_cfgs,"_Muon_syst_ResolutionUp");
    Create_N1_histos("emu", emu_cut_cfgs,"_Muon_syst_ResolutionDown");

    channel_stages["etau"] = 1;

    Create_Resonance_histograms(channel_stages["etau"], "etau", "ele", "tau");
    Create_Resonance_histograms(channel_stages["etau"], "etau", "ele", "tau","_Ele_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["etau"], "etau", "ele", "tau","_Ele_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["etau"], "etau", "ele", "tau","_Tau_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["etau"], "etau", "ele", "tau","_Tau_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["etau"], "etau", "ele", "tau","_Muon_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["etau"], "etau", "ele", "tau","_Muon_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["etau"], "etau", "ele", "tau","_Muon_syst_ResolutionUp");
    Create_Resonance_histograms(channel_stages["etau"], "etau", "ele", "tau","_Muon_syst_ResolutionDown");
    Init_etau_cuts();
    Create_N1_histos("etau", etau_cut_cfgs);
    Create_N1_histos("etau", etau_cut_cfgs,"_Ele_syst_ScaleUp");
    Create_N1_histos("etau", etau_cut_cfgs,"_Ele_syst_ScaleDown");
    Create_N1_histos("etau", etau_cut_cfgs,"_Tau_syst_ScaleUp");
    Create_N1_histos("etau", etau_cut_cfgs,"_Tau_syst_ScaleDown");
    Create_N1_histos("etau", etau_cut_cfgs,"_Muon_syst_ScaleUp");
    Create_N1_histos("etau", etau_cut_cfgs,"_Muon_syst_ScaleDown");
    Create_N1_histos("etau", etau_cut_cfgs,"_Muon_syst_ResolutionUp");
    Create_N1_histos("etau", etau_cut_cfgs,"_Muon_syst_ResolutionDown");

    channel_stages["mutau"] = 7;

    Create_Resonance_histograms(channel_stages["mutau"], "mutau", "muo", "tau");
    Create_Resonance_histograms(channel_stages["mutau"], "mutau", "muo", "tau","_Ele_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["mutau"], "mutau", "muo", "tau","_Ele_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["mutau"], "mutau", "muo", "tau","_Tau_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["mutau"], "mutau", "muo", "tau","_Tau_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["mutau"], "mutau", "muo", "tau","_Muon_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["mutau"], "mutau", "muo", "tau","_Muon_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["mutau"], "mutau", "muo", "tau","_Muon_syst_ResolutionUp");
    Create_Resonance_histograms(channel_stages["mutau"], "mutau", "muo", "tau","_Muon_syst_ResolutionDown");
    Init_mutau_cuts();
    Create_N1_histos("mutau", mutau_cut_cfgs);
    Create_N1_histos("mutau", mutau_cut_cfgs,"_Ele_syst_ScaleUp");
    Create_N1_histos("mutau", mutau_cut_cfgs,"_Ele_syst_ScaleDown");
    Create_N1_histos("mutau", mutau_cut_cfgs,"_Tau_syst_ScaleUp");
    Create_N1_histos("mutau", mutau_cut_cfgs,"_Tau_syst_ScaleDown");
    Create_N1_histos("mutau", mutau_cut_cfgs,"_Muon_syst_ScaleUp");
    Create_N1_histos("mutau", mutau_cut_cfgs,"_Muon_syst_ScaleDown");
    Create_N1_histos("mutau", mutau_cut_cfgs,"_Muon_syst_ResolutionUp");
    Create_N1_histos("mutau", mutau_cut_cfgs,"_Muon_syst_ResolutionDown");

    channel_stages["etaue"] = 1;

    Create_Resonance_histograms(channel_stages["etaue"], "etaue", "ele", "tau_ele");
    Create_Resonance_histograms(channel_stages["etaue"], "etaue", "ele", "tau_ele","_Ele_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["etaue"], "etaue", "ele", "tau_ele","_Ele_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["etaue"], "etaue", "ele", "tau_ele","_Tau_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["etaue"], "etaue", "ele", "tau_ele","_Tau_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["etaue"], "etaue", "ele", "tau_ele","_Muon_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["etaue"], "etaue", "ele", "tau_ele","_Muon_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["etaue"], "etaue", "ele", "tau_ele","_Muon_syst_ResolutionUp");
    Create_Resonance_histograms(channel_stages["etaue"], "etaue", "ele", "tau_ele","_Muon_syst_ResolutionDown");
    Init_etaue_cuts();
    Create_N1_histos("etaue", etaue_cut_cfgs);
    Create_N1_histos("etaue", etaue_cut_cfgs,"_Ele_syst_ScaleUp");
    Create_N1_histos("etaue", etaue_cut_cfgs,"_Ele_syst_ScaleDown");
    Create_N1_histos("etaue", etaue_cut_cfgs,"_Tau_syst_ScaleUp");
    Create_N1_histos("etaue", etaue_cut_cfgs,"_Tau_syst_ScaleDown");
    Create_N1_histos("etaue", etaue_cut_cfgs,"_Muon_syst_ScaleUp");
    Create_N1_histos("etaue", etaue_cut_cfgs,"_Muon_syst_ScaleDown");
    Create_N1_histos("etaue", etaue_cut_cfgs,"_Muon_syst_ResolutionUp");
    Create_N1_histos("etaue", etaue_cut_cfgs,"_Muon_syst_ResolutionDown");

    channel_stages["etaumu"] = 1;

    Create_Resonance_histograms(channel_stages["etaumu"], "etaumu", "ele", "tau_muo");
    Create_Resonance_histograms(channel_stages["etaumu"], "etaumu", "ele", "tau_muo","_Ele_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["etaumu"], "etaumu", "ele", "tau_muo","_Ele_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["etaumu"], "etaumu", "ele", "tau_muo","_Tau_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["etaumu"], "etaumu", "ele", "tau_muo","_Tau_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["etaumu"], "etaumu", "ele", "tau_muo","_Muon_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["etaumu"], "etaumu", "ele", "tau_muo","_Muon_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["etaumu"], "etaumu", "ele", "tau_muo","_Muon_syst_ResolutionUp");
    Create_Resonance_histograms(channel_stages["etaumu"], "etaumu", "ele", "tau_muo","_Muon_syst_ResolutionDown");
    Init_etaumu_cuts();
    Create_N1_histos("etaumu", etaumu_cut_cfgs);
    Create_N1_histos("etaumu", etaumu_cut_cfgs,"_Ele_syst_ScaleUp");
    Create_N1_histos("etaumu", etaumu_cut_cfgs,"_Ele_syst_ScaleDown");
    Create_N1_histos("etaumu", etaumu_cut_cfgs,"_Tau_syst_ScaleUp");
    Create_N1_histos("etaumu", etaumu_cut_cfgs,"_Tau_syst_ScaleDown");
    Create_N1_histos("etaumu", etaumu_cut_cfgs,"_Muon_syst_ScaleUp");
    Create_N1_histos("etaumu", etaumu_cut_cfgs,"_Muon_syst_ScaleDown");
    Create_N1_histos("etaumu", etaumu_cut_cfgs,"_Muon_syst_ResolutionUp");
    Create_N1_histos("etaumu", etaumu_cut_cfgs,"_Muon_syst_ResolutionDown");

    channel_stages["mutaue"] = 4;

    Create_Resonance_histograms(channel_stages["mutaue"], "mutaue", "muo", "tau_ele");
    Create_Resonance_histograms(channel_stages["mutaue"], "mutaue", "muo", "tau_ele","_Ele_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["mutaue"], "mutaue", "muo", "tau_ele","_Ele_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["mutaue"], "mutaue", "muo", "tau_ele","_Tau_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["mutaue"], "mutaue", "muo", "tau_ele","_Tau_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["mutaue"], "mutaue", "muo", "tau_ele","_Muon_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["mutaue"], "mutaue", "muo", "tau_ele","_Muon_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["mutaue"], "mutaue", "muo", "tau_ele","_Muon_syst_ResolutionUp");
    Create_Resonance_histograms(channel_stages["mutaue"], "mutaue", "muo", "tau_ele","_Muon_syst_ResolutionDown");
    Init_mutaue_cuts();
    Create_N1_histos("mutaue", mutaue_cut_cfgs);
    Create_N1_histos("mutaue", mutaue_cut_cfgs,"_Ele_syst_ScaleUp");
    Create_N1_histos("mutaue", mutaue_cut_cfgs,"_Ele_syst_ScaleDown");
    Create_N1_histos("mutaue", mutaue_cut_cfgs,"_Tau_syst_ScaleUp");
    Create_N1_histos("mutaue", mutaue_cut_cfgs,"_Tau_syst_ScaleDown");
    Create_N1_histos("mutaue", mutaue_cut_cfgs,"_Muon_syst_ScaleUp");
    Create_N1_histos("mutaue", mutaue_cut_cfgs,"_Muon_syst_ScaleDown");
    Create_N1_histos("mutaue", mutaue_cut_cfgs,"_Muon_syst_ResolutionUp");
    Create_N1_histos("mutaue", mutaue_cut_cfgs,"_Muon_syst_ResolutionDown");

    channel_stages["mutaumu"] = 1;

    Create_Resonance_histograms(channel_stages["mutaumu"], "mutaumu", "muo", "tau_muo");
    Create_Resonance_histograms(channel_stages["mutaumu"], "mutaumu", "muo", "tau_muo","_Ele_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["mutaumu"], "mutaumu", "muo", "tau_muo","_Ele_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["mutaumu"], "mutaumu", "muo", "tau_muo","_Tau_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["mutaumu"], "mutaumu", "muo", "tau_muo","_Tau_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["mutaumu"], "mutaumu", "muo", "tau_muo","_Muon_syst_ScaleUp");
    Create_Resonance_histograms(channel_stages["mutaumu"], "mutaumu", "muo", "tau_muo","_Muon_syst_ScaleDown");
    Create_Resonance_histograms(channel_stages["mutaumu"], "mutaumu", "muo", "tau_muo","_Muon_syst_ResolutionUp");
    Create_Resonance_histograms(channel_stages["mutaumu"], "mutaumu", "muo", "tau_muo","_Muon_syst_ResolutionDown");
    Init_mutaumu_cuts();
    Create_N1_histos("mutaumu", mutaumu_cut_cfgs);
    Create_N1_histos("mutaumu", mutaumu_cut_cfgs,"_Ele_syst_ScaleUp");
    Create_N1_histos("mutaumu", mutaumu_cut_cfgs,"_Ele_syst_ScaleDown");
    Create_N1_histos("mutaumu", mutaumu_cut_cfgs,"_Tau_syst_ScaleUp");
    Create_N1_histos("mutaumu", mutaumu_cut_cfgs,"_Tau_syst_ScaleDown");
    Create_N1_histos("mutaumu", mutaumu_cut_cfgs,"_Muon_syst_ScaleUp");
    Create_N1_histos("mutaumu", mutaumu_cut_cfgs,"_Muon_syst_ScaleDown");
    Create_N1_histos("mutaumu", mutaumu_cut_cfgs,"_Muon_syst_ResolutionUp");
    Create_N1_histos("mutaumu", mutaumu_cut_cfgs,"_Muon_syst_ResolutionDown");
}

specialAna::~specialAna() {
}

void specialAna::analyseEvent( const pxl::Event* event ) {
    initEvent( event );

    if(tail_selector(event)) return;
    
    if(not runOnData){
        Fill_Gen_Controll_histo();
    }

    GenSelector();

    for(uint i = 0; i < MuonList->size(); i++){
        if(MuonList->at(i)->getPt() < 25 or TMath::Abs(MuonList->at(i)->getEta()) > 2.1)continue;
        Fill_Particle_histos(0, MuonList->at(i));
    }
    HistClass::Fill("Muon_num",MuonList->size(),weight);

    for(uint i = 0; i < EleList->size(); i++){
        if(EleList->at(i)->getPt() < 25 or TMath::Abs(EleList->at(i)->getEta()) > 2.5 or (TMath::Abs(EleList->at(i)->getEta()) > 1.442 and TMath::Abs(EleList->at(i)->getEta()) < 1.56))continue;
        Fill_Particle_histos(0, EleList->at(i));
    }
    HistClass::Fill("Ele_num",EleList->size(),weight);

    for(uint i = 0; i < TauList->size(); i++){
        Fill_Particle_histos(0, TauList->at(i));
    }
    HistClass::Fill("Tau_num",TauList->size(),weight);

    for(uint i = 0; i < METList->size(); i++){
        Fill_Particle_histos(0, METList->at(i));
    }
    HistClass::Fill("MET_num",METList->size(),weight);

    if (!TriggerSelector(event)) return;

    for(uint i = 0; i < MuonList->size(); i++){
        if(MuonList->at(i)->getPt() < 25 or TMath::Abs(MuonList->at(i)->getEta()) > 2.1)continue;
        Fill_Particle_histos(1, MuonList->at(i));
    }

    for(uint i = 0; i < EleList->size(); i++){
        if(EleList->at(i)->getPt() < 25 or TMath::Abs(EleList->at(i)->getEta()) > 2.5 or (TMath::Abs(EleList->at(i)->getEta()) > 1.442 and TMath::Abs(EleList->at(i)->getEta()) < 1.56))continue;
        Fill_Particle_histos(1, EleList->at(i));
    }

    for(uint i = 0; i < TauList->size(); i++){
        Fill_Particle_histos(1, TauList->at(i));
    }

    for(uint i = 0; i < METList->size(); i++){
        Fill_Particle_histos(1, METList->at(i));
    }

    for(uint i = 0; i < MuonList->size(); i++){
        if(MuonList->at(i)->getPt() < 25 or TMath::Abs(MuonList->at(i)->getEta()) > 2.1)continue;
        if(Check_Muo_ID(MuonList->at(i))){
            Fill_Particle_histos(2, MuonList->at(i));
        }
    }

    for(uint i = 0; i < EleList->size(); i++){
        if(EleList->at(i)->getPt() < 25 or TMath::Abs(EleList->at(i)->getEta()) > 2.5 or (TMath::Abs(EleList->at(i)->getEta()) > 1.442 and TMath::Abs(EleList->at(i)->getEta()) < 1.56))continue;
        if(EleList->at(i)->getUserRecord("IDpassed").asBool()){
            Fill_Particle_histos(2, EleList->at(i));
        }
    }

    for(uint i = 0; i < TauList->size(); i++){
        if(Check_Tau_ID(TauList->at(i))){
            Fill_Particle_histos(2, TauList->at(i));
        }
    }

    for(uint i = 0; i < METList->size(); i++){
        Fill_Particle_histos(2, METList->at(i));
    }

    KinematicsSelector();

    FillSystematics(event, "Ele");

    FillSystematics(event, "Muon");
    FillSystematicsUpDown(event, "Muon", "Up", "Resolution");
    FillSystematicsUpDown(event, "Muon", "Down", "Resolution");

    FillSystematics(event, "Tau");

    endEvent( event );
}

bool specialAna::tail_selector(const pxl::Event* event) {
    std::string datastream = event->getUserRecord("Dataset").asString();
    TString Datastream = datastream;

    double cut_w_mass = 0;
    double cut_w_pt = 0;
    if(Datastream.Contains("WJetsToLNu") || Datastream.Contains("WTo")){
        for(uint i = 0; i < S3ListGen->size(); i++){
            if(TMath::Abs(S3ListGen->at(i) -> getPdgNumber()) == 24){
                if(S3ListGen->at(i)->getMass() > cut_w_mass){
                    cut_w_mass = S3ListGen->at(i)->getMass();
                    cut_w_pt = S3ListGen->at(i)->getPt();
                }
            }
        }
    }

    if(Datastream.Contains("WJetsToLNu_TuneZ2Star_8TeV")) {
        if(cut_w_pt > 55)return true;
    }

    if(Datastream.Contains("WJetsToLNu_PtW")) {
        if(cut_w_pt <= 55)return true;
    }

    if(Datastream.Contains("WJetsToLNu")) {
        if(cut_w_mass > 300)return true;
    }

    if(Datastream.Contains("WTo")) {
        if(cut_w_mass <= 300)return true;
    }

    /// Diboson tail fitting
    if(Datastream.Contains("WW_") || Datastream.Contains("WZ_") || Datastream.Contains("ZZ_")) {
        for(uint i = 0; i < S3ListGen->size(); i++){
            int part_id = TMath::Abs(S3ListGen->at(i) -> getPdgNumber());
            if(part_id == 23 || part_id == 22){
                if(S3ListGen->at(i)->getPt() > 500)return true;
            }
        }
    }

    /// ttbar tail fitting
    if(Datastream.Contains("TT_CT10_powheg") && !Datastream.Contains("Mtt")) {
        for(uint i = 0; i < S3ListGen->size(); i++){
            if(S3ListGen->at(i) -> getPdgNumber() == 6){
                for(uint j = 0; j < S3ListGen->size(); j++){
                    if(S3ListGen->at(j) -> getPdgNumber() == -6){
                        double mass = (S3ListGen->at(j)->getE() + S3ListGen->at(i)->getE())  *(S3ListGen->at(j)->getE() + S3ListGen->at(i)->getE())
                                                    - (S3ListGen->at(j)->getPx() + S3ListGen->at(i)->getPx())*(S3ListGen->at(j)->getPx() + S3ListGen->at(i)->getPx())
                                                    - (S3ListGen->at(j)->getPy() + S3ListGen->at(i)->getPy())*(S3ListGen->at(j)->getPy() + S3ListGen->at(i)->getPy())
                                                    - (S3ListGen->at(j)->getPz() + S3ListGen->at(i)->getPz())*(S3ListGen->at(j)->getPz() + S3ListGen->at(i)->getPz());
                        if(mass > 700)return true;
                    }
                }
            }
        }
    }

    /// Signal parameter selection
    if(Datastream.Contains("RPVresonantToMuTau")){
        HistClass::Fill("LLE_Gen",m_GenEvtView->getUserRecord( "MC_LLE" ).asDouble(),weight);
        HistClass::Fill("LQD_Gen",m_GenEvtView->getUserRecord( "MC_LQD" ).asDouble(),weight);
        HistClass::Fill("MSnl_Gen",m_GenEvtView->getUserRecord( "MC_MSnl" ).asDouble(),weight);
        double gen_mass = m_GenEvtView->getUserRecord( "MC_MSnl").asDouble();
        double gen_coupling = m_GenEvtView->getUserRecord( "MC_LLE").asDouble();
        if(!(gen_mass > config_.GetItem< double >( "rpv_mass.min" ) && gen_mass < config_.GetItem< double >( "rpv_mass.max" )))return true;
        if(!(gen_coupling > config_.GetItem< double >( "rpv_coupling.min" ) && gen_coupling < config_.GetItem< double >( "rpv_coupling.max" )))return true;
    }

    return false;
}

void specialAna::FillSystematics(const pxl::Event* event, std::string const particleName){
    FillSystematicsUpDown(event, particleName, "Up", "Scale");
    FillSystematicsUpDown(event, particleName, "Down", "Scale");
    //FillSystematicsUpDown(event, particleName, "Up", "Resolution");
    //FillSystematicsUpDown(event, particleName, "Down", "Resolution");
}

void specialAna::FillSystematicsUpDown(const pxl::Event* event, std::string const particleName, std::string const updown, std::string const shiftType){
    pxl::EventView *tempEventView;

    /// extract one EventView
    /// make sure the object key is the same as in Systematics.cc specified
    tempEventView = event->findObject< pxl::EventView >(particleName + "_syst" + shiftType + updown);

    if(tempEventView == 0){
        throw std::runtime_error("specialAna.cc: no EventView '" + particleName + "_syst" + shiftType + updown + "' found!");
    }
    /// get all particles
    std::vector< pxl::Particle* > shiftedParticles;
    tempEventView->getObjectsOfType< pxl::Particle >(shiftedParticles);

    /// backup OldList
    RememberMET=METList;
    METList = new vector< pxl::Particle* >;
    if(particleName=="Muon"){
        RememberPart=MuonList;
        MuonList = new vector< pxl::Particle* >;
        for( vector< pxl::Particle* >::const_iterator part_it = shiftedParticles.begin(); part_it != shiftedParticles.end(); ++part_it ) {
            pxl::Particle *part = *part_it;
            string Name = part->getName();
            if(      Name == "Muon"    ) MuonList->push_back( part );
            else if( Name == m_METType ) METList->push_back( part );
        }
    }else if(particleName=="Ele"){
        RememberPart=EleList;
        EleList = new vector< pxl::Particle* >;
        for( vector< pxl::Particle* >::const_iterator part_it = shiftedParticles.begin(); part_it != shiftedParticles.end(); ++part_it ) {
            pxl::Particle *part = *part_it;
            string Name = part->getName();
            if(      Name == "Ele"     ) EleList->push_back( part );
            else if( Name == m_METType ) METList->push_back( part );
        }
    }else if(particleName=="Tau"){
        RememberPart=TauList;
        TauList = new vector< pxl::Particle* >;
        for( vector< pxl::Particle* >::const_iterator part_it = shiftedParticles.begin(); part_it != shiftedParticles.end(); ++part_it ) {
            pxl::Particle *part = *part_it;
            string Name = part->getName();
            if(      Name == m_TauType ) TauList->push_back( part );
            else if( Name == m_METType ) METList->push_back( part );
        }
    }//else if(particleName=="JET"){
    //}else if(particleName==m_METType){}

    /// reset the chosen MET and lepton
    if(METList->size()>0){
        sel_met=METList->at(0);
    }else{
        sel_met=0;
    }
    sel_lepton_prompt = 0;
    sel_lepton_nprompt = 0;
    sel_lepton_nprompt_corr = 0;

    resonance_mass = 0;

    KinematicsSelector("_" + particleName + "_syst_" + shiftType + updown);

    /// return to backup
    delete METList;
    METList = RememberMET;
    if(particleName=="Muon"){
        delete MuonList;
        MuonList = RememberPart;
    }else if(particleName=="Ele"){
        delete EleList;
        EleList = RememberPart;
    }else if(particleName=="Tau"){
        delete TauList;
        TauList = RememberPart;
    }//else if(particleName=="JET"){
    //}else if(particleName==m_METType){}
}

void specialAna::Init_emu_cuts() {
    emu_cut_cfgs["kinematics"] = Cuts("kinematics",500,0,500);
}

void specialAna::Init_etau_cuts() {
    etau_cut_cfgs["kinematics"] = Cuts("kinematics",500,0,500);
}

void specialAna::Init_mutau_cuts() {
    mutau_cut_cfgs["kinematics"] = Cuts("kinematics",500,0,500);
    mutau_cut_cfgs["zeta"] = Cuts("zeta",500,0,500,500,0,500,"p_{#zeta} (GeV)","p_{#zeta}^{vis} (GeV)");
    mutau_cut_cfgs["DeltaPhi_tauMET"] = Cuts("DeltaPhi_tauMET",100,0,3.2);
    mutau_cut_cfgs["DeltaPhi_mutau"] = Cuts("DeltaPhi_mutau",100,0,3.2);
    mutau_cut_cfgs["BJet_veto"] = Cuts("BJet_veto",10,0,9);
    mutau_cut_cfgs["OppSign_charge"] = Cuts("OppSign_charge",5,-2,2);
    mutau_cut_cfgs["MT_cut"] = Cuts("MT_cut",5000,0,5000);
}

void specialAna::Init_etaue_cuts() {
    etaue_cut_cfgs["kinematics"] = Cuts("kinematics",500,0,500);
}

void specialAna::Init_etaumu_cuts() {
    etaumu_cut_cfgs["kinematics"] = Cuts("kinematics",500,0,500);
}

void specialAna::Init_mutaue_cuts() {
    mutaue_cut_cfgs["kinematics"] = Cuts("kinematics",500,0,500);
    mutaue_cut_cfgs["BJet_veto"] = Cuts("BJet_veto",10,0,9);
    mutaue_cut_cfgs["DeltaPhi_emu"] = Cuts("DeltaPhi_emu",100,0,3.2);
    mutaue_cut_cfgs["lep_fraction"] = Cuts("lep_fraction",100,0,5);
}

void specialAna::Init_mutaumu_cuts() {
    mutaumu_cut_cfgs["kinematics"] = Cuts("kinematics",500,0,500);
}

void specialAna::KinematicsSelector(std::string const endung) {
    /// Selection for the e-mu channel
    if(b_emu) {
        bool b_emu_success = false;
        if(FindResonance(*EleList, *MuonList)) {
            Fill_Resonance_histograms(0, "emu", "ele", "muo", endung);
            b_emu_success = true;
            emu_cut_cfgs["kinematics"].SetPassed(true);
            emu_cut_cfgs["kinematics"].SetVars(resonance_mass);
        }else{
            b_emu_success = false;
            emu_cut_cfgs["kinematics"].SetPassed(false);
            emu_cut_cfgs["kinematics"].SetVars(resonance_mass);
        }
        Fill_N1_histos("emu", emu_cut_cfgs, endung);
    }
    ///-----------------------------------------------------------------
    /// Selection for the e-tau_h channel
    if(b_etau) {
        bool b_etau_success = false;
        if(FindResonance(*EleList, *TauList, *METList)) {
            Fill_Resonance_histograms(0, "etau", "ele", "tau", endung);
            b_etau_success = true;
            etau_cut_cfgs["kinematics"].SetPassed(true);
            etau_cut_cfgs["kinematics"].SetVars(resonance_mass);
        }else{
            b_etau_success = false;
            etau_cut_cfgs["kinematics"].SetPassed(false);
            etau_cut_cfgs["kinematics"].SetVars(resonance_mass);
        }
        Fill_N1_histos("etau", etau_cut_cfgs, endung);
    }
    ///-----------------------------------------------------------------
    /// Selection for the muo-tau_h channel
    if(b_mutau) {
        bool b_mutau_success = false;
        /// Find the actual resonance
        if(FindResonance(*MuonList, *TauList, *METList)) {
            Fill_Resonance_histograms(0, "mutau", "muo", "tau", endung);
            b_mutau_success = true;
            mutau_cut_cfgs["kinematics"].SetPassed(true);
            mutau_cut_cfgs["kinematics"].SetVars(resonance_mass);
        }else{
            b_mutau_success = false;
            mutau_cut_cfgs["kinematics"].SetPassed(false);
            mutau_cut_cfgs["kinematics"].SetVars(resonance_mass);
        }
        /// Make the cut on zeta
        if(Make_zeta_cut(mutau_cut_cfgs["zeta"])) {
            if(b_mutau_success) {
                Fill_Resonance_histograms(1, "mutau", "muo", "tau", endung);
                b_mutau_success = true;
            }
            mutau_cut_cfgs["zeta"].SetPassed(true);
        }else{
            b_mutau_success = false;
            mutau_cut_cfgs["zeta"].SetPassed(false);
        }
        /// Make the cut on DeltaPhi(tau,MET)
        if(Make_DeltaPhi_tauMET(mutau_cut_cfgs["DeltaPhi_tauMET"])) {
            if(b_mutau_success) {
                Fill_Resonance_histograms(2, "mutau", "muo", "tau", endung);
                b_mutau_success = true;
            }
            mutau_cut_cfgs["DeltaPhi_tauMET"].SetPassed(true);
        }else{
            b_mutau_success = false;
            mutau_cut_cfgs["DeltaPhi_tauMET"].SetPassed(false);
        }
        /// Make the cut on DeltaPhi(mu,tau)
        if(Make_DeltaPhi_mutau(mutau_cut_cfgs["DeltaPhi_mutau"])) {
            if(b_mutau_success) {
                Fill_Resonance_histograms(3, "mutau", "muo", "tau", endung);
                b_mutau_success = true;
            }
            mutau_cut_cfgs["DeltaPhi_mutau"].SetPassed(true);
        }else{
            b_mutau_success = false;
            mutau_cut_cfgs["DeltaPhi_mutau"].SetPassed(false);
        }
        /// Make the b-jet veto
        if(Bjet_veto(mutau_cut_cfgs["BJet_veto"])) {
            if(b_mutau_success) {
                Fill_Resonance_histograms(4, "mutau", "muo", "tau", endung);
                b_mutau_success = true;
            }
            mutau_cut_cfgs["BJet_veto"].SetPassed(true);
        }else{
            b_mutau_success = false;
            mutau_cut_cfgs["BJet_veto"].SetPassed(false);
        }
        /// Make the same-sign charge cut
        if(OppSign_charge(mutau_cut_cfgs["OppSign_charge"])) {
            if(b_mutau_success) {
                Fill_Resonance_histograms(5, "mutau", "muo", "tau", endung);
                b_mutau_success = true;
            }
            mutau_cut_cfgs["OppSign_charge"].SetPassed(true);
        }else{
            b_mutau_success = false;
            mutau_cut_cfgs["OppSign_charge"].SetPassed(false);
        }
        /// Make the M_T cut
        if(MT_cut(mutau_cut_cfgs["MT_cut"])) {
            if(b_mutau_success) {
                Fill_Resonance_histograms(6, "mutau", "muo", "tau", endung);
                b_mutau_success = true;
            }
            mutau_cut_cfgs["MT_cut"].SetPassed(true);
        }else{
            b_mutau_success = false;
            mutau_cut_cfgs["MT_cut"].SetPassed(false);
        }
        /// Fill the N-1 histograms
        Fill_N1_histos("mutau", mutau_cut_cfgs, endung);
    }
    ///-----------------------------------------------------------------
    /// Selection for the e-tau_e channel
    if(b_etaue) {
        bool b_etaue_success = false;
        if(FindResonance(*EleList, *EleList, *METList)) {
            Fill_Resonance_histograms(0, "etaue", "ele", "tau_ele", endung);
            b_etaue_success = true;
            etaue_cut_cfgs["kinematics"].SetPassed(true);
            etaue_cut_cfgs["kinematics"].SetVars(resonance_mass);
        }else{
            b_etaue_success = false;
            etaue_cut_cfgs["kinematics"].SetPassed(false);
            etaue_cut_cfgs["kinematics"].SetVars(resonance_mass);
        }
        Fill_N1_histos("etaue", etaue_cut_cfgs, endung);
    }
    ///-----------------------------------------------------------------
    /// Selection for the e-tau_muo channel
    if(b_etaumu) {
        bool b_etaumu_success = false;
        if(FindResonance(*EleList, *MuonList, *METList)) {
            Fill_Resonance_histograms(0, "etaumu", "ele", "tau_muo", endung);
            b_etaumu_success = true;
            etaumu_cut_cfgs["kinematics"].SetPassed(true);
            etaumu_cut_cfgs["kinematics"].SetVars(resonance_mass);
        }else{
            b_etaumu_success = false;
            etaumu_cut_cfgs["kinematics"].SetPassed(false);
            etaumu_cut_cfgs["kinematics"].SetVars(resonance_mass);
        }
        Fill_N1_histos("etaumu", etaumu_cut_cfgs, endung);
    }
    ///-----------------------------------------------------------------
    /// Selection for the muo-tau_e channel
    if(b_mutaue) {
        bool b_mutaue_success = false;
        /// Find the actual resonance
        if(FindResonance(*MuonList, *EleList, *METList)) {
            Fill_Resonance_histograms(0, "mutaue", "muo", "tau_ele", endung);
            b_mutaue_success = true;
            mutaue_cut_cfgs["kinematics"].SetPassed(true);
            mutaue_cut_cfgs["kinematics"].SetVars(resonance_mass);
        }else{
            b_mutaue_success = false;
            mutaue_cut_cfgs["kinematics"].SetPassed(false);
            mutaue_cut_cfgs["kinematics"].SetVars(resonance_mass);
        }
        /// Make the b-jet veto
        if(Bjet_veto(mutaue_cut_cfgs["BJet_veto"])) {
            if(b_mutaue_success) {
                Fill_Resonance_histograms(1, "mutaue", "muo", "tau_ele", endung);
                b_mutaue_success = true;
            }
            mutaue_cut_cfgs["BJet_veto"].SetPassed(true);
        }else{
            b_mutaue_success = false;
            mutaue_cut_cfgs["BJet_veto"].SetPassed(false);
        }
        /// Make the cut on DeltaPhi(e,mu)
        if(Make_DeltaPhi_tauemu(mutaue_cut_cfgs["DeltaPhi_emu"])) {
            if(b_mutaue_success) {
                Fill_Resonance_histograms(2, "mutaue", "muo", "tau_ele", endung);
                b_mutaue_success = true;
            }
            mutaue_cut_cfgs["DeltaPhi_emu"].SetPassed(true);
        }else{
            b_mutaue_success = false;
            mutaue_cut_cfgs["DeltaPhi_emu"].SetPassed(false);
        }
        /// Make the cut on the leptonic pT fraction
        if(Leptonic_fraction_cut(mutaue_cut_cfgs["lep_fraction"])) {
            if(b_mutaue_success) {
                Fill_Resonance_histograms(3, "mutaue", "muo", "tau_ele", endung);
                b_mutaue_success = true;
            }
            mutaue_cut_cfgs["lep_fraction"].SetPassed(true);
        }else{
            b_mutaue_success = false;
            mutaue_cut_cfgs["lep_fraction"].SetPassed(false);
        }
        Fill_N1_histos("mutaue", mutaue_cut_cfgs, endung);
    }
    ///-----------------------------------------------------------------
    /// Selection for the muo-tau_muo channel
    if(b_mutaumu) {
        bool b_mutaumu_success = false;
        if(FindResonance(*MuonList, *MuonList, *METList)) {
            Fill_Resonance_histograms(0, "mutaumu", "muo", "tau_muo", endung);
            b_mutaumu_success = true;
            mutaumu_cut_cfgs["kinematics"].SetPassed(true);
            mutaumu_cut_cfgs["kinematics"].SetVars(resonance_mass);
        }else{
            b_mutaumu_success = false;
            mutaumu_cut_cfgs["kinematics"].SetPassed(false);
            mutaumu_cut_cfgs["kinematics"].SetVars(resonance_mass);
        }
        Fill_N1_histos("mutaumu", mutaumu_cut_cfgs, endung);
    }
}

void specialAna::Create_N1_histos(const char* channel, std::map< std::string, Cuts > &m_cfg, std::string const endung) {
    for(auto iterator = m_cfg.begin(); iterator != m_cfg.end(); iterator++) {
        std::string dummy_key = iterator->first;
        if (m_cfg[dummy_key].dim() == 1) {
            HistClass::CreateHisto(2,TString::Format("N-1_%s_", channel) + dummy_key + endung, m_cfg[dummy_key].bx(), m_cfg[dummy_key].xmi(), m_cfg[dummy_key].xma(), m_cfg[dummy_key].xt() );
        }else if(m_cfg[dummy_key].dim() == 2) {
            HistClass::CreateHisto(TString::Format("0_N-1_%s_", channel) + dummy_key + endung, m_cfg[dummy_key].bx(), m_cfg[dummy_key].xmi(), m_cfg[dummy_key].xma(), m_cfg[dummy_key].by(), m_cfg[dummy_key].ymi(), m_cfg[dummy_key].yma(), m_cfg[dummy_key].xt(), m_cfg[dummy_key].yt() );
            HistClass::CreateHisto(TString::Format("1_N-1_%s_", channel) + dummy_key + endung, m_cfg[dummy_key].bx(), m_cfg[dummy_key].xmi(), m_cfg[dummy_key].xma(), m_cfg[dummy_key].by(), m_cfg[dummy_key].ymi(), m_cfg[dummy_key].yma(), m_cfg[dummy_key].xt(), m_cfg[dummy_key].yt() );
        }else{
            std::cerr << "At the moment only one and two dimensional N-1 distributions are supported!" << std::endl;
        }
    }
}

void specialAna::Fill_N1_histos(const char* channel, std::map< std::string, Cuts > &m_cfg, std::string const endung) {
    for(auto iterator = m_cfg.begin(); iterator != m_cfg.end(); iterator++) {
        std::string dummy_key = iterator->first;
        bool do_n_plot = true;
        for(auto jterator = m_cfg.begin(); jterator != m_cfg.end(); jterator++) {
            if(dummy_key == jterator->first) continue;
            if(not m_cfg[jterator->first].pass()){
                do_n_plot = false;
                break;
            }
        }
        if (m_cfg[dummy_key].dim() == 1) {
            HistClass::Fill(0,TString::Format("N-1_%s_", channel) + dummy_key + endung, m_cfg[dummy_key].v1(), weight );
            if(do_n_plot){
                HistClass::Fill(1,TString::Format("N-1_%s_", channel) + dummy_key + endung, m_cfg[dummy_key].v1(), weight );
            }
        }else if(m_cfg[dummy_key].dim() == 2) {
            HistClass::Fill(TString::Format("0_N-1_%s_", channel) + dummy_key + endung, m_cfg[dummy_key].v1(), m_cfg[dummy_key].v2(), weight );
            if(do_n_plot){
                HistClass::Fill(TString::Format("1_N-1_%s_", channel) + dummy_key + endung, m_cfg[dummy_key].v1(), m_cfg[dummy_key].v2(), weight );
            }
        }else{
            std::cerr << "At the moment only one and two dimensional N-1 distributions are supported!" << std::endl;
            return;
        }
    }
}

void specialAna::GenSelector() {
    if(b_emu) {
        if(FindResonance("emu", *S3ListGen)) {
            Fill_Gen_histograms("emu", "ele", "muo");
        }
    }
    if(b_etau) {
        if(FindResonance("etau", *S3ListGen)) {
            Fill_Gen_histograms("etau", "ele", "tau");
        }
    }
    if(b_mutau) {
        if(FindResonance("mutau", *S3ListGen)) {
            Fill_Gen_histograms("mutau", "muo", "tau");
        }
    }
    if(b_etaue) {
        if(FindResonance("etaue", *S3ListGen)) {
            Fill_Gen_histograms("etaue", "ele", "tau_ele");
        }
    }
    if(b_etaumu) {
        if(FindResonance("etaumu", *S3ListGen)) {
            Fill_Gen_histograms("etaumu", "ele", "tau_muo");
        }
    }
    if(b_mutaue) {
        if(FindResonance("mutaue", *S3ListGen)) {
            Fill_Gen_histograms("mutaue", "muo", "tau_ele");
        }
    }
    if(b_mutaumu) {
        if(FindResonance("mutaumu", *S3ListGen)) {
            Fill_Gen_histograms("mutaumu", "muo", "tau_muo");
        }
    }
}

void specialAna::Create_Gen_histograms(const char* channel, const char* part1, const char* part2) {
    /// Resonant mass histogram
    HistClass::CreateHisto(TString::Format("%s_Mass_Gen",            channel),             5000, 0, 5000, TString::Format("M_{%s,%s}(gen) (GeV)",                    part1, part2) );
    /// First particle histograms
    HistClass::CreateHisto(TString::Format("%s_pT_%s_Gen",           channel,part1),       5000, 0, 5000, TString::Format("p_{T}^{%s,gen} (GeV)",                    part1) );
    HistClass::CreateHisto(TString::Format("%s_eta_%s_Gen",          channel,part1),       80, -4, 4,     TString::Format("#eta^{%s,gen}",                           part1) );
    HistClass::CreateHisto(TString::Format("%s_phi_%s_Gen",          channel,part1),       40, -3.2, 3.2, TString::Format("#phi^{%s,gen} (rad)",                     part1) );
    /// Second particle histograms
    HistClass::CreateHisto(TString::Format("%s_pT_%s_Gen",           channel,part2),       5000, 0, 5000, TString::Format("p_{T}^{%s,gen} (GeV)",                    part2) );
    HistClass::CreateHisto(TString::Format("%s_eta_%s_Gen",          channel,part2),       80, -4, 4,     TString::Format("#eta^{%s,gen}",                           part2) );
    HistClass::CreateHisto(TString::Format("%s_phi_%s_Gen",          channel,part2),       40, -3.2, 3.2, TString::Format("#phi^{%s,gen} (rad)",                     part2) );
    /// Delta phi between the two particles
    HistClass::CreateHisto(TString::Format("%s_Delta_phi_%s_%s_Gen", channel,part1,part2), 40, 0, 3.2,    TString::Format("#Delta#phi(%s(gen),%s(gen)) (rad)",       part1, part2) );
    /// pT ratio of the two particles
    HistClass::CreateHisto(TString::Format("%s_pT_ratio_%s_%s_Gen",  channel,part1,part2), 50, 0, 10,     TString::Format("#frac{p_{T}^{%s(gen)}}{p_{T}^{%s(gen)}}", part1, part2) );
}

void specialAna::Fill_Gen_histograms(const char* channel, const char* part1, const char* part2) {
    /// Resonant mass histogram
    HistClass::Fill(TString::Format("%s_Mass_Gen",            channel),             resonance_mass_gen,                                  weight );
    /// First particle histograms
    HistClass::Fill(TString::Format("%s_pT_%s_Gen",           channel,part1),       sel_part1_gen -> getPt(),                            weight );
    HistClass::Fill(TString::Format("%s_eta_%s_Gen",          channel,part1),       sel_part1_gen -> getEta(),                           weight );
    HistClass::Fill(TString::Format("%s_phi_%s_Gen",          channel,part1),       sel_part1_gen -> getPhi(),                           weight );
    /// Second particle histograms
    HistClass::Fill(TString::Format("%s_pT_%s_Gen",           channel,part2),       sel_part2_gen -> getPt(),                            weight );
    HistClass::Fill(TString::Format("%s_eta_%s_Gen",          channel,part2),       sel_part2_gen -> getEta(),                           weight );
    HistClass::Fill(TString::Format("%s_phi_%s_Gen",          channel,part2),       sel_part2_gen -> getPhi(),                           weight );
    /// Delta phi between the two particles
    HistClass::Fill(TString::Format("%s_Delta_phi_%s_%s_Gen", channel,part1,part2), DeltaPhi(sel_part1_gen, sel_part2_gen),              weight );
    /// pT ratio of the two particles
    HistClass::Fill(TString::Format("%s_pT_ratio_%s_%s_Gen",  channel,part1,part2), sel_part1_gen -> getPt() / sel_part2_gen -> getPt(), weight );
}

void specialAna::Create_Resonance_histograms(int n_histos, const char* channel, const char* part1, const char* part2, std::string const endung) {
    /// Resonant mass histogram
    HistClass::CreateHisto(n_histos,TString::Format("%s_Mass",                 channel) + endung,             5000, 0, 5000, TString::Format("M_{%s,%s} (GeV)",                         part1, part2) );
    /// First particle histograms
    HistClass::CreateHisto(n_histos,TString::Format("%s_pT_%s",                channel,part1) + endung,       5000, 0, 5000, TString::Format("p_{T}^{%s} (GeV)",                        part1) );
    HistClass::CreateHisto(n_histos,TString::Format("%s_eta_%s",               channel,part1) + endung,       80, -4, 4,     TString::Format("#eta^{%s}",                               part1) );
    HistClass::CreateHisto(n_histos,TString::Format("%s_phi_%s",               channel,part1) + endung,       40, -3.2, 3.2, TString::Format("#phi^{%s} (rad)",                         part1) );
    /// Second particle histograms
    HistClass::CreateHisto(n_histos,TString::Format("%s_pT_%s",                channel,part2) + endung,       5000, 0, 5000, TString::Format("p_{T}^{%s} (GeV)",                        part2) );
    HistClass::CreateHisto(n_histos,TString::Format("%s_eta_%s",               channel,part2) + endung,       80, -4, 4,     TString::Format("#eta^{%s}",                               part2) );
    HistClass::CreateHisto(n_histos,TString::Format("%s_phi_%s",               channel,part2) + endung,       40, -3.2, 3.2, TString::Format("#phi^{%s} (rad)",                         part2) );
    /// Delta phi between the two particles
    HistClass::CreateHisto(n_histos,TString::Format("%s_Delta_phi_%s_%s",      channel,part1,part2) + endung, 40, 0, 3.2,    TString::Format("#Delta#phi(%s,%s) (rad)",                 part1, part2) );
    /// pT ratio of the two particles
    HistClass::CreateHisto(n_histos,TString::Format("%s_pT_ratio_%s_%s",       channel,part1,part2) + endung, 50, 0, 10,     TString::Format("#frac{p_{T}^{%s}}{p_{T}^{%s}}",           part1, part2) );
    /// Create histograms for channels with MET
    if(channel != (char*)"emu") {
        /// MET histograms
        HistClass::CreateHisto(n_histos,TString::Format("%s_MET",                  channel) + endung,             5000, 0, 5000, "E_{T}^{miss} (GeV)");
        HistClass::CreateHisto(n_histos,TString::Format("%s_phi_MET",              channel) + endung,             40, -3.2, 3.2, "#phi^{E_{T}^{miss}} (rad)");
        /// Corrected second particle histogram
        HistClass::CreateHisto(n_histos,TString::Format("%s_pT_%s_corr",           channel,part2) + endung,       5000, 0, 5000, TString::Format("p_{T}^{%s(corr)} (GeV)",                  part2) );
        HistClass::CreateHisto(n_histos,TString::Format("%s_eta_%s_corr",          channel,part2) + endung,       80, -4, 4,     TString::Format("#eta^{%s(corr)}",                         part2) );
        HistClass::CreateHisto(n_histos,TString::Format("%s_phi_%s_corr",          channel,part2) + endung,       40, -3.2, 3.2, TString::Format("#phi^{%s(corr)} (rad)",                   part2) );
        /// Delta phi between the other particles
        HistClass::CreateHisto(n_histos,TString::Format("%s_Delta_phi_%s_MET",     channel,part1) + endung,       40, 0, 3.2,    TString::Format("#Delta#phi(%s,E_{T}^{miss}) (rad)",       part1) );
        HistClass::CreateHisto(n_histos,TString::Format("%s_Delta_phi_%s_MET",     channel,part2) + endung,       40, 0, 3.2,    TString::Format("#Delta#phi(%s,E_{T}^{miss}) (rad)",       part2) );
        HistClass::CreateHisto(n_histos,TString::Format("%s_Delta_phi_%s_MET_corr",channel,part2) + endung,       40, 0, 3.2,    TString::Format("#Delta#phi(%s(corr),E_{T}^{miss}) (rad)", part2) );
        HistClass::CreateHisto(n_histos,TString::Format("%s_Delta_phi_%s_%s_corr", channel,part1,part2) + endung, 40, 0, 3.2,    TString::Format("#Delta#phi(%s,%s(corr)) (rad)",           part1, part2) );
        /// pT ratio of the other particles
        HistClass::CreateHisto(n_histos,TString::Format("%s_pT_ratio_%s_MET",      channel,part1) + endung,       50, 0, 10,     TString::Format("#frac{p_{T}^{%s}}{E_{T}^{miss}}",         part1) );
        HistClass::CreateHisto(n_histos,TString::Format("%s_pT_ratio_%s_MET",      channel,part2) + endung,       50, 0, 10,     TString::Format("#frac{p_{T}^{%s}}{E_{T}^{miss}}",         part2) );
        HistClass::CreateHisto(n_histos,TString::Format("%s_pT_ratio_%s_MET_corr", channel,part2) + endung,       50, 0, 10,     TString::Format("#frac{p_{T}^{%s(corr)}}{E_{T}^{miss}}",   part2) );
        HistClass::CreateHisto(n_histos,TString::Format("%s_pT_ratio_%s_%s_corr",  channel,part1,part2) + endung, 50, 0, 10,     TString::Format("#frac{p_{T}^{%s}}{p_{T}^{%s(corr)}}",     part1, part2) );
    }
}

void specialAna::Fill_Resonance_histograms(int n_histos, const char* channel, const char* part1, const char* part2, std::string const endung) {
    /// Resonant mass histogram
    HistClass::Fill(n_histos,TString::Format("%s_Mass",                 channel) + endung,             resonance_mass,                                                    weight );
    /// First particle histograms
    HistClass::Fill(n_histos,TString::Format("%s_pT_%s",                channel,part1) + endung,       sel_lepton_prompt -> getPt(),                                      weight );
    HistClass::Fill(n_histos,TString::Format("%s_eta_%s",               channel,part1) + endung,       sel_lepton_prompt -> getEta(),                                     weight );
    HistClass::Fill(n_histos,TString::Format("%s_phi_%s",               channel,part1) + endung,       sel_lepton_prompt -> getPhi(),                                     weight );
    /// Second particle histograms
    HistClass::Fill(n_histos,TString::Format("%s_pT_%s",                channel,part2) + endung,       sel_lepton_nprompt -> getPt(),                                     weight );
    HistClass::Fill(n_histos,TString::Format("%s_eta_%s",               channel,part2) + endung,       sel_lepton_nprompt -> getEta(),                                    weight );
    HistClass::Fill(n_histos,TString::Format("%s_phi_%s",               channel,part2) + endung,       sel_lepton_nprompt -> getPhi(),                                    weight );
    /// Delta phi between the two particles
    HistClass::Fill(n_histos,TString::Format("%s_Delta_phi_%s_%s",      channel,part1,part2) + endung, DeltaPhi(sel_lepton_prompt, sel_lepton_nprompt),                   weight );
    /// pT ratio of the two particles
    HistClass::Fill(n_histos,TString::Format("%s_pT_ratio_%s_%s",       channel,part1,part2) + endung, sel_lepton_prompt -> getPt() / sel_lepton_nprompt -> getPt(),      weight );
    /// Create histograms for channels with MET
    if(channel != (char*)"emu") {
        /// MET histograms
        HistClass::Fill(n_histos,TString::Format("%s_MET",                  channel) + endung,             sel_met -> getPt(),                                                weight );
        HistClass::Fill(n_histos,TString::Format("%s_phi_MET",              channel) + endung,             sel_met -> getPhi(),                                               weight );
        /// Corrected second particle histogram
        HistClass::Fill(n_histos,TString::Format("%s_pT_%s_corr",           channel,part2) + endung,       sel_lepton_nprompt_corr -> getPt(),                                weight );
        HistClass::Fill(n_histos,TString::Format("%s_eta_%s_corr",          channel,part2) + endung,       sel_lepton_nprompt_corr -> getEta(),                               weight );
        HistClass::Fill(n_histos,TString::Format("%s_phi_%s_corr",          channel,part2) + endung,       sel_lepton_nprompt_corr -> getPhi(),                               weight );
        /// Delta phi between the other particles
        HistClass::Fill(n_histos,TString::Format("%s_Delta_phi_%s_MET",     channel,part1) + endung,       DeltaPhi(sel_lepton_prompt, sel_met),                              weight );
        HistClass::Fill(n_histos,TString::Format("%s_Delta_phi_%s_MET",     channel,part2) + endung,       DeltaPhi(sel_lepton_nprompt, sel_met),                             weight );
        HistClass::Fill(n_histos,TString::Format("%s_Delta_phi_%s_MET_corr",channel,part2) + endung,       DeltaPhi(sel_lepton_nprompt_corr, sel_met),                        weight );
        HistClass::Fill(n_histos,TString::Format("%s_Delta_phi_%s_%s_corr", channel,part1,part2) + endung, DeltaPhi(sel_lepton_prompt, sel_lepton_nprompt_corr),              weight );
        /// pT ratio of the other particles
        HistClass::Fill(n_histos,TString::Format("%s_pT_ratio_%s_MET",      channel,part1) + endung,       sel_lepton_prompt -> getPt() / sel_met -> getPt(),                 weight );
        HistClass::Fill(n_histos,TString::Format("%s_pT_ratio_%s_MET",      channel,part2) + endung,       sel_lepton_nprompt -> getPt() / sel_met -> getPt(),                weight );
        HistClass::Fill(n_histos,TString::Format("%s_pT_ratio_%s_MET_corr", channel,part2) + endung,       sel_lepton_nprompt_corr -> getPt() / sel_met -> getPt(),           weight );
        HistClass::Fill(n_histos,TString::Format("%s_pT_ratio_%s_%s_corr",  channel,part1,part2) + endung, sel_lepton_prompt -> getPt() / sel_lepton_nprompt_corr -> getPt(), weight );
    }
}

bool specialAna::FindResonance(const char* channel, vector< pxl::Particle* > gen_list) {
    int id_1, id_2;
    if(channel == (char*)"emu") {
        id_1 = 11;
        id_2 = 13;
    }else if(channel == (char*)"etau" or channel == (char*)"etaue" or channel == (char*)"etaumu") {
        id_1 = 11;
        id_2 = 15;
    }else if(channel == (char*)"mutau" or channel == (char*)"mutaue" or channel == (char*)"mutaumu") {
        id_1 = 13;
        id_2 = 15;
    }else {
        return false;
    }

    resonance_mass_gen = 0;
    for( vector< pxl::Particle* >::const_iterator part_it = gen_list.begin(); part_it != gen_list.end(); ++part_it ) {
        pxl::Particle *part_i = *part_it;
        if(TMath::Abs(part_i -> getPdgNumber()) == id_1) {
            for( vector< pxl::Particle* >::const_iterator part_jt = gen_list.begin(); part_jt != gen_list.end(); ++part_jt ) {
                pxl::Particle *part_j = *part_jt;
                if (TMath::Abs(part_j -> getPdgNumber()) != id_2) continue;
                pxl::Particle *part_sum = (pxl::Particle*) part_i->clone();
                part_sum -> addP4(part_j);
                if(part_sum -> getMass() > resonance_mass_gen) {
                    resonance_mass_gen = part_sum -> getMass();
                    sel_part1_gen = (pxl::Particle*) part_i->clone();
                    sel_part2_gen = (pxl::Particle*) part_j->clone();
                }
            }
        }
    }
    if(resonance_mass_gen > 0){
        return true;
    }else{
        return false;
    }
}

bool specialAna::FindResonance(vector< pxl::Particle* > part1_list, vector< pxl::Particle* > part2_list) {
    resonance_mass = 0;
    for( vector< pxl::Particle* >::const_iterator part_it = part1_list.begin(); part_it != part1_list.end(); ++part_it ) {
        pxl::Particle *part_i = *part_it;
        if(Check_Par_ID(part_i)) {
            for( vector< pxl::Particle* >::const_iterator part_jt = part2_list.begin(); part_jt != part2_list.end(); ++part_jt ) {
                pxl::Particle *part_j = *part_jt;
                if (not Check_Par_ID(part_j)) continue;
                pxl::Particle *part_sum = (pxl::Particle*) part_i->clone();
                part_sum -> addP4(part_j);
                if(part_sum -> getMass() > resonance_mass) {
                    resonance_mass = part_sum -> getMass();
                    sel_lepton_prompt = (pxl::Particle*) part_i->clone();
                    sel_lepton_nprompt = (pxl::Particle*) part_j->clone();
                }
            }
        }
    }
    if(resonance_mass > 0){
        return true;
    }else{
        return false;
    }
}

bool specialAna::FindResonance(vector< pxl::Particle* > part1_list, vector< pxl::Particle* > part2_list, vector< pxl::Particle* > met_list) {
    resonance_mass = 0;
    if( not sel_met ) return false;
    for( vector< pxl::Particle* >::const_iterator part_it = part1_list.begin(); part_it != part1_list.end(); ++part_it ) {
        pxl::Particle *part_i = *part_it;
        if(Check_Par_ID(part_i)) {
            for( vector< pxl::Particle* >::const_iterator part_jt = part2_list.begin(); part_jt != part2_list.end(); ++part_jt ) {
                pxl::Particle *part_j = *part_jt;
                if (not Check_Par_ID(part_j)) continue;
                pxl::Particle* dummy_taumu = ( pxl::Particle* ) part_i->clone();
                pxl::Particle* dummy_taumu_uncorr = ( pxl::Particle* ) part_i->clone();
                dummy_taumu->addP4(part_j);
                dummy_taumu_uncorr->addP4(part_j);
                pxl::Particle* dummy_met = new pxl::Particle();
                /// use tau eta to project MET
                TLorentzVector* calc_met = new TLorentzVector();
                calc_met -> SetPtEtaPhiM(sel_met->getPt(),part_j->getEta(),sel_met->getPhi(),0);
                dummy_met->setP4(calc_met->Px(),calc_met->Py(),calc_met->Pz(),calc_met->E());
                dummy_taumu->addP4(dummy_met);
                delete calc_met;

                /// rotate MET to tau direction
                //TLorentzVector* calc_met = new TLorentzVector();
                //calc_met -> SetPtEtaPhiM(METList->at(0)->getPt(),0,METList->at(0)->getPhi(),0);
                //TVector3* tau_direction = new TVector3(TauList->at(i)->getPx(),TauList->at(i)->getPy(),TauList->at(i)->getPz());
                //*tau_direction = tau_direction -> Unit();
                //calc_met -> RotateUz(*tau_direction);
                //dummy_met->setP4(calc_met->Px(),calc_met->Py(),calc_met->Pz(),calc_met->E());
                //dummy_taumu->addP4(dummy_met);
                //delete tau_direction;
                //delete calc_met;

                /// project MET to tau direction
                //double dummy_p1 = METList->at(0)->getPx()/(TMath::Sin(TauList->at(i)->getTheta()) * TMath::Cos(TauList->at(i)->getPhi()));
                //double dummy_p2 = METList->at(0)->getPy()/(TMath::Sin(TauList->at(i)->getTheta()) * TMath::Sin(TauList->at(i)->getPhi()));
                //double dummy_p = (dummy_p1 + dummy_p2) / 2.;
                //dummy_met->setP4(dummy_p*TMath::Sin(TauList->at(i)->getTheta()) * TMath::Cos(TauList->at(i)->getPhi()),dummy_p*TMath::Sin(TauList->at(i)->getTheta()) * TMath::Sin(TauList->at(i)->getPhi()),dummy_p*TMath::Cos(TauList->at(i)->getTheta()),dummy_p);
                //dummy_taumu->addP4(dummy_met);

                /// project MET parallel to tau direction
                //double value = (METList->at(0)->getPx() * TauList->at(i)->getPx() + METList->at(0)->getPy() * TauList->at(i)->getPy()) / sqrt(pow(TauList->at(i)->getPx(),2) + pow(TauList->at(i)->getPy(),2));
                //TLorentzVector* calc_met = new TLorentzVector();
                //calc_met -> SetPtEtaPhiM(value,TauList->at(i)->getEta(),TauList->at(i)->getPhi(),0);
                //dummy_taumu->addP4(dummy_met);

                if (dummy_taumu->getMass() > resonance_mass){
                    resonance_mass = dummy_taumu->getMass();
                    sel_lepton_prompt = ( pxl::Particle* ) part_i -> clone();
                    sel_lepton_nprompt = ( pxl::Particle* ) part_j -> clone();
                    sel_lepton_nprompt_corr = ( pxl::Particle* ) part_j -> clone();
                    sel_lepton_nprompt_corr -> addP4(dummy_met);
                }
            }
        }
    }
    if(resonance_mass > 0){
        return true;
    }else{
        return false;
    }
}

bool specialAna::Check_Par_ID(pxl::Particle* part) {
    string name = part -> getName();
    if(name == m_TauType){
        bool tau_id = Check_Tau_ID(part);
        return tau_id;
    }else if(name == "Ele"){
        bool ele_id = Check_Ele_ID(part);
        return ele_id;
    }else if(name == "Muon"){
        bool muo_id = Check_Muo_ID(part);
        return muo_id;
    }else{
        return false;
    }
}

bool specialAna::Check_Tau_ID(pxl::Particle* tau) {
    bool passed = false;
    bool tau_ID = tau->getUserRecord("decayModeFindingOldDMs").asFloat() >= 1 ? true : false;
    bool tau_ISO = tau->getUserRecord("byLooseIsolationMVA3oldDMwLT").asFloat() >= 1 ? true : false;
    bool tau_ELE = tau->getUserRecord("againstElectronLooseMVA5"/*"againstElectronTightMVA5"*/).asFloat() >= 1 ? true : false;
    bool tau_MUO = tau->getUserRecord("againstMuonTight3"/*"againstMuonTightMVA"*/).asFloat() >= 1 ? true : false;
    if (tau_ID && tau_ISO && tau_ELE && tau_MUO) passed = true;
    return passed;
}

bool specialAna::Check_Muo_ID(pxl::Particle* muon) {
    bool passed = false;
    bool muon_ID = muon->getUserRecord("isHighPtMuon").asBool() ? passed = true : passed = false;
    bool muon_ISO = muon -> getUserRecord("IsoR3SumPt").asDouble() / muon -> getPt() < 0.1 ? true : false;
    bool muon_eta = TMath::Abs(muon -> getEta()) < 2.1 ? true : false;
    bool muon_pt = muon -> getPt() > 45. ? true : false;
    if (muon_ID && muon_ISO && muon_eta && muon_pt) return true;
    return passed;
}

bool specialAna::Check_Ele_ID(pxl::Particle* ele) {
    return true;
}

vector<double> specialAna::Make_zeta_stuff(pxl::Particle* muon, pxl::Particle* tau, pxl::Particle* met) {
    double p_zeta_vis = 0;
    double p_zeta = 0;

    if(met and muon and tau){
        TVector3* vec_mu = new TVector3();
        TVector3* vec_tau = new TVector3();

        vec_mu -> SetXYZ(muon->getPx(),muon->getPy(),0);
        vec_tau -> SetXYZ(tau->getPx(),tau->getPy(),0);

        TVector3 bisec = vec_mu->Unit() + vec_tau->Unit();
        TVector3 bisec_norm = bisec.Unit();

        p_zeta_vis = (tau->getPx() * bisec_norm.X() + tau->getPy() * bisec_norm.Y()) + (muon->getPx() * bisec_norm.X() + muon->getPy() * bisec_norm.Y());
        p_zeta = p_zeta_vis + (met->getPx() * bisec_norm.X() + met->getPy() * bisec_norm.Y());
        delete vec_mu;
        delete vec_tau;
    }

    vector<double> out;
    out.push_back(p_zeta_vis);
    out.push_back(p_zeta);

    return out;
}

bool specialAna::Make_zeta_cut(Cuts& cuts) {
    vector<double> zeta_vals = Make_zeta_stuff(sel_lepton_prompt, sel_lepton_nprompt, sel_met);
    double zeta_steepnes_cut_value = -1.21;
    double zeta_offset_cut_value   = -24.1;
    cuts.SetVars(zeta_vals[0],zeta_vals[1]);
    if ((zeta_vals[0] + zeta_steepnes_cut_value * zeta_vals[1]) > zeta_offset_cut_value) {
        return true;
    }else{
        return false;
    }
}

bool specialAna::Make_DeltaPhi_tauMET(Cuts& cuts) {
    double delta_phi = 10.;
    if(sel_met and sel_lepton_nprompt) {
        delta_phi = DeltaPhi(sel_lepton_nprompt,sel_met);
    }
    double delta_phi_tau_met_cut_value = 1.3;
    cuts.SetVars(delta_phi);
    if(delta_phi < delta_phi_tau_met_cut_value) {
        return true;
    }else{
        return false;
    }
}

bool specialAna::Make_DeltaPhi_mutau(Cuts& cuts) {
    double delta_phi = 0.;
    if(sel_lepton_prompt and sel_lepton_nprompt) {
        delta_phi = DeltaPhi(sel_lepton_nprompt,sel_lepton_prompt);
    }
    double delta_phi_mu_tau_cut_value = 2.3;
    cuts.SetVars(delta_phi);
    if(delta_phi > delta_phi_mu_tau_cut_value) {
        return true;
    }else{
        return false;
    }
}

bool specialAna::Make_DeltaPhi_tauemu(Cuts& cuts) {
    double delta_phi = 0.;
    if(sel_lepton_prompt and sel_lepton_nprompt) {
        delta_phi = DeltaPhi(sel_lepton_nprompt,sel_lepton_prompt);
    }
    double delta_phi_mu_tau_cut_value = 2.7;
    cuts.SetVars(delta_phi);
    if(delta_phi > delta_phi_mu_tau_cut_value) {
        return true;
    }else{
        return false;
    }
}

bool specialAna::Bjet_veto(Cuts& cuts) {
    /// TODO: include b-jet veto
    cuts.SetVars(0);
    return true;
}

bool specialAna::OppSign_charge(Cuts& cuts) {
    double charge_product = 0;
    if(sel_lepton_prompt and sel_lepton_nprompt) {
        charge_product = sel_lepton_prompt->getCharge() * sel_lepton_nprompt->getCharge();
    }
    cuts.SetVars(charge_product);
    if(charge_product < 0) {
        return true;
    }else{
        return false;
    }
}

bool specialAna::MT_cut(Cuts& cuts) {
    double mt = 0;
    if(sel_lepton_prompt and sel_met) {
        mt = MT(sel_lepton_prompt,sel_met);
    }
    cuts.SetVars(mt);
    double mt_min_cut_value = 180;
    if(mt > mt_min_cut_value) {
        return true;
    }else{
        return false;
    }
}

double specialAna::calc_lep_fraction() {
    double pT_sum_lep = sel_lepton_prompt->getPt() + sel_lepton_nprompt->getPt();
    double pT_sum_had = 0;
    for( vector< pxl::Particle* >::const_iterator part_it = JetList->begin(); part_it != JetList->end(); ++part_it ) {
        pxl::Particle *part_i = *part_it;
        pT_sum_had += part_i->getPt();
    }
    double pT_sum_all = pT_sum_lep + pT_sum_had;
    return pT_sum_lep / pT_sum_all;
}

bool specialAna::Leptonic_fraction_cut(Cuts& cuts) {
    double lep_fraction = 0;
    if(sel_lepton_nprompt and sel_lepton_prompt) {
        lep_fraction = calc_lep_fraction();
    }
    cuts.SetVars(lep_fraction);
    double lep_fraction_cut_value = 0.8;
    if(lep_fraction > lep_fraction_cut_value) {
        return true;
    }else{
        return false;
    }
}

bool specialAna::TriggerSelector(const pxl::Event* event){
    bool triggered=false;

    pxl::UserRecords::const_iterator us = m_TrigEvtView->getUserRecords().begin();
    for( ; us != m_TrigEvtView->getUserRecords().end(); ++us ) {
        if (
            string::npos != (*us).first.find( "HLT_HLT_Ele90_CaloIdVT_GsfTrkIdT") or
            string::npos != (*us).first.find( "HLT_Ele80_CaloIdVT_GsfTrkIdT") or
            string::npos != (*us).first.find( "HLT_Ele80_CaloIdVT_TrkIdT") or
            //string::npos != (*us).first.find( "HLT_HLT_Ele27_WP80_v") or
            string::npos != (*us).first.find( "HLT_HLT_Mu40_v") or
            string::npos != (*us).first.find( "HLT_HLT_Mu40_eta2p1_v") or
            //string::npos != (*us).first.find( "HLT_HLT_IsoMu30_v") or
            string::npos != (*us).first.find( "HLT_MonoCentralPFJet80")
        ){
            triggered=(*us).second;
            if(triggered){
                break;
            }
        }
    }

     return (triggered);
}

void specialAna::Fill_Gen_Controll_histo() {
    int muon_gen_num=0;
    int ele_gen_num=0;
    int tau_gen_num=0;
    for(uint i = 0; i < S3ListGen->size(); i++){
        if (S3ListGen->at(i)->getPt()<10 && not (TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 24)   ){
            continue;
        }
        if(S3ListGen->at(i)->getPdgNumber()==0){
            if(S3ListGen->at(i)->hasUserRecord("id")){
                S3ListGen->at(i)->setPdgNumber(S3ListGen->at(i)->getUserRecord("id"));
            }
        }
        if(TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 13){
            muon_gen_num++;
            HistClass::Fill(0,"Muon_pt_Gen",S3ListGen->at(i)->getPt(),m_GenEvtView->getUserRecord( "Weight" ));
            HistClass::Fill(0,"Muon_eta_Gen",S3ListGen->at(i)->getEta(),m_GenEvtView->getUserRecord( "Weight" ));
            HistClass::Fill(0,"Muon_phi_Gen",S3ListGen->at(i)->getPhi(),m_GenEvtView->getUserRecord( "Weight" ));
        }else if(TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 15){
            tau_gen_num++;
            HistClass::Fill(0,"Tau_pt_Gen",S3ListGen->at(i)->getPt(),m_GenEvtView->getUserRecord( "Weight" ));
            HistClass::Fill(0,"Tau_eta_Gen",S3ListGen->at(i)->getEta(),m_GenEvtView->getUserRecord( "Weight" ));
            HistClass::Fill(0,"Tau_phi_Gen",S3ListGen->at(i)->getPhi(),m_GenEvtView->getUserRecord( "Weight" ));
        }else if(TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 11){
            ele_gen_num++;
            HistClass::Fill(0,"Ele_pt_Gen",S3ListGen->at(i)->getPt(),m_GenEvtView->getUserRecord( "Weight" ));
            HistClass::Fill(0,"Ele_eta_Gen",S3ListGen->at(i)->getEta(),m_GenEvtView->getUserRecord( "Weight" ));
            HistClass::Fill(0,"Ele_phi_Gen",S3ListGen->at(i)->getPhi(),m_GenEvtView->getUserRecord( "Weight" ));
        }
    }

    HistClass::Fill(0,"Tau_num_Gen",tau_gen_num,m_GenEvtView->getUserRecord( "Weight" ));
    HistClass::Fill(0,"Muon_num_Gen",muon_gen_num,m_GenEvtView->getUserRecord( "Weight" ));
    HistClass::Fill(0,"Ele_num_Gen",ele_gen_num,m_GenEvtView->getUserRecord( "Weight" ));
}

void specialAna::Fill_Particle_histos(int hist_number, pxl::Particle* lepton){
    string name=lepton->getName();
    if(lepton->getName()==m_TauType){
        name="Tau";
    }
    if(lepton->getName()==m_METType){
        name="MET";
    }
    HistClass::Fill(hist_number,str(boost::format("%s_pt")%name ),lepton->getPt(),weight);
    HistClass::Fill(hist_number,str(boost::format("%s_eta")%name ),lepton->getEta(),weight);
    HistClass::Fill(hist_number,str(boost::format("%s_phi")%name ),lepton->getPhi(),weight);
}

double specialAna::DeltaPhi(double a, double b) {
    double temp = fabs(a-b);
    if (temp <= TMath::Pi()) {
        return temp;
    }else{
        return  2.*TMath::Pi() - temp;
    }
}

double specialAna::DeltaPhi(pxl::Particle* lepton, pxl::Particle* met) {
    double a=lepton->getPhi();
    double b=met->getPhi();
    double temp = fabs(a-b);
    if (temp <= TMath::Pi()) {
        return temp;
    }else{
        return  2.*TMath::Pi() - temp;
    }
}

double specialAna::MT(pxl::Particle* lepton, pxl::Particle* met) {
    double mm = 2 * lepton->getPt() * met->getPt() * ( 1. - cos(lepton->getPhi() - met->getPhi()) );
    return sqrt(mm);
}

double specialAna::getPtHat(){
    double pthat=0;
    pxl::Particle* w=0;
    pxl::Particle* lepton=0;
    for(uint i = 0; i < S3ListGen->size(); i++){
        if(TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 24){
            w=S3ListGen->at(i);
        }
        //take the neutrio to avoid showering and so on!!
        if((TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 12 || TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 14 || TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 16) && lepton==0){
            lepton=S3ListGen->at(i);
        }
        if(w!=0 && lepton!=0){
            break;
        }
    }

    if(w!=0 && lepton!=0){
        //boost in the w restframe
        lepton->boost( -(w->getBoostVector()) );
        pthat=lepton->getPt();
    }else{
        pthat=-1;
    }
    return pthat;
}

void specialAna::channel_writer(TFile* file, const char* channel) {
    file1->cd();
    file1->mkdir(channel);
    for ( int i = 0; i < channel_stages[channel]; i++) {
        char n_satge = (char)(((int)'0')+i);
        file1->mkdir(TString::Format("%s/Stage_%c", channel, n_satge));
        file1->cd(TString::Format("%s/Stage_%c/", channel, n_satge));
        HistClass::WriteAll(TString::Format("_%s_", channel),TString::Format("%s:_%c_", channel, n_satge),TString::Format("sys:N-1"));
        HistClass::WriteAll2(TString::Format("_%s_", channel),TString::Format("%s:_%c_", channel, n_satge),TString::Format("sys:N-1"));
        file1->cd();
        file1->mkdir(TString::Format("%s/Stage_%c/sys", channel, n_satge));
        file1->cd(TString::Format("%s/Stage_%c/sys/", channel, n_satge));
        HistClass::WriteAll(TString::Format("_%s_", channel),TString::Format("_%c_:sys", n_satge),TString::Format("N-1"));
        HistClass::WriteAll2(TString::Format("_%s_", channel),TString::Format("_%c_:sys", n_satge),TString::Format("N-1"));
    }
    file1->cd();
    file1->mkdir(TString::Format("%s/N-1", channel));
    file1->cd(TString::Format("%s/N-1/", channel));
    HistClass::WriteAll(TString::Format("_%s_", channel),TString::Format("N-1"),TString::Format("sys"));
    HistClass::WriteAll2(TString::Format("_%s_", channel),TString::Format("N-1"),TString::Format("sys"));
    file1->cd();
    file1->mkdir(TString::Format("%s/N-1/sys", channel));
    file1->cd(TString::Format("%s/N-1/sys/", channel));
    HistClass::WriteAll(TString::Format("_%s_", channel),TString::Format("N-1:sys"));
    HistClass::WriteAll2(TString::Format("_%s_", channel),TString::Format("N-1:sys"));
    file1->cd();
}

void specialAna::endJob( const Serializable* ) {

    file1->cd();
    HistClass::WriteAll("counters");
    if(not runOnData){
        file1->mkdir("MC");
        file1->cd("MC/");
        HistClass::WriteAll("_Gen");
    }
    file1->cd();
    file1->mkdir("Taus");
    file1->cd("Taus/");
    HistClass::WriteAll("_Tau_");
    file1->cd();
    file1->mkdir("Muons");
    file1->cd("Muons/");
    HistClass::WriteAll("_Muon_");
    file1->cd();
    file1->mkdir("METs");
    file1->cd("METs/");
    HistClass::WriteAll("_MET_");
    file1->cd();
    file1->mkdir("Eles");
    file1->cd("Eles/");
    HistClass::WriteAll("_Ele_");
    file1->cd();
    
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

void specialAna::initEvent( const pxl::Event* event ){
    HistClass::Fill("h_counters", 1, 1); // increment number of events
    events_++;

    //no pu weight at the moment!!

    weight = 1;
    m_RecEvtView = event->getObjectOwner().findObject< pxl::EventView >( "Rec" );
    m_GenEvtView = event->getObjectOwner().findObject< pxl::EventView >( "Gen" );
    if(event->getObjectOwner().findObject< pxl::EventView >( "Trig" )){
        m_TrigEvtView = event->getObjectOwner().findObject< pxl::EventView >( "Trig" );
    }else{
        m_TrigEvtView = event->getObjectOwner().findObject< pxl::EventView >( "Rec" );
    }

    temp_run = event->getUserRecord( "Run" );
    temp_ls = event->getUserRecord( "LumiSection" );
    temp_event = event->getUserRecord( "EventNum" );

    numMuon  = m_RecEvtView->getUserRecord( "NumMuon" );
    numEle   = m_RecEvtView->getUserRecord( "NumEle" );
    numGamma = m_RecEvtView->getUserRecord( "NumGamma" );
    numTau   = m_RecEvtView->getUserRecord( "Num" + m_TauType );
    numMET   = m_RecEvtView->getUserRecord( "Num" + m_METType );
    numJet   = m_RecEvtView->getUserRecord( "Num" + m_JetAlgo );
    numBJet  = m_RecEvtView->getUserRecord_def( "Num" + m_BJets_algo,-1 );

    EleList   = new vector< pxl::Particle* >;
    MuonList  = new vector< pxl::Particle* >;
    GammaList = new vector< pxl::Particle* >;
    METList   = new vector< pxl::Particle* >;
    JetList   = new vector< pxl::Particle* >;
    TauList   = new vector< pxl::Particle* >;

    // get all particles
    vector< pxl::Particle* > AllParticles;
    m_RecEvtView->getObjectsOfType< pxl::Particle >( AllParticles );
    pxl::sortParticles( AllParticles );
    // push them into the corresponding vectors
    for( vector< pxl::Particle* >::const_iterator part_it = AllParticles.begin(); part_it != AllParticles.end(); ++part_it ) {
        pxl::Particle *part = *part_it;
        string Name = part->getName();
        part->setP4(part->getPx() * 1.05,part->getPy() * 1.05, part->getPz(),part->getE() * 0.95);
        // Only fill the collection if we want to use the particle!
        if(      Name == "Muon"    ) MuonList->push_back( part );
        else if( Name == "Ele"     ) EleList->push_back( part );
        else if( Name == "Gamma"   ) GammaList->push_back( part );
        else if( Name == m_TauType   ) TauList->push_back( part );
        else if( Name == m_METType ) METList->push_back( part );
        else if( Name == m_JetAlgo ) JetList->push_back( part );
    }

    if(METList->size()>0){
        sel_met=METList->at(0);
    }else{
        sel_met=0;
    }
    sel_lepton_prompt = 0;
    sel_lepton_nprompt = 0;
    sel_lepton_nprompt_corr = 0;

    resonance_mass = 0;
    resonance_mass_gen = 0;

    EleListGen     = new vector< pxl::Particle* >;
    MuonListGen    = new vector< pxl::Particle* >;
    GammaListGen   = new vector< pxl::Particle* >;
    METListGen     = new vector< pxl::Particle* >;
    JetListGen     = new vector< pxl::Particle* >;
    TauListGen     = new vector< pxl::Particle* >;
    S3ListGen      = new vector< pxl::Particle* >;

    if( not runOnData ){

        double event_weight = m_GenEvtView->getUserRecord( "Weight" );
        //double varKfactor_weight = m_GenEvtView->getUserRecord_def( "kfacWeight",1. );
        double pileup_weight = m_GenEvtView->getUserRecord_def( "PUWeight",1.);

        if(b_13TeV){
            weight = event_weight ;
        }else if(b_8TeV){
            weight = event_weight  * pileup_weight;
        }else{
            stringstream error;
            error << "The data period "<<m_dataPeriod<<" is not supported by this analysis!\n";
            throw Tools::config_error( error.str() );
        }

        // get all particles
        vector< pxl::Particle* > AllParticlesGen;
        m_GenEvtView->getObjectsOfType< pxl::Particle >( AllParticlesGen );
        pxl::sortParticles( AllParticlesGen );
        // push them into the corresponding vectors
        string genCollection="gen";
        if(b_8TeV){
            genCollection="S3";
        }
        for( vector< pxl::Particle* >::const_iterator part_it = AllParticlesGen.begin(); part_it != AllParticlesGen.end(); ++part_it ) {
            pxl::Particle *part = *part_it;
            string Name = part->getName();
            // Only fill the collection if we want to use the particle!
            if(      Name == "Muon"    ) MuonListGen->push_back( part );
            else if( Name == "Ele"     ) EleListGen->push_back( part );
            else if( Name == "Gamma"   ) GammaListGen->push_back( part );
            else if( Name == "Tau"     ) TauListGen->push_back( part );
            else if( Name == (m_METType+"_gen") ) METListGen->push_back( part );
            else if( Name == m_JetAlgo ) JetListGen->push_back( part );
            else if( Name == genCollection) S3ListGen->push_back( part );
        }
    }
}

void specialAna::endEvent( const pxl::Event* event ){

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

    if( not runOnData ){
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
}
