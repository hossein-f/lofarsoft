# +-----------------------------------------------------------------------------+
# | $Id:: IO.h 393 2007-06-13 10:49:08Z baehren                               $ |
# +-----------------------------------------------------------------------------+
# |   Copyright (C) 2007                                                        |
# |   Lars B"ahren (bahren@astron.nl)                                           |
# |                                                                             |
# |   This program is free software; you can redistribute it and/or modify      |
# |   it under the terms of the GNU General Public License as published by      |
# |   the Free Software Foundation; either version 2 of the License, or         |
# |   (at your option) any later version.                                       |
# |                                                                             |
# |   This program is distributed in the hope that it will be useful,           |
# |   but WITHOUT ANY WARRANTY; without even the implied warranty of            |
# |   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             |
# |   GNU General Public License for more details.                              |
# |                                                                             |
# |   You should have received a copy of the GNU General Public License         |
# |   along with this program; if not, write to the                             |
# |   Free Software Foundation, Inc.,                                           |
# |   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                 |
# +-----------------------------------------------------------------------------+

# - Check for the presence of <PACKAGE>
#
# The following variables are set when PYTHON is found:
#
#  HAVE_PYTHON            = Set to true, if all components of PYTHON have been
#                           found.
#  PYTHON_EXECUTABLE      = Location of the Python executable
#  PYTHON_INCLUDES        = Include path for the header files of PYTHON
#  PYTHON_LIBRARIES       = Link these to use PYTHON
#  PYTHON_VERSION         = Python version, as return from "python --version"
#  PYTHON_MAJOR_VERSION   = (not yet implemented)
#  PYTHON_MINOR_VERSION   = (not yet implemented)
#  PYTHON_RELEASE_VERSION = (not yet implemented)
#  PYTHON_LFGLAS          = Linker flags (optional)
#
# Beside the core components of Python we also include a search for optional
# packages which might be installed as well:
#
#  NUM_UTIL_INCLUDES = Include path for the header files of NUM_UTIL package
#

## -----------------------------------------------------------------------------
## Standard locations where to look for required components

include (CMakeSettings)

## -----------------------------------------------------------------------------
## Is the root of the Python installation defined through environment variable?

set (PYTHON_PYTHONHOME $ENV{PYTHONHOME})

if (PYTHON_PYTHONHOME)
  message (STATUS "Found environment variable PYTHONHOME.")
endif (PYTHON_PYTHONHOME)

## -----------------------------------------------------------------------------

foreach (python_version 2.6 2.5 2.4 2.3)

  if (NOT HAVE_PYTHON)
    
    ## Check for the Python executable
    
    if (PYTHON_PYTHONHOME)
      find_program (PYTHON_EXECUTABLE python python${python_version}
	PATHS ${PYTHON_PYTHONHOME} ${PYTHON_PYTHONHOME}/bin
	NO_DEFAULT_PATH
	)
    else (PYTHON_PYTHONHOME)
      find_program (PYTHON_EXECUTABLE python${python_version}
	PATHS ${bin_locations}
	NO_DEFAULT_PATH
	)
    endif (PYTHON_PYTHONHOME)

    ## Extract the full version information from the Python executable
    
    if (PYTHON_EXECUTABLE)
      execute_process (
	COMMAND ${PYTHON_EXECUTABLE} --version
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	RESULT_VARIABLE version_test_result
	OUTPUT_VARIABLE version_test_output
	ERROR_VARIABLE version_test_error
	OUTPUT_STRIP_TRAILING_WHITESPACE
	ERROR_STRIP_TRAILING_WHITESPACE
	)
      if (version_test_error)
	string (REGEX REPLACE "Python " "" version_test_error ${version_test_error})
      endif (version_test_error)
    endif(PYTHON_EXECUTABLE)
    
    ## Check for the Python header files
    
    find_path (PYTHON_INCLUDES Python.h
      PATHS ${include_locations}
      PATH_SUFFIXES python${python_version}
      NO_DEFAULT_PATH
      )
    
    find_path (HAVE_PYCONFIG_H pyconfig.h
      PATHS ${include_locations}
      PATH_SUFFIXES python${python_version}
      NO_DEFAULT_PATH
      )
    
    ## Check for the Python library
    
    find_library (PYTHON_LIBRARIES python${python_version}
      PATHS
      ${lib_locations}
      PATH_SUFFIXES
      python${python_version}/config
      NO_DEFAULT_PATH
      )
    
    # Check if we have been able to find a consistent set of exectable,
    # header files and library. If this is the case, we stop looking for
    # such a combination in an older Python release.

    if (PYTHON_INCLUDES AND PYTHON_LIBRARIES)
      set (PYTHON_VERSION ${python_version})
      set (HAVE_PYTHON TRUE)
    else (PYTHON_INCLUDES AND PYTHON_LIBRARIES)
      if (NOT PYTHON_FIND_QUIETLY)
	message (STATUS "No consistent set of files found for Python ${python_version}")
      endif (NOT PYTHON_FIND_QUIETLY)
    endif (PYTHON_INCLUDES AND PYTHON_LIBRARIES)
    
  endif (NOT HAVE_PYTHON)
  
endforeach (python_version)

## -----------------------------------------------------------------------------
## Actions taken when all components have been found

## Python itself

if (PYTHON_INCLUDES AND PYTHON_LIBRARIES)
  set (HAVE_PYTHON TRUE)
else (PYTHON_INCLUDES AND PYTHON_LIBRARIES)
  set (HAVE_PYTHON FALSE)
  if (NOT PYTHON_FIND_QUIETLY)
    if (NOT PYTHON_INCLUDES)
      message (STATUS "Unable to find PYTHON header files!")
    endif (NOT PYTHON_INCLUDES)
    if (NOT PYTHON_LIBRARIES)
      message (STATUS "Unable to find PYTHON library files!")
    endif (NOT PYTHON_LIBRARIES)
  endif (NOT PYTHON_FIND_QUIETLY)
endif (PYTHON_INCLUDES AND PYTHON_LIBRARIES)

## NumUtil

if (NUM_UTIL_INCLUDES AND NUM_UTIL_LIBRARIES)
  set (HAVE_NUM_UTIL TRUE)
else (NUM_UTIL_INCLUDES AND NUM_UTIL_LIBRARIES) 
  set (HAVE_NUM_UTIL FALSE)
endif (NUM_UTIL_INCLUDES AND NUM_UTIL_LIBRARIES) 

## Feedback

if (HAVE_PYTHON)
  if (NOT PYTHON_FIND_QUIETLY)
    message (STATUS "Found components for PYTHON")
    message (STATUS "PYTHON_EXECUTABLE  = ${PYTHON_EXECUTABLE}")
    message (STATUS "PYTHON_INCLUDES    = ${PYTHON_INCLUDES}")
    message (STATUS "PYTHON_LIBRARIES   = ${PYTHON_LIBRARIES}")
  endif (NOT PYTHON_FIND_QUIETLY)
else (HAVE_PYTHON)
  if (PYTHON_FIND_REQUIRED)
    message (FATAL_ERROR "Could not find PYTHON!")
  endif (PYTHON_FIND_REQUIRED)
endif (HAVE_PYTHON)

## -----------------------------------------------------------------------------
## Mark advanced variables

mark_as_advanced (
  PYTHON_INCLUDES
  PYTHON_LIBRARIES
  )
