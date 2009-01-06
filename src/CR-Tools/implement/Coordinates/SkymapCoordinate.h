/*-------------------------------------------------------------------------*
 | $Id:: NewClass.h 1964 2008-09-06 17:52:38Z baehren                    $ |
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

#ifndef SKYMAPCOORDINATE_H
#define SKYMAPCOORDINATE_H

// Standard library header files
#include <iostream>
#include <string>

// CR-Tools header files
#include <Coordinates/SpatialCoordinate.h>
#include <Coordinates/TimeFreqCoordinate.h>
#include <Observation/ObservationData.h>

// Namespace usage
using CR::SpatialCoordinate;
using CR::TimeFreqCoordinate;

namespace CR { // Namespace CR -- begin
  
  /*!
    \class SkymapCoordinate
    
    \ingroup CR_Coordinates
    
    \brief Container for the coordinates associated with a sky map
    
    \author Lars B&auml;hren

    \date 2009/01/06

    \test tSkymapCoordinate.cc
    
    <h3>Prerequisite</h3>
    
    <ul type="square">
      <li>CR::SpatialCoordinate
      <li>CR::TimeFreqCoordinate
    </ul>
    
    <h3>Synopsis</h3>

    This class mainly operates as a container for the coordinate objects handling
    the spatial and temporal-spectral characteristics of a sky map.
    
    <h3>Example(s)</h3>
    
  */  
  class SkymapCoordinate {
    
    //! Observation data (epoch, location, etc.)
    ObservationData obsData_p;
    //! Container for the spatial coordinates
    CR::SpatialCoordinate spatialCoord_p;
    //! Container for the time-frequency axes
    CR::TimeFreqCoordinate timeFreqCoord_p;
    
  public:
    
    // ------------------------------------------------------------- Construction
    
    //! Default constructor
    SkymapCoordinate ();
    
    /*!
      \brief Argumented constructor

      \param obsData       -- Observation data (epoch, location, etc.)
      \param timeFreqCoord -- Container for the coupled time and frequency axes
    */
    SkymapCoordinate (ObservationData const &obsData,
		      TimeFreqCoordinate const &timeFreqCoord);

    /*!
      \brief Argumented constructor

      \param obsData       -- Observation data (epoch, location, etc.)
      \param spatialCoord  -- 
      \param timeFreqCoord -- Container for the coupled time and frequency axes
    */
    SkymapCoordinate (ObservationData const &obsData,
		      SpatialCoordinate const &spatialCoord,
		      TimeFreqCoordinate const &timeFreqCoord);
    
    /*!
      \brief Copy constructor
      
      \param other -- Another SkymapCoordinate object from which to create this new
             one.
    */
    SkymapCoordinate (SkymapCoordinate const &other);
    
    // -------------------------------------------------------------- Destruction

    //! Destructor
    ~SkymapCoordinate ();
    
    // ---------------------------------------------------------------- Operators
    
    /*!
      \brief Overloading of the copy operator
      
      \param other -- Another SkymapCoordinate object from which to make a copy.
    */
    SkymapCoordinate& operator= (SkymapCoordinate const &other); 
    
    // --------------------------------------------------------------- Parameters

    /*!
      \brief Get some basic information on the observation (epoch, location, etc.)
      
      \return obsData -- Observation data (epoch, location, etc.)
    */
    inline ObservationData observationData () const {
      return obsData_p;
    }

    /*!
      \brief Set some basic information on the observation (epoch, location, etc.)
      
      \param obsData -- Observation data (epoch, location, etc.)

      \return status -- Status of the operation; returns <tt>false</tt> in case
              an error was encountered.
    */
    bool setObservationData (ObservationData const &obsData);

    /*!
      \brief Get the object encapsulating the spatial coordinates
    */
    inline SpatialCoordinate spatialCoordinate () const {
      return spatialCoord_p;
    }
    
    /*!
      \brief Set the object encapsulating the spatial coordinates

      \param coord -- 

      \return status -- Status of the operation; returns <tt>false</tt> in case
              an error was encountered.
    */
    bool setSpatialCoordinate (SpatialCoordinate const &coord);

    /*!
      \brief Get the object encapsulating the temporal-spectral coordinates
    */
    inline TimeFreqCoordinate timeFreqCoordinate () const {
      return timeFreqCoord_p;
    }
    
    /*!
      \brief Set the object encapsulating the temporal-spectral coordinates

      \param coord -- 

      \return status -- Status of the operation; returns <tt>false</tt> in case
              an error was encountered.
    */
    bool setTimeFreqCoordinate (TimeFreqCoordinate const &coord);

    /*!
      \brief Get the name of the class
      
      \return className -- The name of the class, SkymapCoordinate.
    */
    inline std::string className () const {
      return "SkymapCoordinate";
    }

    /*!
      \brief Provide a summary of the internal status
    */
    inline void summary () {
      summary (std::cout);
    }

    /*!
      \brief Provide a summary of the internal status

      \param os -- Output stream to which the summary is written.
    */
    void summary (std::ostream &os);    

    // ------------------------------------------------------------------ Methods
    
    
    
  private:
    
    //! Unconditional copying
    void copy (SkymapCoordinate const &other);
    
    //! Unconditional deletion 
    void destroy(void);

    void init ();

    void init (ObservationData const &obsData,
	       SpatialCoordinate const &spatialCoord,
	       TimeFreqCoordinate const &timeFreqCoord);
    
    
  };
  
} // Namespace CR -- end

#endif /* SKYMAPCOORDINATE_H */
  
