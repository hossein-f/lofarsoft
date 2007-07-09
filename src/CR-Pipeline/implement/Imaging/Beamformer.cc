/*-------------------------------------------------------------------------*
 | $Id$ |
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

// CASA header files
#ifdef HAVE_CASA
#include <scimath/Mathematics.h>
#include <scimath/Mathematics/FFTServer.h>
using casa::IPosition;
using casa::FFTServer;
using casa::Matrix;
using casa::Vector;
#endif
// Custom header files
#include <Imaging/Beamformer.h>

namespace CR { // Namespace CR -- begin
  
  // ============================================================================
  //
  //  Construction
  //
  // ============================================================================

  // ----------------------------------------------------------------- Beamformer
  
  Beamformer::Beamformer ()
    : GeometricalWeight()
  {
    init ();
  }
  
  // ----------------------------------------------------------------- Beamformer
  
#ifdef HAVE_CASA
  Beamformer::Beamformer (Matrix<double> const &antPositions,
			  Matrix<double> const &skyPositions,
			  Vector<double> const &frequencies,
			  bool const &bufferDelays,
			  bool const &bufferPhases,
			  bool const &bufferWeights)
    : GeometricalWeight(antPositions,
			skyPositions,
			frequencies,
			bufferDelays,
			bufferPhases,
			bufferWeights)
  {
    init ();
  }
#else
#ifdef HAVE_BLITZ
  Beamformer::Beamformer (const blitz::Array<double,2> &antPositions,
			  const blitz::Array<double,2> &skyPositions,
			  blitz::Array<double,1> const &frequencies,
			  bool const &bufferDelays,
			  bool const &bufferPhases,
			  bool const &bufferWeights)
    : GeometricalWeight(antPositions,
			skyPositions,
			frequencies,
			bufferDelays,
			bufferPhases,
			bufferWeights)
  {
    init ();
  }
#endif
#endif
  
  Beamformer::Beamformer (Beamformer const &other)
  {
    copy (other);
  }
  
  // ============================================================================
  //
  //  Destruction
  //
  // ============================================================================
  
  Beamformer::~Beamformer ()
  {
    destroy();
  }
  
  void Beamformer::destroy ()
  {;}
  
  // ============================================================================
  //
  //  Operators
  //
  // ============================================================================
  
  Beamformer& Beamformer::operator= (Beamformer const &other)
  {
    if (this != &other) {
      destroy ();
      copy (other);
    }
    return *this;
  }
  
  void Beamformer::copy (Beamformer const &other)
  {;}

  // ============================================================================
  //
  //  Parameters
  //
  // ============================================================================

  // ----------------------------------------------------------------------- init

  void Beamformer::init ()
  {
    bool status (true);
    // Activate the buffering of the weighting factors
    bufferWeights_p = true;
    // default setting for the used beamforming method
    status = setBeamType (FREQ_POWER);
  }
  
  // ---------------------------------------------------------------- setBeamType

  bool Beamformer::setBeamType (BeamType const &beam)
  {
    bool status (true);
    
    switch (beam) {
    case FREQ_FIELD:
      std::cerr << "[Beamformer::setBeamType] FREQ_FIELD not yet supported!"
		<< std::endl;
      status = false;
      break;
    case FREQ_POWER:
      beamType_p    = beam;
      processData_p = &Beamformer::freq_power;
      break;
    case TIME_FIELD:
      std::cerr << "[Beamformer::setBeamType] TIME_FIELD not yet supported!"
		<< std::endl;
      status = false;
      break;
    case TIME_POWER:
      beamType_p    = beam;
      processData_p = &Beamformer::time_power;
      break;
    case TIME_CC:
      beamType_p    = beam;
      processData_p = &Beamformer::time_cc;
      break;
    case TIME_X:
      beamType_p    = beam;
      processData_p = &Beamformer::time_x;
      break;
    }
    
    return status;
  }

  // --------------------------------------------------------------- beamTypeName

  std::string Beamformer::beamTypeName ()
  {
    return beamTypeName(beamType_p);
  }

  // ------------------------------------------------------------------- beamType

  bool Beamformer::beamType (BeamType &beamType,
			     string const &domain,
			     string const &quantity)
  {
    bool ok (true);

    if (domain == "time" || domain == "Time" || domain == "TIME") {
      if (quantity == "field" || quantity == "Field" || quantity == "FIELD") {
	beamType = TIME_FIELD;
      } else if (quantity == "power" || quantity == "Power" || quantity == "POWER") {
	beamType = TIME_POWER;
      } else if (quantity == "cc" || quantity == "CC") {
	beamType = TIME_CC;
      } else if (quantity == "x" || quantity == "X") {
	beamType = TIME_X;
      } else {
	std::cerr << "[Beamformer::beamType] Unknown signal quantity "
		  << quantity << std::endl;
	ok = false;
      }
    } else if (domain == "freq" || domain == "Freq" || domain == "FREQ") {
      if (quantity == "field" || quantity == "Field" || quantity == "FIELD") {
	beamType = FREQ_FIELD;
      } else if (quantity == "power" || quantity == "Power" || quantity == "POWER") {
	beamType = FREQ_POWER;
      } else {
	std::cerr << "[SkymapCoordinates::setMapQuantity] Unknown signal quantity "
		  << quantity << std::endl;
	ok = false;
      }
    } else {
      std::cerr << "[SkymapCoordinates::setMapQuantity] Unknown signal domain "
		<< domain << std::endl;
      ok = false;
    }

    return ok;
  }

  void Beamformer::summary (std::ostream &os)
  {
    os << "[Beamformer] Summary of object"                     << std::endl;
    os << "-- Sky positions      : " << skyPositions_p.shape() << std::endl;
    os << "-- Antenna positions  : " << antPositions_p.shape() << std::endl;
    os << "-- Frequency values   : " << frequencies_p.shape()  << std::endl;
    os << "-- Buffer delays?     : " << bufferDelays_p         << std::endl;
    os << "-- Buffer phases?     : " << bufferPhases_p         << std::endl;
    os << "-- Buffer weights?    : " << bufferWeights_p        << std::endl;
    os << "-- Beamforming method : " << beamType_p
       << " / " << beamTypeName() << std::endl;
  }
  
  // ============================================================================
  //
  //  Methods
  //
  // ============================================================================

  // ----------------------------------------------------------------- freq_field
  
#ifdef HAVE_CASA
  bool Beamformer::freq_field (Matrix<DComplex> &beam,
			       const Matrix<DComplex> &data)
  {
    bool status (true);
    int nofSkyPositions (skyPositions_p.nrow());
    int nofFrequencies (frequencies_p.nelements());
    IPosition shapeData (data.shape());

    /*
      Check if the shape of the array with the input data matched the internal
      parameters.
    */
    if (shapeData(0) == nofSkyPositions &&
	shapeData(1) == nofFrequencies) {
      // additional local variables
      int direction (0);
      uint antenna (0);
      int freq (0);
      // resize array returning the beamformed data
      beam.resize (nofSkyPositions,nofFrequencies,0.0);
      /*
	Compute the beams for all combinations of sky positions and frequency
	values. We unfortunately need the innermost loop over the frequencies,
	since the CASA Array module appears not to support the "+=" operator
	for sub-arrays (e.g. to process all values along the frequency axis).
      */
      for (direction=0; direction<nofSkyPositions; direction++) {
	for (antenna=0; antenna<nofAntennas_p; antenna++) {
	  for (freq=0; freq<nofFrequencies; freq++) {
	    beam(direction,freq) = data(antenna,freq)*weights_p(antenna,direction,freq);
	  }
	}
      }
    } else {
      std::cerr << "[Beamformer::freq_field]" << std::endl;
      std::cerr << "-- Wrong shape of array with input data!" << std::endl;
      status = false;
    }

    return status;
  }
