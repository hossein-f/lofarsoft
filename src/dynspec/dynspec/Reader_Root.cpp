#include <iostream>
#include <string>

#include <sstream>

#include "Reader_Root.h"
#include "Stock_Write_Root_Metadata.h"

#include <dal/lofar/BF_File.h>


using namespace DAL;


using namespace std;


  Reader_Root::Reader_Root(){}	// constructor 
  Reader_Root::~Reader_Root(){}	// destructor
  
  
    
  void Reader_Root::readRoot(string pathFile,string obsName,Stock_Write_Root_Metadata *rootMetadata,int obsNofSource,int obsNofBeam,int obsNofFrequencyBand,int obsNofTiledDynspec)//,int obsNofSource,int obsNofBeam,int obsNofStockes,int obsNofFrequencyBand,int obsNofTiledDynspec,vector<double> obsMinFrequency,
			    //vector<double> obsMaxFrequency,vector<double> obsCentralFrequency)
    { 
   
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Programm  Load the hdf5's root part

      BF_File file(pathFile);  							// generate an object called file contains Root
  

      BF_SubArrayPointing SAP 		= file.subArrayPointing(0);		// generate an object called SAP contains subarray pointings

      BF_BeamGroup BEAM			= SAP.beam(0);      			// generate an object called BEAM contains Beams

      
      CoordinatesGroup COORDS		= BEAM.coordinates();      		// generate an object called COORDS contains Coodinates directory
      Coordinate *TIME_COORDS		= COORDS.coordinate(0);			// generate an object called TIME_COORDS contains time data
      Coordinate *SPECTRAL_COORDS	= COORDS.coordinate(1);			// generate an object called SPECTRAL_COORDS contains spectral data

      NumericalCoordinate *NUM_COORD_TIME = dynamic_cast<NumericalCoordinate*>(TIME_COORDS); // load Numerical coordinates lib 
      DirectionCoordinate *DIR_COORD_TIME = dynamic_cast<DirectionCoordinate*>(TIME_COORDS); // load Direction coordinates lib
      
      NumericalCoordinate *NUM_COORD_SPECTRAL = dynamic_cast<NumericalCoordinate*>(SPECTRAL_COORDS); // load Numerical coordinates lib 
      DirectionCoordinate *DIR_COORD_SPECTRAL = dynamic_cast<DirectionCoordinate*>(SPECTRAL_COORDS); // load Direction coordinates lib

      BF_StokesDataset STOCKES 		= BEAM.stokes(0);			// generate an object called STOCKES contains stockes data
      

      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // ROOT
      ///////////////////////////////////////////////////////////      
      //Variables Initialization && General Root group variables 

      string GROUPETYPE("Root");
      string FILENAME(obsName+".h5");     
      string FILEDATE(file.fileDate().get());
      
      string FILETYPE("dynspec");
      string TELESCOPE("LOFAR");
      string OBSERVER(file.observer().get());
      string PROJECT_ID(file.projectID().get());
      string PROJECT_TITLE(file.projectTitle().get());
      string PROJECT_PI(file.projectPI().get());
      string PROJECT_CO_I(file.projectCOI().get());   
      string PROJECT_CONTACT(file.projectContact().get());
      
      string OBSERVATION_ID(file.observationID().get());
      string OBSERVATION_START_UTC(file.observationStartUTC().get());
      double OBSERVATION_START_MJD(file.observationStartMJD().get());
      string OBSERVATION_START_TAI(file.observationStartTAI().get());
      string OBSERVATION_END_UTC(file.observationEndUTC().get());
      double OBSERVATION_END_MJD(file.observationEndMJD().get());
      string OBSERVATION_END_TAI(file.observationEndTAI().get());
      int    OBSERVATION_NOF_STATIONS(file.observationNofStations().get());
    
      vector<string>  OBSERVATION_STATIONS_LIST(file.observationStationsList().get());
      
      double OBSERVATION_FREQUENCY_MAX(file.observationFrequencyMax().get());
      double OBSERVATION_FREQUENCY_MIN(file.observationFrequencyMin().get());
      double OBSERVATION_FREQUENCY_CENTER((file.observationFrequencyCenter().get())/1E6);	// conversion to Mhz
      
      string OBSERVATION_FREQUENCY_UNIT("Mhz");      
      int    OBSERVATION_NOF_BITS_PER_SAMPLE(file.observationNofBitsPerSample().get());    
      
      
      double CLOCK_FREQUENCY(file.clockFrequency().get());
      string CLOCK_FREQUENCY_UNIT("Mhz");
      string ANTENNA_SET(file.antennaSet().get());
      
      string FILTER_SELECTION(file.filterSelection().get());
      
      vector<string> TARGET(obsNofSource);
      int i(0);
      for (i=0;i<obsNofSource;i++){std::ostringstream oss;oss << i;string index(oss.str());TARGET[i]="SRC"+index;}
      
      string SYSTEM_VERSION("DAL v2.0");
      string PIPELINE_NAME("DYNAMIC SPECTRUM");
      string PIPELINE_VERSION("Dynspec:v-1.2");
      string ICD_NUMBER("ICD6");
      string ICD_VERSION("ICD6:v-2.03.09");
      string NOTES("19/06/2012");
      
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //Variables Initialization && Additionnal Root group variables      
      
      bool DYNSPEC_GROUP(true);
      int NOF_DYNSPEC(obsNofSource*obsNofBeam*obsNofFrequencyBand);
      int NOF_TILED_DYNSPEC(obsNofTiledDynspec);
      string CREATE_OFFLINE_ONLINE("Offline");
      
      string BF_FORMAT("TAB");
      string BF_VERSION("ICD3:v-2.04.27");
      
      double PRIMARY_POINTING_DIAMETER(0);
      double POINT_RA(SAP.pointRA().get());
      double POINT_DEC(SAP.pointDEC().get());
      
      vector<string> POINT_ALTITUDE(OBSERVATION_STATIONS_LIST.size(),"none");
      vector<string> POINT_AZIMUTH(OBSERVATION_STATIONS_LIST.size(),"none");
      
      double CLOCK_RATE(CLOCK_FREQUENCY);
      string CLOCK_RATE_UNIT(CLOCK_FREQUENCY_UNIT);
      
      int NOF_SAMPLES(STOCKES.nofSamples().get()); 			//Time samples
      
      //double SAMPLING_RATE((SAP.samplingRate().get())/1E6);
      double SAMPLING_RATE(STOCKES.nofSamples().get()/file.totalIntegrationTime().get()/1E6);	
      string SAMPLING_RATE_UNIT("Mhz");
      
      double SAMPLING_TIME(1/SAMPLING_RATE);
      string SAMPLING_TIME_UNIT("μs");
      
      double TOTAL_INTEGRATION_TIME(file.totalIntegrationTime().get());
      string TOTAL_INTEGRATION_TIME_UNIT("s");
      
      
      //vector<unsigned> suBandsChannels(STOCKES.nofChannels().get());
      //int CHANNELS_PER_SUBANDS(suBandsChannels[0]);
      
      int CHANNELS_PER_SUBANDS(OBSERVATION_NOF_BITS_PER_SAMPLE);
      
      double SUBBAND_WIDTH((BEAM.subbandWidth().get())/1E6);
      string SUBBAND_WIDTH_UNIT("Mhz");     
      
      double CHANNEL_WIDTH(SUBBAND_WIDTH/CHANNELS_PER_SUBANDS); 
      string CHANNEL_WIDTH_UNIT("Mhz");
      
      double TOTAL_BAND_WIDTH(OBSERVATION_FREQUENCY_MAX-OBSERVATION_FREQUENCY_MIN);
      
      
      vector<string>  WHEATER_STATIONS_LIST(OBSERVATION_STATIONS_LIST.size(),"none");
      vector<double>  WHEATER_TEMPERATURE(OBSERVATION_STATIONS_LIST.size(),0);
      vector<double>  WHEATER_HUMIDITY(OBSERVATION_STATIONS_LIST.size(),0);
      vector<double>  SYSTEM_TEMPERATURE(OBSERVATION_STATIONS_LIST.size(),0);


      
      ///////////////////////////////////////////////////////////////////////////////////////       
				  // END ROOT DATA  //
      //////////////////////////////////////////////////////////////////////////////////////       
      
      
    
      //Object generation => STOCK ROOT METADATA
  
      
      rootMetadata->stockRootMetadata(GROUPETYPE,FILENAME,FILEDATE,FILETYPE,TELESCOPE,OBSERVER,PROJECT_ID,PROJECT_TITLE,PROJECT_PI,PROJECT_CO_I,   // attr10
			       PROJECT_CONTACT,OBSERVATION_ID,OBSERVATION_START_UTC,OBSERVATION_START_MJD,OBSERVATION_START_TAI,OBSERVATION_END_UTC,OBSERVATION_END_MJD,OBSERVATION_END_TAI,OBSERVATION_NOF_STATIONS,  //attr19
			       OBSERVATION_STATIONS_LIST,OBSERVATION_FREQUENCY_MAX,OBSERVATION_FREQUENCY_MIN,OBSERVATION_FREQUENCY_CENTER,OBSERVATION_FREQUENCY_UNIT,OBSERVATION_NOF_BITS_PER_SAMPLE,CLOCK_FREQUENCY,CLOCK_FREQUENCY_UNIT,
			       ANTENNA_SET,FILTER_SELECTION,TARGET,SYSTEM_VERSION,PIPELINE_NAME,PIPELINE_VERSION,ICD_NUMBER,ICD_VERSION,NOTES,
			       DYNSPEC_GROUP,NOF_DYNSPEC,NOF_TILED_DYNSPEC,CREATE_OFFLINE_ONLINE,BF_FORMAT,BF_VERSION,PRIMARY_POINTING_DIAMETER,POINT_RA,POINT_DEC,
			       POINT_ALTITUDE,POINT_AZIMUTH,CLOCK_RATE,CLOCK_RATE_UNIT,NOF_SAMPLES,SAMPLING_RATE,SAMPLING_RATE_UNIT,SAMPLING_TIME,SAMPLING_TIME_UNIT,TOTAL_INTEGRATION_TIME,TOTAL_INTEGRATION_TIME_UNIT,
			       CHANNELS_PER_SUBANDS,SUBBAND_WIDTH,SUBBAND_WIDTH_UNIT,CHANNEL_WIDTH,CHANNEL_WIDTH_UNIT,TOTAL_BAND_WIDTH,WHEATER_STATIONS_LIST,WHEATER_TEMPERATURE,
			       WHEATER_HUMIDITY,SYSTEM_TEMPERATURE);
   
      

      // Memory desallocation

      delete NUM_COORD_TIME;
      delete DIR_COORD_TIME;
      delete NUM_COORD_SPECTRAL;
      delete DIR_COORD_SPECTRAL;

      
      
    }  // end of the Reader_Root object