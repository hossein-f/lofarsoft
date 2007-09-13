#!/bin/sh

##-------------------------------------------------------------------------------
## $Id:: CMakeLists.txt 724 2007-09-10 10:17:21Z baehren                        $
##-------------------------------------------------------------------------------
##
## A simple shell script to configure and install as many as possible of
## the packages in the LOFAR USG distribution. Simply run
## 
##   ./build.sh
##
## from within the build directory - and the script should take care of
## the rest.
##

basedir=`pwd`

## -----------------------------------------------------------------------------
## Helper functions

# \param buildDir  -- directory in which to perform the build; this is a 
#                     sub-directory created below "build"
# \param sourceDir -- directory in which the source code of the package can be
#                     found; the path is given relativ to $LOFARSOFT
build_package ()
{
    buildDir=$1
    sourceDir=$2
    configureOption=$3
    # check if the build directory exists
    cd $basedir
    if test -d $buildDir ; then
	{
	# change into the build directory
	cd $buildDir
	# clean up the directory before we do anything else
	rm -rf *
	# run cmake on the source directory
	if test -z $configureOption ; then
	    cmake $basedir/../$sourceDir
	else 
	    cmake $basedir/../$sourceDir $configureOption
	fi
	# build the package
	if test -z "`make help | grep install`" ; then
	    echo "-- No target install for $buildDir."
	else
	    make install;
	fi
	}
    else 
	{
	echo "No build directory $buildDir - creating it now."
	mkdir $buildDir;
	# recursive call
	build_package $buildDir $sourceDir
	}
    fi
}

## -----------------------------------------------------------------------------
## Build individual/multiple packages

case $1 in 
    bison)
	echo "[build] Selected package Bison"
	build_package bison external/bison
    ;;
    blitz)
	echo "[build] Selected package Blitz++"
	build_package blitz external/blitz -DBLITZ_FORCE_BUILD:BOOL=1
    ;;
    boost)
	echo "[build] Selected package Boost"
	build_package boost external/boost -DBOOST_FORCE_BUILD:BOOL=1
    ;;
    casacore)
	echo "[build] Selected package CASACORE"
	build_package casacore external/casacore
    ;;
    cfitsio)
	echo "[build] Selected package CFITSIO"
	build_package cfitsio external/cfitsio
    ;;
    cmake)
	echo "[build] Selected package CMake"
	## Check if the source code to build cmake is available; if this
	## is not the case with error, since this is the bottom-most 
	## position in the dependency chain
	if test -d $basedir/../external/cmake ; then
	    echo " --> Cleaning up source directory ..."
	    rm -rf $basedir/../external/cmake/Bootstrap.cmk
	    rm -rf $basedir/../external/cmake/CMakeCache.txt
	    rm -rf $basedir/../external/cmake/Source/cmConfigure.h
	else
	    echo " ==> Missing source directory for cmake! Unable to continue!"
	    exit 1;
	fi
	## prepare to build cmake from its source
	if test -d cmake ; then
	    cd cmake
	else
	    mkdir cmake
	    cd cmake
	fi
	## run the configure script
	echo " --> Running configure script for cmake ..."
	$basedir/../external/cmake/configure --prefix=$basedir/../external
	## build and install
	echo " --> initiating build and install ..."
	make install
	## check if we have been able to create a cmake executable
	if test -f ../../external/bin/cmake ; then
	    echo " ==> Found newly created cmake executable."
	else
	    echo " ==> No cmake executable found in release/bin! Unable to continue!"
	    exit 1;
	fi
    ;;
    flex)
	echo "[build] Selected package Flex"
	build_package flex external/flex
    ;;
    hdf5)
	echo "[build] Selected package Hdf5"
	build_package hdf5 external/hdf5 -DHDF5_FORCE_BUILD:BOOL=1
    ;;
    pgplot)
	echo "[build] Selected package Pgplot"
	build_package pgplot external/pgplot -DCMAKE_INSTALL_PREFIX:STRING=$basedir/../release
    ;;
    plplot)
	echo "[build] Selected package Plplot"
	build_package plplot external/plplot
    ;;
    python)
	echo "[build] Selected package PYTHON"
	build_package python external/python
    ;;
    wcslib)
	echo "[build] Selected package WCSLIB"
	build_package wcslib external/wcslib
    ;;
    dal)
	echo "[build] Selected package DAL"
	## external packages
	./build.sh cmake
	./build.sh wcslib
	./build.sh cfitsio
	./build.sh casacore
	./build.sh blitz
	./build.sh python
	./build.sh boost
	./build.sh hdf5
	## USG packages
	build_package dal src/DAL
    ;;
    cr)
	echo "[build] Selected package CR-Tools"
	./build.sh dal
	build_package cr src/CR-Tools
    ;;
    all)
	echo "[build] Building all external packsges"
	./build.sh bison
	./build.sh blitz
	./build.sh python
	./build.sh boost
	./build.sh flex
	./build.sh pgplot
	./build.sh wcslib
	./build.sh cfitsio
	./build.sh casacore
	./build.sh hdf5
	echo "[build] Building all USG packsges"
	build_package dal src/DAL
	build_package cr src/CR-Tools
    ;;
    clean)
    rm -f *~
    rm -rf bison
    rm -rf blitz
    rm -rf boost
    rm -rf casacore
    rm -rf cfitsio
    rm -rf cmake
    rm -rf cr
    rm -rf dal
    rm -rf dsp
    rm -rf flex
    rm -rf hdf5
    rm -rf pgplot
    rm -rf python
    rm -rf wcslib
    ;;
    *)
    echo "";
    ;;
esac
