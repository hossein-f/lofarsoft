/*-------------------------------------------------------------------------*
 | $Id::                                                                 $ |
 *-------------------------------------------------------------------------*
 ***************************************************************************
 *   Copyright (C) 2008                                                  *
 *   Frank Schroeder (<mail>)                                                     *
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

namespace CR { // Namespace CR -- begin
  
  // ============================================================================
  //
  //  Construction
  //
  // ============================================================================
  
  analyseLOPESevent2::analyseLOPESevent2 ():
    upsamplingExponent(0)
  {;}
  
  // ============================================================================
  //
  //  Destruction
  //
  // ============================================================================
  
  analyseLOPESevent2::~analyseLOPESevent2 ()
  {
    // Set pipeline_p back to NULL (to avoid that analyseLOPESevent deletes pipeline)
    pipeline_p = NULL;
  }

  // ============================================================================
  //
  //  Parameters
  //
  // ============================================================================
  
  void analyseLOPESevent2::summary (std::ostream &os)
  {;}
  
 
  // ============================================================================
  //
  //  Methods
  //
  // ============================================================================
  
  
  Bool analyseLOPESevent2::initPipeline(Record ObsRecord){
    try {
      clear();
      // To use the methods of analyseLOPESevent, a pointer to the pipeline is needed.
      pipeline_p = &pipeline;
      lev_p = new LopesEventIn();
      pipeline.SetObsRecord(ObsRecord);
      cout << "\nPipeline initialised." << endl;
      
    } catch (AipsError x) {
      cerr << "analyseLOPESevent2::initPipeline: " << x.getMesg() << endl;
      return False;
    }; 
    return True;
  }; 

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
					  Vector<Int> FlaggedAntIDs, 
					  Bool verbose,
					  Bool simplexFit,
					  Double ExtraDelay,
					  int doTVcal,
					  bool SinglePlots,
					  bool PlotRawData,
  					  bool CalculateMaxima,
					  bool listCalcMaxima,
					  bool printShowerCoordinates){
    Record erg;
    try {
      //      ofstream latexfile;	// WARNING: causes problem in fitCR2gauss.cc line 200, left here for future tests
      Int nsamples;
      Vector<Double> Times, ccBeam, xBeam, pBeam, tmpvec;
      Matrix<Double> TimeSeries;
      Record fiterg;
      
      pipeline.setVerbosity(verbose);
      // Generate the Data Reader
      if (! lev_p->attachFile(evname) ){
	cerr << "analyseLOPESevent2::ProcessEvent: " << "Failed to attach file: " << evname << endl;
	return Record();
      };
      // initialize the Data Reader
      if (! pipeline.InitEvent(lev_p)){
	cerr << "analyseLOPESevent2::ProcessEvent: " << "Failed to initialize the DataReader!" << endl;
	return Record();
      };
         
      //  Enable/disable doing the phase calibration as requested
      switch (doTVcal){
      case 0:
	pipeline.doPhaseCal(False);
	break;
      case 1:
	pipeline.doPhaseCal(True);
	break;
      default:
	if ( lev_p->header().isDefined("EventClass") &&
	     lev_p->header().asInt("EventClass") == LopesEventIn::Simulation ){
	  if (verbose){
	    cout << "Simulation event: ";
	  };
	  pipeline.doPhaseCal(False);
	} else {
	  pipeline.doPhaseCal(True);
	};
	break;
      };
      
      
      // Generate the antenna selection
      Vector <Bool> AntennaSelection;
      Int i,j,id,nants,nselants, nflagged=FlaggedAntIDs.nelements();
      AntennaSelection = pipeline.GetAntennaMask(lev_p);
      nants = AntennaSelection.nelements();
      Vector<Int> AntennaIDs,selAntIDs;
      lev_p->header().get("AntennaIDs",AntennaIDs);
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
	    cout << "analyseLOPESevent2::ProcessEvent: " << "AntennaID: " << id 
		 << " not found -> no antenna flagged." << endl;
	  };
	};
      };

      
      // Plot the raw data, if desired
      if (PlotRawData)       
      {
        // Set Plot Interval
        pipeline.setPlotInterval(plotStart(),plotStop());
        
	// plot upsampled raw data: either in seperated plots seperated or all traces in one plot
        if (SinglePlots)
          pipeline.plotAllAntennas(PlotPrefix+ "-raw", lev_p, AntennaSelection, true, getUpsamplingExponent(),true);
        else
          pipeline.plotAllAntennas(PlotPrefix + "-raw", lev_p, AntennaSelection, false, getUpsamplingExponent(), true);
          
	// calculate the maxima
        if (CalculateMaxima) pipeline.calculateMaxima(lev_p, AntennaSelection, getUpsamplingExponent(), true);
      }

      // if there are no plots and now fits requested, stop here
      if (!generatePlots && !simplexFit)
      {
        if (verbose)		// give out the names of the created plots
        {
          vector<string> plotlist = pipeline.getPlotList();
          std::cout <<"\nList of generated plots:\n";
          for (int i = 0; i < plotlist.size(); i++) std::cout << plotlist[i] << "\n";
          std::cout << std::endl;
        }
        return Record();
      }

      //initialize the pipeline
      Times = lev_p->timeValues();
      nsamples = Times.nelements();
      if (! pipeline.setPhaseCenter(XC, YC, RotatePos)){
	cerr << "analyseLOPESevent2::ProcessEvent: " << "Error during setPhaseCenter()!" << endl;
	return Record();
      };

      //Flagg antennas
      flagger.calcWeights(pipeline.GetTimeSeries(lev_p));
      AntennaSelection = AntennaSelection && flagger.parameters().asArrayBool("AntennaMask");
      nselants = ntrue(AntennaSelection);


      //initialize the fitter
      Vector<uInt> remoteRange(2,0);
      remoteRange(0) = (uInt)(nsamples*remoteStart_p);
      remoteRange(1) = (uInt)(nsamples*remoteStop_p);
      fitObject.setTimeAxis(Times);
      fitObject.setRemoteRange(remoteRange);
      fitObject.setFitRangeSeconds(fitStart_p,fitStop_p);

      //perform the position fitting
      Double center=-1.8e-6;
      if (simplexFit) {
	if (! evaluateGrid(Az, El, distance, AntennaSelection, &center) ){
	  cerr << "analyseLOPESevent2::ProcessEvent: " << "Error during evaluateGrid()!" << endl;
	  return Record();
	};
	if (verbose) { cout << "analyseLOPESevent2::ProcessEvent: starting SimplexFit()." << endl;};
	//if (! SimplexFit2(Az, El, distance, center, AntennaSelection) ){
	//  cerr << "analyseLOPESevent::ProcessEvent: " << "Error during SimplexFit2()!" << endl;
	//  return Record();
	//};
	if (! SimplexFit(Az, El, distance, center, AntennaSelection) ){
	  cerr << "analyseLOPESevent2::ProcessEvent: " << "Error during SimplexFit()!" << endl;
	  return Record();
	};
	pipeline.setVerbosity(verbose);
      };
      
      // Get the beam-formed data
      if (! pipeline.setDirection(Az, El, distance)){
	cerr << "analyseLOPESevent2::ProcessEvent: " << "Error during setDirection()!" << endl;
	return Record();
      };
      if (! pipeline.GetTCXP(lev_p, TimeSeries, ccBeam, xBeam, pBeam, AntennaSelection)){
	cerr << "analyseLOPESevent2::ProcessEvent: " << "Error during GetTCXP()!" << endl;
	return Record();
      };
      
      // smooth the data
      StatisticsFilter<Double> mf(3,FilterType::MEAN);
      ccBeam = mf.filter(ccBeam);
      xBeam = mf.filter(xBeam);
      pBeam = mf.filter(pBeam);

      // Calculate noise in the beamforming direction and store it for later subtraction
      Slice remoteRegion(remoteRange(0),(remoteRange(1)-remoteRange(0)));
      double ccBeamMean = mean(ccBeam(remoteRegion));
      double xBeamMean = mean(xBeam(remoteRegion));
      double pBeamMean = mean(pBeam(remoteRegion));
      ccBeam -= ccBeamMean;
      xBeam -= xBeamMean;
      pBeam -= pBeamMean;

      
      // do the fitting
      fiterg = fitObject.Fitgauss(xBeam, ccBeam, True, center);
      if ( !fiterg.isDefined("Xconverged") || !fiterg.isDefined("CCconverged") ){
	cerr << "analyseLOPESevent2::ProcessEvent: " << "Error during fit!" << endl;
	return Record();	
      };

      if (verbose) {
	cout << "Event \"" << evname << "\" fit results:" << endl;
	if (fiterg.asBool("Xconverged")){
	  cout << " Fit to X-Beam has converged:" << endl;
	} else {
	  cout << " Fit to X-Beam has _not_ converged:" << endl;
	};
	cout << "  XHeight: " << fiterg.asDouble("Xheight") << " +/- " 
	     <<fiterg.asDouble("Xheight_error") << endl;
	cout << "  XWidth : " << fiterg.asDouble("Xwidth") << " +/- "
	     <<fiterg.asDouble("Xwidth_error") << endl;
	cout << "  XCenter: " << fiterg.asDouble("Xcenter") << " +/- "
	     <<fiterg.asDouble("Xcenter_error") << endl;
	if (fiterg.asBool("CCconverged")){
	  cout << " Fit to CC-Beam has converged:" << endl;
	} else {
	  cout << " Fit to CC-Beam has _not_ converged:" << endl;
	};
	cout << "  CCHeight: " << fiterg.asDouble("CCheight") << " +/- " 
	     <<fiterg.asDouble("CCheight_error") << endl;
	cout << "  CCWidth : " << fiterg.asDouble("CCwidth") << " +/- "
	     <<fiterg.asDouble("CCwidth_error") << endl;
	cout << "  CCCenter: " << fiterg.asDouble("CCcenter") << " +/- "
	     <<fiterg.asDouble("CCcenter_error") << endl;
      };
      
      //copy fit results to record
      erg.mergeField(fiterg,"Xconverged", RecordInterface::OverwriteDuplicates);
      erg.mergeField(fiterg,"Xheight",RecordInterface::OverwriteDuplicates);
      erg.mergeField(fiterg,"Xwidth",RecordInterface::OverwriteDuplicates);
      erg.mergeField(fiterg,"Xcenter",RecordInterface::OverwriteDuplicates);
      erg.mergeField(fiterg,"Xheight_error",RecordInterface::OverwriteDuplicates);
      erg.mergeField(fiterg,"Xwidth_error",RecordInterface::OverwriteDuplicates);
      erg.mergeField(fiterg,"Xcenter_error",RecordInterface::OverwriteDuplicates);
      erg.mergeField(fiterg,"CCconverged",RecordInterface::OverwriteDuplicates);
      erg.mergeField(fiterg,"CCheight",RecordInterface::OverwriteDuplicates);
      erg.mergeField(fiterg,"CCwidth",RecordInterface::OverwriteDuplicates);
      erg.mergeField(fiterg,"CCcenter",RecordInterface::OverwriteDuplicates);
      erg.mergeField(fiterg,"CCheight_error",RecordInterface::OverwriteDuplicates);
      erg.mergeField(fiterg,"CCwidth_error",RecordInterface::OverwriteDuplicates);
      erg.mergeField(fiterg,"CCcenter_error",RecordInterface::OverwriteDuplicates);

      // calculate other stuff
      erg.define("rmsC",stddev(ccBeam(remoteRegion)));
      erg.define("rmsX",stddev(xBeam(remoteRegion)));
      Matrix<Double> AntPos; 
      AntPos=pipeline.GetAntPositions();
      AntPos = toShower(AntPos, Az, El);
      Vector<Double> distances(nants);

      // print distances in shower coordinates if requested
      if (printShowerCoordinates)
      for (i=0; i<nants; i++) {
	distances(i) = sqrt( square(AntPos.row(i)(0)) + square(AntPos.row(i)(1)) );
        // output of shower coordinates
	std::cout << i+1 << " " << AntPos.row(i)(0) << " " << AntPos.row(i)(1) << " " <<distances(i)<<std::endl;
      };
      erg.define("distances",distances);
      tmpvec.resize(nselants);
      tmpvec = distances(AntennaSelection).getCompressedArray() ;
      erg.define("meandist",mean(tmpvec));
      erg.define("Azimuth",Az);
      erg.define("Elevation",El);
      erg.define("Distance",distance);

      // Generate the plots
      if (generatePlots)
      {
        // Set Plot Interval
        pipeline.setPlotInterval(plotStart(),plotStop());

        // Plot CC-beam; if fit has converged, then also plot the result of the fit
    	if (fiterg.asBool("CCconverged"))
          pipeline.plotCCbeam(PlotPrefix + "-CC", lev_p, fiterg.asArrayDouble("Cgaussian"), AntennaSelection, ccBeamMean, pBeamMean);
        else
          pipeline.plotCCbeam(PlotPrefix + "-CC", lev_p, Vector<Double>(), AntennaSelection, ccBeamMean, pBeamMean);

        // Plot X-beam; if fit has converged, then also plot the result of the fit
    	if (fiterg.asBool("Xconverged"))
          pipeline.plotXbeam(PlotPrefix + "-X", lev_p, fiterg.asArrayDouble("Xgaussian"), AntennaSelection, xBeamMean, pBeamMean);
        else
          pipeline.plotXbeam(PlotPrefix + "-X", lev_p, Vector<Double>(), AntennaSelection, xBeamMean, pBeamMean);

        // Plot of all antenna traces together
        pipeline.plotAllAntennas(PlotPrefix + "-all", lev_p, AntennaSelection, false, getUpsamplingExponent(),false);
        
	// Plot of upsampled antenna traces (seperated) 
        if (SinglePlots)
          pipeline.plotAllAntennas(PlotPrefix, lev_p, AntennaSelection, true, getUpsamplingExponent(),false);

        // calculate the maxima
	if (CalculateMaxima) pipeline.calculateMaxima(lev_p, AntennaSelection, getUpsamplingExponent(), false);
        // user friendly list of calculated maxima
        if (listCalcMaxima) pipeline.listCalcMaxima(lev_p, AntennaSelection, getUpsamplingExponent(),pBeam, false);
        
        if (verbose)		// give out the names of the created plots
        {
          vector<string> plotlist = pipeline.getPlotList();
          std::cout <<"\nList of generated plots:\n";
          for (int i = 0; i < plotlist.size(); i++) std::cout << plotlist[i] << "\n";
          std::cout << std::endl;
        }
      };
    } catch (AipsError x) 
    {
      std::cerr << "analyseLOPESevent2::ProcessEvent: " << x.getMesg() << std::endl;
      return Record();
    } 
    return erg;
  }

  void analyseLOPESevent2::summaryPlot(string filename,
                                       unsigned int columns)
  {
    try 
    {
      // check if anything should be done (means columns must be > 0)
      if (columns <= 0) return;

      // create latexfilename
      string latexfilename = filename + ".tex";

      // open latexfile
      ofstream latexfile;
      latexfile.open(latexfilename.c_str(), ofstream::out);

      // check if file could be opened
      if (!(latexfile.is_open()))
      {
        std::cerr << "Failed to write to file \"" << latexfilename <<"\"." << std::endl;
      } else 
      {
        // Create header for latex file
        latexfile << "\\documentclass[a4paper]{article}\n";
        latexfile << "\\usepackage{graphicx,a4wide}\n";
        latexfile << "\\setlength{\\textheight}{24cm}\n";
        latexfile << "\\setlength{\\topmargin}{0.0cm}\n";
        latexfile << "\\parindent=0pt\n";
        latexfile << "\\begin{document}\n";

        // get list of generated plots and loop through the list
        vector<string> plotlist = pipeline.getPlotList();

        for (int i = 0; i < plotlist.size(); i++)
        {
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
        string shellCommand = "latex " + latexfilename +" > /dev/null";  // don't show the output to stdout
        system(shellCommand.c_str());

	// execute dvips to create postscript file
        shellCommand = "dvips " + filename + " 2> /dev/null"; // don't show the output to stderr
        system(shellCommand.c_str());

        std::cout << "Created postscript summary: " << filename << ".ps" << std::endl;
      }
    } catch (AipsError x) {
      cerr << "analyseLOPESevent2::summaryPlot: " << x.getMesg() << endl;
    }
  }

} // Namespace CR -- end
