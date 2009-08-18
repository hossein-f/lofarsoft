/*-------------------------------------------------------------------------*
 | $Id:: snstestnewskymapping.cc 2009-07-13 swelles                $ |
 *-------------------------------------------------------------------------*
 ***************************************************************************
 *   Copyright (C) 2006                                                    *
 *   Andreas Horneffer (A.Horneffer@astro.ru.nl)                           *
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

#include <casa/aips.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Exceptions/Error.h>
#include <casa/System/Aipsrc.h>
#include <casa/System/ProgressMeter.h>
#include <casa/OS/Directory.h>
#include <coordinates/Coordinates/ObsInfo.h>
#include <images/Images/PagedImage.h>

using casa::AipsError;
using casa::Array;
using casa::IPosition;
using casa::ObsInfo;
using casa::PagedImage;
using casa::TiledShape;

#include <crtools.h>
#include <Coordinates/SkymapCoordinate.h>
#include <Data/LOFAR_TBB.h>
#include <Analysis/CRinvFFT.h>
#include <Imaging/Beamformer.h>
#include <Imaging/Skymapper.h>
#include <Utilities/ProgressBar.h>

using CR::Beamformer;
using CR::ProgressBar;
using CR::SkymapCoordinate;
using CR::Skymapper;
using CR::TimeFreq;

#define HDF5_DISABLE_VERSION_CHECK 1
/*!
  \file snstestnewskymapping.cc

  \ingroup CR_Applications

  \brief Test the Skymapper with different input files with multiple antennas
 
  \author Sander & Sef
 
  \date 2009/08/04

  <h3>Synopsis</h3>

*/

// -----------------------------------------------------------------------------


//#define upload	//uncomment to upload if it does not compile.

/*!
 \brief This will give the positions back from the selected antennes of the inner array of station CS302 in local coordinates. 
 
 \param rcu_ids       -- casa::Vector<uint> with the selected RCUs
 
 \return selected_positions --  The positions of the selected antenna's 
*/

