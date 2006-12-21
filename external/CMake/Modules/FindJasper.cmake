# - Try to find the Jasper JPEG2000 library
# Once done this will define
#
#  JASPER_FOUND - system has Jasper
#  JASPER_INCLUDE_DIR - the Jasper include directory
#  JASPER_LIBRARIES - The libraries needed to use Jasper

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


FIND_PACKAGE(JPEG)

IF (JASPER_INCLUDE_DIR AND JASPER_LIBRARIES AND JPEG_LIBRARIES)
  # Already in cache, be silent
  SET(Jasper_FIND_QUIETLY TRUE)
ENDIF (JASPER_INCLUDE_DIR AND JASPER_LIBRARIES AND JPEG_LIBRARIES)

FIND_PATH(JASPER_INCLUDE_DIR jasper/jasper.h)

FIND_LIBRARY(JASPER_LIBRARY NAMES jasper libjasper)

IF (JASPER_INCLUDE_DIR AND JASPER_LIBRARY AND JPEG_LIBRARIES)
   SET(JASPER_FOUND TRUE)
   SET(JASPER_LIBRARIES ${JASPER_LIBRARY} ${JPEG_LIBRARIES} )
ELSE (JASPER_INCLUDE_DIR AND JASPER_LIBRARY AND JPEG_LIBRARIES)
   SET(JASPER_FOUND FALSE)
ENDIF (JASPER_INCLUDE_DIR AND JASPER_LIBRARY AND JPEG_LIBRARIES)


IF (JASPER_FOUND)
   IF (NOT Jasper_FIND_QUIETLY)
      MESSAGE(STATUS "Found jasper: ${JASPER_LIBRARIES}")
   ENDIF (NOT Jasper_FIND_QUIETLY)
ELSE (JASPER_FOUND)
   IF (Jasper_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could NOT find jasper library")
   ENDIF (Jasper_FIND_REQUIRED)
ENDIF (JASPER_FOUND)

MARK_AS_ADVANCED(JASPER_INCLUDE_DIR JASPER_LIBRARIES JASPER_LIBRARY)
