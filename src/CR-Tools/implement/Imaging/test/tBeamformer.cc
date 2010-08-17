/*-------------------------------------------------------------------------*
 | $Id::                                                                 $ |
 *-------------------------------------------------------------------------*
 ***************************************************************************
 *   Copyright (C) 2007                                                    *
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

#include <casa/BasicMath/Random.h>

#include <Coordinates/SkymapCoordinate.h>
#include <Coordinates/TimeFreq.h>
#include <Imaging/Beamformer.h>
#include <Utilities/TestsCommon.h>
#include "create_data.h"

using std::cerr;
using std::cout;
using std::endl;

using casa::DComplex;
using casa::Matrix;
using casa::Vector;

using CR::GeomDelay;
using CR::GeomPhase;
using CR::GeomWeight;
using CR::Beamformer;
using CR::SkymapQuantity;

/*!
  \file tBeamformer.cc

  \ingroup CR_Imaging

  \brief A collection of test routines for the Beamformer class
 
  \author Lars B&auml;hren
 
  \date 2007/06/13
*/

//_______________________________________________________________________________
//                                                                test_Beamformer

/*!
  \brief Test constructors for a new Beamformer object
  
  There are quite a number of ways in which a new Beamformer object can be
  created, most importantly using an object of on the the base classes.
  
  \return nofFailedTests -- The number of failed tests.
*/
int test_Beamformer (uint blocksize=1024,
		     uint fftLength=513)
{
  cout << "\n[tBeamformer::test_Beamformer]\n" << endl;

  int nofFailedTests (0);

  std::cout << "-- Blocksize  = " << blocksize << std::endl;
  std::cout << "-- FFT length = " << fftLength << std::endl;
  
  cout << "[1] Testing default constructor ..." << endl;
  try {
    Beamformer bf;
    bf.summary();
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }
  
  cout << "[2] Construction using (GeomWeight) ..." << endl;
  try {
    Vector<MVFrequency> frequencies (fftLength);
    CR::GeomPhase phase (frequencies,false);
    GeomWeight weight (phase);
    //
    Beamformer bf (weight);
    bf.summary();
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }
  
  cout << "[3] Construction using (GeomPhase,bool) ..." << endl;
  try {
    // parameters for GeomPhase
    Vector<MVFrequency> frequencies (fftLength);
    CR::GeomPhase phase (frequencies,false);
    //
    Beamformer bf (phase);
    bf.summary();
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }
  
  cout << "[4] Construction using (GeomDelay,Vector<double>,bool,bool) ..." << endl;
  try {
    // parameters for GeomDelay
    casa::Matrix<double> antPositions (2,3);
    casa::Matrix<double> skyPositions (10,3);
    bool anglesInDegrees (true);
    bool farField (false);
    bool bufferDelays (false);
    GeomDelay geomDelay (antPositions,
			 skyPositions,
			 anglesInDegrees,
			 farField,
			 bufferDelays);
    // parameters for GeomPhase
    Vector<double> frequencies (fftLength);
    bool bufferPhases (false);
    // parameters for GeomWeights
    bool bufferWeights (false);

    Beamformer bf (geomDelay,
		   frequencies,
		   bufferPhases,
		   bufferWeights);
    bf.summary(); 
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  cout << "[5] Construction using (GeomDelay,Vector<MVFrequency>,bool,bool) ..."
       << endl;
  try {
    // parameters for GeomDelay
    casa::Matrix<double> antPositions (2,3);
    casa::Matrix<double> skyPositions (10,3);
    bool anglesInDegrees (true);
    bool farField (false);
    bool bufferDelays (false);
    GeomDelay geomDelay (antPositions,
			 skyPositions,
			 anglesInDegrees,
			 farField,
			 bufferDelays);
    // parameters for GeomPhase
    Vector<MVFrequency> frequencies (fftLength);
    bool bufferPhases (false);
    // parameters for GeomWeights
    bool bufferWeights (false);

    Beamformer bf (geomDelay,
		   frequencies,
		   bufferPhases,
		   bufferWeights);
    bf.summary(); 
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  cout << "[6] Testing fully argumented constructor ..." << endl;
  try {
    // parameters for GeomDelay
    casa::Matrix<double> antPositions (2,3);
    casa::Matrix<double> skyPositions (10,3);
    bool anglesInDegrees (true);
    bool farField (false);
    bool bufferDelays (false);
    // parameters for GeomPhase
    Vector<double> frequencies (fftLength);
    bool bufferPhases (false);
    // parameters for GeomWeights
    bool bufferWeights (false);

    Beamformer bf (antPositions,
		   CR::CoordinateType::Cartesian,
		   skyPositions,
		   CR::CoordinateType::Cartesian,
		   anglesInDegrees,
		   farField,
		   bufferDelays,
		   frequencies,
		   bufferPhases,
		   bufferWeights);
    
    bf.summary();
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  cout << "[7] Testing fully argumented constructor ..." << endl;
  try {
    // parameters for GeomDelay
    casa::Vector<casa::MVPosition> antPositions (2);
    casa::Vector<casa::MVPosition> skyPositions (10);
    bool farField (false);
    bool bufferDelays (false);
    // parameters for GeomPhase
    casa::Vector<casa::MVFrequency> frequencies (fftLength);
    bool bufferPhases (false);
    // parameters for GeomWeights
    bool bufferWeights (false);
    //
    Beamformer bf (antPositions,
		   skyPositions,
		   farField,
		   bufferDelays,
		   frequencies,
		   bufferPhases,
		   bufferWeights);
    //
    bf.summary();
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }
  
  cout << "[8] Testing copy constructor ..." << endl;
  try {
    Vector<MVFrequency> frequencies (fftLength);
    CR::GeomPhase phase (frequencies,false);
    GeomWeight weight (phase);
    //
    Beamformer bf (weight);
    bf.setSkymapType (CR::SkymapQuantity::TIME_CC);
    cout << "--> original object:" << endl;
    bf.summary();
    //
    Beamformer bfCopy (bf);
    cout << "-> object constructed as copy:" << endl;
    bfCopy.summary();
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  return nofFailedTests;
}

//_______________________________________________________________________________
//                                                                test_parameters

/*!
  \brief Test the methods providing access to internal parameters

  \param blocksize -- The number of samples per input block of data

  \return nofFailedTests -- The number of failed test encountered within this
          function.
*/
int test_parameters (uint blocksize=1024)
{
  cout << "\n[tBeamformer::test_parameters]\n" << endl;

  int nofFailedTests (0);
  Beamformer bf;
  uint fftLength = (blocksize+1)/2;

  cout << "[1] Set antenna positions ..." << endl;
  try {
    uint nelem (10);
    Vector<MVPosition> antPositions (nelem);
    //
    cout << "-- old values = " << bf.antPositions().shape() << endl;
    cout << "-- new values = " << antPositions.shape()      << endl;
    //
    bf.setAntPositions (antPositions);
    //
    bf.summary();
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  cout << "[2] Set sky positions ..." << endl;
  try {
    uint nelem (50);
    Vector<MVPosition> skyPositions (nelem);
    //
    cout << "-- old values = " << bf.skyPositions().shape() << endl;
    cout << "-- new values = " << skyPositions.shape()      << endl;
    //
    bf.setSkyPositions (skyPositions);
    //
    bf.summary();
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  cout << "[3] Set frequency values ..." << endl;
  try {
    Vector<MVFrequency> freq (fftLength);
    //
    cout << "-- old values = " << bf.frequencies().shape() << endl;
    cout << "-- new values = " << freq.shape()             << endl;
    //
    bf.setFrequencies (freq);
    //
    bf.summary();
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  cout << "[4] Set the skymap quantity for which to form the beam ..." << endl;
  try {
    bool ok (true);

    cout << "-- Field in the frequency domain..." << endl;
    ok = bf.setSkymapType (SkymapQuantity::FREQ_FIELD);
    cout << "--> Beam domain name   = " << bf.domainName()        << endl;
    cout << "--> Beamforming method = " << bf.skymapType().name() << endl;
    
    cout << "-- Power in the frequency domain..." << endl;
    ok = bf.setSkymapType (SkymapQuantity::FREQ_POWER);
    cout << "--> Beam domain name   = " << bf.domainName()        << endl;
    cout << "--> Beamforming method = " << bf.skymapType().name() << endl;
    
    cout << "-- Field in the time domain..." << endl;
    ok = bf.setSkymapType (SkymapQuantity::TIME_FIELD);
    cout << "--> Beam domain name   = " << bf.domainName()        << endl;
    cout << "--> Beamforming method = " << bf.skymapType().name() << endl;
    
    cout << "-- Power in the time domain..." << endl;
    ok = bf.setSkymapType (SkymapQuantity::TIME_POWER);
    cout << "--> Beam domain name   = " << bf.domainName()        << endl;
    cout << "--> Beamforming method = " << bf.skymapType().name() << endl;
    
    cout << "-- cc-beam in the time domain..." << endl;
    ok = bf.setSkymapType (SkymapQuantity::TIME_CC);
    cout << "--> Beam domain name   = " << bf.domainName()        << endl;
    cout << "--> Beamforming method = " << bf.skymapType().name() << endl;
    
    cout << "-- Power-beam in the time domain..." << endl;
    ok = bf.setSkymapType (SkymapQuantity::TIME_P);
    cout << "--> Beam domain name   = " << bf.domainName()        << endl;
    cout << "--> Beamforming method = " << bf.skymapType().name() << endl;
    
    cout << "-- Excess-beam in the time domain..." << endl;
    ok = bf.setSkymapType (SkymapQuantity::TIME_X);
    cout << "--> Beam domain name   = " << bf.domainName()        << endl;
    cout << "--> Beamforming method = " << bf.skymapType().name() << endl;
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }
  
  return nofFailedTests;
}

//_______________________________________________________________________________
//                                                                   test_methods

/*!
  \brief Test the various public methods of the Beamformer class

  \return nofFailedTests -- The number of failed tests encountered within this
          function.
*/
int test_methods ()
{
  cout << "\n[tBeamformer::test_methods]\n" << endl;

  int nofFailedTests (0);
  Vector<casa::MVPosition> antPositions (3);
  Matrix<double> pos (2,3);
  Vector<double> freq (10);
  Beamformer bf;

  cout << "[1] Storing parameters to Beamformer object ..." << endl;
  try {
    pos(0,0) = 1;
    pos(0,1) = 90;
    pos(0,2) = 0;
    pos(1,0) = 1;
    pos(1,1) = 0;
    pos(1,2) = 90;
    cout << "-- setting sky positions ..." << endl;
    bf.setSkyPositions (pos,
			CR::CoordinateType::Spherical,
			true);
    //
    antPositions(0) = casa::MVPosition (100,0,0);
    antPositions(1) = casa::MVPosition (0,100,0);
    antPositions(2) = casa::MVPosition (100,100,0);
    cout << "-- setting antenna positions ..." << endl;
    bf.setAntPositions (antPositions);
    //
    for (uint n(0); n<freq.nelements(); n++) { freq(n) = n; } 
    cout << "-- setting frequency values ..." << endl;
    bf.setFrequencies(freq);
    // summary
    bf.summary();
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  cout << "[2] Retrieve geometrical delays ..." << endl;
  try {
    Matrix<double> delays;
    //
    delays = bf.delays();
    cout << "-- delays by return = " << delays << endl;
    //
    delays = 1;
    bf.delays(delays);
    cout << "-- delays by ref    = " << delays << endl;
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  cout << "[3] Retrieve geometrical phases ..." << endl;
  try {
    Cube<double> phases;
    //
    phases = bf.phases();
    cout << "-- phases by return = " << phases << endl;
    //
    phases = 1;
    bf.phases(phases);
    cout << "-- phases by ref    = " << phases << endl;
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  cout << "[4] Retrieve geometrical weights ..." << endl;
  try {
    Cube<casa::DComplex> weights;
    //
    weights = bf.weights();
    cout << "-- weights by return = " << weights << endl;
    //
    weights = 1;
    bf.weights(weights);
    cout << "-- weights by ref    = " << weights << endl;
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  cout << "[5] Retrieve Beamfomer weights ..." << endl;
  try {
    Cube<casa::DComplex> weights;
    //
    weights = bf.beamformerWeights();
    cout << "-- weights by return = " << weights << endl;
    //
    weights = 1;
    bf.beamformerWeights(weights);
    cout << "-- weights by ref    = " << weights << endl;
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  return nofFailedTests;
}

//_______________________________________________________________________________
//                                                                   test_weights

/*!
  \brief Test modification of the Beamformer weights

  \param blocksize -- The number of samples per input block of data
  \param fftLength -- Output size of the FFT.

  \return nofFailedTests -- The number of failed tests.
*/
int test_weights (uint blocksize=1024)
{
  cout << "\n[tBeamformer::test_weights]\n" << endl;

  int nofFailedTests (0);

  /* Create beamformer object to work with */
  CR::Beamformer bf (get_antennaPositions(),
		     CR::CoordinateType::Cartesian,
		     get_skyPositions(),
		     CR::CoordinateType::Cartesian,
		     false,
		     false,
		     false,
		     CR::test_frequencyValues(),
		     false);

  std::cout << "-- blocksize = " << blocksize << std::endl;

  /* Work with default settings (geometrical weights only) */
  try {
    bf.summary();
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  /* Redo computation after setting vales for the antenna gains */
  try {
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  /* Unset effect of the antenna gains, reverting to geometrical weights only */
  try {
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  return nofFailedTests;
}

//_______________________________________________________________________________
//                                                               test_coordinates

/*!
  \brief Test extraction of parameters from the SkymapCoordinate class

  In the case of the Skymapper class, where a Beamformer object is available
  only as an internal element, all the configuration in terms of coordinates
  is done through an instance of the SkymapCoordinate class.

  \return nofFailedTests -- The number of failed tests encountered within this
          function.
*/
int test_coordinates (uint blocksize=512)
{
  cout << "\n[tBeamformer::test_coordinates]\n" << endl;

  int nofFailedTests (0);

  /* Define antenna positions */
  uint nofAntennas (4);
  Vector<casa::MVPosition> antPositions (nofAntennas);
  antPositions(0) = casa::MVPosition (100,0,0);
  antPositions(1) = casa::MVPosition (0,100,0);
  antPositions(2) = casa::MVPosition (-100,0,0);
  antPositions(3) = casa::MVPosition (0,-100,0);

  /* Define frequency values */
  double sampleFreq (200e06);
  uint nyquistZone (1);
  CR::TimeFreq tf (blocksize,
		   sampleFreq,
		   nyquistZone);
  
  /* Define data input */
  Matrix<casa::DComplex> data;
   CR::test_getData (data,nofAntennas,tf.fftLength(),false);

  cout << "[1] Geometrical delays for default SpatialCoordinate ..." << endl;
  try {
    // create coordinate object
    SpatialCoordinate coord;
    Vector<double> refValue = coord.referenceValue();
    refValue(2) = 1000;
    coord.setReferenceValue(refValue);
    // retrieve position values
    Matrix<double> positions = coord.worldAxisValues();
    // Create object to hold geometrical phases
    GeomDelay delay;
    delay.farField(true);
    delay.setAntPositions (antPositions);
    delay.setSkyPositions (positions,
			   CoordinateType::DirectionRadius,
			   false);
    //
    CR::test_showGeomDelay(delay);
  } catch (std::string message) {
    std::cerr << message << std::endl;
    nofFailedTests++;
  }
  
  cout << "[2] Geometrical phases for default SpatialCoordinate ..." << endl;
  try {
    // create coordinate object
    SpatialCoordinate coord;
    Vector<double> refValue = coord.referenceValue();
    refValue(2) = 1000;
    coord.setReferenceValue(refValue);
    // retrieve position values
    Matrix<double> positions = coord.worldAxisValues();
    // Create object to hold geometrical phases
    GeomPhase phase;
    phase.farField(true);
    phase.setAntPositions (antPositions);
    phase.setSkyPositions (positions,
			   CoordinateType::DirectionRadius,
			   false);
    phase.setFrequencies (tf.frequencyValues());
    //
    CR::test_showGeomPhase(phase,4,4);
  } catch (std::string message) {
    std::cerr << message << std::endl;
    nofFailedTests++;
  }
  
  cout << "[3] Geometrical weights for default SpatialCoordinate ..." << endl;
  try {
    // create coordinate object
    SpatialCoordinate coord;
    Vector<double> refValue = coord.referenceValue();
    refValue(2) = 1000;
    coord.setReferenceValue(refValue);
    // retrieve position values
    Matrix<double> positions = coord.worldAxisValues();
    // Create object to hold geometrical phases
    Beamformer bf;
    bf.setAntPositions (antPositions);
    bf.setSkyPositions (positions,
			   CoordinateType::DirectionRadius,
			   false);
    bf.setFrequencies (tf.frequencyValues());
    bf.farField (true);
    bf.summary();
    // display internal parameters
    CR::test_showGeomWeight (bf);
    // process block of data
    {
      Matrix<double> beam (bf.shapeBeam());
      //
      cout << "-- Processing block of input data ... " << endl;
      bf.processData (beam,data);
      cout << "-- Data values " << data.shape() << " :" << endl;
      cout << "\t[,0]\t" << data.column(0) << endl;
      cout << "\t[,1]\t" << data.column(1) << endl;
      cout << "\t[,2]\t" << data.column(2) << endl;
      cout << "-- Beamformed data " << beam.shape() << " :" << endl;
      cout << "\t[,0]\t" << beam.column(0) << endl;
      cout << "\t[,1]\t" << beam.column(1) << endl;
      cout << "\t[,2]\t" << beam.column(2) << endl;
    }
  } catch (std::string message) {
    std::cerr << message << std::endl;
    nofFailedTests++;
  }

  return nofFailedTests;
}

//_______________________________________________________________________________
//                                                                test_processing

/*!
  \brief Test processing of incoming data

  Test the <tt>processData</tt> routines, which is responsible for converting
  incoming antennas (in the frequency domain) into beamformed data.

  \param blocksize -- The number of samples per input block of data

  \return nofFailedTests -- The number of failed tests encountered within this
          function.
*/
int test_processing (uint blocksize=1024)
{
  cout << "\n[tBeamformer::test_processing]\n" << endl;

  int nofFailedTests (0);
  bool status (true);
  // uint fftLength = (blocksize+1)/2;
  
  // parameters for GeomDelay
  uint nofAntennas (4);
  Vector<casa::MVPosition> antPositions (nofAntennas);
  antPositions(0) = casa::MVPosition (100,0,0);
  antPositions(1) = casa::MVPosition (0,100,0);
  antPositions(2) = casa::MVPosition (-100,0,0);
  antPositions(3) = casa::MVPosition (0,-100,0);
  Vector<casa::MVPosition> skyPositions (5);
  skyPositions(0) = casa::MVPosition (1000,0,0);
  skyPositions(1) = casa::MVPosition (1000,1000,0);
  skyPositions(2) = casa::MVPosition (1000,1000,1000);
  skyPositions(3) = casa::MVPosition (0,1000,0);
  skyPositions(4) = casa::MVPosition (0,1000,1000);
  GeomDelay delay (antPositions,skyPositions,false,false);
  // parameters for GeomPhase
  CR::TimeFreq timeFreq (blocksize,200e06,2);
  Vector<double> frequencies = timeFreq.frequencyValues();
  bool bufferPhases (false);
  // parameters for GeomWeights
  bool bufferWeights (false);
  // Create beamformer
  Beamformer bf (delay,
		 frequencies,
		 bufferPhases,
		 bufferWeights);
  bf.summary();
  
  /* Some data to test the processing */
  Matrix<DComplex> data (timeFreq.fftLength(),nofAntennas);
  for (uint n(0); n<nofAntennas; n++) {
    for (uint k(0); k<timeFreq.fftLength(); k++) {
      data(k,n) = casa::DComplex(frequencies(k),0);
    }
  }
  /* Array to store the beamformed data */
  Matrix<double> beam;

  cout << "[1] Power in the frequency domain (FREQ_POWER)" << endl;
  try {
    bf.setSkymapType(SkymapQuantity::FREQ_POWER);
    beam.resize(bf.shapeBeam());
    status = bf.processData (beam,data);
#ifdef DEBUGGING_MESSAGES
    CR::test_showGeomPhase (bf);
    cout << "-- Beamformed data [,0] = " << beam.column(0)    << endl;
#endif
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }
  
  cout << "[2] cc-beam in the time domain (TIME_CC)" << endl;
  try {
    bf.setSkymapType(SkymapQuantity::TIME_CC);
    beam.resize(bf.shapeBeam());
    status = bf.processData (beam,data);
#ifdef DEBUGGING_MESSAGES
    cout << "-- Antenna positions = " << bf.antPositions() << endl;
    cout << "-- Sky positions     = " << bf.skyPositions() << endl;
    cout << "-- Frequency values  = " << bf.frequencies()  << endl;
    cout << "-- Beamformed data   = " << beam              << endl;
#endif
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }
  
  cout << "[3] powerbeam in the time domain (TIME_P)" << endl;
  try {
    bf.setSkymapType(SkymapQuantity::TIME_P);
    beam.resize(bf.shapeBeam());
    status = bf.processData (beam,data);
#ifdef DEBUGGING_MESSAGES
    cout << "-- Antenna positions = " << bf.antPositions() << endl;
    cout << "-- Sky positions     = " << bf.skyPositions() << endl;
    cout << "-- Frequency values  = " << bf.frequencies()  << endl;
    cout << "-- Beamformed data   = " << beam              << endl;
#endif
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }
  
  return nofFailedTests;
}

//_______________________________________________________________________________
//                                                                           main

int main ()
{
  int nofFailedTests (0);

  // Test for the constructor(s)
  nofFailedTests += test_Beamformer ();
  
  if (!nofFailedTests) {
    nofFailedTests += test_parameters();
    nofFailedTests += test_methods();
    nofFailedTests += test_coordinates();
    nofFailedTests += test_processing ();
  }
  
  return nofFailedTests;
}