casa::Vector<MVPosition> CS302_antenna_position_value(casa::Vector<Int> rcu_ids){
	casa::Vector<MVPosition> all_positions;
	casa::Vector<MVPosition> selected_positions;
	all_positions.resize(48);
	Int nrRCUs;
	rcu_ids.shape(nrRCUs);
	cout << "nr of RCUs: " << nrRCUs << endl;
	selected_positions.resize(nrRCUs);
	
  all_positions(0)=MVPosition(0.000,0.000,0.000);//RCU0,1
	all_positions(1)=MVPosition(0.000,2.550,0.000);//RCU2,3
	all_positions(2)=MVPosition(2.250,1.350,0.000);//RCU4,5
	all_positions(3)=MVPosition(2.250,-1.350,0.000);//RCU6,7
	all_positions(4)=MVPosition(0.000,-2.550,0.000);//RCU8,9
	all_positions(5)=MVPosition(-2.250,-1.350,0.000);//RCU10,11
	all_positions(6)=MVPosition(-2.250,1.350,0.000);//RCU12,13
	all_positions(7)=MVPosition(0.405,-5.786,0.000);//RCU14,15
	all_positions(8)=MVPosition(-3.409,-4.692,0.000);//RCU16,17
	all_positions(9)=MVPosition(-5.628,-1.403,0.000);//RCU18,19
	all_positions(10)=MVPosition(-5.213,2.543,0.000);//RCU20,21
	all_positions(11)=MVPosition(-2.359,5.299,0.000);//RCU22,23
	all_positions(12)=MVPosition(1.599,5.575,0.000);//RCU24,25
	all_positions(13)=MVPosition(4.808,3.243,0.000);//RCU26,27
	all_positions(14)=MVPosition(5.768,-0.606,0.000);//RCU28,29
	all_positions(15)=MVPosition(4.029,-4.172,0.000);//RCU30,31
	all_positions(16)=MVPosition(5.867,-7.250,0.000);//RCU32,33
	all_positions(17)=MVPosition(2.937,-9.438,0.000);//RCU34,35
	all_positions(18)=MVPosition(-2.056,-10.119,0.000);//RCU36,37
	all_positions(19)=MVPosition(-8.087,-6.048,0.000);//RCU38,39
	all_positions(20)=MVPosition(-9.641,-3.219,0.000);//RCU40,41
	all_positions(21)=MVPosition(-10.034,1.056,0.000);//RCU42,43
	all_positions(22)=MVPosition(-7.920,4.754,0.000);//RCU44,45
	all_positions(23)=MVPosition(-3.362,8.331,0.000);//RCU46,47
	all_positions(24)=MVPosition(1.177,9.843,0.000);//RCU48,49
	all_positions(25)=MVPosition(5.365,8.543,0.000);//RCU50,51
	all_positions(26)=MVPosition(7.642,5.322,0.000);//RCU52,53
	all_positions(27)=MVPosition(9.020,0.672,0.000);//RCU54,55
	all_positions(28)=MVPosition(8.319,-3.940,0.000);//RCU56,57
	all_positions(29)=MVPosition(7.905,-11.973,0.000);//RCU58,59
	all_positions(30)=MVPosition(4.686,-14.025,0.000);//RCU60,61
	all_positions(31)=MVPosition(1.561,-14.072,0.000);//RCU62,63
	all_positions(32)=MVPosition(-6.074,-11.713,0.000);//RCU64,65
	all_positions(33)=MVPosition(-9.040,-12.128,0.000);//RCU66,67
	all_positions(34)=MVPosition(-12.203,-5.337,0.000);//RCU68,69
	all_positions(35)=MVPosition(-13.111,-0.342,0.000);//RCU70,71
	all_positions(36)=MVPosition(-14.322,4.832,0.000);//RCU72,73
	all_positions(37)=MVPosition(-11.636,6.845,0.000);//RCU74,75
	all_positions(38)=MVPosition(-8.210,10.139,0.000);//RCU76,77
	all_positions(39)=MVPosition(-1.857,13.089,0.000);//RCU78,79
	all_positions(40)=MVPosition(0.857,13.724,0.000);//RCU80,81
	all_positions(41)=MVPosition(5.865,11.900,0.000);//RCU82,83
	all_positions(42)=MVPosition(9.981,11.338,0.000);//RCU84,85
	all_positions(43)=MVPosition(13.747,3.311,0.000);//RCU86,87
	all_positions(44)=MVPosition(13.900,-0.670,0.000);//RCU88,89
	all_positions(45)=MVPosition(13.136,-6.101,0.000);//RCU90,91
	all_positions(46)=MVPosition(-65.054,0.000,0.000);//RCU92,93
	all_positions(47)=MVPosition(0.000,65.054,0.000);//RCU94,95

	cout << "filled all_positions" << endl;
	int rcu_id;
	for(int i=0;i<nrRCUs;i++){
		rcu_id=rcu_ids(i);
		cout << "rcu id = " << rcu_id << endl;
 		selected_positions(i)=all_positions(rcu_id/2);///2);
	}
	
	return selected_positions;
	
}

