/*-------------------------------------------------------------------------*
 | $Id::                                                                 $ |
 *-------------------------------------------------------------------------*
 ***************************************************************************
 *   Copyright (C) 2008                                                    *
 *   Lars B"ahren (lbaehren@gmail.com)                                     *
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

#include <string>
#include <vector>

// ==============================================================================
//
//  Header files
//
// ==============================================================================

// CR-Tools header files
#include <crtools.h>
#include <Coordinates/CoordinateType.h>
#include <Coordinates/TimeFreq.h>
#include <Filters/HanningFilter.h>
#include <IO/LOFAR_TBB.h>
#include <IO/DataIterator.h>
#include <IO/DataReader.h>
#include <Analysis/DynamicSpectrum.h>

// Basic Python header
#include <Python.h>

//________________________________________________
//                       Boost.Python header files

#include <boost/python.hpp>
#include <boost/python/object.hpp>
#include <boost/python/list.hpp>
#include <boost/python/extract.hpp>

// ==============================================================================
//
//  Namespace usage
//
// ==============================================================================

namespace bpl = boost::python;

using CR::CoordinateType;
using CR::DataIterator;
using CR::DataReader;
using CR::HanningFilter;
using CR::TimeFreq;
using CR::LOFAR_TBB;
using CR::DynamicSpectrum;

// ==============================================================================
//
//  Function prototypes
//
// ==============================================================================

/* Coordinates */

void export_CoordinateType ();
void export_TimeFreq ();

/* IO */

void export_LOFAR_TBB ();

