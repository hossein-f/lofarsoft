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

#include <iostream>
#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <Imaging/Skymapper.h>
#include <Skymap/SkymapperTools.h>
#include <Utilities/ProgressBar.h>

using std::cerr;
using std::cout;
using std::endl;

namespace CR {  // Namespace CR -- begin
  
  // ============================================================================
  //
  //  Construction
  //
  // ============================================================================
  
  // ------------------------------------------------------------------ Skymapper
  
  Skymapper::Skymapper ()
  {
    init (0,
	  false,
	  0,
	  "skymap.img",
	  SkymapCoordinates(),
	  Beamformer());
  }
  
  // ------------------------------------------------------------------ Skymapper
  
  Skymapper::Skymapper (SkymapCoordinates const &coordinates)
  {
    init (0,
	  false,
	  0,
	  "skymap.img",
	  coordinates,
	  Beamformer());
  }
  
  // ------------------------------------------------------------------ Skymapper
  
  Skymapper::Skymapper (SkymapCoordinates const &coordinates,
			Beamformer const &beamformer)
  {
    init (0,
	  false,
	  0,
	  "skymap.img",
	  coordinates,
	  beamformer);
  }

  // ------------------------------------------------------------------ Skymapper
  
  Skymapper::Skymapper (Skymapper const& other)
  {
    copy (other);
  }
  
  // ============================================================================
  //
  //  Destruction
  //
  // ============================================================================
  
  Skymapper::~Skymapper ()
  {
    destroy();
  }
  
  // ============================================================================
  //
  //  Operators
  //
  // ============================================================================
  
  // ------------------------------------------------------------------ operator=
  
  Skymapper &Skymapper::operator= (Skymapper const &other)
  {
    if (this != &other) {
      destroy ();
      copy (other);
    }
    return *this;
  }
  
  // ----------------------------------------------------------------------- copy
  
  void Skymapper::copy (Skymapper const& other)
  {
    beamformer_p         = other.beamformer_p;
    coordinates_p        = other.coordinates_p;
    filename_p           = other.filename_p;
    nofProcessedBlocks_p = other.nofProcessedBlocks_p;
    quantity_p           = other.quantity_p;
    verbose_p            = other.verbose_p;
  }
  
  // -------------------------------------------------------------------- destroy
  
  void Skymapper::destroy ()
  {;}
  
  // ============================================================================
  //
  //  Parameters
  //
  // ============================================================================

  // ----------------------------------------------------------------------- init 
  
  void Skymapper::init (short const &verbose,
			bool const &isOperational,
			uint const &nofProcessedBlocks,
			std::string const &filename,
			SkymapCoordinates const &coordinates,
			Beamformer const &beamformer)
  {
    // Store atomic parameters

    verbose_p            = verbose;
    isOperational_p      = isOperational;
    nofProcessedBlocks_p = nofProcessedBlocks;
    filename_p           = filename;

    // Store embedded objects
    
    if (!setBeamformer (beamformer)) {
      cerr << "[Skymapper::init] Error setting Beamformer object!"
	   << endl;
    }
    
    if (!setSkymapCoordinates (coordinates)) {
      cerr << "[Skymapper::init] Error setting SkymapCoordinates object!"
	   << endl;
    }
    
  }
  
  // -------------------------------------------------------------- setBeamformer
  
  bool Skymapper::setBeamformer (Beamformer const &beamformer)
  {
    bool status (true);
    
    try {
      beamformer_p = beamformer;
    } catch (std::string message) {
      cerr << "[Skymapper::setBeamformer] Error setting Beamformer object!"
	   << endl;
      cerr << "--> " << message << endl;
    }
    
    return status;
  }
  
  // -------------------------------------------------------------- setBeamformer
  
  bool Skymapper::setBeamformer (Matrix<double> const &antPositions)
  {
    return beamformer_p.setAntPositions (antPositions);
  }

  // ------------------------------------------------------- setSkymapCoordinates

  bool Skymapper::setSkymapCoordinates (SkymapCoordinates const &coordinates)
  {
    bool status (true);
    
    try {
      coordinates_p = coordinates;
    } catch (std::string message) {
      cerr << "[Skymapper::setSkymapCoordinates] "
		<< "Error setting setSkymapCoordinates object!"
		<< endl;
      cerr << "--> " << message << endl;
    }
    
    return status;
  }