casa::Vector<MVPosition> CS302_antenna_position_value_outer(casa::Vector<Int> rcu_ids){
	casa::Vector<MVPosition> all_positions;
	casa::Vector<MVPosition> selected_positions;
	all_positions.resize(48);
	Int nrRCUs;
	rcu_ids.shape(nrRCUs);
	cout << "nr of RCUs: " << nrRCUs << endl;
	selected_positions.resize(nrRCUs);
	
  all_positions(0)=MVPosition(14.698,-10.311,0.000);//RCU0,1
	all_positions(1)=MVPosition(15.232,-13.338,0.000);//RCU2,3
	all_positions(2)=MVPosition(1.377,-20.345,0.000);//RCU4,5
	all_positions(3)=MVPosition(-13.608,-14.411,0.000);//RCU6,7
	all_positions(4)=MVPosition(-17.281,-11.123,0.000);//RCU8,9
	all_positions(5)=MVPosition(-18.305,0.971,0.000);//RCU10,11
	all_positions(6)=MVPosition(-12.430,12.886,0.000);//RCU12,13
	all_positions(7)=MVPosition(-7.399,19.058,0.000);//RCU14,15
	all_positions(8)=MVPosition(6.837,16.723,0.000);//RCU16,17
	all_positions(9)=MVPosition(14.416,13.666,0.000);//RCU18,19
	all_positions(10)=MVPosition(18.192,4.459,0.000);//RCU20,21
	all_positions(11)=MVPosition(24.840,-0.328,0.000);//RCU22,23
	all_positions(12)=MVPosition(21.410,-12.882,0.000);//RCU24,25
	all_positions(13)=MVPosition(12.443,-22.123,0.000);//RCU26,27
	all_positions(14)=MVPosition(6.726,-24.705,0.000);//RCU28,29
	all_positions(15)=MVPosition(-1.476,-26.687,0.000);//RCU30,31
	all_positions(16)=MVPosition(-16.470,-17.401,0.000);//RCU32,33
	all_positions(17)=MVPosition(-21.667,-11.351,0.000);//RCU34,35
	all_positions(18)=MVPosition(-26.101,1.776,0.000);//RCU36,37
	all_positions(19)=MVPosition(-20.349,14.772,0.000);//RCU38,39
	all_positions(20)=MVPosition(-14.252,21.805,0.000);//RCU40,41
	all_positions(21)=MVPosition(-2.861,24.616,0.000);//RCU42,43
	all_positions(22)=MVPosition(13.224,23.568,0.000);//RCU44,45
	all_positions(23)=MVPosition(24.993,10.289,0.000);//RCU46,47
	all_positions(24)=MVPosition(34.641,0.400,0.000);//RCU48,49
	all_positions(25)=MVPosition(28.484,-13.172,0.000);//RCU50,51
	all_positions(26)=MVPosition(19.357,-24.589,0.000);//RCU52,53
	all_positions(27)=MVPosition(13.304,-29.830,0.000);//RCU54,55
	all_positions(28)=MVPosition(-5.553,-31.094,0.000);//RCU56,57
	all_positions(29)=MVPosition(-22.720,-25.520,0.000);//RCU58,59
	all_positions(30)=MVPosition(-30.213,-15.775,0.000);//RCU60,61
	all_positions(31)=MVPosition(-29.298,-6.451,0.000);//RCU62,63
	all_positions(32)=MVPosition(-33.339,7.631,0.000);//RCU64,65
	all_positions(33)=MVPosition(-23.863,24.118,0.000);//RCU66,67
	all_positions(34)=MVPosition(-16.634,29.422,0.000);//RCU68,69
	all_positions(35)=MVPosition(-1.557,34.648,0.000);//RCU70,71
	all_positions(36)=MVPosition(6.143,31.746,0.000);//RCU72,73
	all_positions(37)=MVPosition(24.552,22.482,0.000);//RCU74,75
	all_positions(38)=MVPosition(26.185,15.967,0.000);//RCU76,77
	all_positions(39)=MVPosition(32.238,17.636,0.000);//RCU78,79
	all_positions(40)=MVPosition(35.877,-21.401,0.000);//RCU80,81
	all_positions(41)=MVPosition(15.445,-36.911,0.000);//RCU82,83
	all_positions(42)=MVPosition(-4.028,-41.786,0.000);//RCU84,85
	all_positions(43)=MVPosition(-18.886,-35.621,0.000);//RCU86,87
	all_positions(44)=MVPosition(-41.007,3.644,0.000);//RCU88,89
	all_positions(45)=MVPosition(-28.864,22.439,0.000);//RCU90,91
	all_positions(46)=MVPosition(-20.085,36.257,0.000);//RCU92,93
	all_positions(47)=MVPosition(22.803,31.368,0.000);//RCU94,95

	cout << "filled all_positions" << endl;
	int rcu_id;
	for(int i=0;i<nrRCUs;i++){
		rcu_id=rcu_ids(i);
		cout << "rcu id = " << rcu_id << endl;
 		selected_positions(i)=all_positions(rcu_id/2);///2);
	}
	
	return selected_positions;
	
}



//int  simpleImage(string const &infile,
//		 string const &outfile,
//		 uint const &blocksize=1024)
//{
//	int nofFailedTests     = 0;


/*!
  \brief Process the data to generate an image

  \param infile       -- input datafile with data input and program configurations
  \param outfile      -- (path-)name of the image-file to be generated
  \param blocksize    -- Number of samples per block of data.

  \return nofFailedTests --  The number of failed tests encountered within this
          fucntion.
*/

