#!/usr/bin/env python

import ROOT as r
from configobj import ConfigObj

lumi = 1000

bgpath = '/disk1/erdweg/out/output2015_6_2_18_23/merged/'
bgsamples = [
 'DYToMuMu_13TeVPhys14DR-AVE20BX25_tsg_PHYS14_25_V3-v1MINI_P8',
 'DYToEE_13TeVPhys14DR-AVE20BX25_tsg_PHYS14_25_V3-v1MINI_P8',
 'WJetsToLNu_13TeVPhys14DR-PU20bx25_PHYS14_25_V1-v1MINI_MG',
 'WJetsToLNu_HT-100to200_13TeVPhys14DR-PU20bx25_PHYS14_25_V1-v1MINI_MG',
 'WJetsToLNu_HT-200to400_13TeVPhys14DR-PU20bx25_PHYS14_25_V1-v1MINI_MG',
 'WJetsToLNu_HT-400to600_13TeVPhys14DR-PU20bx25_PHYS14_25_V1-v1MINI_MG',
 'WJetsToLNu_HT-600toInf_13TeVPhys14DR-PU20bx25_PHYS14_25_V1-v1MINI_MG',
 'T_tW-channel-DR_13TeV-CSA14Phys14DR-PU20bx25_PHYS14_25_V1-v1MINI_PH',
 'Tbar_tW-channel-DR_13TeV-CSA14Phys14DR-PU20bx25_PHYS14_25_V1-v1MINI_PH',
 'QCD_Pt-15to30_13TeVPhys14DR-PU20bx25_trkalmb_castor_PHYS14_25_V1-v1MINI_P8',
 'QCD_Pt-30to50_13TeVPhys14DR-PU20bx25_trkalmb_castor_PHYS14_25_V1-v1MINI_P8',
 'QCD_Pt-50to80_13TeVPhys14DR-PU20bx25_trkalmb_castor_PHYS14_25_V1-v2MINI_P8',
 'QCD_Pt-80to120_13TeVPhys14DR-PU20bx25_trkalmb_castor_PHYS14_25_V1-v2MINI_P8',
 'QCD_Pt-120to170_13TeVPhys14DR-PU20bx25_trkalmb_castor_PHYS14_25_V1-v2MINI_P8',
 'QCD_Pt-300to470_13TeVPhys14DR-PU20bx25_trkalmb_castor_PHYS14_25_V1-v2MINI_P8',
 'QCD_Pt-470to600_13TeVPhys14DR-PU20bx25_trkalmb_castor_PHYS14_25_V1-v2MINI_P8',
 'QCD_Pt-600to800_13TeVPhys14DR-PU20bx25_trkalmb_castor_PHYS14_25_V1-v1MINI_P8',
 'QCD_Pt-800to1000_13TeVPhys14DR-PU20bx25_trkalmb_castor_PHYS14_25_V1-v2MINI_P8',
 'QCD_Pt-1000to1400_13TeVPhys14DR-PU20bx25_trkalmb_castor_PHYS14_25_V1-v1MINI_P8',
 'QCD_Pt-1400to1800_13TeVPhys14DR-PU20bx25_trkalmb_castor_PHYS14_25_V1-v1MINI_P8',
 'QCD_Pt-1800to2400_13TeVPhys14DR-PU20bx25_trkalmb_PHYS14_25_V1-v2MINI_P8',
 'QCD_Pt-2400to3200_13TeVPhys14DR-PU20bx25_trkalmb_PHYS14_25_V1-v1MINI_P8',
 'QCD_Pt-3200_13TeVPhys14DR-PU20bx25_trkalmb_PHYS14_25_V1-v1MINI_P8',
 'WZJetsTo3LNu_13TeVPhys14DR-PU20bx25_PHYS14_25_V1-v1MINI_MG',
 'ZZTo4L_13TeVPhys14DR-PU20bx25_PHYS14_25_V1-v1MINI_PH',
 'WWToLLNuNu_13TeV_PHYS14_PU20bx25_powheg',
 'TT_13TeVPhys14DR-PU20bx25_tsg_PHYS14_25_V1-v1MINI_P8',
]