#else
#ifdef HAVE_BLITZ
  bool Beamformer::freq_field (blitz::Array<complex<double>,2> &beam,
			       const blitz::Array<complex<double>,2> &data)
  {
    bool status (true);

    return status;
  }
#endif
#endif
  
  // ----------------------------------------------------------------- freq_power

#ifdef HAVE_CASA
  bool Beamformer::freq_power (Matrix<double> &beam,
			       const Matrix<DComplex> &data)
  {
    bool status (true);
    int nofSkyPositions (skyPositions_p.nrow());
    int nofFrequencies (frequencies_p.nelements());
    IPosition shapeData (data.shape());

    /*
      Check if the shape of the array with the input data matched the internal
      parameters.
    */
    if (shapeData(0) == nofSkyPositions &&
	shapeData(1) == nofFrequencies) {
      // additional local variables
      int direction (0);
      uint antenna (0);
      int freq (0);
      casa::DComplex tmp;
      // resize array returning the beamformed data
      beam.resize (nofSkyPositions,nofFrequencies,0.0);
      /*
	Compute the beams for all combinations of sky positions and frequency
	values.
      */
      for (direction=0; direction<nofSkyPositions; direction++) {
	for (antenna=0; antenna<nofAntennas_p; antenna++) {
	  for (freq=0; freq<nofFrequencies; freq++) {
	    tmp = data(antenna,freq)*weights_p(antenna,direction,freq);
	    beam(direction,freq) += real(tmp*conj(tmp));
	  }
	}
      }
    } else {
      std::cerr << "[Beamformer::freq_power]" << std::endl;
      std::cerr << "-- Wrong shape of array with input data!" << std::endl;
      status = false;
    }

    return status;
  }
#else
#ifdef HAVE_BLITZ
  bool Beamformer::freq_power (blitz::Array<double,2> &beam,
			       const blitz::Array<complex<double>,2> &data)
  {
    bool status (true);
    
    return status;
  }
#endif
#endif
  
  // ----------------------------------------------------------------- time_power

#ifdef HAVE_CASA
  bool Beamformer::time_power (Matrix<double> &beam,
			       const Matrix<DComplex> &data)
  {
    bool status (true);

    return status;
  }
