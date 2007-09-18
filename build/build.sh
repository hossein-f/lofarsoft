#!/bin/sh

##-------------------------------------------------------------------------------
## $Id:: CMakeLists.txt 724 2007-09-10 10:17:21Z baehren                        $
##-------------------------------------------------------------------------------
##
## A simple shell script to configure and install as many as possible of
## the packages in the LOFAR USG distribution. Simply run
## 
##   ./build.sh <package>
##
## from within the build directory - and the script should take care of
## the rest.
##

basedir=`pwd`

## -----------------------------------------------------------------------------
## check command line parameters

echo "[build]";

## param 1 -- Name of the package to build

if test -z $1 ; then
    echo " -- Missing input parameters.";
    echo "";
    echo " Usage:  build.sh <package> [build-option]";
    echo "";
else
    param_packageName=$1;
    echo " -- Selected package: $param_packageName";
fi

## param 2 -- Force build of package from provided source?

if test -z $2 ; then
    echo " -- No further build options.";
    param_forceBuild=0;
else
    case $2 in
	--force-build)
	    param_forceBuild=1;
	    echo " -- Recognized build option; forcing build."; 
	;;
	*)
	    echo " --> Unknown parameter $2"
	;;
    esac
fi

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
	    echo "[build] No target install for $buildDir."
	else
	    make install;
	fi
	}
    else 
	{
	echo "[build] No build directory $buildDir - creating it now."
	mkdir $buildDir;
	# recursive call
	build_package $buildDir $sourceDir
	}
    fi
}

## -----------------------------------------------------------------------------
## Build individual/multiple packages

case $param_packageName in 
    bison)
	echo "[build] Selected package Bison"
	build_package bison external/bison
    ;;
    blitz)
	echo "[build] Selected package Blitz++"
	build_package blitz external/blitz -DBLITZ_FORCE_BUILD:BOOL=$param_forceBuild
    ;;
    boost)
	echo "[build] Selected package Boost"
	build_package boost external/boost -DBOOST_FORCE_BUILD:BOOL=$param_forceBuild
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
	    echo "[build] Cleaning up source directory ..."
	    rm -rf $basedir/../external/cmake/Bootstrap.cmk
	    rm -rf $basedir/../external/cmake/CMakeCache.txt
	    rm -rf $basedir/../external/cmake/Source/cmConfigure.h
	else
	    echo "[build] Missing source directory for cmake! Unable to continue!"
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
	echo "[build] Running configure script for cmake ..."
	$basedir/../external/cmake/configure --prefix=$basedir/../release
	## build and install
	echo "[build] Initiating build and install of cmake ..."
	make && make install
	## check if we have been able to create a cmake executable
	if test -f ../../release/bin/cmake ; then
	    echo "[build] Found newly created cmake executable."
	    export PATH=$PATH:$basedir/../release/bin
	else
	    echo "[build] No cmake executable found in release/bin! Unable to continue!"
	    exit 1;
	fi
    ;;
    flex)
	echo "[build] Selected package Flex"
	build_package flex external/flex
    ;;
    hdf5)
	echo "[build] Selected package Hdf5"
	build_package hdf5 external/hdf5 -DHDF5_FORCE_BUILD:BOOL=$param_forceBuild
    ;;
    pgplot)
	echo "[build] Selected package Pgplot"
	build_package pgplot external/pgplot -DCMAKE_INSTALL_PREFIX:STRING=$basedir/../release
    ;;
    plplot)
	echo "[build] Selected package Plplot"
	if test -d $basedir/../external/plplot ; then
	    build_package plplot external/plplot;
	else
	    cd $basedir/../external
	    ## download the source tar-ball from source forge
	    wget -c http://ovh.dl.sourceforge.net/sourceforge/plplot/plplot-5.7.4.tar.gz
	    ## unpack the tar-ball and adjust the name of the newly created directory
	    tar -xvzf plplot-5.7.4.tar.gz
	    mv plplot-5.7.4 plplot
	    ## remove the tar-ball
	    rm -f plplot-5.7.4.tar.gz
	    ## recursive call of this method
	    cd $basedir
	    ./build.sh plplot
	fi
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
	echo "[build] Selected package CR-Tools";
	./build.sh dal;
	./build.sh pgplot;
	build_package cr src/CR-Tools;
    ;;
    all)
	echo "[build] Building all external packages";
	./build.sh bison;
	./build.sh blitz;
	./build.sh python;
	./build.sh boost;
	./build.sh flex;
	./build.sh pgplot;
	./build.sh wcslib;
	./build.sh cfitsio;
	./build.sh casacore;
	./build.sh hdf5;
	echo "[build] Building all USG packsges";
	build_package dal src/DAL;
	build_package cr src/CR-Tools;
    ;;
    clean)
    rm -f *~;
    rm -f *.log;
    rm -rf bison;
    rm -rf blitz;
    rm -rf boost;
    rm -rf casacore;
    rm -rf cfitsio;
    rm -rf cmake;
    rm -rf cr;
    rm -rf dal;
    rm -rf dsp;
    rm -rf flex;
    rm -rf hdf5;
    rm -rf pgplot;
    rm -rf plplot;
    rm -rf python;
    rm -rf wcslib;
    ;;
esac
