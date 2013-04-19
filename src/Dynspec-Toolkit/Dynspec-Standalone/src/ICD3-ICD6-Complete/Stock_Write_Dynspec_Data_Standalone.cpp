#include <iostream>
#include <string>
#include <fstream>
#include <math.h>

#include "Stock_Write_Dynspec_Data_Standalone.h"

#include <dal/lofar/BF_File.h>

/// \file Stock_Write_Dynspec_Data_Standalone.cpp
///  \brief File C++ (associated to Stock_Write_Dynspec_Data_Standalone_Part.h) for stock process parameters (for rebinning the data)  and write data Matrix for the ICD6's Dynspec Groups 
///  \details  
/// <br /> Overview:
/// <br /> Functions stockDynspecData and writeDynspecData are described (programmed) here. 
/// The first one (stockDynspecData) will take as parameter processing parameters and stok them as private attributes. 
/// The second function (writeDynspecData) will write them in the hdf5 output file (in the corresponding dynspec).
/// Data processing is coded here !!

using namespace dal;
using namespace std;


  Stock_Write_Dynspec_Data_Standalone::Stock_Write_Dynspec_Data_Standalone(){}
  Stock_Write_Dynspec_Data_Standalone::~Stock_Write_Dynspec_Data_Standalone(){}
  
  void Stock_Write_Dynspec_Data_Standalone::stockDynspecData(int Ntime,int Nspectral)
  {
    
  /// <br /> Usage:
  /// <br />     void Stock_Write_Dynspec_Data_Standalone::stockDynspecData(int Ntime,int Nspectral)
  /// \param   Ntime   time binning
  /// \param   Nspectral  spectral (frequency)  binning (number of channels per subbands)
    
    m_Ntime = Ntime;
    m_Nspectral = Nspectral;
  }


  
   bool is_readable( const std::string & file ) 
  { 
    /// \param file
    /// <br />Check if file is readable, so if file exists !
    /// \return boolean value 

    std::ifstream fichier( file.c_str() ); 
    return !fichier.fail(); 
  } 
    
  
  
  void Stock_Write_Dynspec_Data_Standalone::writeDynspecData(Group &dynspec_grp,string obsName,string pathFile,string outputFile,File &root_grp,int i,int j,int k,int l,int obsNofStockes,vector<string> stokesComponent,float memoryRAM, vector<string> listOfFiles)  
  {
      
    
  /// <br /> Usage:
  /// <br />   void Stock_Write_Dynspec_Data_Standalone::writeDynspecData(Group &dynspec_grp,string pathDir,string obsName,string pathFile,string outputFile,File &root_grp,int i,int j,int k,int l,int q,int obsNofStockes,vector<string> stokesComponent,float memoryRAM, int SAPindex)  

  /// \param  &dynspec_grp Group Object(Dynamic spectrum object)
  /// \param  pathDir ICD3 path directory
  /// \param  obsName Observation ID
  /// \param  pathFile ICD3 path for loading file
  /// \param  outputFile output file 
  /// \param  &root_grp File Object (Root Group object)
  /// \param  i Subarray pointing loop index
  /// \param  j Beams loop index
  /// \param  k loop index
  /// \param  l loop index
  /// \param  q Pxx nomenclature loop index
  /// \param  obsNofStockes number of Stokes components
  /// \param  stokesComponent vector which contains all Stokes components
  /// \param  memoryRAM RAM memory consuption by processing
  /// \param  SAPindex Subarray pointings to process

            
      ////////////////////////////////////////////////////////////////
      // Redefine ObsStokes for particluar Cases
      
      ////////////////////////////////////////////////////////////////
      // Redefine ObsStokes for particluar Cases
      
      BF_File file(pathFile);  							// generate an object called file contains Root
      BF_SubArrayPointing SAP 		= file.subArrayPointing(i);		// generate an object called SAP contains subarray pointings
      BF_BeamGroup BEAM			= SAP.beam(j);      			// generate an object called BEAM contains Beams 
      
      
      BF_StokesDataset STOCKES 		= BEAM.stokes(0);			// generate an object called STOCKES contains stockes data  
      
      
      CoordinatesGroup COORDS		= BEAM.coordinates();      		// generate an object called COORDS contains Coodinates directory
      
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //WRITE THE DATA Matrix
            
      // generating the dataset
      
      Dataset<float> data_grp(dynspec_grp, "DATA");
      vector<ssize_t> dimensions(3);
      dimensions[0] = m_Ntime;
      dimensions[1] = m_Nspectral;
      dimensions[2] = obsNofStockes;      
      data_grp.create( dimensions );
      
      
      // define the time step for filling the dataset
      int p(0),r(0),n(0);
      int sizeTimeLimit((2.5E8*memoryRAM)/(m_Nspectral*obsNofStockes));	// time step ~ 1Go RAM memory maximum !
      int fracTime((m_Ntime/sizeTimeLimit)+1);
      int nofLastElements((m_Ntime)-((fracTime-1)*sizeTimeLimit));
      
      
      for (p=0;p<fracTime;p++)
	{
	  
	  
	vector<float> DATA_3D(sizeTimeLimit*m_Nspectral*obsNofStockes);
	vector<float> DATA_Pol(sizeTimeLimit*m_Nspectral*obsNofStockes);
      
	if (p==fracTime-1){if (nofLastElements > 0){DATA_3D.resize(nofLastElements*m_Nspectral*obsNofStockes);DATA_Pol.resize(nofLastElements*m_Nspectral*obsNofStockes);}}	    

	for (l=0;l<obsNofStockes;l++)
	  {
	      vector<float> DATA_2D(sizeTimeLimit * m_Nspectral);
	      if (p==fracTime-1){if (nofLastElements > 0){DATA_2D.resize(nofLastElements * m_Nspectral);}} 
	      
		    pathFile 	= listOfFiles[l+j*l];
		    
 		    int pathFileSize(pathFile.length());
		    string pathDir(pathFile.substr(0,pathFileSize-2));
		    string pathRaw(pathDir+"raw");

		    
		    
		    if ( is_readable( pathRaw ) ) 
		    { 			    

			    // Load Stokes data
			    
			    BF_File file(pathFile);  							// generate an object called file contains Root
			    BF_SubArrayPointing SAP 		= file.subArrayPointing(i);		// generate an object called SAP contains subarray pointings
			    BF_BeamGroup BEAM			= SAP.beam(j);      			// generate an object called BEAM contains Beams
			    

			    BF_StokesDataset STOCKES 		= BEAM.stokes(l);
			    
	      
			    //Load 2D Stockes data in a 2D array DATA0 with DAL and stock data in DATA3D for writting (by time step)     
			    if (p!=fracTime-1)
			      {    
				vector<size_t> pos(2);
				pos[0]=p*sizeTimeLimit;
				pos[1]=0;
			  
				vector<size_t> size(2);
				size[0] = sizeTimeLimit;
				size[1] = m_Nspectral;
			      
				STOCKES.getMatrix( pos, &DATA_2D[0], size );
				
			  
				for (r=0;r<sizeTimeLimit;r++)
				  {
				    for (n=0;n<m_Nspectral;n++)
				      {		  			  
					if (l==0){DATA_3D[obsNofStockes*((m_Nspectral*r)+n)]=DATA_2D[(r*m_Nspectral)+n];}
					if (l==1){DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+1]=DATA_2D[(r*m_Nspectral)+n];}
					if (l==2){DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+2]=DATA_2D[(r*m_Nspectral)+n];}
					if (l==3){DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+3]=DATA_2D[(r*m_Nspectral)+n];}
								
				      }	  
				  }		    
				}
			      else
				{
				  if (nofLastElements > 0)
				    {		  
				      vector<size_t> pos(2);
				      pos[0]=p*sizeTimeLimit;
				      pos[1]=0;
			    
				      vector<size_t> size(2);
				      size[0] = nofLastElements;
				      size[1] = m_Nspectral;
				  
				      STOCKES.getMatrix( pos, &DATA_2D[0], size );
					      
				      for (r=0;r<nofLastElements;r++)
					{
					  for (n=0;n<m_Nspectral;n++)
					    {		  			  
					      if (l==0){DATA_3D[obsNofStockes*((m_Nspectral*r)+n)]=DATA_2D[(r*m_Nspectral)+n];}
					      if (l==1){DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+1]=DATA_2D[(r*m_Nspectral)+n];}
					      if (l==2){DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+2]=DATA_2D[(r*m_Nspectral)+n];}
					      if (l==3){DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+3]=DATA_2D[(r*m_Nspectral)+n];}
								      
					    }	  
					}
				    }
			      } // end loop of else
			} // end of Existing test 
			else
			{

			Group process_histo_grp(dynspec_grp, "PROCESS_HISTORY");
			Attribute<string> missedData(process_histo_grp, "MISSED DATA");
			missedData.value = stokesComponent[l];
 			       
			}// end of else existing test			      

	  }  //end loop on l (Stockes)// data are loaded in 3D array


	  
	  // Convert to I,U,Q & V
	  
	  
	  if (p!=fracTime-1)
	    {    	  
	      for (r=0;r<sizeTimeLimit;r++)
		  {
		    for (n=0;n<m_Nspectral;n++)
		      {		  			  			
			DATA_Pol[obsNofStockes*((m_Nspectral*r)+n)] 	= (pow(DATA_3D[obsNofStockes*((m_Nspectral*r)+n)],2)+pow(DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+1],2)+pow(DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+2],2)+pow(DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+3],2))/2.0;
			DATA_Pol[obsNofStockes*((m_Nspectral*r)+n)+1]	= (pow(DATA_3D[obsNofStockes*((m_Nspectral*r)+n)],2)+pow(DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+1],2)-pow(DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+2],2)-pow(DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+3],2))/2.0;
			DATA_Pol[obsNofStockes*((m_Nspectral*r)+n)+2]	= (DATA_3D[obsNofStockes*((m_Nspectral*r)+n)]*DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+2])+(DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+1]*DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+3]);
			DATA_Pol[obsNofStockes*((m_Nspectral*r)+n)+3]	= (DATA_3D[obsNofStockes*((m_Nspectral*r)+n)]*DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+3])-(DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+1]*DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+2]);					
		      }
		  }
	    }			
	   else
	     {
		if (nofLastElements > 0)
		    {		  
		      for (r=0;r<nofLastElements;r++)
			{
			  for (n=0;n<m_Nspectral;n++)
			    {		  			  
			      DATA_Pol[obsNofStockes*((m_Nspectral*r)+n)] 	= (pow(DATA_3D[obsNofStockes*((m_Nspectral*r)+n)],2)+pow(DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+1],2)+pow(DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+2],2)+pow(DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+3],2))/2.0;
			      DATA_Pol[obsNofStockes*((m_Nspectral*r)+n)+1]	= (pow(DATA_3D[obsNofStockes*((m_Nspectral*r)+n)],2)+pow(DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+1],2)-pow(DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+2],2)-pow(DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+3],2))/2.0;
			      DATA_Pol[obsNofStockes*((m_Nspectral*r)+n)+2]	= (DATA_3D[obsNofStockes*((m_Nspectral*r)+n)]*DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+2])+(DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+1]*DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+3]);
			      DATA_Pol[obsNofStockes*((m_Nspectral*r)+n)+3]	= (DATA_3D[obsNofStockes*((m_Nspectral*r)+n)]*DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+3])-(DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+1]*DATA_3D[obsNofStockes*((m_Nspectral*r)+n)+2]);		      
			    }
			}
		    }
	     }
	  
	  
	  
	  // Write data by time steps	
	  if (p!=fracTime-1)
	    {	  
	      vector<size_t> data_grp_size(3);
	      vector<size_t> data_grp_pos(3);

	      data_grp_pos[0] = p*sizeTimeLimit;
	      data_grp_pos[1] = 0;
	      data_grp_pos[2] = 0;   	  
	  
	      data_grp_size[0] = sizeTimeLimit;
	      data_grp_size[1] = m_Nspectral;
	      data_grp_size[2] = obsNofStockes;
	    
	      data_grp.setMatrix( data_grp_pos, &DATA_Pol[0], data_grp_size );
	    }
	  else
	    {		  
	      if (nofLastElements > 0)
		{
		  vector<size_t> data_grp_size(3);
		  vector<size_t> data_grp_pos(3);

		  data_grp_pos[0] = p*sizeTimeLimit;
		  data_grp_pos[1] = 0;
		  data_grp_pos[2] = 0;   	  
	  
		  data_grp_size[0] = nofLastElements;
		  data_grp_size[1] = m_Nspectral;
		  data_grp_size[2] = obsNofStockes;
	    
		  data_grp.setMatrix( data_grp_pos, &DATA_Pol[0], data_grp_size );		    
		}
	    }
	    


	} // end loop on p (time step)

	      
	      	      
	    //META-DATA in DATA  writter
	      
	    string GROUPE_TYPE_DATA("Data");
	    string WCSINFO("/Coordinates");
	    unsigned long int DATASET_NOF_AXIS(3);
	    vector<unsigned long int> DATASET_SHAPE(1);DATASET_SHAPE[0];
		      
	    Attribute<string> attr_143(data_grp, "GROUPE_TYPE");
	    Attribute<string> attr_144 (data_grp, "WCSINFO");
	    Attribute<unsigned long int> attr_145(data_grp, "DATASET_NOF_AXIS");
	    Attribute< vector<unsigned long int> > attr_146(data_grp, "DATASET_SHAPE");
		
	    attr_143.value = GROUPE_TYPE_DATA;
	    attr_144.value = WCSINFO;
	    attr_145.value = DATASET_NOF_AXIS;
	    attr_146.create().set(DATASET_SHAPE);      
	    
	    
	    cout << "SAP N° "<< i <<" and BEAM N° " << j  << endl;
	    cout << "  " << endl;   
	  
	    
  }
