/*-------------------------------------------------------------------------*
 | $Id::                                                                 $ |
 *-------------------------------------------------------------------------*
 ***************************************************************************
 *   Copyright (C) 2010                                                    *
 *   Frank Schröder , Nunzia Palmieri                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <time.h>

#include <TFile.h>
#include <TTree.h>
#include <TMath.h>
#include <TCut.h>
#include <Rtypes.h>

#include <boost/lexical_cast.hpp>

#include <crtools.h>
#include <Analysis/lateralDistribution.h>
#include <Analysis/PulseProperties.h>

// using CR::analyseLOPESevent2;
// using CR::LopesEventIn;

/// all the formula are developed considering angles into the LOPES coordinates system!

/*!
  \file compareLOPES2sim.cc

  \ingroup CR_Applications

  \brief Compares lateral distribution of LOPES events to REAS simulation

  \author Frank Schröder, Nunzia Palmieri

  \date 2008/20/05

  <h3>Motivation</h3>

 
  <h3>Usage</h3>

  \verbatim
  ./compareLOPES2sim --data results.root  --dict dictionary.txt  --path ../reas3simulations/lopeseventsR3i/
  \endverbatim

  <ul>
    Optional argumens:
    <li> --help                           prints a help message
  </ul>

  <h3>Prerequisites</h3>

  ROOT
  
  <h3>Examples</h3>

  \verbatim
  ./compareLOPES2sim --help
  \endverbatim

  <h3>Usage of the dictionary file</h3>
 #LOPES-GT,simName,source,KRETA_ver,az_in(deg),errAz_in(deg),ze_in(deg),errZe_in(deg),coreN_in(m),coreE_in(m),errCore(m),Ne_in,Nmu_in,Nmu^tr_in,E_in(eV),errlgE
*/

const static bool simulationDistances = true; //true;  // decide wether to use the lateral distances of simulation or of data
const static double gradeg=(180./TMath::Pi());
//const static double antennaHeightOffset = 12615.9 -8.17; // used in REAS, for Steffen's events (corresponds to average antenna height)
const static double antennaHeightOffset = 11000+14; // height of antenna 1 used for REAS
// time offset, because ground plane in REAS is defined as average of antenna heights, not as KASCADE height or 0
const static double groundPlaneTimeOffset = 0;
//const static double groundPlaneTimeOffset = 0.0817 * 3.33564; // height diff = 14 cm, conversion to ns
const static double generalREAStimeOffset = 2.4; // general offset to avoid negative times with REAS (fine tuned by hand)


/*
  Function to read Xmax of CORSIKA shower used for REAS input from .reas file
*/
double reasXmaxFromREAS(string filename)
{
  double Xmax = 0;
  
  cout << "\nReading Xmax from file: " << filename << endl;
  
  // parse .reas file line by line and search for "DepthOfShowerMaximum = ...  ; ..."
  ifstream file(filename.c_str());
  if(file.is_open()) {
    while(!file.eof()) {
      string line;
      getline(file, line);
      if(line.find_first_of("=") != string::npos) {
        string varName = line.substr(0, line.find_first_of("="));
        // look for comments when reading the value
        string value("");
        if(line.find_first_of(";") != string::npos)
          value = line.substr(line.find_first_of("=") + 1, line.find_first_of(";")-line.find_first_of("=")-1);
        else  
          value = line.substr(line.find_first_of("=") + 1, line.length());
          
        //cout << varName << " = " << value << endl;

        if (varName.find("DepthOfShowerMaximum") != string::npos) {
          stringstream tempstream(value);
          tempstream >> Xmax;       
          break; // no need to read further lines
        }  
      }    
    }
  } else {
    cerr << "\nError: Cannot open file: "<< filename  << endl;
    return 0;
  }
  
  file.close();
  
  return Xmax;
}