int main (int argc,
			  char *argv[])
	{
		cout<<"Dit is de door Sef & Sander aangepaste versie van testLOPESskymapping"<<endl;
		
		uint nofFailedTests=0, blocksize=1024;
		std::string infile, outfile="snstestnewskymapping.img";
		
		/*
		 Check if filename of the dataset is provided on the command line; if only
		 a fraction of the possible tests can be carried out.
		 */
		if (argc < 2) {
			std::cout << "Usage: testnewskymapping <inputfile.dat> [<output-image>]. Now using snstestnewskymapping.dat" << endl;
			infile = "../../../src/CR-Tools/apps/snstestnewskymapping.dat"; 
		} else {
			infile = argv[1];
			if (argc > 2) {
				outfile  = argv[2];
			};
		}
		
		//nofFailedTests += simpleImage (infile,
//									   outfile,
//									   blocksize);
		
	
	
	
	
  

  //_______________________________________________________
  // Read input paramaters from infile
  
     //Opens for reading the file
  ifstream b_file (infile.c_str()); 
  
  int ninputfiles;
  //int nantsinfile;
  int nants=0;

  string pathname;
 
  b_file >> ninputfiles;
  b_file >> pathname;
  vector<string> inputfiles(ninputfiles);
  vector<int> nantsinfile(ninputfiles);

	
  CR::LOFAR_TBB **dr;
  dr = new CR::LOFAR_TBB*[ninputfiles];
 
  Vector< int >*rcu_ids;
  rcu_ids=new Vector< int >[ninputfiles];	
	
#ifndef upload
  for(int i=0; i<ninputfiles; i++){
  	b_file>> inputfiles[i];
	//cout<<endl<<i<<endl<<endl;
  	dr[i] = new CR::LOFAR_TBB(pathname+inputfiles[i], blocksize);
	nantsinfile[i] = dr[i]->fx().shape()[1];
	rcu_ids[i]=dr[i]->channelID()%1000; 
	cout << "rcus for file" << i << ": " << rcu_ids[i] << endl;
	nants+=nantsinfile[i];
  }
  cout<<"total number of antennas = "<<nants<<endl;
  cout << "shape of dr[0]->fx() = " << dr[0]->fx().shape() << endl;
  cout << "shape of dr[0]->fft() = " << dr[0]->fft().shape() << endl;
  cout <<"dr->fft for antenna "<<0<<" = "<<dr[0]->fft().row(0)[0]<<endl;
  cout <<"dr->fft for antenna "<<1<<" = "<<dr[0]->fft().row(1)[0]<<endl;
	

	

/*  CR::LOFAR_TBB **dr;
  dr = new CR::LOFAR_TBB*[nants];
  int counter = 0;
  for(int i=0; i<ninputfiles; i++){
		nantsinfile = dr[i]->fx().shape()[1];
		for(int j=0; j<nantsinfile; j++){
			Vector<Bool> antennasetter(nantsinfile);
			antennasetter = False;
			antennasetter[j]=True;
			cout<<"antennasetter = "<<antennasetter<<endl;
			cout<<"counter = "<<counter<<endl;
		 	//dr[i]->setSelectedAntennas(antennasetter);
			//cout<<dr[i]->selectedAntennas();
			dr[counter] = dr[i];
			counter++;
		}
  }
  
	cout << "shape of dr[0] = " << dr[0]->shape() << endl;
  cout << "shape of dr[0]->fx() = " << dr[0]->fx().shape() << endl;
  cout << "shape of dr[0]->fx() = " << dr[0]->fx().shape() << endl;
  cout << "shape of dr[0]->fft() = " << dr[0]->fft().shape() << endl;
  cout << "shape of dr[0]->fft() = " << dr[0]->fft().shape() << endl;

	for(int i=0;i<nants;i++){
		cout <<"dr->fft for antenna "<<i<<" = "<<dr[i]->fft()[0][0]<<endl;
	}
*/
	cout <<"dr->fft for antenna "<<0<<" = "<<dr[0]->fft().row[0]<<endl;
	//cout <<"dr->fft for antenna "<<1<<" = "<<dr[0]->fft()[1][0]<<endl;

  
  int pixels;
  float increment;
  int depth;
  float startdepth;
  float depthstr;
  
  int nframes;
  int blocksperframe;
  int startblock;

  int ndim;
  int startant;
  int nant;
  int antstr;
  int startfreq;
  int nfreq;
  int freqstr;
  int startcoord;
  int ncoord;
  int coordstr;
	
	string arrayspread;
  string selection;


  //Read file:
  b_file>> pixels;
  increment = 231./pixels;
  b_file>> depth;
  b_file>> startdepth;
  b_file>> depthstr;
  
  //cout<<"depthstr = " <<depthstr<<endl;
  
  b_file>> nframes;
  b_file>> blocksperframe;
  b_file>> startblock;
  
  //cout<<"startblock = " <<startblock<<endl;
  
  b_file>> ndim;
  b_file>> startant;
  b_file>> nant;
  b_file>> antstr;
  b_file>> startfreq;
  b_file>> nfreq;
  b_file>> freqstr;
  b_file>> startcoord;
  b_file>> ncoord;
  b_file>> coordstr;
  
  cout<<"ncoord = " <<ncoord<<" (should be 3)"<<endl;
  
/*
  cout << "pixels: "<<pixels<<endl;
  cout<< "depth: " <<depth<<endl;
  cout<<"blocksperframe: " << blocksperframe<<endl;
  cout<<"starts: " <<start[0]<<" "<<start[1]<<" "<<start[2]<<endl;
*/
	
	
 // check out the antennaselection

   Vector< bool >*antennaselection;
   antennaselection=new Vector< bool>[ninputfiles];
//	
//	Vector<uInt>* rcu_ids;
//	rcu_ids=new Vector<uInt>[ninputfiles];

  b_file >> selection;
	if(selection=="even"){
		for(int i=0;i<ninputfiles;i++){
			antennaselection[i] = (rcu_ids[i]%2==0);
			cout << "Ant selection for file " << i << " = " << antennaselection[i] << endl;
		}
	} else if(selection=="odd"){
		for(int i=0;i<ninputfiles;i++){
			antennaselection[i] = (rcu_ids[i]%2==1);
			cout << "Ant selection for file " << i << " = " << antennaselection[i] << endl;
		}
	} else if(selection=="all"){
		for(int i=0;i<ninputfiles;i++){
			antennaselection[i] = (rcu_ids[i]%2==1||rcu_ids[i]%2==0);
			cout << "Ant selection for file " << i << " = " << antennaselection[i] << endl;
		}
	} else {
		cout <<"Another selection than odd, even or all is not implemented yet";
	}
		
		
	int nUsedants=0;
	vector<int> nUsedantsinfile(ninputfiles);
	Vector< int >*used_rcu_ids;
	used_rcu_ids=new Vector< int >[ninputfiles];
	
	for(int i=0;i<ninputfiles;i++){
		nUsedantsinfile[i]=0;
		for(int j=0;j<nantsinfile[i];j++){
			if(antennaselection[i][j]){nUsedantsinfile[i]++;}
		}
		used_rcu_ids[i].resize(nUsedantsinfile[i]);
	    nUsedants+=nUsedantsinfile[i];
		int counter=0;
		for(int j=0;j<nantsinfile[i];j++){
			if(antennaselection[i][j]){used_rcu_ids[i][counter]=rcu_ids[i][j]; counter++;}
		}
		cout << "used rcus for file " << i << " : " << used_rcu_ids[i] << endl;
	}	
	//nUsedants=nants;
	//nUsedantsinfile=nantsinfile;
  

	
	//used_rcu_ids=rcu_ids;
		
		
		
    try {    
    
    //________________________________________________________
    cout << "testnewskymapping::simpleImage reading in event and setting up the pipeline"  << endl;
	
    CR::CRinvFFT pipeline;

    //________________________________________________________
    // Set up the skymap coordinates and infos
     // Observation info
    cout << "testnewskymapping::simpleImage Setting up ObsInfo"  << endl;
    std::string telescope  = "LOFAR CS302";
    std::string observer   = "John Doe";
    casa::ObsInfo info;
    info.setTelescope(telescope);
    info.setObserver(observer);
    // Spatial coordinates
    cout << "testnewskymapping::simpleImage Setting up SpatialCoordinate"  << endl;
    std::string refcode    = "AZEL";
    std::string projection = "STG";
    IPosition shape (3,pixels,pixels,depth);
    SpatialCoordinate spatial (CoordinateType::DirectionRadius, refcode,projection);
    spatial.setShape(shape);
    //set reference pixel, reference value, and coord increment
    Vector<double> tmpvec;
    tmpvec = spatial.referencePixel();
    tmpvec(0) = (pixels-1.)/2; tmpvec(1)=(pixels-1.)/2; tmpvec(2)=0.;
    spatial.setReferencePixel(tmpvec);
    tmpvec = spatial.referenceValue();
    tmpvec(0) = 180.; tmpvec(1)=90.;  tmpvec(2)=startdepth;
    spatial.setReferenceValue(tmpvec,true);
    tmpvec = spatial.increment();
    tmpvec(0) = increment; tmpvec(1)=increment;  tmpvec(2)=depthstr;
    spatial.setIncrement(tmpvec,true);
    // Time-Frequency coordinate
    cout << "testnewskymapping::simpleImage Setting up TimeFreqCoordinate"  << endl;
    uint nofBlocksPerFrame = blocksperframe ;
    uint nofFrames         = nframes;
    TimeFreqCoordinate timeFreq (blocksize, nofBlocksPerFrame, nofFrames,false);
    timeFreq.setNyquistZone(2);
    // Skymap coordinate
    cout << "testnewskymapping::simpleImage Setting up SkymapCoordinate"  << endl;
    SkymapCoordinate coord (info,
			    spatial,
			    timeFreq);
    
    //________________________________________________________
    // retrieve the antenna positions

//    cout << "testnewskymapping::simpleImage Retrieving the antenna positions"  << endl;
//    Matrix<double> antPositions(nants,3);
//    for (uInt j=0; j<antPositions.nrow(); j++) {
//       for (uInt i=0; i<antPositions.ncolumn(); i++) {
//          b_file >> antPositions(j,i);
//       }      
//    }
//		Matrix<double> subantPositions;
//		IPosition start_1 (ndim,startant,startcoord), length_1 (ndim,nant,ncoord), stride_1 (ndim,antstr,coordstr);
//    Slicer slicer1 (start_1, length_1, stride_1);
//		subantPositions = antPositions(slicer1);
//		cout << "the antanna positions are: " << subantPositions <<endl;
	  
	//******* NEW METHOD ***************
	  cout << "testnewskymapping::simpleImage Retrieving the antenna positions"  << endl;
		
	  Vector<MVPosition> subantPositions;
	  Vector<MVPosition> tempantPositions;
	  
	  subantPositions.resize(nUsedants);
	  uInt counter=0;
	  Int nRCU;
	  
		b_file>>arrayspread;
		
	  for(int i=0; i<ninputfiles; i++){
		  
		  used_rcu_ids[i].shape(nRCU);
		  tempantPositions.resize(nRCU);
		  
			if(arrayspread=="inner"){
				tempantPositions = CS302_antenna_position_value(used_rcu_ids[i]);
		  } else if(arrayspread=="outer"){
				tempantPositions = CS302_antenna_position_value_outer(used_rcu_ids[i]);
			} else cout<<"Valid antennaspreads are 'inner' and 'outer'."<<endl;
			
		  //nrRCUs = new int(0);
		  
		  for(int j=0; j<nRCU;j++){
			  subantPositions(counter)=tempantPositions(j);
			  cout<<counter;
			  counter++;
		  }		  
	  }
/*		
	  Matrix<double> antPositions(nRCU,3);
    for (uInt j=0; j<antPositions.nrow(); j++) {
       for (uInt i=0; i<antPositions.ncolumn(); i++) {
          antPositions(j,i) = tempantPositions[j].getValue()[i];
       }      
    }
*/
	  cout << "The antenna positions are "<< subantPositions << endl;

    
    //________________________________________________________
    // Set up the skymapper 
    
    cout << "testnewskymapping::simpleImage Setting up the Skymapper..."  << endl;
    
    Skymapper skymapper (coord,
			 subantPositions,		// change for number of antennas
			 outfile,
			 Skymapper::HDF5Image);
    
    cout << "                                                         ... done."  << endl;

	skymapper.setFarField();				// Not for imaging lightning!!!
    skymapper.summary();
    
    //________________________________________________________
    // process the event
    
 
		// correct for the offset relative to the last antenna. Be aware that reading from the beginning of the file may give problems.
		
    uint nofBlocks = nofBlocksPerFrame * nofFrames;

    Matrix<casa::DComplex> data(dr[0]->fftLength(),nUsedants); 
	
		Matrix<casa::DComplex> subdata; 
		IPosition start (ndim,startfreq,startant), length (ndim,nfreq,nant), stride (ndim,freqstr,antstr);
    Slicer slicer (start, length, stride);
		
																							// Sander's new implementation
		//casa::Vector<uint> offset(nantsinfile[ninputfiles-1]);
		//std::vector<int> offset1(nantsinfile[ninputfiles-1]);
		
   	Vector< uint >*offset;
   	Vector< int >*offset1;
		offset= new Vector< uint>[ninputfiles];
		offset1=new Vector< int>[ninputfiles];
		
		int offset_zero;
		offset_zero = dr[ninputfiles-1]->sample_number()[nantsinfile[ninputfiles-1]-1];
		
		cout<<"offset 0 = "<<offset_zero<<endl;
		
 		for(int i=0; i<ninputfiles; i++){
			offset[i] = dr[i]->sample_number();
			offset1[i].resize(offset[i].shape()[0]);
	 		for(int j=0; j<nantsinfile[i]; j++){
				offset1[i][j] = -1*offset[i][j] + offset_zero;
			}
			cout<<"offset["<<i<<"] = "<<offset[i]<<endl;
			cout<<"offset1["<<i<<"] = "<<offset1[i]<<endl;
		}
		
		cout <<"dr->fft for antenna "<<0<<" = "<<dr[0]->fft().row(0)[0]<<endl;
																			
		
    for (uint blocknum=startblock; blocknum<startblock+nofBlocks; blocknum++){
    	//if(blocknum%10==0){
			cout<<"Processing block "<<blocknum-startblock<<" out of "<<nofBlocks<<endl;
			//}
			counter = 0;
	 		for(int i=0; i<ninputfiles; i++){
				//cout<<"time for "<<i<<" = "<<dr[i]->time()<<endl;
				dr[i]->setBlock(blocknum);
	  		for(int j=0; j<nantsinfile[i]; j++){
					if(!antennaselection[i][j]){continue;} //skip antenna's that are not selected
					
					dr[i]->setShift(offset1[i][j]);
					
	  	    //cout<<"so far so good. counter i j = "<<counter<<" "<<i<<" "<<j<<endl;
					//cout<<dr[i]->fft()[0][0]<<endl;
    	  	data.row(counter) = dr[i]->fft().column(j);
					for(int k=0; k < nfreq*30/100; k++){
						data.row(counter)[k] = 0.;
					}
					for(int k=0; k < nfreq*10/100; k++){
						data.row(counter)[nfreq-nfreq*10/100+k] = 0.;
					}
					counter++;
					//cout<<"weer een antenne toegevoegd..."<<endl;
 	 		  }
			}
	  	//subdata = data (slicer);
       skymapper.processData(data);		// Change if slicer is used!
    };
			
   cout << "datashape = " << data.shape() << endl;
   //cout << "subdatashape = " << subdata.shape() << endl;

  } catch (AipsError x) {
    cerr << "[testnewskymapping::simpleImage] " << x.getMesg() << endl;
    nofFailedTests++;
  }
#endif  
  return nofFailedTests;
}









// -----------------------------------------------------------------------------

//int main (int argc,
//	  char *argv[])
//{
//  cout<<"Dit is de door Sef & Sander aangepaste versie van testLOPESskymapping"<<endl;
//
//  uint nofFailedTests=0, blocksize=1024;
//  std::string infile, outfile="snstestnewskymapping.img";
//
//  /*
//    Check if filename of the dataset is provided on the command line; if only
//    a fraction of the possible tests can be carried out.
//  */
//  if (argc < 2) {
//    std::cout << "Usage: testnewskymapping <inputfile.dat> [<output-image>]. Now using snstestnewskymapping.dat" << endl;
//	infile = "../../../src/CR-Tools/apps/seftestnewskymapping.dat"; 
//  } else {
//    infile = argv[1];
//     if (argc > 2) {
//       outfile  = argv[2];
//     };
//  }
//  
//  nofFailedTests += simpleImage (infile,
//				 outfile,
//				 blocksize);
//  
//  return nofFailedTests;
//}