sgpath = '/disk1/erdweg/out/output2015_6_2_17_17/merged/'
sgsamples = [
 'RPVresonantToEMu_M-200_LLE_LQD_001_TuneCUETP8M1_13TeV-calchep-pythia8',
 'RPVresonantToEMu_M-500_LLE_LQD_001_TuneCUETP8M1_13TeV-calchep-pythia8',
 'RPVresonantToEMu_M-1000_LLE_LQD_001_TuneCUETP8M1_13TeV-calchep-pythia8',
 'RPVresonantToEMu_M-1400_LLE_LQD_001_TuneCUETP8M1_13TeV-calchep-pythia8',
 'RPVresonantToEMu_M-2000_LLE_LQD_001_TuneCUETP8M1_13TeV-calchep-pythia8',
 'RPVresonantToEMu_M-3000_LLE_LQD_001_TuneCUETP8M1_13TeV-calchep-pythia8',
 'RPVresonantToEMu_M-4000_LLE_LQD_001_TuneCUETP8M1_13TeV-calchep-pythia8',
 'RPVresonantToEMu_M-4000_LLE_LQD_01_TuneCUETP8M1_13TeV-calchep-pythia8',
 'RPVresonantToEMu_M-4000_LLE_LQD_02_TuneCUETP8M1_13TeV-calchep-pythia8',
 'RPVresonantToEMu_M-4000_LLE_LQD_05_TuneCUETP8M1_13TeV-calchep-pythia8',
 'RPVresonantToEMu_M-5000_LLE_LQD_001_TuneCUETP8M1_13TeV-calchep-pythia8',
 'RPVresonantToEMu_M-6000_LLE_LQD_001_TuneCUETP8M1_13TeV-calchep-pythia8'
]

out_file_name = 'test.root'
hist_name = 'emu/Stage_0/h1_0_emu_Mass'

sys_hist_name = 'emu/Stage_0/sys/h1_0_emu_Mass'
sys_labels = [
 '_Ele_syst_ScaleUp',
 '_Ele_syst_ScaleDown',
 '_Tau_syst_ScaleUp',
 '_Tau_syst_ScaleDown',
 '_MET_syst_ScaleUp',
 '_MET_syst_ScaleDown',
 '_Muon_syst_ScaleUp',
 '_Muon_syst_ScaleDown',
 '_Muon_syst_ResolutionUp',
 '_Muon_syst_ResolutionDown',
 '_Jet_syst_ScaleUp',
 '_Jet_syst_ScaleDown',
 '_Jet_syst_ResolutionUp',
 '_Jet_syst_ResolutionDown',
]

def get_hist_from_file(path, hist_name_temp, file_name, weight = 1):
    in_file = r.TFile(path + file_name + '.root', 'READ')
    hist = in_file.Get(hist_name_temp)
    try:
        hist.SetDirectory(0)
    except(AttributeError):
        print('can not assign directory')
        print(' In file: %s, histogram: %s'%(file_name, hist_name_temp))
    in_file.Close()
    hist.Scale(weight)
    return hist

def write_sample(path, item, xs):
    sample_name = item

    Nev=0
    try:
        counter = get_hist_from_file(path, "h_counters", item)
        Nev=max(counter.GetBinContent(1),counter.GetBinContent(2))
        del counter
    except:
        print("[Info] If you want to use Nev from the root file store 'h_counters'")
        Nev=xs[item].as_float("Nev")

    BGtotalweight=xs[item].as_float("xs")*xs[item].as_float("weight")*lumi/Nev

    hist = get_hist_from_file(path, hist_name, item, BGtotalweight)

    sys_hists = []
    for name in sys_labels:
        sys_hists.append(get_hist_from_file(path, sys_hist_name + name, item, BGtotalweight))

    out_file = r.TFile(out_file_name, 'UPDATE')
    out_file.mkdir(sample_name)
    out_file.Cd(sample_name + '/')

    hist.Write()

    out_file.Cd('../')
    out_file.mkdir(sample_name + '/sys/')
    out_file.Cd(sample_name + '/sys/')

    for thingy in sys_hists:
        thingy.Write()

    out_file.Close()

def write_sum():
    counting = 0
    for item in bgsamples:
        print(item)
        if counting == 0:
            dummy_hist = get_hist_from_file('./', item + '/' + sys_hist_name.split('/')[-1], out_file_name.replace('.root',''))
        else:
            dummy_hist.Add(get_hist_from_file('./', item + '/' + sys_hist_name.split('/')[-1], out_file_name.replace('.root','')))
        counting += 1

    out_file = r.TFile(out_file_name, 'UPDATE')

    dummy_hist.Write()

    out_file.Close()
    del dummy_hist

    for name in sys_labels:
        counting = 0
        for item in bgsamples:
            if counting == 0:
                dummy_hist = get_hist_from_file('./', item + '/sys/' + sys_hist_name.split('/')[-1] + name, out_file_name.replace('.root',''))
            else:
                dummy_hist.Add(get_hist_from_file('./', item + '/sys/' + sys_hist_name.split('/')[-1] + name, out_file_name.replace('.root','')))
            counting += 1

        out_file = r.TFile(out_file_name, 'UPDATE')

        out_file.Cd('sys/')

        dummy_hist.Write()

        out_file.Close()
        del dummy_hist

def main():
    try: 
        xs = ConfigObj('/disk1/erdweg/plotting/xs_Phys14.cfg')
    except:
        print("no xs file /disk1/erdweg/plotting/xs_Phys14.cfg")
        exit()

    out_file = r.TFile(out_file_name, 'RECREATE')
    out_file.mkdir('sys/')
    out_file.Close()
    for item in bgsamples:
        write_sample(bgpath, item, xs)
    for item in sgsamples:
        write_sample(sgpath, item, xs)
    write_sum()

if __name__ == '__main__':
    main()
