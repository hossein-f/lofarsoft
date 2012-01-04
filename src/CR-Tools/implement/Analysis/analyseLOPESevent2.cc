/*-------------------------------------------------------------------------*
 | $Id::                                                                 $ |
 *-------------------------------------------------------------------------*
 ***************************************************************************
 *   Copyright (C) 2008                                                    *
 *   Frank Schroeder (<mail>)                                              *
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

#include <Analysis/analyseLOPESevent2.h>

#include "TROOT.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TPaveStats.h"
#include "TAxis.h"

#ifdef HAVE_STARTOOLS

namespace CR { // Namespace CR -- begin

  // ============================================================================
  //
  //  Construction
  //
  // ============================================================================

  analyseLOPESevent2::analyseLOPESevent2 ()
    : analyseLOPESevent(),
      CompleteBeamPipe_p(NULL),
      minBeamformingAntennas(4),
      upsamplingExponent(0),
      ccWindowWidth_p(0.045e-6),
      spectrumStart_p(40e6),
      spectrumStop_p(80e6),
      rawPulses( map<int,PulseProperties>() ),
      calibPulses( map<int,PulseProperties>() ),
      coreError(0.),
      zenithError(0.),
      azimuthError(0.),
      noiseMethod(4)
  {;}

  // ============================================================================
  //
  //  Destruction
  //
  // ============================================================================

  analyseLOPESevent2::~analyseLOPESevent2 ()
  {
    // Set pipeline_p back to NULL (to avoid that analyseLOPESevent deletes pipeline)
    pipeline_p         = NULL;
    upsamplePipe_p     = NULL;
    CompleteBeamPipe_p = NULL;
  }

  // ============================================================================
  //
  //  Parameters
  //
  // ============================================================================

  void analyseLOPESevent2::summary (std::ostream &os)
  {
    os << "[analyseLOPESevent2] Summary of internal parameters" << std::endl;
  }

  // ============================================================================
  //
  //  Methods
  //
  // ============================================================================

  // --------------------------------------------------------------- initPipeline

  void analyseLOPESevent2::clear() {
    rawPulses = map<int,PulseProperties>();
    calibPulses = map<int,PulseProperties>();

    // from analyseLOPESevent::clear():
    if (lev_p != NULL) {
      delete lev_p;
      lev_p = NULL;
    }
    if (upsampler_p != NULL) {
      delete upsampler_p;
      upsampler_p = NULL;
    }
    
    pipeline.init();
    upsamplePipe.init();

    // To use the methods of analyseLOPESevent, a pointer to the pipeline is needed.
    pipeline_p = &pipeline;
    upsamplePipe_p = &upsamplePipe;
  }

  Bool analyseLOPESevent2::initPipeline(Record ObsRecord){
    try {
      clear();

      // use pointers if code from analyseLOPESevent.cc is reused
      lev_p = new LopesEventIn();
      pipeline_p->SetObsRecord(ObsRecord);
      upsamplePipe_p->SetObsRecord(ObsRecord);

      cout << "\nanalyseLOPESevent2: Pipeline initialised." << endl;
    } catch (AipsError x) {
      cerr << "analyseLOPESevent2::initPipeline: " << x.getMesg() << endl;
      return False;
    }
    return True;
  }

  // --------------------------------------------------------------- ProcessEvent

  Record analyseLOPESevent2::RunPipeline(const string& evname,
                                         Double Az,
                                         Double El,
                                         Double distance, 
                                         Double XC,
                                         Double YC,
                                         Bool RotatePos,
                                         string PlotPrefix, 
                                         Bool generatePlots,
                                         Bool generateSpectra,
                                         Vector<Int> FlaggedAntIDs, 
                                         Bool verbose,
                                         Bool simplexFit,
                                         Double ExtraDelay,
                                         int doTVcal,
                                         bool doGainCal,
                                         bool doDispersionCal,
                                         bool doDelayCal,
                                         bool doRFImitigation,
                                         bool doFlagNotActiveAnts,
                                         bool doAutoFlagging,
                                         Double UpSamplingRate,
                                         String Polarization,
                                         bool SinglePlots,
                                         bool PlotRawData,
                                         bool CalculateMaxima,
                                         bool listCalcMaxima,
                                         bool printShowerCoordinates,
                                         bool ignoreDistance,
                                         bool conicalBeamforming,
                                         double randomDelay,
                                         unsigned int randomSeed) {
    Record erg;
    try {
      // ofstream latexfile;  // WARNING: causes problem in fitCR2gauss.cc line 200, left here for future tests
      Vector <Bool> AntennaSelection;
      Record fiterg;       // for results of spherical and conical beamforming
      Double center=0.;                // position of the cc-beam (set in doPositionFitting)
      // define new variables for azimuth and elevation, since they are changed during beamforming
      Double beamformingAz = Az;
      Double beamformingEl = El;
      Double coneAngle = 0.015; // starting coneAngle should have no effect, since the simplex fit determines an independent starting value


      // define default values for return record, to prevent crash of call_pipeline
      // in the case of any error during RunPipeline
      erg.define("goodReconstructed",false);  // will be set to true at the end of the reconstruction
      erg.define("Azimuth",double(0));
      erg.define("Elevation",double(0));
      erg.define("Distance",double(0));
      erg.define("CCheight",double(0));
      erg.define("CCwidth",double(0));
      erg.define("CCheight_error",double(0));
      erg.define("CCconverged",false);
      erg.define("Xheight",double(0));
      erg.define("Xheight_error",double(0));
      erg.define("Xconverged",false);
      erg.define("rmsCCbeam",double(0));
      erg.define("rmsXbeam",double(0));
      erg.define("rmsPbeam",double(0));
      erg.define("meandist",double(0));
      erg.define("Date",uInt(0));
      erg.define("NCCbeamAntennas",uInt(0));
      erg.define("coneAngle",double(0));

      // store a copy of the input antenna selection for later use
      InputFlaggedAntIDs = FlaggedAntIDs.copy();

      // initialize the pipeline (flag not active antennas and antennas with bad signals)
      if (! SetupEvent(evname, doTVcal, FlaggedAntIDs, AntennaSelection, 
                       UpSamplingRate, ExtraDelay, verbose, doGainCal, doDispersionCal, doDelayCal, doRFImitigation,
                       doFlagNotActiveAnts, doAutoFlagging)) {
        cerr << "analyseLOPESevent2::RunPipeline: " << "Error during SetupEvent()!" << endl;
        return erg;
      }

      // Check if any antennas are left unflagged (otherwise quit)
      if ( ntrue(AntennaSelection) == 0) {
        cerr << "analyseLOPESevent2::RunPipeline: " << "All antennas flagged!" << endl;
        return erg;
      }

      // storte GT in return record
      erg.mergeField(lev_p->headerRecord(),"Date", RecordInterface::OverwriteDuplicates);

      // set Complete Pipeline and print output, which case is set!
      CompleteBeamPipe_p = static_cast<CompletePipeline*>(beamPipe_p);
      if (verbose) {
        if (CompleteBeamPipe_p == pipeline_p)
          cout << "analyseLOPESevent2::RunPipeline: " << "Proceed without (new) upsampling." << endl;
        else
          cout << "analyseLOPESevent2::RunPipeline: " << "(New) upsampling enabled." << endl;
      }

      // initialize Complete Pipeline
      CompleteBeamPipe_p->setPlotInterval(plotStart(),plotStop());
      CompleteBeamPipe_p->setCCWindowWidth(getCCWindowWidth());
      CompleteBeamPipe_p->setSpectrumInterval(getSpectrumStart(),getSpectrumStop());
      CompleteBeamPipe_p->setPolarization(Polarization);
      CompleteBeamPipe_p->setCalibrationMode(false);

      // create antenna selection of correct polarization (for Steffen's lateral output)
      PolarizationAntennaSelection = AntennaSelection.copy();
      PolarizationAntennaSelection.set(true);
      CompleteBeamPipe_p->deselectectPolarization(beamformDR_p,PolarizationAntennaSelection);

      // flag antennas with wrong polarization for normal analysis
      CompleteBeamPipe_p->deselectectPolarization(beamformDR_p,AntennaSelection);

      // Plot the raw data, if desired
      if (PlotRawData) {
        // plot upsampled raw data: either in seperated plots seperated or all traces in one plot
        if (SinglePlots)
          CompleteBeamPipe_p->plotAllAntennas(PlotPrefix+ "-raw", beamformDR_p, AntennaSelection, true,
                                              getUpsamplingExponent(),true);
        else
          CompleteBeamPipe_p->plotAllAntennas(PlotPrefix + "-raw", beamformDR_p, AntennaSelection, false,
                                              getUpsamplingExponent(), true);

        // calculate the maxima
        if (CalculateMaxima)
          rawPulses = CompleteBeamPipe_p->calculateMaxima(beamformDR_p, AntennaSelection, getUpsamplingExponent(), true);
      }

      // add random delay for timing uncertainty studies
      if (randomDelay > 0.)
        CompleteBeamPipe_p->addRandomDelay(beamformDR_p,randomDelay, randomSeed);

      // Check if there are enough antennas left unflagged (otherwise quit)
      // beam forming is not useful with only 3 or less antennas.
      if ( ntrue(AntennaSelection) < minBeamformingAntennas) {
        cerr << "analyseLOPESevent2::RunPipeline: " 
             << "Bad event: only "
             << ntrue(AntennaSelection)
             << " antenna(s) left unflagged (at least "
             << minBeamformingAntennas
             << " required)."
             << endl;
        return erg;
      }
      // store number of antennas used for CC beam
      erg.define("NCCbeamAntennas",uInt(ntrue(AntennaSelection)));

      Double dummyDistance = 1e5;  // Dummy distance for conical beamforming
      if (conicalBeamforming) {
        cout << "\nStarting conical beamforming...\n" << endl;
        distance = dummyDistance;
        
        // set starting coneAngle (should have no effect, since the simplex fit determines an independent starting value)
        CompleteBeamPipe_p->setConeAngle(coneAngle);
        
        if (! doConicalPositionFitting(beamformingAz, beamformingEl, coneAngle, center, XC, YC, RotatePos,
                                       AntennaSelection, Polarization, simplexFit, verbose, ignoreDistance) ) {
          cerr << "analyseLOPESevent2::RunPipeline: " << "Error during doConicalPositionFitting()!" << endl;
          return erg;
        }
      } else {
        cout << "\nStarting spherical beamforming...\n" << endl;
        coneAngle = 0;
        
        //perform the position fitting (if simplexFit = false, then only the PhaseCenter is set)
        if (! doPositionFitting(beamformingAz, beamformingEl, distance, center, XC, YC, RotatePos,
                                AntennaSelection, Polarization, simplexFit, verbose, ignoreDistance) ){
          cerr << "analyseLOPESevent2::RunPipeline: " << "Error during doPositionFitting()!" << endl;
          return erg;
        }
      }

      // make gauss fit to CC-beam
      if (! GaussFitData(beamformingAz, beamformingEl, distance, center, AntennaSelection, evname, erg, fiterg, 
                         Polarization, verbose, coneAngle) ){
        cerr << "analyseLOPESevent2::RunPipeline: " << "Error during GaussFitData()!" << endl;
        return erg;
      }
      
      // check if time of CC beam is within fit range
      if ( (erg.asDouble("CCcenter") < fitStart()) || (erg.asDouble("CCcenter") > fitStop()) ) {
        cout << "\nBad reconstruction: Time of CC beam is outside of fit range!\n" << endl;
        fiterg.define("CCconverged", false);
        fiterg.define("Xconverged", false);
      }
      

      // Generate plots
      if (generatePlots) {
        // Plot CC-beam; if fit has converged, then also plot the result of the fit
        if (fiterg.asBool("CCconverged"))
          CompleteBeamPipe_p->plotCCbeam(PlotPrefix + "-CC", beamformDR_p, fiterg.asArrayDouble("Cgaussian"),
                                         AntennaSelection, filterStrength_p, remoteRange_p(0), remoteRange_p(1));
        else
          CompleteBeamPipe_p->plotCCbeam(PlotPrefix + "-CC", beamformDR_p, Vector<Double>(),
                                         AntennaSelection, filterStrength_p, remoteRange_p(0), remoteRange_p(1));

        // Plot X-beam; if fit has converged, then also plot the result of the fit
        if (fiterg.asBool("Xconverged"))
          CompleteBeamPipe_p->plotXbeam(PlotPrefix + "-X", beamformDR_p, fiterg.asArrayDouble("Xgaussian"),
                                        AntennaSelection, filterStrength_p, remoteRange_p(0), remoteRange_p(1) );
        else
          CompleteBeamPipe_p->plotXbeam(PlotPrefix + "-X", beamformDR_p, Vector<Double>(),
                                        AntennaSelection, filterStrength_p, remoteRange_p(0), remoteRange_p(1));

        // Plot of all antenna traces together
        CompleteBeamPipe_p->plotAllAntennas(PlotPrefix + "-all", beamformDR_p, AntennaSelection, false,
                                            getUpsamplingExponent(),false);

        // Plot of upsampled antenna traces (seperated), only if no conical beamforming will be done afterwards
        if (SinglePlots)
          CompleteBeamPipe_p->plotAllAntennas(PlotPrefix, beamformDR_p, AntennaSelection, true,
                                              getUpsamplingExponent(),false,true);
                                              
        // calculate the maxima (only if CC-beam was reconstructed successfully and its time is ok):
        // calculate noise as mean of local maxima of the envelope in time range of -10.5 to -0.5 µs before CC center)      
        if (fiterg.asBool("CCconverged")) {
          if (CalculateMaxima) {
            calibPulses = CompleteBeamPipe_p->calculateMaxima(beamformDR_p, AntennaSelection, getUpsamplingExponent(),
                                                            false, fiterg.asDouble("CCcenter"), noiseMethod,
                                                            fiterg.asDouble("CCcenter") - 10.5e-6, fiterg.asDouble("CCcenter") - 0.5e-6);
            caculateNoiseInfluence();
          }                                                
          // user friendly list of calculated maxima
          if (listCalcMaxima)
            CompleteBeamPipe_p->listCalcMaxima(beamformDR_p, AntennaSelection, getUpsamplingExponent(),fiterg.asDouble("CCcenter"));
        }    
      } // if (generatePlots)

      // calculate the rms values of p-, cc- and xbeam in the remote region
      if (remoteRange_p(1) != 0 ) {
        Slice remoteRegion( remoteRange_p(0),(remoteRange_p(1) - remoteRange_p(0)) );
        Vector<Double> ccbeam, xbeam, pbeam;

        // get the beam data
        ccbeam = CompleteBeamPipe_p->GetCCBeam(beamformDR_p, AntennaSelection, Polarization).copy();
        xbeam = CompleteBeamPipe_p->GetXBeam(beamformDR_p, AntennaSelection, Polarization).copy();
        pbeam = CompleteBeamPipe_p->GetPBeam(beamformDR_p, AntennaSelection, Polarization).copy();

        // smooth the data
        if (filterStrength_p > 0) {
            StatisticsFilter<Double> mf(filterStrength_p,FilterType::MEAN);
          ccbeam = mf.filter(ccbeam);
          pbeam = mf.filter(pbeam);
          xbeam = mf.filter(xbeam);
        }

        // removing offset
        double ccBeamOffset = mean(ccbeam(remoteRegion));
        double pBeamOffset  = mean(pbeam(remoteRegion));
        double xBeamOffset  = mean(xbeam(remoteRegion));
        ccbeam -= ccBeamOffset;
        pbeam  -= pBeamOffset;
        xbeam  -= xBeamOffset;

        // calculating rms values
        erg.define("rmsCCbeam",rms(ccbeam(remoteRegion)));
        erg.define("rmsPbeam",rms(pbeam(remoteRegion)));
        erg.define("rmsXbeam",rms(xbeam(remoteRegion)));
      }
      
      
      // Generate spectra
      if (generateSpectra) {
        if (SinglePlots)
          CompleteBeamPipe_p->plotSpectra(PlotPrefix+"-spec", beamformDR_p, AntennaSelection, true);
        else
          CompleteBeamPipe_p->plotSpectra(PlotPrefix + "-spec-all", beamformDR_p, AntennaSelection, false);
      }

      // output of antnenna to core distances in shower coordinates, if requested
      if (printShowerCoordinates)
        printAntennaDistances(erg.asArrayDouble("distances"),
                              toShower(beamPipe_p->GetAntPositions(), erg.asDouble("Azimuth"),
                              erg.asDouble("Elevation")),
                              erg.asDouble("Elevation"), erg.asDouble("Azimuth"), XC, YC,
                              beamformDR_p->headerRecord().asInt("Date"));
        
      // store shower coordinates for LOPES reconstruction and KASACADE-Grande input direction
      storeShowerCoordinates(erg.asDouble("Elevation")*PI/180., erg.asDouble("Azimuth")*PI/180.,
                             El*PI/180., Az*PI/180.);
      

      // give out the names of the created plots
      if (verbose) {
        vector<string> plotlist = CompleteBeamPipe_p->getPlotList();
        cout <<"\nList of generated plots:\n";
        for (unsigned int i = 0; i < plotlist.size(); i++) 
          cout << plotlist[i] << "\n";
        cout << endl;
      }

      // print resutls of reconstruction          
      if (conicalBeamforming)           
        cout << "\nRestuls of reconstruction with conical beamforming:\n"
            << "Azimuth = " << erg.asDouble("Azimuth") << " degree \t"
            << "Elevation = " << erg.asDouble("Elevation") << " degree\n"
            << "cone angle (rho) = " << erg.asDouble("coneAngle") 
            << " rad (=" << erg.asDouble("coneAngle")*180/M_PI << " degree)\n"
            << "CC-beam = " << erg.asDouble("CCheight")*1e6 << " µV/m/MHz \t"
            << " X-beam = " << erg.asDouble("Xheight")*1e6  << " µV/m/MHz \n"
            << endl;
      else
        cout << "\nRestuls of reconstruction with spherical beamforming:\n"
            << "Azimuth = " << erg.asDouble("Azimuth") << " degree \t"
            << "Elevation = " << erg.asDouble("Elevation") << " degree\n"
            << "Radius of curvature (of CC-beam) = " << erg.asDouble("Distance") << " m\n"
            << "CC-beam = " << erg.asDouble("CCheight")*1e6 << " µV/m/MHz \t"
            << " X-beam = " << erg.asDouble("Xheight")*1e6  << " µV/m/MHz \n"
            << endl;      
             
      // check if reconstruction was good (only for spherical beamforming)
      if ( (fiterg.asBool("CCconverged")) && (fiterg.asBool("Xconverged")) ) {
        // require that cc-time is inseide of fit range, and that x-beam and cc-beam are roughly at the same position
        if ( (erg.asDouble("CCcenter") < fitStart()) || (erg.asDouble("CCcenter") > fitStop()) ) {
          cout << "Bad reconstruction: Time of CC beam is outside of fit range!" << endl;
        } else if ( abs(erg.asDouble("CCcenter") - erg.asDouble("Xcenter")) > getCCWindowWidth()/2.) {
          cout << "Bad reconstruction: Time of CC beam and X beam differs by more than " << getCCWindowWidth()/2.*1e9 << " ns." << endl;
        } else {
          erg.define("goodReconstructed",true);  // assume that everything was fine, then this position is reached.
        }  
      }   
    } catch (AipsError x) {
      cerr << "analyseLOPESevent2::RunPipeline: " << x.getMesg() << endl;
      return Record();
    }
    return erg;
  }


  // -------------------------------------- Process Calibration Event

  Record analyseLOPESevent2::CalibrationPipeline (const string& evname,
                                                  string PlotPrefix, 
                                                  Bool generatePlots,
                                                  Bool generateSpectra,
                                                  Vector<Int> FlaggedAntIDs, 
                                                  Bool verbose,
                                                  bool doGainCal,
                                                  bool doDispersionCal,
                                                  bool doDelayCal,
                                                  bool doRFImitigation,
                                                  bool SinglePlots,
                                                  bool PlotRawData,
                                                  bool CalculateMaxima)
  {
    Record erg;
    try {
      // ofstream latexfile;  // WARNING: causes problem in fitCR2gauss.cc line 200, left here for future tests
      Vector <Bool> AntennaSelection;

      erg.define("goodReconstructed",false);  // will be set to true at the end of the reconstruction

      // store a copy of the input antenna selection for later use
      InputFlaggedAntIDs = FlaggedAntIDs.copy();

      pipeline_p->setVerbosity(verbose);
      // Generate the Data Reader
      if (! lev_p->attachFile(evname) ) {
        cerr << "analyseLOPESevent::SetupEvent: " << "Failed to attach file: " << evname << endl;
        return Record();
      }

      //  Enable/disable calibration steps of the FirstStagePipeline (must be done before InitEvent)
      //  parameters are initialized with 'true' by default
      pipeline_p->doGainCal(doGainCal);
      pipeline_p->doDispersionCal(doDispersionCal);
      pipeline_p->doDelayCal(doDelayCal);
      pipeline_p->doFlagNotActiveAnts(false);                // use all antennas in calibration mode
      pipeline_p->setFreqInterval(getFreqStart(),getFreqStop());

      // initialize the Data Reader
      if (! pipeline_p->InitEvent(lev_p)) {
        cerr << "analyseLOPESevent2::CalibrationPipeline: " << "Failed to initialize the DataReader!" << endl;
        return Record();
      }

      //  Enable/disable calibration steps for the SecondStagePipeline
      pipeline_p->doPhaseCal(false);
      pipeline_p->doRFImitigation(doRFImitigation);

      // Generate the antenna selection
      Int i,j,id,nants, nflagged=FlaggedAntIDs.nelements();
      AntennaSelection.resize();
      AntennaSelection = pipeline_p->GetAntennaMask(lev_p);
      nants = AntennaSelection.nelements();
      Vector<Int> AntennaIDs,selAntIDs;
      lev_p->headerRecord().get("AntennaIDs",AntennaIDs);
      if (nflagged >0){
        for (i=0; i<nflagged; i++){
          id = FlaggedAntIDs(i);
          for (j=0; j<nants; j++){
            if (AntennaIDs(j) == id) {
              AntennaSelection(j) = False;
              id = 0;
              break;
            };
          };
          if (verbose && (id !=0)){
            cout << "analyseLOPESevent::SetupEvent: " << "AntennaID: " << id 
                 << " not found -> no antenna flagged." << endl;
          }
        }
      }

      // storte GT in return record
      erg.mergeField(lev_p->headerRecord(),"Date", RecordInterface::OverwriteDuplicates);

      // initialize Complete Pipeline
      CompleteBeamPipe_p = static_cast<CompletePipeline*>(pipeline_p);
      CompleteBeamPipe_p->setPlotInterval(plotStart(),plotStop());
      CompleteBeamPipe_p->setSpectrumInterval(getSpectrumStart(),getSpectrumStop());
      CompleteBeamPipe_p->setCalibrationMode(true);

      // Plot the raw data, if desired
      if (PlotRawData)
      {
        // plot upsampled raw data: either in seperated plots seperated or all traces in one plot
        if (SinglePlots)
          CompleteBeamPipe_p->plotAllAntennas(PlotPrefix+ "-raw", lev_p, AntennaSelection, true,
                                              getUpsamplingExponent(),true, true);
        else
          CompleteBeamPipe_p->plotAllAntennas(PlotPrefix + "-raw", lev_p, AntennaSelection, false,
                                              getUpsamplingExponent(), true, false);

        // calculate the maxima
        if (CalculateMaxima)
          rawPulses = CompleteBeamPipe_p->calculateMaxima(lev_p, AntennaSelection, getUpsamplingExponent(), true);
      }

      // Generate plots
      if (generatePlots)
      {
        // Plot of upsampled antenna traces (seperated with envelope or together without envelope) 
        if (SinglePlots)
          CompleteBeamPipe_p->plotAllAntennas(PlotPrefix, lev_p, AntennaSelection, true,
                                              getUpsamplingExponent(),false, true);
        else
          CompleteBeamPipe_p->plotAllAntennas(PlotPrefix + "-all", lev_p, AntennaSelection, false,
                                              getUpsamplingExponent(),false, false);

        // calculate the maxima
        if (CalculateMaxima)
          calibPulses = CompleteBeamPipe_p->calculateMaxima(lev_p, AntennaSelection, getUpsamplingExponent(), false);
      }

      // Generate spectra
      if (generateSpectra) {
        if (SinglePlots)
          CompleteBeamPipe_p->plotSpectra(PlotPrefix+"-spec", lev_p, AntennaSelection, true);
        else
          CompleteBeamPipe_p->plotSpectra(PlotPrefix + "-spec-all", lev_p, AntennaSelection, false);
      }

      // give out the names of the created plots
      if (verbose) {
        vector<string> plotlist = CompleteBeamPipe_p->getPlotList();
        cout <<"\nList of generated plots:\n";
        for (unsigned int i = 0; i < plotlist.size(); i++) 
          cout << plotlist[i] << "\n";
        cout << endl;
      }

      erg.define("goodReconstructed",true);  // assume that everything was fine, then this position is reached.

    } catch (AipsError x) {
      cerr << "analyseLOPESevent2::CalibrationPipeline: " << x.getMesg() << endl;
      return Record();
    }
    return erg;
  }


  void analyseLOPESevent2::printAntennaDistances (const Vector <double>& distances,
                                                  const Matrix<double>& AntPos,
                                                  const double& Az,
                                                  const double& El,
                                                  const double& Xc,
                                                  const double& Yc,
                                                  const unsigned int date) const
  {
    try {
      // check if size is consistent
      if ( AntPos.nrow() != distances.size()) {
        cerr << "analyseLOPESevent2::printAntennaDistances: " 
             << "ERROR: number of distances and antenna positions are different!" <<  endl;
        return;                // exit function without showing (confusing) results
      }

      // print header
      std::cout << "GT "<< date << " " << Az << " " << El << " " << Xc << " " <<Yc <<std::endl;
      std::cout << "An  dist_x    dist_y    dist"<<std::endl;

      // loop through all antennas in print the distances (should be in shower coordinates)
      for (unsigned int i=0 ; i < distances.size(); i++) {
        std::cout << std::setw(2) << i+1 << " " << std::setw(8) << AntPos.row(i)(0) << "  ";
        std::cout << std::setw(8) << AntPos.row(i)(1) << "  " << std::setw(8) <<distances(i)<<std::endl;
      }
    } catch (AipsError x) {
      cerr << "analyseLOPESevent2::printAntennaDistances: " << x.getMesg() << endl;
    }
  }



 void analyseLOPESevent2::storeShowerCoordinates (const double elevation,
                                                  const double azimuth,
                                                  const double elevationKG,
                                                  const double azimuthKG)
  {
    try {
      // get AntennaIDs to store pulse parameters in corresponding map
      Vector<int> antennaIDs;
      beamformDR_p->headerRecord().get("AntennaIDs",antennaIDs);

      // get antenna positions (shifted, so that core position is at 0)
      Matrix <double> antPos = beamPipe_p->GetAntPositions(beamformDR_p);
      // get antenna positions in shower coordinates
      Matrix <double> antPosS = toShower(beamPipe_p->GetAntPositions(beamformDR_p), azimuth/PI*180., elevation/PI*180.);
      Matrix <double> antPosSKG = toShower(beamPipe_p->GetAntPositions(beamformDR_p), azimuthKG/PI*180., elevationKG/PI*180.);

      // for error calculation
      // errors of GPS measurement (< 1-2 cm) is neglected
      // possible shift between KASCADE and LOPES coordinate system ~20 cm is also negelected
      // calcualte squared errors for x, y, and z in shower coordinates
      // formulas: error propagation of transformation to shower coordinates
      double sinEl = sin(elevation);
      double cosEl = cos(elevation);
      double sinAz = sin(azimuth);
      double cosAz = cos(azimuth);
      double sinElKG = sin(elevationKG);
      double cosElKG = cos(elevationKG);
      double sinAzKG = sin(azimuthKG);
      double cosAzKG = cos(azimuthKG);

      // loop through antenna IDs and write distances in pulseProperties if element exists
      for (unsigned int i=0 ; i < antennaIDs.size(); i++)
        if (calibPulses.find(antennaIDs(i)) != calibPulses.end()) {
          // substitution for better readablitiy
          double x = antPos.row(i)(0);
          double y = antPos.row(i)(1);
          // double z = antPos.row(i)(2); // not used
          double xS = antPosS.row(i)(0);
          double yS = antPosS.row(i)(1);
          double zS = antPosS.row(i)(2);
          double xSKG = antPosSKG.row(i)(0);
          double ySKG = antPosSKG.row(i)(1);
          double zSKG = antPosSKG.row(i)(2);
          calibPulses[antennaIDs(i)].distX    = xS;
          calibPulses[antennaIDs(i)].distY    = yS;
          calibPulses[antennaIDs(i)].distZ    = zS;
          calibPulses[antennaIDs(i)].dist     = sqrt(xS*xS+yS*yS);  // perpendicular to axis --> no zS!
          calibPulses[antennaIDs(i)].KGdistX    = xSKG;
          calibPulses[antennaIDs(i)].KGdistY    = ySKG;
          calibPulses[antennaIDs(i)].KGdistZ    = zSKG;
          calibPulses[antennaIDs(i)].KGdist     = sqrt(xSKG*xSKG+ySKG*ySKG);
          // calcualte errors
          calibPulses[antennaIDs(i)].distXerr = sqrt(  pow(coreError, 2)
                                                     + pow(( x*cosAz + y*sinAz)*azimuthError, 2)); 
          calibPulses[antennaIDs(i)].distYerr = sqrt(  pow(coreError*sinEl, 2)
                                                     + pow((-x*sinAz + y*cosAz)*azimuthError*sinEl, 2)
                                                     + pow(( x*cosAz + y*sinAz)*zenithError *cosEl, 2)); 
          calibPulses[antennaIDs(i)].KGdistXerr = sqrt(  pow(coreError, 2)
                                                       + pow(( x*cosAzKG + y*sinAzKG)*azimuthError, 2)); 
          calibPulses[antennaIDs(i)].KGdistYerr = sqrt(  pow(coreError*sinElKG, 2)
                                                       + pow((-x*sinAzKG + y*cosAzKG)*azimuthError*sinElKG, 2)
                                                       + pow(( x*cosAzKG + y*sinAzKG)*zenithError *cosElKG, 2)); 
          
          // The error of z due to the core uncertainty should affect all z-values exactly in the same direction.
          // That means a wrong core position should either decrease or increase all z-values the same way.
          // Thus, it should not be considered as statistical error in any fit.
          // Remark: for x and y this is also true, but the lateral distance R can increase or decrease
          //         for different antennas if the core is wrong. Hence, for x and y the core uncertainty
          //         cannot be negelected (maybe it can be paritially neglected, but I have not investigated this).
          calibPulses[antennaIDs(i)].distZerr = sqrt(  // pow(coreError*cosEl, 2)
                                                     + pow((-x*sinAz + y*cosAz)*azimuthError*cosEl, 2)
                                                     + pow(( x*cosAz + y*sinAz)*zenithError *sinEl, 2)); 
                                                    
          calibPulses[antennaIDs(i)].disterr  = sqrt(  pow(xS*calibPulses[antennaIDs(i)].distXerr,2) 
                                                     + pow(yS*calibPulses[antennaIDs(i)].distYerr,2))
                                                / calibPulses[antennaIDs(i)].dist;
                                                
          calibPulses[antennaIDs(i)].KGdistZerr = sqrt(  // pow(coreError*cosKGEl, 2)
                                                       + pow((-x*sinAzKG + y*cosAzKG)*azimuthError*cosElKG, 2)
                                                       + pow(( x*cosAzKG + y*sinAzKG)*zenithError *sinElKG, 2)); 
                                                      
          calibPulses[antennaIDs(i)].KGdisterr  = sqrt(  pow(xSKG*calibPulses[antennaIDs(i)].KGdistXerr,2) 
                                                       + pow(ySKG*calibPulses[antennaIDs(i)].KGdistYerr,2))
                                                  / calibPulses[antennaIDs(i)].KGdist;          // for debug output                                                
          /* cout << calibPulses[antennaIDs(i)].distX << " +/-" << calibPulses[antennaIDs(i)].distXerr << "\t "
               << calibPulses[antennaIDs(i)].distY << " +/-" << calibPulses[antennaIDs(i)].distYerr << "\t "
               << calibPulses[antennaIDs(i)].distZ << " +/-" << calibPulses[antennaIDs(i)].distZerr << "\t "
               << calibPulses[antennaIDs(i)].dist  << " = "
               << sqrt(xS*xS+yS*yS) << " +/-" << calibPulses[antennaIDs(i)].disterr  << "\t " << endl;  */  
               
          // calculate azimuth on ground for each antenna (angle between core and antenna position)     
          calibPulses[antennaIDs(i)].angleToCore = atan2(x,y);
          // relative error corresponds roughly to relative error of coordinates
          calibPulses[antennaIDs(i)].angleToCoreerr =
            abs(calibPulses[antennaIDs(i)].angleToCore) * (2.*coreError)/(abs(x)+abs(y));
          // limit angular error to Pi
          if (calibPulses[antennaIDs(i)].angleToCoreerr > M_PI)
            calibPulses[antennaIDs(i)].angleToCoreerr = M_PI;
        }

    } catch (AipsError x) {
      cerr << "analyseLOPESevent2::storeShowerCoordinates: " << x.getMesg() << endl;
    }
  }


  Vector<int> analyseLOPESevent2::getAntennaIDs (void)
  {
    try {
      // get AntennaIDs if there is an event
      if (beamformDR_p != NULL) {
        Vector<int> antennaIDs;
        beamformDR_p->headerRecord().get("AntennaIDs",antennaIDs);
        return antennaIDs;
      }
    } catch (AipsError x) {
      cerr << "analyseLOPESevent2::getAntennaIDs: " << x.getMesg() << endl;
    }
    return Vector<int>();
  }


  Matrix<Double> analyseLOPESevent2::getAntennaPositions(void)
  {
    try {
      // get antenna positions: not shifted for reference (core?) position
      if ((beamPipe_p != NULL) && (beamformDR_p != NULL)) {
        return beamPipe_p->GetAntPositions(beamformDR_p, false).copy();
      }
    } catch (AipsError x) {
      cerr << "analyseLOPESevent2::getAntennaPositions: " << x.getMesg() << endl;
    }
    return Matrix<Double>();
  }


  void analyseLOPESevent2::summaryPlot(string filename,
                                       unsigned int columns)
  {
    try {
      // check if anything should be done (means columns must be > 0)
      if (columns <= 0) return;

      // create latexfilename
      string latexfilename = filename + ".tex";

      // open latexfile
      ofstream latexfile;
      latexfile.open(latexfilename.c_str(), ofstream::out);

      // check if file could be opened
      if (!(latexfile.is_open())) {
        std::cerr << "Failed to write to file \"" << latexfilename <<"\"." << std::endl;
      } else {
        // Create header for latex file
        latexfile << "\\documentclass[a4paper]{article}\n";
        latexfile << "\\usepackage{graphicx,a4wide}\n";
        latexfile << "\\setlength{\\textheight}{24cm}\n";
        latexfile << "\\setlength{\\topmargin}{0.0cm}\n";
        latexfile << "\\parindent=0pt\n";
        latexfile << "\\begin{document}\n";

        // get list of generated plots and loop through the list
        vector<string> plotlist = CompleteBeamPipe_p->getPlotList();

        for (unsigned int i = 0; i < plotlist.size(); i++) {
          // line break after #columns plots
          if ( (i > 0) && ((i % columns) == 0) ) latexfile << "\\newline\n";

          // add plot to latexfile
          latexfile << "\\begin{minipage}[h]{" << (0.999/columns) 
                    << "\\textwidth}\\includegraphics[angle=-90,width=\\textwidth]{"
                    << plotlist[i] << "}\\end{minipage}\n";        
        }
        // finish the latex file and close it
        latexfile << "\\end{document}\n";
        latexfile.close();

        // execute latex without creating output at the term
        string shellCommand = "latex " + latexfilename + " > /dev/null";  // don't show the output to stdout
        if (system(shellCommand.c_str()) != 0)
          cout << "\nWARNING: Possible error while executing shell command:\n" << shellCommand << endl;

        // execute dvips to create postscript file
        shellCommand = "dvips " + filename + " 2> /dev/null"; // don't show the output to stderr
        if (system(shellCommand.c_str()) != 0)
          cout << "\nWARNING: Possible error while executing shell command:\n" << shellCommand << endl;

        // delete temporary files
        shellCommand = "rm -f " + filename + ".aux "
                                + filename + ".dvi "
                                + filename + ".tex "
                                + filename + ".log ";
        if (system(shellCommand.c_str()) != 0)
          cout << "\nWARNING: Possible error while executing shell command:\n" << shellCommand << endl;


        cout << "Created postscript summary: " << filename << ".ps" << endl;
      }
    } catch (AipsError x) {
      cerr << "analyseLOPESevent2::summaryPlot: " << x.getMesg() << endl;
    }
  }

 void analyseLOPESevent2::createLateralOutput (const string& filePrefix, 
                                               const Record& erg,
                                               const double& Xc,
                                               const double& Yc)
 {
    try {
      unsigned int GT = erg.asuInt("Date");

      // get AntennaIDs to loop through pulse parameters
      Vector<int> antennaIDs;
      beamformDR_p->headerRecord().get("AntennaIDs",antennaIDs);

      // create filename
      stringstream filenameStream;
      filenameStream << filePrefix << GT << ".dat";
      string filename = filenameStream.str();

      // loop through antenna and verify that there are at least 2 pulse informations
      unsigned int NpulseInformation = 0;
      for (unsigned int i=0 ; i < antennaIDs.size(); ++i)
        if (calibPulses.find(antennaIDs(i)) != calibPulses.end())
          ++NpulseInformation;
      if (NpulseInformation < minBeamformingAntennas) {
        cerr << "analyseLOPESevent2::createLateralOutput: " 
             << "Event does not contain at least "
             << minBeamformingAntennas
             << " good pulses!"
             << endl;
        return;
      }

      // open file
      ofstream lateralfile;
      lateralfile.open(filename.c_str(), ofstream::out);

      cout << "\nWriting output for lateral distribution into file: " << filename << endl;

      // get antenna positions and distances in shower coordinates
      Vector <double> distances = erg.asArrayDouble("distances");
      Matrix <double> antPos = toShower(beamPipe_p->GetAntPositions(), erg.asDouble("Azimuth"), erg.asDouble("Elevation"));

      // loop through antenna and create output for every antenna with existing pulse information
      for (unsigned int i=0 ; i < antennaIDs.size(); ++i)
        if (calibPulses.find(antennaIDs(i)) != calibPulses.end()) {
          lateralfile << i + 1 << "  "
                      << antPos.row(i)(0) << "  "
                      << antPos.row(i)(1) << "  "
                      << distances(i) << "  "
                      << i + 1 << "  "
                      << calibPulses[antennaIDs(i)].envelopeMaximum << "  "
                      << calibPulses[antennaIDs(i)].noise << "  "
                      << calibPulses[antennaIDs(i)].envelopeTime << endl;
        } else {
          // if the antenna was not flagged manually and has the correct polarization
          // then wirte -1 in the output file
          // break if antennaID was flagged
          bool manuallyFlagged = false;
          for (unsigned int j=0 ; j < InputFlaggedAntIDs.size(); ++j)
            if (antennaIDs(i) == InputFlaggedAntIDs(j))
              manuallyFlagged = true;

          if ( !(manuallyFlagged) && PolarizationAntennaSelection(i) ) {
            lateralfile << i + 1 << "  "
                        << "-1" << "  "
                        << "-1" << "  "
                        << "-1" << "  "
                        << i + 1 << "  "
                        << "-1" << "  "
                        << "-1" << "  "
                        << "-1" << endl;
          }
        }

     // final line
     lateralfile << "# " << GT
                 << " "  << erg.asDouble("Azimuth")
                 << " "  << erg.asDouble("Elevation")
                 << " "  << erg.asDouble("Distance")
                 << " "  << Xc
                 << " "  << Yc << endl;

     // close file
     lateralfile.close();
    } catch (AipsError x) {
      cerr << "analyseLOPESevent2::createLateralOutput: " << x.getMesg() << endl;
    }
  }

