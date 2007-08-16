/***************************************************************************
 *   Copyright (C) 2007                                                    *
 *   Kalpana Singh (<k.singh@astro.ru.nl>)                                 *
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

/* $Id: subbandID.h,v 1.2 2007/08/06 14:43:10 singh Exp $*/

#ifndef SUBBANDID_H
#define SUBBANDID_H

// Standard library header files
#include <string.h>

// AIPS++/CASA header files
#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/fstream.h>
#include <casa/string.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Exceptions/Error.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ScaRecordColDesc.h>

#include <casa/namespace.h>


namespace CR { // Namespace CR -- begin
  
  /*!
    \class subbandID
    
    \ingroup Analysis
    
    \brief Brief description for class subbandID
    
    \author Kalpana Singh

    \date 2007/06/05

    \test tsubbandID.cc
    
    <h3>Prerequisite</h3>
    
    <ul type="square">
      <li>[ This class is developed to calculate the subband IDs that can be used for polyphase filter bank inversion or
            for ionospheric calibration.
            In case if subband IDs are provided then in that case to calculate the starting frequency of the subband with 
	    bandwidth depending on the sampling frequency, in other specific case if vector of various subband (initial)
	    frequencies is provided then in that case also subband IDs can be generated ]
    </ul>
    
    <h3>Synopsis</h3>
    
    <h3>Example(s)</h3>
    
  */  
  class subbandID {
    
  public:
    
    // ------------------------------------------------------------- Construction
    
    /*!
      \brief Default constructor
    */
    subbandID ();

   /*!
    \brief Argumented Constructor

    sets a vector of initial subband frequencies of all the subbands if ID of first subband is given 
    and number of subbands is given


   \param  samplingFreq    --  clock rate at which data is sampled

   \param  bandID          --  subband ID of first subband used for observation

   \param  nofsubbands     --  number of subbands used for observation, basically for beamforming, number of datafiles
                               which is given for processing

  */

   subbandID ( const Double& samplingFreq,
               const uint& bandID,
               const uint& nofsubbands  );

   /*!
   \brief Argumented Constructor

   sets the  vector of subband IDs if the initial subband frequency of first subband, which is used in observation 
   is given, and number of subbands used in observation is given.

   \param  samplingFreq   --  clock rate at which data is sampled

   \param subband_freq_1  --  initial frequency of first subband which is used for observation

   \param nofsubbands     --  number of subbands used for observation, basically for beamforming, number of datafiles
                              which is given for processing

   */

  subbandID ( const Double& samplingFreq,
              const Double& subband_freq_1,
              const uint& nofsubbands  ) ;

  
  /*!
  \brief Arguemented Constructor
  
  sets the vector of subband IDs for given vector of subband intial frequencies which are used for beamforming at
  staion level or for tied array beam
  
  \param  samplingFreq           --  clock rate at which data is sampled
  
  \param   subband_frequencies   --  vector of subband frequencies.
  
  */
   
   subbandID ( const Double& samplingFreq,
               const Vector<Double>& subband_frequencies ) ;

   
    /*!
      \brief Copy constructor
      
      \param other -- Another subbandID object from which to create this new
      one.
    */
    subbandID (subbandID const &other);
    
    // -------------------------------------------------------------- Destruction

    /*!
      \brief Destructor
    */
    ~subbandID ();
    
    // ---------------------------------------------------------------- Operators
    
    /*!
      \brief Overloading of the copy operator
      
      \param other -- Another subbandID object from which to make a copy.
    */
    subbandID& operator= (subbandID const &other); 
    
    // --------------------------------------------------------------- Parameters
    
    /*!
      \brief Get the name of the class
      
      \return className -- The name of the class, subbandID.
    */
    std::string className () const {
      return "subbandID";
    }

    /*!
      \brief Provide a summary of the internal status
    */
    void summary ();    

    // --------------------------------- Computational Methods --------------------

     /*!
   \brief calculating initial frequencies of subbands which are used for beamforming.

   \param  sampling_Freq 	--  clock rate at which data is sampled

   \param  subband_ID 		--  subband ID of first subband used for observation

   \param n_subbands            --  number of subbands used for observation, basically for beamforming,
                                   number of datafiles which is given for processing
				   
   returns vector of vector of initial frequencies of subbands which are used for beamforming.				  

   */
   
    Vector<Double> calcFrequency ( const Double& sampling_Freq,
                                   const uint& subband_ID,
                                   const uint& n_subbands ) ;

     /*!
   \brief calculating subbands IDs which are used for beamforming.

   \param  sampling_Freq 	--  clock rate at which data is sampled

   \param  subband_freq		--  subband frequency of first subband of the subband vector which are 
   				    used for observation

   \param n_subbands            --  number of subbands used for observation, basically for beamforming,
                                   number of datafiles which is given for processing
				   
   returns vector of vector of subbands IDs which are used for beamforming.				  

   */
   
   Vector<uint> calcSubbandID ( const Double& sampling_Freq,
                                const Double& subband_freq,
                                const uint& n_subbands  ) ;
    
     /*!
   \brief calculating subbands IDs which are used for beamforming.

   \param  sampling_Freq 	--  clock rate at which data is sampled

   \param  subband_frequencies	--  vector of intial frequencies of all subbands which are 
   				    used for beamforming.

   returns vector of vector of subbands IDs which are used for beamforming.				  

   */
   
   Vector<uint> calcbandIDVector ( const Double& sampling_freq,
                                   const Vector<Double>& subband_frequencies ) ;
    
  private:
    
    /*!
      \brief Unconditional copying
    */
    void copy(subbandID const &other);
    
    /*!
      \brief Unconditional deletion 
    */
    void destroy(void);
    
  };
  
} // Namespace CR -- end

#endif /* SUBBANDID_H */
  
