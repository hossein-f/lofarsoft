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

#include <Imaging/GeometricalWeight.h>
#include <Utilities/ProgressBar.h>

namespace CR { // NAMESPACE CR -- BEGIN
  
  // ============================================================================
  //
  //  Construction
  //
  // ============================================================================
  
  GeometricalWeight::GeometricalWeight ()
    : GeometricalPhase (),
      bufferWeights_p (false)
  {
    // nothing else to do here, since we do not buffer the weights by default
  }
  
#ifdef HAVE_CASA
  GeometricalWeight::GeometricalWeight (casa::Matrix<double> const &antPositions,
					casa::Matrix<double> const &skyPositions,
					casa::Vector<double> const &frequencies,
					bool const &bufferDelays,
					bool const &bufferPhases,
					bool const &bufferWeights)
    : GeometricalPhase (antPositions,
			skyPositions,
			frequencies,
			bufferDelays,
			bufferPhases),
      bufferWeights_p (bufferWeights)
  {
    if (bufferWeights) {
      setWeights();
    }
  }
#else
#ifdef HAVE_BLITZ
  GeometricalWeight::GeometricalWeight (const blitz::Array<double,2> &antPositions,
					const blitz::Array<double,2> &skyPositions,
					blitz::Array<double,1> const &frequencies,
					bool const &bufferDelays,
					bool const &bufferPhases,
					bool const &bufferWeights)
    : GeometricalPhase (antPositions,
			skyPositions,
			frequencies,
			bufferDelays,
			bufferPhases),
      bufferWeights_p (bufferWeights)
  {
    if (bufferWeights) {
      setWeights();
    }
  }
#endif
#endif

  GeometricalWeight::GeometricalWeight (GeometricalWeight const &other)
  {
    copy (other);
  }
  
  // ============================================================================
  //
  //  Destruction
  //
  // ============================================================================
  
  GeometricalWeight::~GeometricalWeight ()
  {
    destroy();
  }
  
  void GeometricalWeight::destroy ()
  {;}
  
  // ============================================================================
  //
  //  Operators
  //
  // ============================================================================
  
  GeometricalWeight& GeometricalWeight::operator= (GeometricalWeight const &other)
  {
    if (this != &other) {
      destroy ();
      copy (other);
    }
    return *this;
  }
  
  void GeometricalWeight::copy (GeometricalWeight const &other)
  {
    // copy the underlying base object
    GeometricalPhase::operator= (other);

    bufferWeights_p = other.bufferWeights_p;

    if (bufferWeights_p) {
      weights_p.resize(other.weights_p.shape());
      weights_p = other.weights_p;
    }
  }

  // ============================================================================
  //
  //  Parameters
  //
  // ============================================================================
  
#ifdef HAVE_CASA
  casa::Cube<DComplex> GeometricalWeight::weights ()
  {
    if (bufferWeights_p) {
      std::cout << "-- Returning buffered weights..." << std::endl;
      return weights_p;
    } else {
      std::cout << "-- Returning recomputed weights..." << std::endl;
      return calcWeights();
    }
  }
#else
#ifdef HAVE_BLITZ
  blitz::Array<complex<double>,3> GeometricalWeight::weights ()
  {
    if (bufferWeights_p) {
      std::cout << "-- Returning buffered weights..." << std::endl;
      return weights_p;
    } else {
      std::cout << "-- Returning recomputed weights..." << std::endl;
      return calcWeights();
    }
  }
#endif
#endif  
  
  void GeometricalWeight::summary (std::ostream &os)
  {
    os << "[GeometricalWeight] Summary of object"             << std::endl;
    os << "-- Sky positions     : " << skyPositions_p.shape() << std::endl;
    os << "-- Antenna positions : " << antPositions_p.shape() << std::endl;
    os << "-- Frequency values  : " << frequencies_p.shape()  << std::endl;
    os << "-- buffer delays     : " << bufferDelays_p         << std::endl;
    os << "-- buffer phases     : " << bufferPhases_p         << std::endl;
    os << "-- buffer weights    : " << bufferWeights_p        << std::endl;
  }
  
  // ============================================================================
  //
  //  Methods reimplemented from base classes
  //
  // ============================================================================

  // ------------------------------------------------------------- setFrequencies

#ifdef HAVE_CASA
  bool GeometricalWeight::setFrequencies (const casa::Vector<double> &frequencies,
					  bool const &bufferPhases)
  {
    bool ok (true);
    
    // forward the input parameters to the base classes
    GeometricalPhase::setFrequencies (frequencies,
				      bufferPhases);
    
    // if necessary updated the buffered values for the weights
    setWeights ();
    
    return ok;
  }
#else
#ifdef HAVE_BLITZ
  bool GeometricalWeight::setFrequencies (const blitz::Array<double,1> &frequencies,
					  bool const &bufferPhases)
  {
    bool ok (true);
    
    return ok;
  }
#endif
#endif

  // ============================================================================
  //
  //  Methods
  //
  // ============================================================================
  
  // ----------------------------------------------------------------- setWeights

  void GeometricalWeight::setWeights ()
  {
    if (bufferWeights_p) {
      IPosition shape (3);

      shape(0) = GeometricalDelay::nofAntennaPositions();
      shape(1) = GeometricalDelay::nofSkyPositions();
      shape(2) = GeometricalPhase::nofFrequencies();

      // adjust the shape of the array storing the weights
      weights_p.resize (shape);
      // compute and assign the new values
      weights_p = calcWeights ();
    }
  }

  // ---------------------------------------------------------------- calcWeights

#ifdef HAVE_CASA
  casa::Cube<DComplex> GeometricalWeight::calcWeights ()
  {
    int ant (0);
    int sky (0);
    int freq (0);
    // get the geometrical phases
    casa::Cube<double> phases = GeometricalPhase::phases();
    casa::IPosition shape     = phases.shape();
    // array to store the computed weights
    casa::Cube<DComplex> weights (shape);
    // progress bar
    CR::ProgressBar progress (shape(2)-1);

    std::cout << "[GeometricalWeight::calcWeights]" << std::endl;

    for (freq=0; freq<shape(2); freq++) {
      for (sky=0; sky<shape(1); sky++) {
 	for (ant=0; ant<shape(0); ant++) {
	  weights(ant,sky,freq) = DComplex(cos(phases(ant,sky,freq)),
					   sin(phases(ant,sky,freq)));
 	}  // end loop: ant
      }  // end loop: sky
      progress.update(freq);
    }  // end loop: freq

    std::cout << " --> Computation of weights completed." << std::endl;
    
    return weights;
  }
#else
#ifdef HAVE_BLITZ
  blitz::Array<complex<double>,3> GeometricalWeight::calcWeights ()
  { 
    int ant (0);
    int sky (0);
    int freq (0);
    // get the geometrical phases
    blitz::Array<double,3> phases = GeometricalPhase::phases();
    blitz::TinyVector shape       = phases.shape();
    // array to store the computed weights
    blitz::Array<complex<double>,3> weights (phases.shape());
    
    for (freq=0; freq<shape(2); freq++) {
      for (sky=0; sky<shape(1); sky++) {
 	for (ant=0; ant<shape(0); ant++) {
	  weights(ant,sky,freq) = DComplex(cos(phases(ant,sky,freq)),
					   sin(phases(ant,sky,freq)));
 	}
      }
    }
    
    return weights;
  }
#endif
#endif
  
  
} // NAMESPACE CR -- END