//---------------------------------------------------------------------------------------------------


 bool analyseLOPESevent2::simpleSkymap (const string& filename,
                                        const int& noBlocks,
                                        const unsigned int& startBlock,
                                        const unsigned int& endBlock,
                                        const double& az_start,
                                        const double& az_stop,
                                        const double& az_d,
                                        const double& ze_start,
                                        const double& ze_stop,
                                        const double& ze_d,
                                        Vector<bool> AntennaSelection,
                                        const double& distance,
                                        const string& beamtype,
                                        const double& hanning)

 {
   bool status=1;
   try {
     cout<<"Generating simple skymap..."<<endl;
     CompleteBeamPipe_p = static_cast<CompletePipeline*>(beamPipe_p);
     CompleteBeamPipe_p->InitEvent(beamformDR_p);
     CompleteBeamPipe_p->setPhaseCenter(0,0);
     int newblocksize = lev_p->blocksize() / noBlocks;
     beamformDR_p->setBlocksize(newblocksize);
     beamformDR_p->setHanningFilter(hanning);
     cout<<"blocksize: "<<beamformDR_p->blocksize()<<endl;
     string fname = filename;
     fname.erase(fname.find_last_of("."), 6);
     fname.append("-skymap");
     beamformDR_p->setBlock(startBlock);

     while( beamformDR_p->block() <= endBlock) {
       cout<<"Processing block "<< beamformDR_p->block()<<" ["<< beamformDR_p->block()-startBlock+1 << "/"<<endBlock-startBlock+1<<"]"<<endl;
       // The following statement does not work any more
       // since TimeFreq.cc::timeValues has been changed to demand a reference instead to return a vector
       // cout<<"startTime: " << beamformDR_p->timeValues()[0]<<endl;
       stringstream str;
       str.clear();
       str<< beamformDR_p->block();
       string buf = str.str();
       if(buf.length()==1) buf="000"+buf;
       if(buf.length()==2) buf="00"+buf;
       if(buf.length()==3) buf="0"+buf;
       string ofname = fname+"-"+buf+".dat";
       string psfname = fname+"-"+buf+".eps";
       ofstream fout(ofname.c_str());

       Vector<Double> power;
       double max=0, min=0, pix;

       // loop over all pixels
       for(float it_ze=ze_start; it_ze<=ze_stop; it_ze+=ze_d) {
         float it_az=az_start;
         while( it_az<=az_stop-az_d) {
           cout<<'\xd';
           cout<<"["<<it_ze<<"]["<<it_az<<"]     ";
           CompleteBeamPipe_p->setDirection(it_az, 90 - it_ze, distance);
           if(beamtype.compare("CC")==0)
             power = CompleteBeamPipe_p->GetCCBeam(beamformDR_p, AntennaSelection, "ANY").copy();
           else
             if (beamtype.compare("P")==0)
               power = CompleteBeamPipe_p->GetPBeam(beamformDR_p, AntennaSelection, "ANY").copy();
             else
               if (beamtype.compare("X")==0)
                 power = CompleteBeamPipe_p->GetXBeam(beamformDR_p, AntennaSelection, "ANY").copy();
               else {
                 cerr<<"Unknown beamtype: "<<beamtype<<endl;
                 return 0 ;
              }
           // choose mean of beam value in block interval (not maximum)
           // maximum would be better if there is a beam
           // mean is usefull for noise, to avoid random coherence
           pix=mean(power);
           fout<<it_az<<"  "<<it_ze<<"   "<<pix<<endl;
           it_az += az_d;
           if(pix>max)
             max=pix;
           if(pix<min)
             min=pix;
         }
       }
       fout.close();
       cout<<'\xd'<<"frame finished...   \nWriting ps-file..."<<endl;
       ifstream fin(ofname.c_str());

       //generate the postscript file...
       ofstream psout(psfname.c_str());
       // writing ps-header...
       psout<<"%!PS-Adobe-2.0 EPSF-2.0\n"
           <<"%%DocumentFonts: Courier-Bold Helvetica-Bold\n"
           <<"%%BoundingBox: -320 -340 360 310\n"
           <<"%%Title: "<< psfname<<endl
           <<"%%EndComments\n"
           <<"gsave\n"
           <<"/PSDict 200 dict def\n"
           <<"PSDict begin\n"
           <<"/a {arc} def\n"
           <<"/s {stroke} def\n"
           <<"/l {lineto} def\n"
           <<"/m {moveto} def\n"
           <<"/n {newpath} def\n"
           <<"/rgb {setrgbcolor} def\n"
           <<"/c {closepath} def\n"
           <<"/f {fill} def\n"
           <<"/gs {gsave} def\n"
           <<"/gr {grestore} def\n"
           <<"/cf {closepath fill} def\n"
           <<"/lw {setlinewidth} def\n"
           <<"end\n"
           <<"PSDict begin\n"
           <<"3 3 scale\n"
           <<"0.1 lw \n\n";

      double r=1,g=1,b=1;
      float az, ze, ax, ay, x1, y1, x2, y2, x3, y3, x4, y4;

      while(fin>>az>>ze>>pix) {
        psout.precision(3);
        if(pix >=0) {
            if(pix <= 0.1 && pix >= 1.e-8) {
                r=1.-(8.+log10(pix))/7.;
                g=1.-(8.+log10(pix))/7.;
                b=1.-0.2*(8.+log10(pix))/7.;
            } else {
              if(pix> 0.1) {
                r=0;
                g=0;
                b=0.8;
              }
              if(pix< 1.e-8) {
                r=1;
                g=1;
                b=1;
              }
            }
        } else {
            if(pix >= -0.1 && pix <= -1.e-8) {
                r=1.-0.2*(8.+log10(-1.*pix))/7.;
                g=1.-(8.+log10(-1.*pix))/7.;
                b=1.-(8.+log10(-1.*pix))/7.;
            } else {
              if(pix< -0.1) {
                r=0.8;
                g=0;
                b=0.;
              }
              if(pix> -1.e-8) {
                r=1;
                g=1;
                b=1;
              }
            }
        }


           psout<<r<<" "<<g<<" "<<b<<" rgb ";
           ax=-1.*ze*sin(az/180.*PI);
           ay= ze*cos(az/180.*PI);
           x1= ax+0.5*ze_d*sin(az/180.* PI) - (ze-ze_d/2.)*sin(az_d/2./180.* PI)*cos(az/180.* PI);
           y1= ay-0.5*ze_d*cos(az/180.* PI) - (ze-ze_d/2.)*sin(az_d/2./180.* PI)*sin(az/180.* PI);
           x2= ax+0.5*ze_d*sin(az/180.* PI) + (ze-ze_d/2.)*sin(az_d/2./180.* PI)*cos(az/180.* PI);
           y2= ay-0.5*ze_d*cos(az/180.* PI) + (ze-ze_d/2.)*sin(az_d/2./180.* PI)*sin(az/180.* PI);
           x3= ax-0.5*ze_d*sin(az/180.* PI) + (ze+ze_d/2.)*sin(az_d/2./180.* PI)*cos(az/180.* PI);
           y3= ay+0.5*ze_d*cos(az/180.* PI) + (ze+ze_d/2.)*sin(az_d/2./180.* PI)*sin(az/180.* PI);
           x4= ax-0.5*ze_d*sin(az/180.* PI) - (ze+ze_d/2.)*sin(az_d/2./180.* PI)*cos(az/180.* PI);
           y4= ay+0.5*ze_d*cos(az/180.* PI) - (ze+ze_d/2.)*sin(az_d/2./180.* PI)*sin(az/180.* PI);
           psout<<"n "<< x1<<" "<<y1<<" m "<< x2<<" "<<y2<<" l "<<x3<< " "<<y3<< " l "<< x4<<" "<<y4<<" l c gs f gr s\n";
      }

      // writing footer...
     psout<<"0.000 0.000 0.800 rgb n 80 -55 m 85 -55 l 85 -50 l 80 -50 l 80 -55 l c gs f gr s\n"
          <<"0.250 0.250 0.850 rgb n 80 -60 m 85 -60 l 85 -55 l 80 -55 l 80 -60 l c gs f gr s\n"
          <<"0.500 0.500 0.900 rgb n 80 -65 m 85 -65 l 85 -60 l 80 -60 l 80 -65 l c gs f gr s\n"
          <<"0.750 0.750 0.950 rgb n 80 -70 m 85 -70 l 85 -65 l 80 -65 l 80 -70 l c gs f gr s\n"
          <<"1.000 1.000 1.000 rgb n 80 -75 m 85 -75 l 85 -70 l 80 -70 l 80 -75 l c gs f gr s\n"
          <<"0.950 0.750 0.750 rgb n 80 -80 m 85 -80 l 85 -75 l 80 -75 l 80 -80 l c gs f gr s\n"
          <<"0.900 0.500 0.500 rgb n 80 -85 m 85 -85 l 85 -80 l 80 -80 l 80 -85 l c gs f gr s\n"
          <<"0.850 0.250 0.250 rgb n 80 -90 m 85 -90 l 85 -85 l 80 -85 l 80 -90 l c gs f gr s\n"
          <<"0.800 0.000 0.000 rgb n 80 -95 m 85 -95 l 85 -90 l 80 -90 l 80 -95 l c gs f gr s\n";






      psout.precision(2);
      psout<<"0.0 0.0 0.0 rgb n 80 -50 m 85 -50 l 85 -95 l 80 -95 l 80 -50 l c s\n"
           <<"0.0 0.0 0.0 rgb /Helvetica 5 selectfont\n"
           <<"n 87 -47 m (CC-Beam: ) show\n"
           <<"n 87 -55 m (1.e-1) show c n 87 -60 m (1.e-3) show c n 87 -65 m (1.e-5) show c\n"
           <<"n 87 -70 m (1.e-7) show c n 87 -75 m (0.0) show c n 87 -80 m (-1.e-7) show c\n"
           <<"n 87 -85 m (-1.e-5) show c n 87 -90 m (-1.e-3) show c n 87 -95 m (-1.e-1) show c\n";


      psout<<"n -97 -110 m\n"
           <<"0.0 0.0 0.0 rgb\n"
           <<"/Helvetica 6 selectfont\n"
           <<"("<<filename<<" - frame "<<buf<<"/"<< noBlocks<<") show c\n"
           <<"n -2 92 m (N) show c\n"
           <<"n -98 -3 m (E) show c\n"
           <<"n -2 -98 m (S) show c\n"
           <<"n 93 -3 m (W) show c\n"
           <<"0.3 0.3 0.3 rgb 0.1 lw\n"
           <<"n 0 0 "<< 90+0.5*ze_d <<" 0 360 a s\n"
           <<"0 0 60 0 360 a s\n"
           <<"0 0 30 0 360 a s\n"
           <<-90-0.5*ze_d<<" 0 m  "<<90+0.5*ze_d<<" 0 l s\n"
           <<"0 "<<-90-0.5*ze_d<<" m 0 "<< 90+0.5*ze_d<<" l s\n"
           <<"grestore\n"
           <<"end\n";
       psout.close();
       cout<<"finished writing of ps file...\n";


       beamformDR_p->nextBlock();
       CompleteBeamPipe_p->resetCachedFlag();
       }

   } catch (AipsError x) {
     cerr << "analyseLOPESevent2::simpleSkymap: " << x.getMesg() << endl;
     status=0;
     }
   return status;
 }


  void analyseLOPESevent2::calculateSign (map <int,PulseProperties> & eventPulses,
                                          double& ratioDiffSign,
                                          double& ratioDiffSignEnv,
                                          double& weightedTotSign,
                                          double& weightedTotSignEnv)

  {
    try {
      int sign=0, signAtEnvTime=0;
      double antP=0., antN=0.; 
      double antPEnv=0., antNEnv=0.; 
      double envMax=0.,noise=0.,snr=0.,nume=0.,numeEnv=0.,denom=0.;
      double total_minMaxSign=0., total_envSign=0.;


      for(map<int,PulseProperties>::iterator iter=eventPulses.begin(); iter !=eventPulses.end(); ++iter) { 
        sign=iter->second.minMaxSign;
        signAtEnvTime=iter->second.envSign;
        envMax=iter->second.envelopeMaximum;
        noise=iter->second.noise;

        snr=(envMax/noise);
        nume += (sign*snr);
        numeEnv += (signAtEnvTime*snr);
        denom += snr;

        if(sign>0)
          antP++;
        if(sign<0)
          antN++;
        if(signAtEnvTime>0)
          antPEnv++;
        if(signAtEnvTime<0)
          antNEnv++;
      }//for map

      total_minMaxSign = (antP-antN);
      total_envSign = (antPEnv-antNEnv);
      // cout<<"Positive:  "<<antP<<"   Negative:  "<<antN<<"   Tot  :"<<(antP+antN)<<endl;
      // cout<<"@ envelope Positive: "<<antPEnv<<" Negative:  "<<antNEnv<<" Tot  :"<<(antPEnv+antNEnv)<<endl;

      ratioDiffSign=(antP-antN)/(antP+antN);
      ratioDiffSignEnv=(antPEnv-antNEnv)/(antPEnv+antNEnv);

      weightedTotSign= nume/denom;
      weightedTotSignEnv= numeEnv/denom;
    } catch (AipsError x) {
      cerr << "analyseLOPESevent2::calculateSign: " << x.getMesg() << endl;
    }
  }


  void analyseLOPESevent2::caculateNoiseInfluence (bool correctInfluence)
  {
    try {
      if (!correctInfluence) {
        cout << "\nCorrecting amplitudes for noise influence is turned OFF!\n" << endl;
        for(map<int,PulseProperties>::iterator it=calibPulses.begin(); it !=calibPulses.end(); ++it) { 
          it->second.height = it->second.envelopeMaximum;          
          it->second.heightError = it->second.noise;
          it->second.time = it->second.envelopeTime;
          it->second.timeError = 2;
        }
      }
    
      cout << "\nCorrecting amplitudes for noise influence, and calculating errorbars.\n" << endl;
      
      // Apply correction formulas from noise study (see PhD thesis of Frank Schröder, in preperation)
      for(map<int,PulseProperties>::iterator it=calibPulses.begin(); it !=calibPulses.end(); ++it) { 
        // calculate SNR, as all formulas depend on signal-to-noise ratio
        double noise = it->second.noise;
        double snr = it->second.envelopeMaximum / noise;
        
        // amplitude correction (all formulas are normalized to height of noise)
        if (snr < 2)       
          it->second.height = (0.4628 + 0.2491*pow(snr,2.349)) * noise;
        else
          it->second.height = sqrt(snr*snr - 1) * noise;
          
        // amplitude error bar
        if (snr < 1.67634)
          it->second.heightError = (0.3103 + 0.0647*exp(+snr)) * noise;
        else  
          it->second.heightError = (0.6162 + 0.213*exp(-snr)) * noise;
          
        // add calibration uncertainty of 5% in square
        it->second.heightError = sqrt( pow(it->second.heightError,2) + pow(0.05 * it->second.height, 2));
          
        // time
        it->second.time = it->second.envelopeTime;
        // time error bar
        if (snr < 1.83889)
          it->second.timeError = 16.6 - pow (2*snr - 1.3, 2);
        else  
          it->second.timeError = 20.5 * pow (snr, -1.03);
          
        // add calibration uncertainty of 2 ns in square
        it->second.timeError = sqrt( pow(it->second.timeError,2) + 4);
        
        bool debug = false;
        if (debug) {
          cout << "For antenna " << it->second.antenna << ":\n";
          cout << "Env. height = " << it->second.envelopeMaximum 
               << " +/- " << noise
               << "\t time = " << it->second.envelopeTime
               << " +/- " << 2            
               << endl;
          cout << "Cor. height = " << it->second.height
               << " +/- " << it->second.heightError
               << "\t time = " << it->second.envelopeTime
               << " +/- " << it->second.timeError
               << endl;
        }       
      }  
    } catch (AipsError x) {
      cerr << "analyseLOPESevent2::caculateNoiseInfluence: " << x.getMesg() << endl;
    }
  }



} // Namespace CR -- end


#endif
