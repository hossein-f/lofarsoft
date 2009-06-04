/*-------------------------------------------------------------------------*
 | $Id::                                                                 $ |
 *-------------------------------------------------------------------------*
 ***************************************************************************
 *   Copyright (C) 2009                                                    *
 *   Lars B"ahren (bahren@astron.nl)                                       *
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

#include <Utilities/TestsCommon.h>

namespace CR { // Namespace CR -- begin

  //_____________________________________________________________________________
  //                                                                      runtime

  double runtime (clock_t const &start,
		  clock_t const &end)
  {
    return ((double)( end - start ) / (double)CLOCKS_PER_SEC );
  }

  //_____________________________________________________________________________
  //                                                              number_sequence

  std::vector<int> number_sequence () {
    // vector returning the result
    std::vector<int> nelem;
    // generate sequence
    nelem.push_back(1);
    nelem.push_back(2);
    nelem.push_back(5);
    nelem.push_back(10);
    nelem.push_back(20);
    nelem.push_back(50);
    nelem.push_back(100);
    nelem.push_back(200);
    nelem.push_back(500);
    nelem.push_back(1000);
    nelem.push_back(2000);
    nelem.push_back(5000);
    nelem.push_back(10000);
    // return sequence of numbers
    return nelem;
  }

  //_____________________________________________________________________________
  //                                                                 test_ObsInfo
  
  casa::ObsInfo test_ObsInfo (std::string const &telescope,
			      std::string const &observer)
  {
    casa::ObsInfo info;
    casa::Time startTime;
    casa::Quantity epoch (startTime.modifiedJulianDay(), "d");
    
    info.setTelescope (telescope);
    info.setObserver (observer);
    return info;
  }

  //_____________________________________________________________________________
  //                                                           coordinate_summary
  
  void coordinate_summary (casa::Coordinate &coord)
  {
    cout << "-- Coord. type      = " << coord.type()           << endl;
    cout << "-- Coord. name      = " << coord.showType()       << endl;
    cout << "-- nof. pixel axes  = " << coord.nPixelAxes()     << endl;
    cout << "-- nof. world axes  = " << coord.nWorldAxes()     << endl;
    cout << "-- World axis names = " << coord.worldAxisNames() << endl;
    cout << "-- World axis units = " << coord.worldAxisUnits() << endl;
    cout << "-- Reference value  = " << coord.referenceValue() << endl;
    cout << "-- Reference pixel  = " << coord.referencePixel() << endl;
    cout << "-- Increment        = " << coord.increment()      << endl;
    
    /* Test pixel to world conversion */
    
    int nofPixelAxes (coord.nPixelAxes());
    casa::Vector<double> pixel (nofPixelAxes);
    casa::Vector<double> world (nofPixelAxes);
    
    for (int n(0); n<nofPixelAxes; n++) {
      pixel    = 0.0;
      pixel(n) = 1.0;
      //conversion
      coord.toWorld(world,pixel);
      // feedback
      cout << "-- Pixel to World   = " << pixel << " -> " << world << endl;
    }
  }
  
  // ============================================================================
  //
  //  Module Coordinates
  //
  // ============================================================================
  
  //_____________________________________________________________________________

  CR::TimeFreqCoordinate test_TimeFreq (uint const &blocksize,
					casa::Quantity const &sampleFreq,
					uint const &nyquistZone,
					uint const &blocksPerFrame,
					uint const &nofFrames)
  {
    CR::TimeFreqCoordinate timeFreq (blocksize,
				     sampleFreq,
				     nyquistZone,
				     blocksPerFrame,
				     nofFrames);
    return timeFreq;
  }

  //_____________________________________________________________________________
  //                                                         test_exportPositions

  void test_exportPositions (Matrix<double> const &pos,
			     std::string const &filename,
			     bool const &screenSummary)
  {
    casa::IPosition shape (pos.shape());
    std::ofstream outfile;
    int npos;
    int ncoord;

    // open the stream for the output file
    outfile.open(filename.c_str(),std::ios::out);

    for (npos=0; npos<shape(0); npos++) {
      outfile << npos;
      for (ncoord=0; ncoord<shape(1); ncoord++) {
	outfile << "\t" << pos(npos,ncoord);
      }
      outfile << std::endl;
    }

    outfile.close();

    /* Provide a short summary to standard output */
    if (screenSummary) {
      cout << "\t" << pos.row(0) << endl;
      cout << "\t" << pos.row(1) << endl;
      cout << "\t" << pos.row(2) << endl;
      cout << "\t..." << endl;
      cout << "\t" << pos.row(shape(0)/2-3) << endl;
      cout << "\t" << pos.row(shape(0)/2-2) << endl;
      cout << "\t" << pos.row(shape(0)/2-1) << endl;
      cout << "\t..." << endl;
      cout << "\t" << pos.row(shape(0)-3) << endl;
      cout << "\t" << pos.row(shape(0)-2) << endl;
      cout << "\t" << pos.row(shape(0)-1) << endl;
    }
  }
  
  // ============================================================================
  //
  //  Module Imaging
  //
  // ============================================================================
  
  //_____________________________________________________________________________
  //                                                            test_antPositions

  void test_antPositions (casa::Matrix<double> &antPositions,
			  CR::CoordinateType::Types const &type,
			  double const &scale)
  {
    int nofAxes (3);

    switch (type) {
    case CR::CoordinateType::Cartesian:
      // adjust the size of the array returning the results
      antPositions.resize(9,nofAxes);
      antPositions = 0;
      //assign values
      antPositions(0,0) = -scale;  antPositions(0,1) = -scale;
      antPositions(1,0) = 0;       antPositions(1,1) = -scale;
      antPositions(2,0) =  scale;  antPositions(2,1) = -scale;
      antPositions(3,0) = -scale;  antPositions(3,1) = 0;
      antPositions(4,0) = 0;       antPositions(4,1) = 0;
      antPositions(5,0) =  scale;  antPositions(5,1) = 0;
      antPositions(6,0) = -scale;  antPositions(6,1) = scale;
      antPositions(7,0) = 0;       antPositions(7,1) = scale;
      antPositions(8,0) =  scale;  antPositions(8,1) = scale;
      break;
    case CR::CoordinateType::Spherical:
      // adjust the size of the array returning the results
      antPositions.resize(9,nofAxes);
      antPositions = 0;
      //assign values
      for (int phi(1); phi<9; phi++) {
	antPositions(phi,0) = scale;
	antPositions(phi,1) = (phi-1)*45.0;
      }
      break;
    case CR::CoordinateType::Cylindrical:
      // adjust the size of the array returning the results
      antPositions.resize(9,nofAxes);
      antPositions = 0;
      //assign values
      for (int phi(1); phi<9; phi++) {
	antPositions(phi,0) = scale;
	antPositions(phi,1) = (phi-1)*45.0;
      }
      break;
    case CR::CoordinateType::DirectionRadius:
      break;
    default:
      std::cout << "[test_antPositions] Unsupported coordinate type!" << std::endl;
      break;
    };
  }
  
  //_____________________________________________________________________________
  //                                                          test_getFrequencies

  casa::Vector<double> test_frequencyValues (uint const &blocksize,
					     double const &sampleFrequency,
					     uint const &nyquistZone)
  {
    CR::TimeFreq timefreq (blocksize,
			   sampleFrequency,
			   nyquistZone);
    return timefreq.frequencyValues();
  }
  
  //_____________________________________________________________________________
  //                                                                 test_getData

  void test_getData (casa::Matrix<casa::DComplex> &data,
		     uint const &nofAntennas,
		     uint const &nofChannels,
		     bool const &antennaIndexFirst)
  {
    // Indices
    uint antenna (0);
    uint channel (0);
    // Random number generator from CASA scimath/Mathematics module
    casa::ACG gen(10, 20);
    casa::Uniform random (&gen);
    
    if (antennaIndexFirst) {
      // set the shape of the array returning the data
      data.resize(nofAntennas,nofChannels,0.0);
      for (channel=0; channel<nofChannels; channel++) {
	for (antenna=0; antenna<nofAntennas; antenna++) {
	  data(antenna,channel) = DComplex(random(),random());
	}
      }
    } else {
      // set the shape of the array returning the data
      data.resize(nofChannels,nofAntennas,0.0);
      for (antenna=0; antenna<nofAntennas; antenna++) {
	for (channel=0; channel<nofChannels; channel++) {
	  data(channel,antenna) = DComplex(random(),random());
	}
      }
    }
  }
  
  //_____________________________________________________________________________
  //                                                                 test_getData
  
  void test_getData (Matrix<DComplex> &data,
		     casa::IPosition const &shape,
		     bool const &antennaIndexFirst)
  {
    uint nofAntennas;
    uint nofChannels;
    
    if (antennaIndexFirst) {
      nofAntennas = shape(0);
      nofChannels = shape(1);
    } else {
      nofAntennas = shape(1);
      nofChannels = shape(0);
    }

    test_getData (data,
		  nofAntennas,
		  nofChannels,
		  antennaIndexFirst);
  }
  
  //_____________________________________________________________________________
  //                                                         test_exportGeomDelay
  
  void test_exportGeomDelay (CR::GeomDelay &delay,
			     std::string const &prefix)
  {
    int ant;
//     int sky;
    std::ofstream outfile;

    std::cout << "-- Exporting the values of the antenna positions ..." << std::endl;
    {
      std::string filename        = prefix+"-antPositions.dat";
      Matrix<double> antPositions = delay.antPositions();
      IPosition shape             = antPositions.shape();
      int nCoord;
      
      outfile.open(filename.c_str(),std::ios::out);
      
      for (ant=0; ant<shape(0); ant++) {
	outfile << ant;
	for (nCoord=0; nCoord<shape(1); nCoord++) {
	  outfile << "\t" << antPositions(ant,nCoord);
	}
	outfile << std::endl;
      }
      
      outfile.close();
    }
  }

  //_____________________________________________________________________________
  //                                                        test_exportBeamformer
  
  void test_exportBeamformer (CR::Beamformer &bf,
			      std::string const &prefix)
  {
    std::cout << "[TestsCommon::export_Beamformer]" << std::endl;

    int ant;
    int sky;
    std::ofstream outfile;
    
    std::cout << "-- Exporting the values of the antenna positions ..." << std::endl;
    {
      std::string filename        = prefix+"-antPositions.dat";
      Matrix<double> antPositions = bf.antPositions();
      IPosition shape             = antPositions.shape();
      int nCoord;
      
      outfile.open(filename.c_str(),std::ios::out);
      
      for (ant=0; ant<shape(0); ant++) {
	outfile << ant;
	for (nCoord=0; nCoord<shape(1); nCoord++) {
	  outfile << "\t" << antPositions(ant,nCoord);
	}
	outfile << std::endl;
      }
      
      outfile.close();
    }
    
    std::cout << "-- Exporting the values of the sky positions ..." << std::endl;
    {
      std::string filename        = prefix+"-skyPositions.dat";
      Matrix<double> skyPositions = bf.skyPositions();
      IPosition shape             = skyPositions.shape();
      int nCoord;
      
      outfile.open(filename.c_str(),std::ios::out);
      
      for (sky=0; sky<shape(0); sky++) {
	outfile << sky;
	for (nCoord=0; nCoord<shape(1); nCoord++) {
	  outfile << "\t" << skyPositions(sky,nCoord);
	}
	outfile << std::endl;
      }
      
      outfile.close();
    }
    
    std::cout << "-- Exporting the values of the geometrical delays ..." << std::endl;
    {
      std::string filename  = prefix+"-delays.dat";
      Matrix<double> delays = bf.delays();
      IPosition shape       = delays.shape(); /* [ant,sky] */
      
      outfile.open(filename.c_str(),std::ios::out);
      
      for (ant=0; ant<shape(0); ant++) {
	for (sky=0; sky<shape(1); sky++) {
	  outfile << ant << "\t" << sky << "\t" << delays(ant,sky) << std::endl;
	}
	outfile << std::endl;
      }
      
      outfile.close();
    }
    
    /* Export the values of the geometrical phases */
  }  

  //_____________________________________________________________________________
  //                                                           test_showGeomDelay
  
  void test_showGeomDelay (CR::GeomDelay &delay,
			   int nofSkyPositions,
			   int nofDelays)
  {
    /* Display antenna positions */
    {
      Matrix<double> antPositions = delay.antPositions();
      IPosition shape = antPositions.shape();  // [ant,3]
      //
      cout << "-- Antenna positions :" << endl;
      for (int n(0); n<shape(0); n++) {
	cout << "\t" << antPositions.row(n) << endl;
      }
    }
    
    /* Display sky positions */
    {
      Matrix<double> skyPositions = delay.skyPositions();
      IPosition shape = skyPositions.shape();  // [sky,3]
      //
      if (nofSkyPositions > shape(0)) {
	nofSkyPositions = shape(0);
      }
      //
      cout << "-- Sky positions :" << endl;
      for (int n(0); n<nofSkyPositions; n++) {
	cout << "\t" << skyPositions.row(n) << endl;
      }
    }
    
    /* Display geometrical delays */
    {
      Matrix<double> delays = delay.delays();
      IPosition shape       = delays.shape();  // [ant,sky]
      //
      if (nofDelays > shape(1)) {
	nofDelays = shape(1);
      }
      //
      cout << "-- Values of the geometrical delays [,pos]:" << endl;
      for (int n(0); n<nofDelays; n++) {
	cout << "\t" << delays.column(n) << endl;
      }
    }
  }
  
  //_____________________________________________________________________________
  //                                                           test_showGeomPhase
  
  void test_showGeomPhase (CR::GeomPhase &phase,
			   int nofSkyPositions,
			   int nofDelays,
			   int nofPhases)
  {
    /* Display the parameters of the underlying GeomDelay object */
    test_showGeomDelay (phase,
			nofSkyPositions,
			nofDelays);
    
    /* Display frequency values */
    {
      Vector<double> freq = phase.frequencies();
      //
      cout << "-- Frequency values :" << endl;
      cout << "\t[" 
	   << freq(0) << " "
	   << freq(1) << " "
	   << freq(2) << " "
	   << freq(3) << " .. ]" << endl;
    }
    
    // display geometrical phases
    {
      Cube<double> phases = phase.phases();
      IPosition shape     = phases.shape();   // [freq,ant,sky]
      //
      if (nofSkyPositions > shape(2)) {
	nofSkyPositions = shape(2);
      }
      //
      cout << "-- Values of the geometrical phases [,,pos]:" << endl;
      for (int n(0); n<nofSkyPositions; n++) {
	cout << "\t[" << phases(1,0,n)
	     << ", "  << phases(1,1,n)
	     << ", "  << phases(1,2,n) 
	     << ", "  << phases(1,3,n)
	     << "]"   << endl;
      }
    }
    
  }

  //_____________________________________________________________________________
  //                                                          test_showGeomWeight
  
  void test_showGeomWeight (CR::GeomWeight &weight,
			    int nofSkyPositions,
			    int nofDelays,
			    int nofPhases)
  {
    /* Display the parameters of the underlying GeomPhase object */
    test_showGeomPhase (weight,
			nofSkyPositions,
			nofDelays,
			nofPhases);
    
    // display geometrical weights
    {
      Cube<casa::DComplex> weights = weight.weights();
      IPosition shape              = weights.shape();   // [freq,ant,sky]
      int freq                     = 1;
      //
      if (nofSkyPositions > shape(2)) {
	nofSkyPositions = shape(2);
      }
      //
      cout << "-- Values of the geometrical weights [,,pos]:" << endl;
      for (int sky(0); sky<nofSkyPositions; sky++) {
	cout << "\t[" << weights(freq,0,sky);
	for (int ant(1); ant<shape(1); ant++) {
	  cout << ", "  << weights(freq,ant,sky);
	}
	cout << "]"   << endl;
      }
    }
  }
  
} // Namespace CR -- end
