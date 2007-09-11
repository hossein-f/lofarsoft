##------------------------------------------------------------------------
## $Id::                                                                 $
##------------------------------------------------------------------------

# - Check for the presence of HDF5
#
# The following variables are set when HDF5 is found:
#  HAVE_HDF5       = Set to true, if all components of HDF5
#                          have been found.
#  HDF5_INCLUDES   = Include path for the header files of HDF5
#  HDF5_LIBRARIES  = Link these to use HDF5

## -----------------------------------------------------------------------------
## Search locations

set (include_locations
  ../release/include
  ../../release/include
  /usr/include
  /usr/local/include
  /sw/include
  /opt
  /opt/include
)

set (lib_locations
  ../release/lib
  ../../release/lib
  /lib
  /usr/lib
  /usr/local/lib
  /sw/lib
  /opt
  /opt/lib
)

## -----------------------------------------------------------------------------
## Check for the header files

set (hdf5_headers
  hdf5.h
  H5Cpp.h
  H5LT.h
  )

## check for the basic header file of the HDF5 library

find_path (HDF5_INCLUDES ${hdf5_headers}
  PATHS ${include_locations}
  PATH_SUFFIXES hdf5
  )

## check for header files available when library was build with "--enable-cxx"

#foreach (header ${hdf5_headers})
#  ## search for the header file
#  find_path (header_path ${header}
#    PATHS ${include_locations}
#    PATH_SUFFIXES hdf5
#    )
#  ## check if the search has been successful
#  if (header_path)
#    list (APPEND HDF5_INCLUDES ${header_path})
#  else (header_path)
#    message (FATAL_ERROR "Unable to find ${header}!")
#  endif (header_path)
#endforeach (header)

## -----------------------------------------------------------------------------
## Check for the library

find_library (HDF5_LIBRARIES hdf5
  PATHS ${lib_locations}
  )

## -----------------------------------------------------------------------------
## Actions taken when all components have been found

if (HDF5_INCLUDES AND HDF5_LIBRARIES)
  set (HAVE_HDF5 TRUE)
else (HDF5_INCLUDES AND HDF5_LIBRARIES)
  if (NOT HDF5_FIND_QUIETLY)
    if (NOT HDF5_INCLUDES)
      message (STATUS "Unable to find HDF5 header files!")
    endif (NOT HDF5_INCLUDES)
    if (NOT HDF5_LIBRARIES)
      message (STATUS "Unable to find HDF5 library files!")
    endif (NOT HDF5_LIBRARIES)
  endif (NOT HDF5_FIND_QUIETLY)
endif (HDF5_INCLUDES AND HDF5_LIBRARIES)

if (HAVE_HDF5)
  if (NOT HDF5_FIND_QUIETLY)
    message (STATUS "Found components for HDF5")
    message (STATUS "HDF5_INCLUDES  = ${HDF5_INCLUDES}")
    message (STATUS "HDF5_LIBRARIES = ${HDF5_LIBRARIES}")
  endif (NOT HDF5_FIND_QUIETLY)
else (HAVE_HDF5)
  if (HDF5_FIND_REQUIRED)
    message (FATAL_ERROR "Could not find HDF5!")
  endif (HDF5_FIND_REQUIRED)
endif (HAVE_HDF5)

## -----------------------------------------------------------------------------
## Mark as advanced ...
