/*-------------------------------------------------------------------------*
 | $Id:: NewClass.cc 1964 2008-09-06 17:52:38Z baehren                   $ |
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

#include <Coordinates/SkymapCoordinate.h>

namespace CR { // Namespace CR -- begin
  
  // ============================================================================
  //
  //  Construction
  //
  // ============================================================================
  
  //____________________________________________________________ SkymapCoordinate

  SkymapCoordinate::SkymapCoordinate ()
  {
    init();
  }
  
  //____________________________________________________________ SkymapCoordinate

  SkymapCoordinate::SkymapCoordinate (ObservationData const &obsData,
				      TimeFreqCoordinate const &timeFreqCoord)
  {
    SpatialCoordinate spatialCoord = SpatialCoordinate();

    init (obsData,
	  spatialCoord,
	  timeFreqCoord);
  }

  //____________________________________________________________ SkymapCoordinate

  SkymapCoordinate::SkymapCoordinate (ObservationData const &obsData,
				      SpatialCoordinate const &spatialCoord,
				      TimeFreqCoordinate const &timeFreqCoord)
  {
    init (obsData,
	  spatialCoord,
	  timeFreqCoord);
  }
  
  //____________________________________________________________ SkymapCoordinate

  SkymapCoordinate::SkymapCoordinate (SkymapCoordinate const &other)
  {
    copy (other);
  }
  
  // ============================================================================
  //
  //  Destruction
  //
  // ============================================================================
  
  SkymapCoordinate::~SkymapCoordinate ()
  {
    destroy();
  }
  
  void SkymapCoordinate::destroy ()
  {;}
  
  // ============================================================================
  //
  //  Operators
  //
  // ============================================================================
  
  SkymapCoordinate& SkymapCoordinate::operator= (SkymapCoordinate const &other)
  {
    if (this != &other) {
      destroy ();
      copy (other);
    }
    return *this;
  }
  
  //________________________________________________________________________ copy
  
  void SkymapCoordinate::copy (SkymapCoordinate const &other)
  {
    obsData_p       = other.obsData_p;
    spatialCoord_p  = other.spatialCoord_p;
    timeFreqCoord_p = other.timeFreqCoord_p;
    csys_p          = other.csys_p;
  }

  // ============================================================================
  //
  //  Parameters
  //
  // ============================================================================

  //________________________________________________________ setSpatialCoordinate
  
  bool SkymapCoordinate::setSpatialCoordinate (SpatialCoordinate const &coord)
  {
    bool status (true);

    // store the input data
    spatialCoord_p = coord;

    // update coordinate system object 
    setCoordinateSystem ();

    return status;
  }

  //_______________________________________________________ setTimeFreqCoordinate
  
  bool SkymapCoordinate::setTimeFreqCoordinate (TimeFreqCoordinate const &coord)
  {
    bool status (true);

    // store the input data
    timeFreqCoord_p = coord;
    
    // update coordinate system object 
    setCoordinateSystem ();

    return status;
  }

  //_______________________________________________________________________ shape

  casa::IPosition SkymapCoordinate::shape ()
  {
    uint counter (0);
    casa::IPosition shapeSkymap (nofAxes());
    casa::IPosition shapeSpatial  = spatialCoord_p.shape();
    casa::IPosition shapeTimeFreq = timeFreqCoord_p.shape();

    for (uint n(0); n<shapeSpatial.nelements(); n++) {
      shapeSkymap(counter) = shapeSpatial(n);
      counter++;
    }

    for (uint n(0); n<shapeTimeFreq.nelements(); n++) {
      shapeSkymap(counter) = shapeTimeFreq(n);
      counter++;
    }

    return shapeSkymap;
  }

  //_____________________________________________________________________ summary
  
  void SkymapCoordinate::summary (std::ostream &os)
  {
    os << "[SkymapCoordinate] Summary of internal parameters." << std::endl;
    os << "-- nof. coordinate objects = " << nofCoordinates()  << std::endl;
    os << "-- nof. coordinate axes    = " << nofAxes()         << std::endl;
    os << "-- Shape of the axes       = " << shape()           << std::endl;
    os << "-- World axis names        = " << csys_p.worldAxisNames() << std::endl;
    os << "-- World axis units        = " << csys_p.worldAxisUnits() << std::endl;
    os << "-- Reference pixel (CRPIX) = " << csys_p.referencePixel() << std::endl;
    os << "-- Increment       (CDELT) = " << csys_p.increment()      << std::endl;
    os << "-- Reference value (CRVAL) = " << csys_p.referenceValue() << std::endl;
  }
  
  // ============================================================================
  //
  //  Methods
  //
  // ============================================================================

  //________________________________________________________________________ init
  
  void SkymapCoordinate::init ()
  {
    ObservationData obsData          = ObservationData();
    SpatialCoordinate spatialCoord   = SpatialCoordinate ();
    TimeFreqCoordinate timeFreqCoord = TimeFreqCoordinate();
    
    init (obsData,
	  spatialCoord,
	  timeFreqCoord);
  }
  
  //________________________________________________________________________ init
  
  void SkymapCoordinate::init (ObservationData const &obsData,
			       SpatialCoordinate const &spatialCoord,
			       TimeFreqCoordinate const &timeFreqCoord)
  {
    obsData_p       = obsData;
    spatialCoord_p  = spatialCoord;
    timeFreqCoord_p = timeFreqCoord;

    setCoordinateSystem ();
  }
  
  //_________________________________________________________ setCoordinateSystem
  
  void SkymapCoordinate::setCoordinateSystem ()
  {
    casa::CoordinateSystem csys;
    
    csys.setObsInfo (obsData_p.obsInfo());
    
    spatialCoord_p.toCoordinateSystem(csys);
    timeFreqCoord_p.toCoordinateSystem(csys);
    
    csys_p = csys;
  }
  

} // Namespace CR -- end
