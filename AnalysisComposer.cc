#include "RPV-LFV-Analyzer/AnalysisComposer.hh"

// some standard libraries
#include <exception>
#include <sstream>
#include <stdexcept>

#include <iostream>
#include <csignal>
#include <iomanip>

// Tools for configs
#include "Tools/Tools.hh"
// Pxl Libraries
#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"

// music includes
#include "RPV-LFV-Analyzer/specialAna.hh"
#include "boost/program_options.hpp"

using std::string;

AnalysisComposer::AnalysisComposer() :
    m_analysisName("RPV-LFV-Analyzer"),
    m_outputDirectory("./MusicOutDir"),
    runOnData(false)
{}

po::options_description AnalysisComposer::getCmdArguments() {
    // You may add your analysis specific options here.
    // you should stor them as member variables of the AnalysisComposer
    // in order to access them later in addForkObjects or endAnalysis
    po::options_description myoptions("Analysis options");
    return myoptions;
}

void AnalysisComposer::endAnalysis() {
    // You may add code here which should be called after the complete
    // analysis has finished. Place e.g. merging of different ForkObject
    // results here.
}

pxl::AnalysisFork AnalysisComposer::addForkObjects(const Tools::MConfig &config,
                                        string outputDirectory,
                                        pdf::PDFInfo const &pdfInfo,
                                        EventSelector &selector,
                                        Systematics &syst_shifter,
                                        const bool debug) {
    // This is the function where you need to initalize your Analysis.
    // Create one or several implementations of pxl::AnalysisProcess and
    // add it / them to the fork which is returned by this function.
    pxl::AnalysisFork fork;
    fork.setName(m_analysisName);
    m_outputDirectory = outputDirectory;

    // add validation to fork
    specialAna *ana = 0;
    ana = new specialAna(config, syst_shifter);
    fork.insertObject(ana, "RPV-LFV-Analyzer");
    return fork;
}