  // ============================================================================
  //
  //  Processing
  //
  // ============================================================================

  // -------------------------------------------------------------- initSkymapper

  bool Skymapper::initSkymapper () 
  {
    bool status (true);

    /*
      For the later Beamforming we need to retrieve the coordinates of the
      pointing positions; since we keep the directions constant and iterate over
      the distance axis, we get the full position information by later
      combination.
    */
    try {
      // Retrieve the values of the direction axes
      Matrix<double> directionValues;
      status = coordinates_p.directionAxisValues ("AZEL",
						  directionValues,
						  directionMask_p,
						  false);

      // Retrieve the values of the distance axis
      Vector<double> distances (coordinates_p.distanceAxisValues());

      // Combine the values from the axes to yield the 3D positions
      bool anglesInDegrees (true);
      bool bufferDelays (false);
      Vector<int> axisOrder (3);
      casa::indgen(axisOrder);
      status = beamformer_p.setSkyPositions(directionValues,
					    distances,
					    axisOrder,
					    CR::Spherical,
					    anglesInDegrees,
					    bufferDelays);
    } catch (std::string message) {
      cerr << "[Skymapper::initSkymapper] Failed assigning beam directions!"
	   << endl;
      cerr << message << endl;
    }

    /*
      In order to set the beamforming weights we require the frequency values
    */
    try {
      Vector<double> frequencies (coordinates_p.frequencyAxisValues());
      status = beamformer_p.setFrequencies (frequencies);
    } catch (std::string message) {
      cerr << "[Skymapper::initSkymapper] Failed assigning frequency values!"
	   << endl;
      cerr << message << endl;
    }
    
    /*
      With the image data written into an AIPS++ PagedImage, we need to create
      and initialize one first, before we can start inserting the computed 
      image data.
    */
    CoordinateSystem csys = coordinates_p.coordinateSystem();
    IPosition shape       = coordinates_p.shape();
    TiledShape tile (shape);

    try {
      image_p = new PagedImage<double> (tile,
					csys,
					filename_p);
    } catch (std::string message) {
      cerr << "[Skymapper::initSkymapper] Failed creating the image file!" << endl;
      cerr << "--> " << message << endl;
      isOperational_p = false;
    }

    // check the image file created in disk
    if (image_p->ok() && image_p->isWritable()) {
      // book-keeping of operational status
      isOperational_p = true;
      // feedback to the outside world
      if (verbose_p) {
	cout << "[Skymapper::initSkymapper] Image file appears ok and is writable."
		  << endl;
      }
    } else {
      isOperational_p = false;
    }
    
    return isOperational_p;
  }
  
  // ---------------------------------------------------------------- processData
  