#else
#ifdef HAVE_BLITZ
  bool Beamformer::time_power (blitz::Array<double,2> &beam,
			       const blitz::Array<complex<double>,2> &data)
  {
    bool status (true);
    int nofSkyPositions (skyPositions_p.nrow());
    int nofFrequencies (frequencies_p.nelements());
    IPosition shapeData (data.shape());

    /*
      Check if the shape of the array with the input data matched the internal
      parameters.
    */
    if (shapeData(0) == nofSkyPositions &&
	shapeData(1) == nofFrequencies) {
      // additional local variables
      int direction (0);
      uint antenna (0);
      int freq (0);
      int blocksize (2*(nofFrequencies-1));
      Vector<DComplex> vectFreq (nofFrequencies);

      for (direction=0; direction<nofSkyPositions; direction++) {
	for (antenna=0; antenna<nofAntennas_p; antenna++) {
	  // (1) Assemble the beamformed spectrum
	  for (freq=0; freq<nofFrequencies; freq++) {
	    vectFreq(freq) = data(antenna,freq)*weights_p(antenna,direction,freq);
	  }
	  // (2) Inverse Fourier transform back to the time domain
	  // (3) Assemble power time-series
	}
      }

    } else {
      std::cerr << "[Beamformer::time_power]" << std::endl;
      std::cerr << "-- Wrong shape of array with input data!" << std::endl;
      status = false;
    }

    return status;
  }
#endif
#endif

  // -------------------------------------------------------------------- time_cc

#ifdef HAVE_CASA
  bool Beamformer::time_cc (Matrix<double> &beam,
			    const Matrix<DComplex> &data)
  {
    bool status (true);
    int nofSkyPositions (skyPositions_p.nrow());
    int nofFrequencies (frequencies_p.nelements());
    IPosition shapeData (data.shape());
    
    /*
      Check if the shape of the array with the input data matched the internal
      parameters.
    */
    if (shapeData(0) == nofSkyPositions &&
	shapeData(1) == nofFrequencies) {
      // additional local variables
      int nofBaselines (GeometricalDelay::nofBaselines());
      int blocksize (2*(nofFrequencies-1));
      int direction (0);
      uint antenna1 (0);
      uint antenna2 (0);
      int n (0);
      Vector<DComplex> tmp1Freq (nofFrequencies);
      Vector<DComplex> tmp2Freq (nofFrequencies);
      Vector<double>   tmp1Time (blocksize);
      Vector<double>   tmp2Time (blocksize);
      /*
	Set up the casa::FFTServer which is t handle the inverse Fourier
	transform taking place before the summation step.
      */
      FFTServer<double,std::complex<double> > server(IPosition(1,blocksize),
						     casa::FFTEnums::REALTOCOMPLEX);
      // resize array returning the beamformed data
      beam.resize (nofSkyPositions,blocksize,0.0);
      /*
	Compute the beams for all combinations of sky positions and frequency
	values.
      */
      for (direction=0; direction<nofSkyPositions; direction++) {
	for (antenna1=0; antenna1<nofAntennas_p; antenna1++) {
	  // Shift the data for antenna 1 ... 
	  for (n=0; n<nofFrequencies; n++) {
	    tmp1Freq(n) = data(antenna1,n)*weights_p(antenna1,direction,n);
	  }
	  // ... and apply inverse Fourier transform
	  server.fft (tmp1Time,tmp1Freq);
	  for (antenna2=antenna1+1; antenna2<nofAntennas_p; antenna2++) {
	    // Shift the data for antenna 2 ... 
	    for (n=0; n<nofFrequencies; n++) {
	      tmp2Freq(n) = data(antenna2,n)*weights_p(antenna2,direction,n);
	    }
	    // ... and apply inverse Fourier transform
	    server.fft (tmp2Time,tmp2Freq);
	    /*
	      With shifted time-series avaiable for both antennas, we can
	      assemble the beam
	    */
	    beam.row(direction) = tmp1Time*tmp2Time;
	  }  // end loop: antenn2
	}  // end loop: antenna1
	// normalization w.r.t. the number of baselines
	for (n=0; n<blocksize; n++) {
	  beam (direction,n) /= nofBaselines;
	}
	//
	beam.row(direction) = CR::sign(beam.row(direction))*sqrt(abs(beam.row(direction)));
      }
    } else {
      std::cerr << "[Beamformer::time_cc]"                    << std::endl;
      std::cerr << "-- Wrong shape of array with input data!" << std::endl;
      status = false;
    }

    return status;
  }
#else
#ifdef HAVE_BLITZ
  bool Beamformer::time_cc (blitz::Array<double,2> &beam,
			    const blitz::Array<conplex<double>,2> &data)
  {
    bool status (true);
    
    return status;
  }
#endif
#endif
  
  // --------------------------------------------------------------------- time_x
  
#ifdef HAVE_CASA
  bool Beamformer::time_x (Matrix<double> &beam,
			   const Matrix<DComplex> &data)
  {
    bool status (true);
    
    return status;
  }
#else
#ifdef HAVE_BLITZ
  bool Beamformer::time_x (blitz::Array<double,2> &beam,
			   const blitz::Array<conplex<double>,2> &data)
  {
    bool status (true);
    
    return status;
  }
#endif
#endif
  
} // Namespace CR -- end