int main (int argc, char *argv[])
{
  try {
    if(argc<=1) {
      cout << "\n Application to generate lateral distribution plots for both LOPES data and simulations,\n"
           << "based on the ROOT file generated by call_pipeline and REAS3 simulations.\n"
           << "Type: ./compareLOPES2sim --help"
           << endl;
      return 1;
    } else {
      cout << "\nStarting compareLOPES2sim.\n" << endl;
    }

    // check arguments
    string resultsName1(""), resultsName2(""), simDictName(""), simPath("");
    // simPath e.g. = /lxdata/d2lx68/huege/lopeseventsR3i
    // or /data/d1lp4/LOPES/simFrank_p_ew_K 
    string index1(""), index2("");  // name indices for lateral distribution
    string outPrefix("simANDrec_fitvalue");  // prefix for output filename
    for (int i=1; i < argc; ++i) {
      // get option and argument
      string option(argv[i]);
      ++i;
      // look for keywords which require no argument
      if ( (option == "--help") || (option == "-help")
        || (option == "--h") || (option == "-h")) {
	cout << "Use:\n"
             << "./compareLOPES2sim --data results1.root --data2 results2.root\n"
             << "or\n"
             << "./compareLOPES2sim --data results.root --dict simDictionary.txt --path pathToSimulations\n"
	     << endl;
	return 0;       // exit here
      } 
      // all other options require also an argument
      if (i >= argc) {
        cerr << "ERROR: No argument given for option or unknown option: \"" << option << "\"\n";
        cerr << "Use --help for more information." << endl;
        return 1;                       // Exit the program if argument is missing
      }
      string argument(argv[i]);
      
      // look for keywords which require an option
      if ( (option == "--data") || (option == "-data")
         ||(option == "--data1") || (option == "-data1")) {
        resultsName1 = argument;
        continue;
      }

      if ( (option == "--data2") || (option == "-data2")) {
        resultsName2 = argument;
        continue;
      }

      if ( (option == "--dict") || (option == "-dict")) {
        simDictName = argument;
        continue;
      }

      if ( (option == "--path") || (option == "-path")) {
        simPath = argument;
        continue;
      }

      if ( (option == "--out") || (option == "-out")
        || (option == "--name") || (option == "-name")) {
        outPrefix = argument;
        continue;
      }

      // if loop comes here, the option was not found
      cerr << "ERROR: Invalid option: \"" << option << "\"\n";
      cerr << "Use --help for more information." << endl;
      return 1;                 // Exit the program if argument is missing
    }

    // check consistency
    if ( (resultsName1=="") || (((simPath=="")||(simDictName=="")) && (resultsName2=="")) ) {
      cerr << "Error: missing / false options specified.\n";
      cerr << "Use --help for more information." << endl;
      return 1;                 
    }
    double AzDictS=0.,ZeDictS=0.;  //from the dict file, azimuth and zenith used for the simulations (so or from KA or from Grande)
    string source, kreta;

    // map dic declaration
    map<int,string> m_dict;
    map<int,double> m_dictAz;
    map<int,double> m_dictZe;

    
    // open dictionary with simulations, if it is provided
    if (simDictName!="") {
      // set indices
      index1 = "dat";
      index2 = "sim";
    
      int GtDict=0;
      string simNameDict;
      double dummy;
      ifstream dictFile(simDictName.c_str());
      if (!dictFile.is_open()) {
        cerr << "ERROR ----- CAN NOT OPEN THE DICTIONARY FILE!" << endl;
        return 1;  
      }
      char buffer[1024];
    
      while (dictFile.is_open()) {
        dictFile.getline(buffer,1024);
        istringstream iss (buffer);
        if(iss.str().size()>0&&iss.str()[0]!='%'&&iss.str()[0]!='#') {

          //#LOPES-GT,simName,source,KRETA_ver,az_in(deg),errAz_in(deg),ze_in(deg),errZe_in(deg),coreN_in(m),coreE_in(m),errCore(m),Ne_in,Nmu_in,Nmu^tr_in,E_in(eV),errlgE
          iss>> GtDict>>simNameDict>>source>>kreta>>AzDictS>>dummy>>ZeDictS>>dummy>>dummy>>dummy>>dummy>>dummy>>dummy>>dummy>>dummy>>dummy;
          m_dict[GtDict]=simNameDict; //fill the map dictionary 
          m_dictAz[GtDict]=AzDictS;
          m_dictZe[GtDict]=ZeDictS;
          cout<<""<<simNameDict <<" - GT " <<GtDict<<endl;
          cout<<"from dict file Az: "<<AzDictS<<"deg ; zeS: " <<ZeDictS<<"deg "<<endl;
        }
        if (!dictFile.good())
          break;
      } // while...
      dictFile.close();
    }  

    /*root file from call_pipeline*/
    int totAntenna = 30;

    // open root file for input
    cout << "\nOpening root file for data input: " << resultsName1 << endl;
    TFile *recRoot = new TFile (resultsName1.c_str(), "READ");
    if(!recRoot || recRoot->IsZombie()) {
      cerr << "Error, cannot open root file: "<< resultsName1 << endl;
      return 1;  
    }       
    
    TTree *recTree =  (TTree*)recRoot->Get("T;1");
    //if(!recTree) return;

    // open root file for output
    TFile *rootOutfile=NULL;
    string outputRootFileName = outPrefix + ".root";
    cout << "\nOpening root file for output: " << outputRootFileName << endl;
    rootOutfile = new TFile(outputRootFileName.c_str(),"RECREATE","Comparison of Lateral Distribution");
    if (rootOutfile->IsZombie()) {
      cerr << "\nError: Could not create file: " << outputRootFileName << "\n" << endl;
      return 1;               // exit program
    }
    TTree *outtree;
    outtree = new TTree("T","LateralComparison");

    //PulseProperties[totAntenna] antPulses; //each antenna 1 pulseProperties
    PulseProperties* antPulses[totAntenna];
    PulseProperties* antPulses2[totAntenna];  // for eventual second root file

    unsigned int Gt = 0;
    float_t Az = 0, Ze = 0, Xc = 0, Yc = 0;                 // KASCADE direction and core
    float_t Azg = 0, Zeg = 0, Xcg = 0, Ycg = 0;             // Grande direction and core
    float_t Size = 0, Sizeg = 0;                            // Electron numbers (KASCADE + Grande)
    float_t Age = 0, Ageg = 0;                              // Age parameter
    float_t Nmu = 0, Lmuo = 0, Sizmg = 0;                   // Muon number, trucated muon number (KASCADE), Muon number (Grande)
    double_t lgE = 0, err_lgE = 0;                          // estimated energy (KASCADE)
    double_t lgEg = 0, err_lgEg = 0;                        // estimated energy (Grande)
    double_t lnA = 0, err_lnA = 0;                          // estimated mass A (KASCADE)
    double_t lnAg = 0, err_lnAg = 0;                        // estimated mass A (Grande)
    double_t kappaMario = 0, lgEMario = 0;                  // mass and energy by Mario's formula
    double_t err_core = 0, err_coreg = 0;                   // error of core position (KASCADE + Grande)
    double_t err_Az = 0, err_Azg = 0;                       // error of azimuth (KASCADE + Grande)
    double_t err_Ze = 0, err_Zeg = 0;                       // error of zenith (KASCADE + Grande)
    double_t geomag_angle = 0, geomag_angleg = 0;           // geomagnetic angle (KASCADE + Grande)
    char KRETAver[1024] = "unknown";
    double_t EfieldMaxAbs = 0;                              // maximum of the absolute e-field strength around +/- 15 min
    double_t EfieldAvgAbs = 0;                              // average of the absolute e-field strength around +/- 15 min    
    char reconstruction = 0;                                // A = KASCADE reconstruction taken, G = Grande reconstruction taken
    bool hasNS = false;                                     // is set to true, if root file contains NS information
    bool hasVE = false;                                     // is set to true, if root file contains VE information
    double CCheight, CCheight_NS, CCheight_VE;                          // CCheight will be used for EW polarization or ANY polarization
    double CCwidth, CCwidth_NS, CCwidth_VE;
    double CCcenter, CCcenter_NS, CCcenter_VE;                    // time of CC beam
    double CCheight_error, CCheight_error_NS, CCheight_error_VE;
    //bool CCconverged, CCconverged_NS, CCconverged_VE;                       // is true if the Gaussian fit to the CCbeam converged
    double Xheight, Xheight_NS, Xheight_VE;                            // CCheight will be used for EW polarization or ANY polarization
    double Xheight_error, Xheight_error_NS, Xheight_error_VE;
    //bool Xconverged, Xconverged_NS, Xconverged_VE;                         // is true if the Gaussian fit to the CCbeam converged
    double AzL, ElL, AzL_NS, ElL_NS, AzL_VE, ElL_VE;                       // Azimuth and Elevation
    double distanceResult = 0, distanceResult_NS = 0, distanceResult_VE = 0;       // distance = radius of curvature
    // values for lateral distribution of arrival times
    double latTimeSphere1DRcurv_EW = 0, latTimeSphere1DRcurv_NS = 0, latTimeSphere1DRcurv_VE = 0;
    double latTimeSphere1DSigRcurv_EW = 0, latTimeSphere1DSigRcurv_NS = 0, latTimeSphere1DSigRcurv_VE = 0;
    double latTimeSphere1DChi2NDF_EW = 0, latTimeSphere1DChi2NDF_NS = 0, latTimeSphere1DChi2NDF_VE = 0;
    double latTimeSphere2DRcurv_EW = 0, latTimeSphere2DRcurv_NS = 0, latTimeSphere2DRcurv_VE = 0;
    double latTimeSphere2DSigRcurv_EW = 0, latTimeSphere2DSigRcurv_NS = 0, latTimeSphere2DSigRcurv_VE = 0;
    double latTimeSphere2DChi2NDF_EW = 0, latTimeSphere2DChi2NDF_NS = 0, latTimeSphere2DChi2NDF_VE = 0;
    
    double latTimeSphere1DRcurv_sim_EW = 0, latTimeSphere1DRcurv_sim_NS = 0, latTimeSphere1DRcurv_sim_VE = 0;
    double latTimeSphere1DSigRcurv_sim_EW = 0, latTimeSphere1DSigRcurv_sim_NS = 0, latTimeSphere1DSigRcurv_sim_VE = 0;
    double latTimeSphere1DChi2NDF_sim_EW = 0, latTimeSphere1DChi2NDF_sim_NS = 0, latTimeSphere1DChi2NDF_sim_VE = 0;
    double latTimeSphere2DRcurv_sim_EW = 0, latTimeSphere2DRcurv_sim_NS = 0, latTimeSphere2DRcurv_sim_VE = 0;
    double latTimeSphere2DSigRcurv_sim_EW = 0, latTimeSphere2DSigRcurv_sim_NS = 0, latTimeSphere2DSigRcurv_sim_VE = 0;
    double latTimeSphere2DChi2NDF_sim_EW = 0, latTimeSphere2DChi2NDF_sim_NS = 0, latTimeSphere2DChi2NDF_sim_VE = 0;
    
    double latTimeCone1DRho_EW = 0, latTimeCone1DRho_NS = 0, latTimeCone1DRho_VE = 0;
    double latTimeCone1DSigRho_EW = 0, latTimeCone1DSigRho_NS = 0, latTimeCone1DSigRho_VE = 0;
    double latTimeCone1DChi2NDF_EW = 0, latTimeCone1DChi2NDF_NS = 0, latTimeCone1DChi2NDF_VE = 0;
    double latTimeCone2DRho_EW = 0, latTimeCone2DRho_NS = 0, latTimeCone2DRho_VE = 0;
    double latTimeCone2DSigRho_EW = 0, latTimeCone2DSigRho_NS = 0, latTimeCone2DSigRho_VE = 0;
    double latTimeCone2DChi2NDF_EW = 0, latTimeCone2DChi2NDF_NS = 0, latTimeCone2DChi2NDF_VE = 0;
 
    double latTimeCone1DRho_sim_EW = 0, latTimeCone1DRho_sim_NS = 0, latTimeCone1DRho_sim_VE = 0;
    double latTimeCone1DSigRho_sim_EW = 0, latTimeCone1DSigRho_sim_NS = 0, latTimeCone1DSigRho_sim_VE = 0;
    double latTimeCone1DChi2NDF_sim_EW = 0, latTimeCone1DChi2NDF_sim_NS = 0, latTimeCone1DChi2NDF_sim_VE = 0;
    double latTimeCone2DRho_sim_EW = 0, latTimeCone2DRho_sim_NS = 0, latTimeCone2DRho_sim_VE = 0;
    double latTimeCone2DSigRho_sim_EW = 0, latTimeCone2DSigRho_sim_NS = 0, latTimeCone2DSigRho_sim_VE = 0;
    double latTimeCone2DChi2NDF_sim_EW = 0, latTimeCone2DChi2NDF_sim_NS = 0, latTimeCone2DChi2NDF_sim_VE = 0;
    
    //bool goodEW = false=0, goodNS = false=0, goodVE = false=0;                // true if reconstruction worked
    double rmsCCbeam=0, rmsCCbeam_NS=0, rmsCCbeam_VE=0;                        // rms values of the beams in remote region
    double rmsXbeam=0, rmsXbeam_NS=0, rmsXbeam_VE=0;
    double rmsPbeam=0, rmsPbeam_NS=0, rmsPbeam_VE=0;
    int NCCbeamAntennas = 0, NlateralAntennas = 0; // antennas used for CC beam and lateral distribution
    int NCCbeamAntennas_NS = 0, NlateralAntennas_NS = 0; // antennas used for CC beam and lateral distribution
    int NCCbeamAntennas_VE = 0, NlateralAntennas_VE = 0; // antennas used for CC beam and lateral distribution
    double latMeanDist=0, latMeanDist_NS=0, latMeanDist_VE=0;                 // mean distance of the antennas in the lateral distribution
    double latMinDist=0, latMinDist_NS=0, latMinDist_VE=0;                 // minimum distance of the antennas in the lateral distribution
    double latMaxDist=0, latMaxDist_NS=0, latMaxDist_VE=0;                 // maximum distance of the antennas in the lateral distribution
    double latMeanDistCC=0, latMeanDistCC_NS=0, latMeanDistCC_VE=0;             // mean distance of the antennas used for the CC beam
    double ratioDiffSign=0, ratioDiffSign_NS=0, ratioDiffSign_VE=0;
    double ratioDiffSignEnv=0, ratioDiffSignEnv_NS=0, ratioDiffSignEnv_VE=0;
    double weightedTotSign=0,weightedTotSign_NS=0,weightedTotSign_VE=0;
    double weightedTotSignEnv=0,weightedTotSignEnv_NS=0,weightedTotSignEnv_VE=0;
    
    // results of lateral distribution fit
    double R_0_EW = 0, sigR_0_EW = 0, eps_EW = 0, sigeps_EW = 0, chi2NDF_EW = 0; 
    double R_0_sim_EW = 0, sigR_0_sim_EW = 0, eps_sim_EW = 0, sigeps_sim_EW = 0, chi2NDF_sim_EW = 0; 
    double R_0_NS = 0, sigR_0_NS = 0, eps_NS = 0, sigeps_NS = 0, chi2NDF_NS = 0; 
    double R_0_sim_NS = 0, sigR_0_sim_NS = 0, eps_sim_NS = 0, sigeps_sim_NS = 0, chi2NDF_sim_NS = 0; 
    double R_0_VE = 0, sigR_0_VE = 0, eps_VE = 0, sigeps_VE = 0, chi2NDF_VE = 0; 
    double R_0_sim_VE = 0, sigR_0_sim_VE = 0, eps_sim_VE = 0, sigeps_sim_VE = 0, chi2NDF_sim_VE = 0; 
    
    double Xmax_sim = 0;
    
    recTree->SetBranchAddress("Gt", &Gt);
    recTree->SetBranchAddress("Xc",&Xc);
    recTree->SetBranchAddress("Xcg",&Xcg);
    recTree->SetBranchAddress("Yc",&Yc);
    recTree->SetBranchAddress("Ycg",&Ycg);
    recTree->SetBranchAddress("Az",&Az);
    recTree->SetBranchAddress("Azg",&Azg);
    recTree->SetBranchAddress("Ze",&Ze);
    recTree->SetBranchAddress("Zeg",&Zeg);
    recTree->SetBranchAddress("Size",&Size);
    recTree->SetBranchAddress("Sizeg",&Sizeg);
    recTree->SetBranchAddress("Age",&Age);
    recTree->SetBranchAddress("Ageg",&Ageg);
    recTree->SetBranchAddress("Nmu",&Nmu);
    recTree->SetBranchAddress("Lmuo",&Lmuo);
    recTree->SetBranchAddress("Sizmg",&Sizmg);
    recTree->SetBranchAddress("lgE",&lgE);
    recTree->SetBranchAddress("lgEg",&lgEg);
    recTree->SetBranchAddress("lnA",&lnA);
    recTree->SetBranchAddress("lnAg",&lnAg);
    recTree->SetBranchAddress("kappaMario",&kappaMario);
    recTree->SetBranchAddress("lgEMario",&lgEMario);
    recTree->SetBranchAddress("err_lgE",&err_lgE);
    recTree->SetBranchAddress("err_lgEg",&err_lgEg);
    recTree->SetBranchAddress("err_lnA",&err_lnA);
    recTree->SetBranchAddress("err_lnAg",&err_lnAg);
    recTree->SetBranchAddress("err_core",&err_core);
    recTree->SetBranchAddress("err_coreg",&err_coreg);
    recTree->SetBranchAddress("err_Az",&err_Az);
    recTree->SetBranchAddress("err_Azg",&err_Azg);
    recTree->SetBranchAddress("err_Ze",&err_Ze);
    recTree->SetBranchAddress("err_Zeg",&err_Zeg);
    recTree->SetBranchAddress("geomag_angle",&geomag_angle);
    recTree->SetBranchAddress("geomag_angleg",&geomag_angleg);
    recTree->SetBranchAddress("reconstruction",&reconstruction);
    recTree->SetBranchAddress("KRETAver",&KRETAver);
    recTree->SetBranchAddress("EfieldMaxAbs",&EfieldMaxAbs);
    recTree->SetBranchAddress("EfieldAvgAbs",&EfieldAvgAbs);

    outtree->Branch("Gt",&Gt,"Gt/i");
    outtree->Branch("Xc",&Xc,"Xc/F");
    outtree->Branch("Xcg",&Xcg,"Xcg/F");
    outtree->Branch("Yc",&Yc,"Yc/F");
    outtree->Branch("Ycg",&Ycg,"Ycg/F");
    outtree->Branch("Az",&Az,"Az/F");
    outtree->Branch("Azg",&Azg,"Azg/F");
    outtree->Branch("Ze",&Ze,"Ze/F");
    outtree->Branch("Zeg",&Zeg,"Zeg/F");
    outtree->Branch("Size",&Size,"Size/F");
    outtree->Branch("Sizeg",&Sizeg,"Sizeg/F");
    outtree->Branch("Age",&Age,"Age/F");
    outtree->Branch("Ageg",&Ageg,"Ageg/F");
    outtree->Branch("Nmu",&Nmu,"Nmu/F");
    outtree->Branch("Lmuo",&Lmuo,"Lmuo/F");
    outtree->Branch("Sizmg",&Sizmg,"Sizmg/F");
    outtree->Branch("lgE",&lgE,"lgE/D");
    outtree->Branch("lgEg",&lgEg,"lgEg/D");
    outtree->Branch("lnA",&lnA,"lnA/D");
    outtree->Branch("lnAg",&lnAg,"lnAg/D");
    outtree->Branch("kappaMario",&kappaMario,"kappaMario/D");
    outtree->Branch("lgEMario",&lgEMario,"lgEMario/D");
    outtree->Branch("err_lgE",&err_lgE,"err_lgE/D");
    outtree->Branch("err_lgEg",&err_lgEg,"err_lgEg/D");
    outtree->Branch("err_lnA",&err_lnA,"err_lnA/D");
    outtree->Branch("err_lnAg",&err_lnAg,"err_lnAg/D");
    outtree->Branch("err_core",&err_core,"err_core/D");
    outtree->Branch("err_coreg",&err_coreg,"err_coreg/D");
    outtree->Branch("err_Az",&err_Az,"err_Az/D");
    outtree->Branch("err_Azg",&err_Azg,"err_Azg/D");
    outtree->Branch("err_Ze",&err_Ze,"err_Ze/D");
    outtree->Branch("err_Zeg",&err_Zeg,"err_Zeg/D");
    outtree->Branch("geomag_angle",&geomag_angle,"geomag_angle/D");
    outtree->Branch("geomag_angleg",&geomag_angleg,"geomag_angleg/D");
    outtree->Branch("reconstruction",&reconstruction,"reconstruction/B");
    outtree->Branch("KRETAver",&KRETAver,"KRETAver/C");
    outtree->Branch("EfieldMaxAbs",&EfieldMaxAbs,"EfieldMaxAbs/D");
    outtree->Branch("EfieldAvgAbs",&EfieldAvgAbs,"EfieldAvgAbs/D");
    outtree->Branch("Xmax_sim", &Xmax_sim, "Xmax_sim/D");

    // look for existing EW branches
    TObjArray* existingBranches = recTree->GetListOfBranches();
    if (existingBranches->FindObject("AzL_EW")==0) {
      cerr << "Error: No EW polarization data found in root file." << endl;
      return 1;
    } else {
      recTree->SetBranchAddress("AzL_EW",&AzL);
      recTree->SetBranchAddress("ElL_EW",&ElL);
      recTree->SetBranchAddress("Distance_EW",&distanceResult);        // radius of curvature
      recTree->SetBranchAddress("CCheight_EW",&CCheight);
      recTree->SetBranchAddress("CCwidth_EW",&CCwidth);
      recTree->SetBranchAddress("CCcenter_EW",&CCcenter);
      recTree->SetBranchAddress("CCheight_error_EW",&CCheight_error);
      //recTree->SetBranchAddress("CCconverged_EW",&CCconverged);
      recTree->SetBranchAddress("Xheight_EW",&Xheight);
      recTree->SetBranchAddress("Xheight_error_EW",&Xheight_error);
      //recTree->SetBranchAddress("Xconverged_EW",&Xconverged);
      //recTree->SetBranchAddress("goodReconstructed_EW",&goodEW);
      recTree->SetBranchAddress("rmsCCbeam_EW",&rmsCCbeam);
      recTree->SetBranchAddress("rmsXbeam_EW",&rmsXbeam);
      recTree->SetBranchAddress("rmsPbeam_EW",&rmsPbeam);
      recTree->SetBranchAddress("latMeanDistCC_EW",&latMeanDistCC);
      recTree->SetBranchAddress("NCCbeamAntennas_EW",&NCCbeamAntennas);
      recTree->SetBranchAddress("ratioDiffSign_EW",&ratioDiffSign);
      recTree->SetBranchAddress("ratioDiffSignEnv_EW",&ratioDiffSignEnv);
      recTree->SetBranchAddress("weightedTotSign_EW",&weightedTotSign);
      recTree->SetBranchAddress("weightedTotSignEnv_EW",&weightedTotSignEnv);
      recTree->SetBranchAddress("latMeanDist_EW",&latMeanDist);
      recTree->SetBranchAddress("latMinDist_EW",&latMinDist);
      recTree->SetBranchAddress("latMaxDist_EW",&latMaxDist);
      recTree->SetBranchAddress("NlateralAntennas_EW",&NlateralAntennas);
      
      outtree->Branch("AzL_EW",&AzL,"AzL_EW/D");
      outtree->Branch("ElL_EW",&ElL,"ElL_EW/D");
      outtree->Branch("Distance_EW",&distanceResult,"Distance_EW/D");        // radius of curvature
      outtree->Branch("CCheight_EW",&CCheight,"CCheight_EW/D");
      outtree->Branch("CCwidth_EW",&CCwidth,"CCwidth_EW/D");
      outtree->Branch("CCcenter_EW",&CCcenter,"CCcenter_EW/D");
      outtree->Branch("CCheight_error_EW",&CCheight_error,"CCheight_error_EW/D");
      //outtree->Branch("CCconverged_EW",&CCconverged,"CCconverged_EW/B");
      outtree->Branch("Xheight_EW",&Xheight,"Xheight_EW/D");
      outtree->Branch("Xheight_error_EW",&Xheight_error,"Xheight_error_EW/D");
      //outtree->Branch("Xconverged_EW",&Xconverged,"Xconverged_EW/B");
      //outtree->Branch("goodReconstructed_EW",&goodEW,"goodReconstructed_EW/B");
      outtree->Branch("rmsCCbeam_EW",&rmsCCbeam,"rmsCCbeam_EW/D");
      outtree->Branch("rmsXbeam_EW",&rmsXbeam,"rmsXbeam_EW/D");
      outtree->Branch("rmsPbeam_EW",&rmsPbeam,"rmsPbeam_EW/D");
      outtree->Branch("latMeanDistCC_EW",&latMeanDistCC,"latMeanDistCC_EW/D");
      outtree->Branch("NCCbeamAntennas_EW",&NCCbeamAntennas,"NCCbeamAntennas_EW/I");
      outtree->Branch("ratioDiffSign_EW",&ratioDiffSign,"ratioDiffSign_EW/D");
      outtree->Branch("ratioDiffSignEnv_EW",&ratioDiffSignEnv,"ratioDiffSignEnv_EW/D");
      outtree->Branch("weightedTotSign_EW",&weightedTotSign,"weightedTotSign_EW/D");
      outtree->Branch("weightedTotSignEnv_EW",&weightedTotSignEnv,"weightedTotSignEnv_EW/D");
      outtree->Branch("latMeanDist_EW",&latMeanDist,"latMeanDist_EW/D");
      outtree->Branch("latMinDist_EW",&latMinDist,"latMinDist_EW/D");
      outtree->Branch("latMaxDist_EW",&latMaxDist,"latMaxDist_EW/D");
      outtree->Branch("NlateralAntennas_EW",&NlateralAntennas,"NlateralAntennas_EW/I");
      
      outtree->Branch("latTimeSphere1DRcurv_EW",&latTimeSphere1DRcurv_EW,"latTimeSphere1DRcurv_EW/D");
      outtree->Branch("latTimeSphere1DSigRcurv_EW",&latTimeSphere1DSigRcurv_EW,"latTimeSphere1DSigRcurv_EW/D");
      outtree->Branch("latTimeSphere1DChi2NDF_EW",&latTimeSphere1DChi2NDF_EW,"latTimeSphere1DChi2NDF_EW/D");
      outtree->Branch("latTimeSphere2DRcurv_EW",&latTimeSphere2DRcurv_EW,"latTimeSphere2DRcurv_EW/D");
      outtree->Branch("latTimeSphere2DSigRcurv_EW",&latTimeSphere2DSigRcurv_EW,"latTimeSphere2DSigRcurv_EW/D");
      outtree->Branch("latTimeSphere2DChi2NDF_EW",&latTimeSphere2DChi2NDF_EW,"latTimeSphere2DChi2NDF_EW/D");
      
      outtree->Branch("latTimeSphere1DRcurv_sim_EW",&latTimeSphere1DRcurv_sim_EW,"latTimeSphere1DRcurv_sim_EW/D");
      outtree->Branch("latTimeSphere1DSigRcurv_sim_EW",&latTimeSphere1DSigRcurv_sim_EW,"latTimeSphere1DSigRcurv_sim_EW/D");
      outtree->Branch("latTimeSphere1DChi2NDF_sim_EW",&latTimeSphere1DChi2NDF_sim_EW,"latTimeSphere1DChi2NDF_sim_EW/D");      
      outtree->Branch("latTimeSphere2DRcurv_sim_EW",&latTimeSphere2DRcurv_sim_EW,"latTimeSphere2DRcurv_sim_EW/D");
      outtree->Branch("latTimeSphere2DSigRcurv_sim_EW",&latTimeSphere2DSigRcurv_sim_EW,"latTimeSphere2DSigRcurv_sim_EW/D");
      outtree->Branch("latTimeSphere2DChi2NDF_sim_EW",&latTimeSphere2DChi2NDF_sim_EW,"latTimeSphere2DChi2NDF_sim_EW/D");
      
      outtree->Branch("latTimeCone1DRho_EW",&latTimeCone1DRho_EW,"latTimeCone1DRho_EW/D");
      outtree->Branch("latTimeCone1DSigRho_EW",&latTimeCone1DSigRho_EW,"latTimeCone1DSigRho_EW/D");
      outtree->Branch("latTimeCone1DChi2NDF_EW",&latTimeCone1DChi2NDF_EW,"latTimeCone1DChi2NDF_EW/D");
      outtree->Branch("latTimeCone2DRho_EW",&latTimeCone2DRho_EW,"latTimeCone2DRho_EW/D");
      outtree->Branch("latTimeCone2DSigRho_EW",&latTimeCone2DSigRho_EW,"latTimeCone2DSigRho_EW/D");
      outtree->Branch("latTimeCone2DChi2NDF_EW",&latTimeCone2DChi2NDF_EW,"latTimeCone2DChi2NDF_EW/D");
      
      outtree->Branch("latTimeCone1DRho_sim_EW",&latTimeCone1DRho_sim_EW,"latTimeCone1DRho_sim_EW/D");
      outtree->Branch("latTimeCone1DSigRho_sim_EW",&latTimeCone1DSigRho_sim_EW,"latTimeCone1DSigRho_sim_EW/D");
      outtree->Branch("latTimeCone1DChi2NDF_sim_EW",&latTimeCone1DChi2NDF_sim_EW,"latTimeCone1DChi2NDF_sim_EW/D");      
      outtree->Branch("latTimeCone2DRho_sim_EW",&latTimeCone2DRho_sim_EW,"latTimeCone2DRho_sim_EW/D");
      outtree->Branch("latTimeCone2DSigRho_sim_EW",&latTimeCone2DSigRho_sim_EW,"latTimeCone2DSigRho_sim_EW/D");
      outtree->Branch("latTimeCone2DChi2NDF_sim_EW",&latTimeCone2DChi2NDF_sim_EW,"latTimeCone2DChi2NDF_sim_EW/D");
      
      outtree->Branch("R_0_EW",&R_0_EW,"R_0_EW/D");
      outtree->Branch("sigR_0_EW",&sigR_0_EW,"sigR_0_EW/D");
      outtree->Branch("eps_EW",&eps_EW,"eps_EW/D");
      outtree->Branch("sigeps_EW",&sigeps_EW,"sigeps_EW/D");
      outtree->Branch("chi2NDF_EW",&chi2NDF_EW,"chi2NDF_EW/D");
      outtree->Branch("R_0_sim_EW",&R_0_sim_EW,"R_0_sim_EW/D");
      outtree->Branch("sigR_0_sim_EW",&sigR_0_sim_EW,"sigR_0_sim_EW/D");
      outtree->Branch("eps_sim_EW",&eps_sim_EW,"eps_sim_EW/D");
      outtree->Branch("sigeps_sim_EW",&sigeps_sim_EW,"sigeps_sim_EW/D");
      outtree->Branch("chi2NDF_sim_EW",&chi2NDF_sim_EW,"chi2NDF_sim_EW/D");
    }
    if (existingBranches->FindObject("AzL_NS")==0) {
      cerr << "Warning: No NS polarization data found in root file.\n" << endl;
      hasNS = false;
    } else {
      hasNS = true;
      recTree->SetBranchAddress("AzL_NS",&AzL_NS);
      recTree->SetBranchAddress("ElL_NS",&ElL_NS);
      recTree->SetBranchAddress("Distance_NS",&distanceResult_NS);        // radius of curvature
      recTree->SetBranchAddress("CCheight_NS",&CCheight_NS);
      recTree->SetBranchAddress("CCwidth_NS",&CCwidth_NS);
      recTree->SetBranchAddress("CCcenter_NS",&CCcenter_NS);
      recTree->SetBranchAddress("CCheight_error_NS",&CCheight_error_NS);
      //recTree->SetBranchAddress("CCconverged_NS",&CCconverged_NS);
      recTree->SetBranchAddress("Xheight_NS",&Xheight_NS);
      recTree->SetBranchAddress("Xheight_error_NS",&Xheight_error_NS);
      //recTree->SetBranchAddress("Xconverged_NS",&Xconverged_NS);
      //recTree->SetBranchAddress("goodReconstructed_NS",&goodNS);
      recTree->SetBranchAddress("rmsCCbeam_NS",&rmsCCbeam_NS);
      recTree->SetBranchAddress("rmsXbeam_NS",&rmsXbeam_NS);
      recTree->SetBranchAddress("rmsPbeam_NS",&rmsPbeam_NS);
      recTree->SetBranchAddress("latMeanDistCC_NS",&latMeanDistCC_NS);
      recTree->SetBranchAddress("NCCbeamAntennas_NS",&NCCbeamAntennas_NS);
      recTree->SetBranchAddress("ratioDiffSign_NS",&ratioDiffSign_NS);
      recTree->SetBranchAddress("ratioDiffSignEnv_NS",&ratioDiffSignEnv_NS);
      recTree->SetBranchAddress("weightedTotSign_NS",&weightedTotSign_NS);
      recTree->SetBranchAddress("weightedTotSignEnv_NS",&weightedTotSignEnv_NS);
      recTree->SetBranchAddress("latMeanDist_NS",&latMeanDist_NS);
      recTree->SetBranchAddress("latMinDist_NS",&latMinDist_NS);
      recTree->SetBranchAddress("latMaxDist_NS",&latMaxDist_NS);
      recTree->SetBranchAddress("NlateralAntennas_NS",&NlateralAntennas_NS);
      
      outtree->Branch("AzL_NS",&AzL_NS,"AzL_NS/D");
      outtree->Branch("ElL_NS",&ElL_NS,"ElL_NS/D");
      outtree->Branch("Distance_NS",&distanceResult_NS,"Distance_NS/D");        // radius of curvature
      outtree->Branch("CCheight_NS",&CCheight_NS,"CCheight_NS/D");
      outtree->Branch("CCwidth_NS",&CCwidth_NS,"CCwidth_NS/D");
      outtree->Branch("CCcenter_NS",&CCcenter_NS,"CCcenter_NS/D");
      outtree->Branch("CCheight_error_NS",&CCheight_error_NS,"CCheight_error_NS/D");
      //outtree->Branch("CCconverged_NS",&CCconverged_NS,"CCconverged_NS/B");
      outtree->Branch("Xheight_NS",&Xheight_NS,"Xheight_NS/D");
      outtree->Branch("Xheight_error_NS",&Xheight_error_NS,"Xheight_error_NS/D");
      //outtree->Branch("Xconverged_NS",&Xconverged_NS,"Xconverged_NS/B");
      //outtree->Branch("goodReconstructed_NS",&goodNS,"goodReconstructed_NS/B");
      outtree->Branch("rmsCCbeam_NS",&rmsCCbeam_NS,"rmsCCbeam_NS/D");
      outtree->Branch("rmsXbeam_NS",&rmsXbeam_NS,"rmsXbeam_NS/D");
      outtree->Branch("rmsPbeam_NS",&rmsPbeam_NS,"rmsPbeam_NS/D");
      outtree->Branch("latMeanDistCC_NS",&latMeanDistCC_NS,"latMeanDistCC_NS/D");
      outtree->Branch("NCCbeamAntennas_NS",&NCCbeamAntennas_NS,"NCCbeamAntennas_NS/I");
      outtree->Branch("ratioDiffSign_NS",&ratioDiffSign_NS,"ratioDiffSign_NS/D");
      outtree->Branch("ratioDiffSignEnv_NS",&ratioDiffSignEnv_NS,"ratioDiffSignEnv_NS/D");
      outtree->Branch("weightedTotSign_NS",&weightedTotSign_NS,"weightedTotSign_NS/D");
      outtree->Branch("weightedTotSignEnv_NS",&weightedTotSignEnv_NS,"weightedTotSignEnv_NS/D");
      outtree->Branch("latMeanDist_NS",&latMeanDist_NS,"latMeanDist_NS/D");
      outtree->Branch("latMinDist_NS",&latMinDist_NS,"latMinDist_NS/D");
      outtree->Branch("latMaxDist_NS",&latMaxDist_NS,"latMaxDist_NS/D");
      outtree->Branch("NlateralAntennas_NS",&NlateralAntennas_NS,"NlateralAntennas_NS/I");
         
      outtree->Branch("latTimeSphere1DRcurv_NS",&latTimeSphere1DRcurv_NS,"latTimeSphere1DRcurv_NS/D");
      outtree->Branch("latTimeSphere1DSigRcurv_NS",&latTimeSphere1DSigRcurv_NS,"latTimeSphere1DSigRcurv_NS/D");
      outtree->Branch("latTimeSphere1DChi2NDF_NS",&latTimeSphere1DChi2NDF_NS,"latTimeSphere1DChi2NDF_NS/D");
      outtree->Branch("latTimeSphere2DRcurv_NS",&latTimeSphere2DRcurv_NS,"latTimeSphere2DRcurv_NS/D");
      outtree->Branch("latTimeSphere2DSigRcurv_NS",&latTimeSphere2DSigRcurv_NS,"latTimeSphere2DSigRcurv_NS/D");
      outtree->Branch("latTimeSphere2DChi2NDF_NS",&latTimeSphere2DChi2NDF_NS,"latTimeSphere2DChi2NDF_NS/D");
      
      outtree->Branch("latTimeSphere1DRcurv_sim_NS",&latTimeSphere1DRcurv_sim_NS,"latTimeSphere1DRcurv_sim_NS/D");
      outtree->Branch("latTimeSphere1DSigRcurv_sim_NS",&latTimeSphere1DSigRcurv_sim_NS,"latTimeSphere1DSigRcurv_sim_NS/D");
      outtree->Branch("latTimeSphere1DChi2NDF_sim_NS",&latTimeSphere1DChi2NDF_sim_NS,"latTimeSphere1DChi2NDF_sim_NS/D");      
      outtree->Branch("latTimeSphere2DRcurv_sim_NS",&latTimeSphere2DRcurv_sim_NS,"latTimeSphere2DRcurv_sim_NS/D");
      outtree->Branch("latTimeSphere2DSigRcurv_sim_NS",&latTimeSphere2DSigRcurv_sim_NS,"latTimeSphere2DSigRcurv_sim_NS/D");
      outtree->Branch("latTimeSphere2DChi2NDF_sim_NS",&latTimeSphere2DChi2NDF_sim_NS,"latTimeSphere2DChi2NDF_sim_NS/D");
      
      outtree->Branch("latTimeCone1DRho_NS",&latTimeCone1DRho_NS,"latTimeCone1DRho_NS/D");
      outtree->Branch("latTimeCone1DSigRho_NS",&latTimeCone1DSigRho_NS,"latTimeCone1DSigRho_NS/D");
      outtree->Branch("latTimeCone1DChi2NDF_NS",&latTimeCone1DChi2NDF_NS,"latTimeCone1DChi2NDF_NS/D");
      outtree->Branch("latTimeCone2DRho_NS",&latTimeCone2DRho_NS,"latTimeCone2DRho_NS/D");
      outtree->Branch("latTimeCone2DSigRho_NS",&latTimeCone2DSigRho_NS,"latTimeCone2DSigRho_NS/D");
      outtree->Branch("latTimeCone2DChi2NDF_NS",&latTimeCone2DChi2NDF_NS,"latTimeCone2DChi2NDF_NS/D");
      
      outtree->Branch("latTimeCone1DRho_sim_NS",&latTimeCone1DRho_sim_NS,"latTimeCone1DRho_sim_NS/D");
      outtree->Branch("latTimeCone1DSigRho_sim_NS",&latTimeCone1DSigRho_sim_NS,"latTimeCone1DSigRho_sim_NS/D");
      outtree->Branch("latTimeCone1DChi2NDF_sim_NS",&latTimeCone1DChi2NDF_sim_NS,"latTimeCone1DChi2NDF_sim_NS/D");      
      outtree->Branch("latTimeCone2DRho_sim_NS",&latTimeCone2DRho_sim_NS,"latTimeCone2DRho_sim_NS/D");
      outtree->Branch("latTimeCone2DSigRho_sim_NS",&latTimeCone2DSigRho_sim_NS,"latTimeCone2DSigRho_sim_NS/D");
      outtree->Branch("latTimeCone2DChi2NDF_sim_NS",&latTimeCone2DChi2NDF_sim_NS,"latTimeCone2DChi2NDF_sim_NS/D");
      
      outtree->Branch("R_0_NS",&R_0_NS,"R_0_NS/D");
      outtree->Branch("sigR_0_NS",&sigR_0_NS,"sigR_0_NS/D");
      outtree->Branch("eps_NS",&eps_NS,"eps_NS/D");
      outtree->Branch("sigeps_NS",&sigeps_NS,"sigeps_NS/D");
      outtree->Branch("chi2NDF_NS",&chi2NDF_NS,"chi2NDF_NS/D");
      outtree->Branch("R_0_sim_NS",&R_0_sim_NS,"R_0_sim_NS/D");
      outtree->Branch("sigR_0_sim_NS",&sigR_0_sim_NS,"sigR_0_sim_NS/D");
      outtree->Branch("eps_sim_NS",&eps_sim_NS,"eps_sim_NS/D");
      outtree->Branch("sigeps_sim_NS",&sigeps_sim_NS,"sigeps_sim_NS/D");
      outtree->Branch("chi2NDF_sim_NS",&chi2NDF_sim_NS,"chi2NDF_sim_NS/D");
    }
   if (existingBranches->FindObject("AzL_VE")==0) {
      cerr << "Warning: No VE polarization data found in root file.\n" << endl;
      hasVE = false;
    } else {
      hasVE = true;
      recTree->SetBranchAddress("AzL_VE",&AzL_VE);
      recTree->SetBranchAddress("ElL_VE",&ElL_VE);
      recTree->SetBranchAddress("Distance_VE",&distanceResult_VE);        // radius of curvature
      recTree->SetBranchAddress("CCheight_VE",&CCheight_VE);
      recTree->SetBranchAddress("CCwidth_VE",&CCwidth_VE);
      recTree->SetBranchAddress("CCcenter_VE",&CCcenter_VE);
      recTree->SetBranchAddress("CCheight_error_VE",&CCheight_error_VE);
      //recTree->SetBranchAddress("CCconverged_VE",&CCconverged_VE);
      recTree->SetBranchAddress("Xheight_VE",&Xheight_VE);
      recTree->SetBranchAddress("Xheight_error_VE",&Xheight_error_VE);
      //recTree->SetBranchAddress("Xconverged_VE",&Xconverged_VE);
      //recTree->SetBranchAddress("goodReconstructed_VE",&goodVE);
      recTree->SetBranchAddress("rmsCCbeam_VE",&rmsCCbeam_VE);
      recTree->SetBranchAddress("rmsXbeam_VE",&rmsXbeam_VE);
      recTree->SetBranchAddress("rmsPbeam_VE",&rmsPbeam_VE);
      recTree->SetBranchAddress("latMeanDistCC_VE",&latMeanDistCC_VE);
      recTree->SetBranchAddress("NCCbeamAntennas_VE",&NCCbeamAntennas_VE);
      recTree->SetBranchAddress("ratioDiffSign_VE",&ratioDiffSign_VE);
      recTree->SetBranchAddress("ratioDiffSignEnv_VE",&ratioDiffSignEnv_VE);
      recTree->SetBranchAddress("weightedTotSign_VE",&weightedTotSign_VE);
      recTree->SetBranchAddress("weightedTotSignEnv_VE",&weightedTotSignEnv_VE);
      recTree->SetBranchAddress("latMeanDist_VE",&latMeanDist_VE);
      recTree->SetBranchAddress("latMinDist_VE",&latMinDist_VE);
      recTree->SetBranchAddress("latMaxDist_VE",&latMaxDist_VE);
      recTree->SetBranchAddress("NlateralAntennas_VE",&NlateralAntennas_VE);
       
      outtree->Branch("AzL_NS",&AzL_NS,"AzL_NS/D");
      outtree->Branch("ElL_VE",&ElL_VE,"ElL_VE/D");
      outtree->Branch("Distance_VE",&distanceResult_VE,"Distance_VE/D");        // radius of curvature
      outtree->Branch("CCheight_VE",&CCheight_VE,"CCheight_VE/D");
      outtree->Branch("CCwidth_VE",&CCwidth_VE,"CCwidth_VE/D");
      outtree->Branch("CCcenter_VE",&CCcenter_VE,"CCcenter_VE/D");
      outtree->Branch("CCheight_error_VE",&CCheight_error_VE,"CCheight_error_VE/D");
      //outtree->Branch("CCconverged_VE",&CCconverged_VE,"CCconverged_VE/B");
      outtree->Branch("Xheight_VE",&Xheight_VE,"Xheight_VE/D");
      outtree->Branch("Xheight_error_VE",&Xheight_error_VE,"Xheight_error_VE/D");
      //outtree->Branch("Xconverged_VE",&Xconverged_VE,"Xconverged_VE/B");
      //outtree->Branch("goodReconstructed_VE",&goodVE,"goodReconstructed_VE/B");
      outtree->Branch("rmsCCbeam_VE",&rmsCCbeam_VE,"rmsCCbeam_VE/D");
      outtree->Branch("rmsXbeam_VE",&rmsXbeam_VE,"rmsXbeam_VE/D");
      outtree->Branch("rmsPbeam_VE",&rmsPbeam_VE,"rmsPbeam_VE/D");
      outtree->Branch("latMeanDistCC_VE",&latMeanDistCC_VE,"latMeanDistCC_VE/D");
      outtree->Branch("NCCbeamAntennas_VE",&NCCbeamAntennas_VE,"NCCbeamAntennas_VE/I");
      outtree->Branch("ratioDiffSign_VE",&ratioDiffSign_VE,"ratioDiffSign_VE/D");
      outtree->Branch("ratioDiffSignEnv_VE",&ratioDiffSignEnv_VE,"ratioDiffSignEnv_VE/D");
      outtree->Branch("weightedTotSign_VE",&weightedTotSign_VE,"weightedTotSign_VE/D");
      outtree->Branch("weightedTotSignEnv_VE",&weightedTotSignEnv_VE,"weightedTotSignEnv_VE/D");
      outtree->Branch("latMeanDist_VE",&latMeanDist_VE,"latMeanDist_VE/D");
      outtree->Branch("latMinDist_VE",&latMinDist_VE,"latMinDist_VE/D");
      outtree->Branch("latMaxDist_VE",&latMaxDist_VE,"latMaxDist_VE/D");
      outtree->Branch("NlateralAntennas_VE",&NlateralAntennas_VE,"NlateralAntennas_VE/I");
            
      outtree->Branch("latTimeSphere1DRcurv_VE",&latTimeSphere1DRcurv_VE,"latTimeSphere1DRcurv_VE/D");
      outtree->Branch("latTimeSphere1DSigRcurv_VE",&latTimeSphere1DSigRcurv_VE,"latTimeSphere1DSigRcurv_VE/D");
      outtree->Branch("latTimeSphere1DChi2NDF_VE",&latTimeSphere1DChi2NDF_VE,"latTimeSphere1DChi2NDF_VE/D");
      outtree->Branch("latTimeSphere2DRcurv_VE",&latTimeSphere2DRcurv_VE,"latTimeSphere2DRcurv_VE/D");
      outtree->Branch("latTimeSphere2DSigRcurv_VE",&latTimeSphere2DSigRcurv_VE,"latTimeSphere2DSigRcurv_VE/D");
      outtree->Branch("latTimeSphere2DChi2NDF_VE",&latTimeSphere2DChi2NDF_VE,"latTimeSphere2DChi2NDF_VE/D");
      
      outtree->Branch("latTimeSphere1DRcurv_sim_VE",&latTimeSphere1DRcurv_sim_VE,"latTimeSphere1DRcurv_sim_VE/D");
      outtree->Branch("latTimeSphere1DSigRcurv_sim_VE",&latTimeSphere1DSigRcurv_sim_VE,"latTimeSphere1DSigRcurv_sim_VE/D");
      outtree->Branch("latTimeSphere1DChi2NDF_sim_VE",&latTimeSphere1DChi2NDF_sim_VE,"latTimeSphere1DChi2NDF_sim_VE/D");      
      outtree->Branch("latTimeSphere2DRcurv_sim_VE",&latTimeSphere2DRcurv_sim_VE,"latTimeSphere2DRcurv_sim_VE/D");
      outtree->Branch("latTimeSphere2DSigRcurv_sim_VE",&latTimeSphere2DSigRcurv_sim_VE,"latTimeSphere2DSigRcurv_sim_VE/D");
      outtree->Branch("latTimeSphere2DChi2NDF_sim_VE",&latTimeSphere2DChi2NDF_sim_VE,"latTimeSphere2DChi2NDF_sim_VE/D");
      
      outtree->Branch("latTimeCone1DRho_VE",&latTimeCone1DRho_VE,"latTimeCone1DRho_VE/D");
      outtree->Branch("latTimeCone1DSigRho_VE",&latTimeCone1DSigRho_VE,"latTimeCone1DSigRho_VE/D");
      outtree->Branch("latTimeCone1DChi2NDF_VE",&latTimeCone1DChi2NDF_VE,"latTimeCone1DChi2NDF_VE/D");
      outtree->Branch("latTimeCone2DRho_VE",&latTimeCone2DRho_VE,"latTimeCone2DRho_VE/D");
      outtree->Branch("latTimeCone2DSigRho_VE",&latTimeCone2DSigRho_VE,"latTimeCone2DSigRho_VE/D");
      outtree->Branch("latTimeCone2DChi2NDF_VE",&latTimeCone2DChi2NDF_VE,"latTimeCone2DChi2NDF_VE/D");
      
      outtree->Branch("latTimeCone1DRho_sim_VE",&latTimeCone1DRho_sim_VE,"latTimeCone1DRho_sim_VE/D");
      outtree->Branch("latTimeCone1DSigRho_sim_VE",&latTimeCone1DSigRho_sim_VE,"latTimeCone1DSigRho_sim_VE/D");
      outtree->Branch("latTimeCone1DChi2NDF_sim_VE",&latTimeCone1DChi2NDF_sim_VE,"latTimeCone1DChi2NDF_sim_VE/D");      
      outtree->Branch("latTimeCone2DRho_sim_VE",&latTimeCone2DRho_sim_VE,"latTimeCone2DRho_sim_VE/D");
      outtree->Branch("latTimeCone2DSigRho_sim_VE",&latTimeCone2DSigRho_sim_VE,"latTimeCone2DSigRho_sim_VE/D");
      outtree->Branch("latTimeCone2DChi2NDF_sim_VE",&latTimeCone2DChi2NDF_sim_VE,"latTimeCone2DChi2NDF_sim_VE/D");
      
      outtree->Branch("R_0_VE",&R_0_VE,"R_0_VE/D");
      outtree->Branch("sigR_0_VE",&sigR_0_VE,"sigR_0_VE/D");
      outtree->Branch("eps_VE",&eps_VE,"eps_VE/D");
      outtree->Branch("sigeps_VE",&sigeps_VE,"sigeps_VE/D");
      outtree->Branch("chi2NDF_VE",&chi2NDF_VE,"chi2NDF_VE/D");
      outtree->Branch("R_0_sim_VE",&R_0_sim_VE,"R_0_sim_VE/D");
      outtree->Branch("sigR_0_sim_VE",&sigR_0_sim_VE,"sigR_0_sim_VE/D");
      outtree->Branch("eps_sim_VE",&eps_sim_VE,"eps_sim_VE/D");
      outtree->Branch("sigeps_sim_VE",&sigeps_sim_VE,"sigeps_sim_VE/D");
      outtree->Branch("chi2NDF_sim_VE",&chi2NDF_sim_VE,"chi2NDF_sim_VE/D");
    }
    
    // open second data file, if requested
    unsigned int Gt2 = 0;
    TFile *recRoot2;
    TTree *recTree2;   
    if (resultsName2 != "") {
      // set indices
      index1 = "old";
      index2 = "new";

      cout << "\nOpening second root file for data input: " << resultsName2 << endl;
      recRoot2 = new TFile (resultsName2.c_str(), "READ");
      if(!recRoot2 || recRoot2->IsZombie()) {
        cerr << "Error, cannot open root file: "<< resultsName2 << endl;
      return 1;  
      } 
      recTree2 = (TTree*)recRoot2->Get("T;1");
      recTree2->SetBranchAddress("Gt", &Gt2);   
      
      cout << "Comparing files: " << resultsName1 << " and " << resultsName2 <<endl;
    }
        
    for (int i=0; i < totAntenna; ++i) {
      antPulses[i] = new PulseProperties();
      antPulses2[i] = new PulseProperties();
    }

    for(int i=0; i< totAntenna; ++i) {
      stringstream antNumber("");
      antNumber << (i+1);
      string antBranchName;
      antBranchName = "Ant_" + antNumber.str() + "_cal.";
      if (existingBranches->FindObject(antBranchName.c_str())==0) {
        cerr << "Branch \"" << antBranchName << "\" does not exist." << endl;
        return 1;
      }
      recTree->SetBranchAddress(antBranchName.c_str(), &antPulses[i]);
      if (resultsName2 != "") 
        recTree2->SetBranchAddress(antBranchName.c_str(), &antPulses2[i]);
    }

    int entries = recTree->GetEntries();
      cout<<"---------- >  Root file entries :   "<<entries<<endl;
    if (resultsName2 != "") {    
      if (entries > recTree2->GetEntries()) {
        cerr << "Error: The bigger root file must be the last one.\n" << endl;
        return 1;
      }
    }
      
    /* output files*/
    string filename = outPrefix + "-EW.dat";
    ofstream outputEW(filename.c_str());
    filename = outPrefix + "-NS.dat";
    ofstream outputNS(filename.c_str());
    outputEW<<"#sim id , eps0 , sigeps0,  R0 ,  sigR0 , chi2 , eps0_sim , sigeps0_sim , R0_sim , sigR0_sim , chi2_sim"<<endl;
    outputNS<<"#sim id , eps0 , sigeps0,  R0 ,  sigR0 , chi2 , eps0_sim , sigeps0_sim , R0_sim , sigR0_sim , chi2_sim"<<endl;

    ofstream checkDistanceEW("checkDistance_EW.dat");
    ofstream checkDistanceNS("checkDistance_NS.dat");
    checkDistanceEW<<"# id      Ant    dist sim    dist lopes  "<<endl;
    checkDistanceNS<<"# id      Ant    dist sim    dist lopes  "<<endl;
    
    // deviation of antenna coordinates
    double meanDevX = 0;
    double meanDevY = 0;
    double meanDevZ = 0;
    int meanDevCounter = 0;
    
    int dat2counter = 0; // counter for second root file
    for(int i=0; i<entries; ++i) { //loop on the events
      for (int k=0; k < totAntenna; ++k) {
          *antPulses[k] = PulseProperties();
        }
      recTree->GetEntry(i);
      if (resultsName2 != "") {
        while (Gt2 < Gt) {
          recTree2->GetEntry(dat2counter);
          ++dat2counter;
          if (dat2counter > recTree2->GetEntries()) {
            cerr << "Error: Reached end of second root file!\n" << endl;
            return 1;
          }
          cout << "Reading in event GT " << Gt2 << " from second root file." << endl;
        }
        if (Gt2 != Gt) {
          cerr << "Error: Event at GT " << Gt << " seems to be missing in second root file.\n" 
               << "GT of second root file is: " << Gt2 << endl;
          return 1;       
        }
      }

      map<int,PulseProperties> m_recPulses;
      map<int,PulseProperties> m_recPulses2;
      map<int,PulseProperties> m_recEW;
      map<int,PulseProperties> m_recNS;
      map<int,PulseProperties> m_simEW;
      map<int,PulseProperties> m_simNS;

      //int NantR[totAntenna]=antPulses[totAntenna].antennaID; 

      //fill the first rec maps
      for(int j=0; j<totAntenna; j++) {
        // consistency check: if one antenna ID is empty, do not consider this antenna
        if (antPulses[j]->antennaID == 0) 
          continue;

        // also fill the second data map, if provided
        if (resultsName2!="") {
          // consistency checks
          if (antPulses2[j]->antennaID == 0) 
            continue;
          if (antPulses[j]->antennaID != antPulses2[j]->antennaID) {
            cerr << "\nError: Antenna IDs of both root files do not match: " 
                 << antPulses[j]->antennaID << " vs. "  
                 << antPulses2[j]->antennaID << " at value number " << j << endl;
            return 1;  
          } else {
            m_recPulses2[antPulses2[j]->antennaID] = *antPulses2[j];
          }  
          if (antPulses[j]->polarization != antPulses2[j]->polarization) {
            cerr << "\nError: Polarization of both root files do not match.\n" << endl;
            return 1;
          }
          if (antPulses[j]->polarization == "EW") {
            m_recEW[antPulses[j]->antennaID] = *antPulses[j]; 
            m_simEW[antPulses2[j]->antennaID] = *antPulses2[j]; 
          } 
          if (antPulses[j]->polarization == "NS") {
            m_recNS[antPulses[j]->antennaID] = *antPulses[j]; 
            m_simNS[antPulses2[j]->antennaID] = *antPulses2[j]; 
          } 
        }    

        m_recPulses[antPulses[j]->antennaID] = *antPulses[j]; //I fill the rec map with pulse properties in the root file
      }
      
      /*sim file*/
      if (simDictName!="") {
        int NantS=0;
        double EWfield=0.,NSfield=0.,VEfield=0.;
        double timeREAS = 0.;
        double distS=0.,azSREAS=0.,azS=0.,azimuth=0.,zenith=0.;
        double distX = 0., distY = 0., distZ = 0.;
        double groundDist= 0., zShower = 0.;
        double sim2lopesField=(2.9979246e+10/31.); //convert to muV/m/MHz and divide by the band-width
        double distanceS=0., distanceSerr=0.,showerCoord=0.;
        double distanceR=0.;

        // continue with next event, if no simulated event is available
        if (m_dict.find(Gt)==m_dict.end()) {
          cout << "No simulated event for Gt: " <<Gt<< endl;
          continue;
        }

        //string reasFileName = simPath+"/"+m_dict[Gt]+ "_lopes_rect43to76/maxamp_summary.dat";
        string reasFileName = simPath+"/"+m_dict[Gt]+ "_lopes_rect43to74/maxamp_summary.dat";
        //string reasFileName = simPath+"/"+m_dict[Gt]+ "_lopesdual_43to74Mhz_allCompMaxima/maxamp_summary.dat";
        //string reasFileName = simPath+"/"+m_dict[Gt]+ "_lopesew_43to74Mhz_allCompMaxima/maxamp_summary.dat"; 
        //string reasFileName = simPath+"/"+m_dict[Gt]+ "_lopesdual_43to74Mhz/maxamp_summary.dat";
        string reasLogFile = simPath+"/"+m_dict[Gt]+ ".reas";
        
        ifstream reasFile(reasFileName.c_str());
        if (!reasFile.is_open()) {
          cerr << "Error canot open REAS file: " << reasFileName << endl;
          return 1;     
        } else {
	  cout << "Reading simulation for GT: " << Gt << "\n" << endl;
	}
	
        // read Xmax value
        Xmax_sim = reasXmaxFromREAS(reasLogFile);
        cout << "\nXmax (simulation) = " << Xmax_sim << " g/cm^2" << endl;
        if (Xmax_sim < 200)
          cout << "\nWARNING: Xmax for simulation " << m_dict[Gt] << " , GT = " << Gt << " too low!\n" << endl;

	
        // get simulation azimuth and zenith form dictionary
        AzDictS = m_dictAz[Gt]; //in deg!
        ZeDictS = m_dictZe[Gt];
        // direction used for lateral distance calculation
        //values from KASCADE or Grande, so taken from the dict file //(Az=0 in the north! same as LOPES!)
        azimuth = AzDictS/gradeg; 
        zenith = ZeDictS/gradeg;            
        
        //check that are the same!
        //cout<<"from dict file Az: "<<AzDictS<<"   ; zeS: " <<ZeDictS<<endl;
        //cout<<"from KA        Az: "<<Az*gradeg<<" ; zeS: " <<Ze*gradeg<<endl;
        char buffer2[1024];
        while (reasFile.good()) {
          reasFile.getline(buffer2,1024);
          istringstream iss2 (buffer2);
          if(iss2.str().size()>0&&iss2.str()[0]!='%'&&iss2.str()[0]!='#') {//in sim file:az in reas sistem
            iss2>>NantS>>distS>>azSREAS>>NSfield>>EWfield>>VEfield>>timeREAS>>distX>>distY>>distZ;
            
            azS=180.-(azSREAS*gradeg); //convert to LOPES coordinates
            
            // preperationis for calculating antenna height in shower coordinates
            distZ -= antennaHeightOffset; // substract offset of antenna height in REAS 3 (defined at beginning of file)
            groundDist = sqrt(distX*distX + distY*distY + distZ*distZ);
            
            // convert time to ns and remove offset
            timeREAS *= 1e9;
            timeREAS += groundPlaneTimeOffset/cos(zenith) + generalREAStimeOffset;
            
            if(azS<0.) {
              azS = azS+360.; //azS in grad
            }
            //cout<<Gt<<"-"<<m_dict[Gt]<<": summaryfile.dat az: "<<azSREAS*gradeg<<" and converted to LOPES: "<<azS<<endl;
            //look if antenna exists at all in data then separe EW and NS
            if (m_recPulses.find(NantS) != m_recPulses.end()) {
              // convert e-field to LOPES units
              NSfield*=sim2lopesField;
              EWfield*=sim2lopesField;
              
              if(NSfield<0.1) {
                cout<<"WARNING: Low field strength in NS simulation (< 0.1)." << endl;
                // NSfield=0.1;  // Steffen used 0.1 as minimum value for simulations
              }
              if(EWfield<0.1) {
                cout<<"WARNING: Low field strength in EW simulation (< 0.1)." << endl;
                // EWfield=0.1;  // Steffen used 0.1 as minimum value for simulations
              }
              
              if (m_recPulses[NantS].polarization == "EW") {
                //cout<<"   xxxxxx   EW polarization   xxxxxx  "<<endl;
                //define recEW map
                m_recEW[NantS] = m_recPulses[NantS];
                //cout<<"rec EW map size   :"<<m_recEW.size()<<endl;
                //define simEW map
                PulseProperties simPropEW;
                simPropEW.antennaID = NantS; //antennaid vs antenna no?
                simPropEW.height = EWfield;
                //cout<<"EW Field  "<<EWfield<<endl;
                simPropEW.heightError = 0.;                
                simPropEW.time = timeREAS; // pulse time in ns
                simPropEW.timeError = 0;
                distanceR=m_recEW[NantS].dist;
                // calculate simulation distance for consistency check
                if (!simulationDistances) { // overwrite direction with values of LOPES reconstruction
                  azimuth = AzL/gradeg;  // EW
                  zenith = (90.-ElL)/gradeg;
                }  
                //cout<<"az sim converted : "<< azS << " az data LOPES antennas : " << azimuth*gradeg <<" ze LOPES "<< zenith*gradeg <<endl;
                showerCoord=sqrt(1.0 - pow(cos((azS/gradeg)-azimuth),2)*pow(sin(zenith),2));
                distanceS=0.01*distS*showerCoord;
                //cout<<"shower coord system: "<<showerCoord<<endl;
                //cout<<"sim       distance : "<<distanceS<<endl; 
                if ((distanceS-distanceR)>((distanceS+0.5)*0.05)) {
                  cout<<"WARNING: distance simulated EW channel:  "<<distanceS<<" distance from LOPES  "<<distanceR<<endl;
                  checkDistanceEW<<m_dict[Gt]<<"\t"<<NantS<<"\t"<<distanceS<<"\t"<<distanceR<<endl;
                }            
                if (simulationDistances) 
                  simPropEW.dist =distanceS;
                else  
                  simPropEW.dist =distanceR;
                simPropEW.disterr = distanceSerr;
                
                // calculate antenna height (z) in shower coordinates
                //zShower = -0.01* groundDist * sin(zenith) * cos(azS/gradeg-azimuth); // wrong
                //zShower = sqrt((groundDist*groundDist*1e-4) - (distanceS*distanceS)); // wrong sign
                // Attention: x = x_lop, y = -y_lop, z = -z_lop ?
                double xShower = -distX*sin(azimuth) - distY*cos(azimuth);
                double yShower = distX*cos(azimuth)*cos(zenith) - distY*sin(azimuth)*cos(zenith) - distZ*sin(zenith);
                zShower = distX*cos(azimuth)*sin(zenith) - distY*sin(azimuth)*sin(zenith) + distZ*cos(zenith);
                xShower *= 1e-2;
                yShower *= 1e-2;
                zShower *= 1e-2; // conversion from cm to m
                
                
                
                if ( abs(zShower-m_recEW[NantS].distZ) > (abs(zShower*0.05)+0.5) ) {
                  cout<<"WARNING: z_shower simulated EW channel:  "<<zShower<<"\t z_shower from LOPES  "<<m_recEW[NantS].distZ<<endl;
                  cout<<"az_sim= " << azimuth*gradeg << " \t az_lop= " << AzL << endl;
                  cout<<"ze_sim= " << zenith*gradeg << " \t ze_lop= " << (90.-ElL) << endl;
                  cout<<"x_sim = " << xShower << " \t x_lop = " << m_recEW[NantS].distX << endl;
                  cout<<"y_sim = " << yShower << " \t y_lop = " << m_recEW[NantS].distY << endl;
                  cout<<"z_sim = " << zShower << " \t z_lop = " << m_recEW[NantS].distZ << endl;
                }
                simPropEW.distZ = zShower;
                simPropEW.distZerr = 0.;
                
                // calculate mean deviation of shower coordinates
                meanDevX += (xShower-m_recEW[NantS].distX);
                meanDevY += (yShower-m_recEW[NantS].distY);
                meanDevZ += (zShower-m_recEW[NantS].distZ);
                ++meanDevCounter;
                
                if (timeREAS + zShower*3.33564 < 0) {
                  cout << "\nNegative time at event GT "<< Gt << " , antenna " << NantS << ", ground azimuth " << azS << " °:\n"
                       << "zenith = " << zenith*180/3.1416 << " °\t azimuth = " << azimuth*180/3.1416 << " °\n"
                       << "Ground distance = " << groundDist << " \t lateral distance = " << distanceS << "\n"
                       << "Antenna height = " << distZ << " \t in shower coordinates = " << zShower << "\n"
                       << "Pulse time = " << timeREAS << "\n"
                       << "Geom. cor. = " << timeREAS + zShower*3.33564 << "\n\n\n"
                       << endl;
                }
                
                m_simEW[NantS] = simPropEW; //fill the sim map
              }
              if (m_recPulses[NantS].polarization == "NS") {
                //cout<<"   kkkkkk   NS polarization   kkkkkk  "<<endl;
                //define recNS map
                m_recNS[NantS] = m_recPulses[NantS];
                //cout<<"rec NS map size   :"<<m_recNS.size()<<endl;
                //define simEW map
                PulseProperties simPropNS;
                simPropNS.antennaID = NantS; //antennaid vs antenna no?
                simPropNS.height = NSfield;
                //cout<<"NS Field  "<<NSfield<<endl;
                simPropNS.heightError = 0.;
                simPropNS.time = timeREAS; // pulse time in ns
                simPropNS.timeError = 0;
                distanceR=m_recNS[NantS].dist;
                // calculate simulation distance for consistency check
                if (!simulationDistances) { // overwrite direction with values of LOPES reconstruction
                  azimuth = AzL_NS/gradeg;  // NS
                  zenith = (90.-ElL_NS)/gradeg;
                }  
                //cout<<"az sim converted : "<< azS << " az data LOPES antennas : " << azimuth*gradeg <<" ze LOPES "<< zenith*gradeg <<endl;
                showerCoord=sqrt(1.0 - pow(cos((azS/gradeg)-azimuth),2)*pow(sin(zenith),2));
                //showerCoord=sqrt(1.0 - pow(cos(azS-(AzL/gradeg)),2)*pow(sin((90.-ElL)/gradeg),2));
                distanceS=0.01*distS*showerCoord;
                //cout<<"shower coord system: "<<showerCoord<<endl;
                //cout<<"sim       distance : "<<distanceS<<endl; 
                if ((distanceS-distanceR)>(distanceS*0.05)) {
                  cout<<"WARNING: distance simulated NS channel:  "<<distanceS<<" distance from LOPES  "<<distanceR<<endl;
                  checkDistanceNS<<m_dict[Gt]<<"\t"<<NantS<<"\t"<<distanceS<<"\t"<<distanceR<<endl;
                }
                if (simulationDistances) 
                  simPropNS.dist =distanceS;
                else  
                  simPropNS.dist =distanceR;
                simPropNS.disterr = distanceSerr;
          
                // calculate antenna height (z) in shower coordinates
                zShower = distX*cos(azimuth)*sin(zenith) - distY*sin(azimuth)*sin(zenith) + distZ*cos(zenith);
                zShower *= 1e-2; // conversion from cm to m
                if ( abs(zShower-m_recNS[NantS].distZ) > (abs(zShower*0.05)+0.5) ) {
                  cout<<"WARNING: z_shower simulated EW channel:  "<<zShower<<"\t z_shower from LOPES  "<<m_recEW[NantS].distZ<<endl;
                }
                simPropNS.distZ = zShower;
                simPropNS.distZerr = 0.;
                
                m_simNS[NantS] = simPropNS;//fill the sim map
              }
            }
          }
        } // while...
        reasFile.close();
      } 
      
      R_0_EW = 0, sigR_0_EW = 0, eps_EW = 0, sigeps_EW = 0, chi2NDF_EW = 0; 
      R_0_sim_EW = 0, sigR_0_sim_EW = 0, eps_sim_EW = 0, sigeps_sim_EW = 0, chi2NDF_sim_EW = 0; 
      R_0_NS = 0, sigR_0_NS = 0, eps_NS = 0, sigeps_NS = 0, chi2NDF_NS = 0; 
      R_0_sim_NS = 0, sigR_0_sim_NS = 0, eps_sim_NS = 0, sigeps_sim_NS = 0, chi2NDF_sim_NS = 0; 
      R_0_VE = 0, sigR_0_VE = 0, eps_VE = 0, sigeps_VE = 0, chi2NDF_VE = 0; 
      R_0_sim_VE = 0, sigR_0_sim_VE = 0, eps_sim_VE = 0, sigeps_sim_VE = 0, chi2NDF_sim_VE = 0; 

      latTimeSphere1DRcurv_EW = 0, latTimeSphere1DRcurv_NS = 0, latTimeSphere1DRcurv_VE = 0;
      latTimeSphere1DSigRcurv_EW = 0, latTimeSphere1DSigRcurv_NS = 0, latTimeSphere1DSigRcurv_VE = 0;
      latTimeSphere1DChi2NDF_EW = 0, latTimeSphere1DChi2NDF_NS = 0, latTimeSphere1DChi2NDF_VE = 0;
      latTimeSphere2DRcurv_EW = 0, latTimeSphere2DRcurv_NS = 0, latTimeSphere2DRcurv_VE = 0;
      latTimeSphere2DSigRcurv_EW = 0, latTimeSphere2DSigRcurv_NS = 0, latTimeSphere2DSigRcurv_VE = 0;
      latTimeSphere2DChi2NDF_EW = 0, latTimeSphere2DChi2NDF_NS = 0, latTimeSphere2DChi2NDF_VE = 0;
    
      latTimeSphere1DRcurv_sim_EW = 0, latTimeSphere1DRcurv_sim_NS = 0, latTimeSphere1DRcurv_sim_VE = 0;
      latTimeSphere1DSigRcurv_sim_EW = 0, latTimeSphere1DSigRcurv_sim_NS = 0, latTimeSphere1DSigRcurv_sim_VE = 0;
      latTimeSphere1DChi2NDF_sim_EW = 0, latTimeSphere1DChi2NDF_sim_NS = 0, latTimeSphere1DChi2NDF_sim_VE = 0;
      latTimeSphere2DRcurv_sim_EW = 0, latTimeSphere2DRcurv_sim_NS = 0, latTimeSphere2DRcurv_sim_VE = 0;
      latTimeSphere2DSigRcurv_sim_EW = 0, latTimeSphere2DSigRcurv_sim_NS = 0, latTimeSphere2DSigRcurv_sim_VE = 0;
      latTimeSphere2DChi2NDF_sim_EW = 0, latTimeSphere2DChi2NDF_sim_NS = 0, latTimeSphere2DChi2NDF_sim_VE = 0;
    
      latTimeCone1DRho_EW = 0, latTimeCone1DRho_NS = 0, latTimeCone1DRho_VE = 0;
      latTimeCone1DSigRho_EW = 0, latTimeCone1DSigRho_NS = 0, latTimeCone1DSigRho_VE = 0;
      latTimeCone1DChi2NDF_EW = 0, latTimeCone1DChi2NDF_NS = 0, latTimeCone1DChi2NDF_VE = 0;
      latTimeCone2DRho_EW = 0, latTimeCone2DRho_NS = 0, latTimeCone2DRho_VE = 0;
      latTimeCone2DSigRho_EW = 0, latTimeCone2DSigRho_NS = 0, latTimeCone2DSigRho_VE = 0;
      latTimeCone2DChi2NDF_EW = 0, latTimeCone2DChi2NDF_NS = 0, latTimeCone2DChi2NDF_VE = 0;
 
      latTimeCone1DRho_sim_EW = 0, latTimeCone1DRho_sim_NS = 0, latTimeCone1DRho_sim_VE = 0;
      latTimeCone1DSigRho_sim_EW = 0, latTimeCone1DSigRho_sim_NS = 0, latTimeCone1DSigRho_sim_VE = 0;
      latTimeCone1DChi2NDF_sim_EW = 0, latTimeCone1DChi2NDF_sim_NS = 0, latTimeCone1DChi2NDF_sim_VE = 0;
      latTimeCone2DRho_sim_EW = 0, latTimeCone2DRho_sim_NS = 0, latTimeCone2DRho_sim_VE = 0;
      latTimeCone2DSigRho_sim_EW = 0, latTimeCone2DSigRho_sim_NS = 0, latTimeCone2DSigRho_sim_VE = 0;
      latTimeCone2DChi2NDF_sim_EW = 0, latTimeCone2DChi2NDF_sim_NS = 0, latTimeCone2DChi2NDF_sim_VE = 0;

      CR::lateralDistribution lateralFitter;
      string plotPrefix = "";
      if(m_recEW.size()>=4) {        
        if (simDictName!="") 
          plotPrefix = "lateral-EW-"+m_dict[Gt]+"-";
        else 
          plotPrefix = "lateral-EW-";
        Record ergEW = lateralFitter.fitLateralDistribution(plotPrefix,
                                                            m_recEW,m_simEW,
                                                            Gt,AzL,90.-ElL,
                                                            index1, index2);
        eps_EW = ergEW.asDouble("eps");
        R_0_EW = ergEW.asDouble("R_0");
        sigeps_EW = ergEW.asDouble("sigeps");
        sigR_0_EW = ergEW.asDouble("sigR_0");
        chi2NDF_EW = ergEW.asDouble("chi2NDF");
        eps_sim_EW = ergEW.asDouble("eps_sim");
        R_0_sim_EW = ergEW.asDouble("R_0_sim");
        sigeps_sim_EW = ergEW.asDouble("sigeps_sim");
        sigR_0_sim_EW = ergEW.asDouble("sigR_0_sim");
        chi2NDF_sim_EW = ergEW.asDouble("chi2NDF_sim");
        
        //write info of the fit into file
        outputEW <<m_dict[Gt]<<"\t"<<eps_EW<<"\t"<<sigeps_EW<<"\t"<<R_0_EW<<"\t"<<sigR_0_EW<<"\t"<<chi2NDF_EW<<"\t"<<eps_sim_EW<<"\t"<<sigeps_sim_EW<<"\t"<<R_0_sim_EW<<"\t"<<sigR_0_sim_EW<<"\t"<<chi2NDF_sim_EW<<endl;			
        
        Record ergTimeEW = lateralFitter.lateralTimeDistribution(plotPrefix,
                                                                 m_recEW,m_simEW,
                                                                 Gt,AzL,90.-ElL,
                                                                 CCcenter,
                                                                 index1, index2);        
                                                                 
        latTimeSphere1DRcurv_EW = ergTimeEW.asDouble("latTime1D_Rcurv");
        latTimeSphere1DSigRcurv_EW = ergTimeEW.asDouble("latTime1D_sigRcurv");
        latTimeSphere1DChi2NDF_EW = ergTimeEW.asDouble("latTime1D_chi2NDF");
        latTimeSphere2DRcurv_EW = ergTimeEW.asDouble("latTime2D_Rcurv");
        latTimeSphere2DSigRcurv_EW = ergTimeEW.asDouble("latTime2D_sigRcurv");
        latTimeSphere2DChi2NDF_EW = ergTimeEW.asDouble("latTime2D_chi2NDF");
        
        latTimeSphere1DRcurv_sim_EW = ergTimeEW.asDouble("latTime1D_Rcurv_sim");
        latTimeSphere1DSigRcurv_sim_EW = ergTimeEW.asDouble("latTime1D_sigRcurv_sim");
        latTimeSphere1DChi2NDF_sim_EW = ergTimeEW.asDouble("latTime1D_chi2NDF_sim");
        latTimeSphere2DRcurv_sim_EW = ergTimeEW.asDouble("latTime2D_Rcurv_sim");
        latTimeSphere2DSigRcurv_sim_EW = ergTimeEW.asDouble("latTime2D_sigRcurv_sim");
        latTimeSphere2DChi2NDF_sim_EW = ergTimeEW.asDouble("latTime2D_chi2NDF_sim");
                                                                 
        latTimeCone1DRho_EW = ergTimeEW.asDouble("latTime1D_ConeRho");
        latTimeCone1DSigRho_EW = ergTimeEW.asDouble("latTime1D_sigConeRho");
        latTimeCone1DChi2NDF_EW = ergTimeEW.asDouble("latTime1D_Conechi2NDF");
        latTimeCone2DRho_EW = ergTimeEW.asDouble("latTime2D_ConeRho");
        latTimeCone2DSigRho_EW = ergTimeEW.asDouble("latTime2D_sigConeRho");
        latTimeCone2DChi2NDF_EW = ergTimeEW.asDouble("latTime2D_Conechi2NDF");
        
        latTimeCone1DRho_sim_EW = ergTimeEW.asDouble("latTime1D_ConeRho_sim");
        latTimeCone1DSigRho_sim_EW = ergTimeEW.asDouble("latTime1D_sigConeRho_sim");
        latTimeCone1DChi2NDF_sim_EW = ergTimeEW.asDouble("latTime1D_Conechi2NDF_sim");
        latTimeCone2DRho_sim_EW = ergTimeEW.asDouble("latTime2D_ConeRho_sim");
        latTimeCone2DSigRho_sim_EW = ergTimeEW.asDouble("latTime2D_sigConeRho_sim");
        latTimeCone2DChi2NDF_sim_EW = ergTimeEW.asDouble("latTime2D_Conechi2NDF_sim");
      }
      if ((hasNS) && (m_recNS.size()>=4)) {
        if (simDictName!="") 
          plotPrefix = "lateral-NS-"+m_dict[Gt]+"-";
        else 
          plotPrefix = "lateral-NS-";
        Record ergNS = lateralFitter.fitLateralDistribution(plotPrefix,
                                                            m_recNS,m_simNS,
                                                            Gt,AzL,90.-ElL,
                                                            index1, index2);
        eps_NS = ergNS.asDouble("eps");
        R_0_NS = ergNS.asDouble("R_0");
        sigeps_NS = ergNS.asDouble("sigeps");
        sigR_0_NS = ergNS.asDouble("sigR_0");
        chi2NDF_NS = ergNS.asDouble("chi2NDF");
        eps_sim_NS = ergNS.asDouble("eps_sim");
        R_0_sim_NS = ergNS.asDouble("R_0_sim");
        sigeps_sim_NS = ergNS.asDouble("sigeps_sim");
        sigR_0_sim_NS = ergNS.asDouble("sigR_0_sim");
        chi2NDF_sim_NS = ergNS.asDouble("chi2NDF_sim");

        //write info of the fit into file
        outputEW <<m_dict[Gt]<<"\t"<<eps_NS<<"\t"<<sigeps_NS<<"\t"<<R_0_NS<<"\t"<<sigR_0_NS<<"\t"<<chi2NDF_NS<<"\t"<<eps_sim_NS<<"\t"<<sigeps_sim_NS<<"\t"<<R_0_sim_NS<<"\t"<<sigR_0_sim_NS<<"\t"<<chi2NDF_sim_NS<<endl; 
        
        Record ergTimeNS = lateralFitter.lateralTimeDistribution(plotPrefix,
                                                                 m_recNS,m_simNS,
                                                                 Gt,AzL,90.-ElL,
                                                                 CCcenter_NS,
                                                                 index1, index2);
                                                                                                                                                                                                 
        latTimeSphere1DRcurv_NS = ergTimeNS.asDouble("latTime1D_Rcurv");
        latTimeSphere1DSigRcurv_NS = ergTimeNS.asDouble("latTime1D_sigRcurv");
        latTimeSphere1DChi2NDF_NS = ergTimeNS.asDouble("latTime1D_chi2NDF");
        latTimeSphere2DRcurv_NS = ergTimeNS.asDouble("latTime2D_Rcurv");
        latTimeSphere2DSigRcurv_NS = ergTimeNS.asDouble("latTime2D_sigRcurv");
        latTimeSphere2DChi2NDF_NS = ergTimeNS.asDouble("latTime2D_chi2NDF");
        
        latTimeSphere1DRcurv_sim_NS = ergTimeNS.asDouble("latTime1D_Rcurv_sim");
        latTimeSphere1DSigRcurv_sim_NS = ergTimeNS.asDouble("latTime1D_sigRcurv_sim");
        latTimeSphere1DChi2NDF_sim_NS = ergTimeNS.asDouble("latTime1D_chi2NDF_sim");
        latTimeSphere2DRcurv_sim_NS = ergTimeNS.asDouble("latTime2D_Rcurv_sim");
        latTimeSphere2DSigRcurv_sim_NS = ergTimeNS.asDouble("latTime2D_sigRcurv_sim");
        latTimeSphere2DChi2NDF_sim_NS = ergTimeNS.asDouble("latTime2D_chi2NDF_sim");
                                                                 
        latTimeCone1DRho_NS = ergTimeNS.asDouble("latTime1D_ConeRho");
        latTimeCone1DSigRho_NS = ergTimeNS.asDouble("latTime1D_sigConeRho");
        latTimeCone1DChi2NDF_NS = ergTimeNS.asDouble("latTime1D_Conechi2NDF");
        latTimeCone2DRho_NS = ergTimeNS.asDouble("latTime2D_ConeRho");
        latTimeCone2DSigRho_NS = ergTimeNS.asDouble("latTime2D_sigConeRho");
        latTimeCone2DChi2NDF_NS = ergTimeNS.asDouble("latTime2D_Conechi2NDF");
        
        latTimeCone1DRho_sim_NS = ergTimeNS.asDouble("latTime1D_ConeRho_sim");
        latTimeCone1DSigRho_sim_NS = ergTimeNS.asDouble("latTime1D_sigConeRho_sim");
        latTimeCone1DChi2NDF_sim_NS = ergTimeNS.asDouble("latTime1D_Conechi2NDF_sim");
        latTimeCone2DRho_sim_NS = ergTimeNS.asDouble("latTime2D_ConeRho_sim");
        latTimeCone2DSigRho_sim_NS = ergTimeNS.asDouble("latTime2D_sigConeRho_sim");
        latTimeCone2DChi2NDF_sim_NS = ergTimeNS.asDouble("latTime2D_Conechi2NDF_sim");
      }
      
      // Fill information in root file
      outtree->Fill();
      rootOutfile->Write("",TObject::kOverwrite);
    }//loop on the rec event 
    checkDistanceEW.close();
    checkDistanceNS.close();
    outputEW.close();
    outputNS.close();
    recRoot->Close();
    rootOutfile->Close();
    
    // Mean deviation of antenna position in shower coordinates:
   meanDevX /= double(meanDevCounter);
   meanDevY /= double(meanDevCounter);
   meanDevZ /= double(meanDevCounter);
   //cout << "\nMean deviation of antenna position in shower coordinates (is ought to be small):\n"
   //     << "x: " << meanDevX << "\t y:" << meanDevY << "\t z:" << meanDevZ << endl;
  } catch (AipsError x) {
    cerr << "compareLOPES2sim: " << x.getMesg() << endl;
  }
  
  return 0;
}