  bool Skymapper::processData (Matrix<DComplex> const &data)
  {
    bool status (true);

    // Check if the Skymapper is operational first; otherwise there is no need
    // to proceed beyond this point
    if (!isOperational_p) {
      cerr << "[Skymapper::processData] Skymapper is not operational!"
		<< endl;
      return isOperational_p;
    }

    // forward the data to the beamformer for processing
    Matrix<double> beam;
    status = beamformer_p.processData (beam,
				       data);

    /*
      Inserted the computed pixel values into the image.
    */
    if (status) {
      // Declare additional variables
      int timeAxisStride (coordinates_p.timeAxisStride());
      IPosition imageShape (coordinates_p.shape());
      IPosition imageStart  (imageShape.nelements(),0);
      IPosition imageStride (imageShape.nelements(),1);
      IPosition beamShape (beam.shape());
      IPosition blc_beam  (beamShape.nelements(),0);
      IPosition trc_beam (beamShape.nelements(),1);

      // Adjust the slicing operators
      imageStart(3) = timeAxisStride*nofProcessedBlocks_p;
      blc_beam(1)  = 0;
      trc_beam(1)    = beamShape(1)-1;

      // Provide some feedback
      cout << "[Skymapper::processData]" << endl;
//       cout << "-- shape of the input data  = " << data.shape()   << endl;
//       cout << "-- Shape of beamformed data = " << beam.shape()   << endl;
//       cout << "-- shape of the image       = " << imageShape     << endl;
//       cout << "-- Stride on the time axis  = " << timeAxisStride << endl;
      
      for (int dist(0); dist<imageShape(2); dist++) {
	// slicing instructions for beam array
	blc_beam(0) = dist*imageShape(0)*imageShape(1);
	trc_beam(0)   = (dist+1)*imageShape(0)*imageShape(1) - 1;
	// slicing instructions for the image pixel array
	imageStart(2) = dist;
	// feedback on the setting
	cout << "\t" << blc_beam << " -> " << trc_beam << "\t=>\t"
	     << imageStart << " -> " << imageStride 
	     << endl;
	// insert beamformed data into the image's pixel array
// 	image_p.putSlice (beam(blc_beam,trc_beam),imageStart,imageStride);
      }
      
    } else {
      cerr << "[Skymapper::processData] No processing due to Beamformer error"
	   << endl;
    }
    
    /*
      Book-keeping of the number of processed blocks so far; this is accounted
      for independent of whether the beamforming stage has been successful or
      not, in order to keep navigation within the image pixel array consistent.
    */
    nofProcessedBlocks_p++;

    return status;
  }
  
  // ============================================================================
  //
  //  Feedback 
  //
  // ============================================================================
  
  // -------------------------------------------------------------------- summary
  
  void Skymapper::summary (std::ostream &os)
  {
    TimeFreq timeFreq        = coordinates_p.timeFreq();
    CoordinateSystem csys    = coordinates_p.coordinateSystem();
    DirectionCoordinate axis = coordinates_p.directionAxis();
    MDirection direction     = axis.directionType();
    String refcode           = direction.getRefString();
    String projection        = axis.projection().name();
    ObsInfo obsInfo          = csys.obsInfo();
    
    os << "[Skymapper] Summary of the internal parameters"             << endl;
    os << " - Observation ..............  "                               << endl;
    os << " -- observer                 = " << obsInfo.observer()         << endl;
    os << " -- telescope                = " << obsInfo.telescope()        << endl;
    os << " -- observation date         = " << obsInfo.obsDate()          << endl;
    os << " - Data I/O .................   "                              << endl;
    os << " -- blocksize      [samples] = " << timeFreq.blocksize()       << endl;
    os << " -- FFT length    [channels] = " << timeFreq.blocksize()       << endl;
    os << " -- sampling rate       [Hz] = " << timeFreq.sampleFrequency() << endl;
    os << " -- Nyquist zone             = " << timeFreq.nyquistZone()     << endl;
    os << " -- nof. antennas            = " << beamformer_p.nofAntennas() << endl;
    os << " - Coordinates .............  "                                << endl;
    os << " -- reference code           = " << refcode                    << endl;
    os << " -- projection               = " << projection                 << endl;
    os << " -- nof. coordinates         = " << csys.nCoordinates()        << endl;
    os << " -- names                    = " << csys.worldAxisNames()      << endl;
    os << " -- units                    = " << csys.worldAxisUnits()      << endl;
    os << " -- ref. pixel       (CRPIX) = " << csys.referencePixel()      << endl;
    os << " -- ref. value       (CRVAL) = " << csys.referenceValue()      << endl;
    os << " -- coord. increment (CDELT) = " << csys.increment()           << endl;
    os << " - Image ....................  "                               << endl;
    os << " -- filename                 = " << filename()                 << endl;
    os << " -- imaged quantity          = " << quantity_p.quantity()      << endl;
    os << " -- shape of pixel array     = " << coordinates_p.shape()      << endl;
//     os << " -- image type               = " << image_p.imageType()        << endl;
//     os << " -- is persistent?           = " << image_p.isPersistent()     << endl;
//     os << " -- is paged?                = " << image_p.isPaged()          << endl;
//     os << " -- is writable?             = " << image_p.isWritable()       << endl;
//     os << " -- has pixel mask?          = " << image_p.hasPixelMask()     << endl;
  }
  
}  // Namespace CR -- end
