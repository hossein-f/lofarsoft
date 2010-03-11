/*-------------------------------------------------------------------------*
 | $Id::                                                                   $ |
 *-------------------------------------------------------------------------*
 ***************************************************************************
 *   Copyright (C) 2009                                                    *
 *   Andreas Horneffer (A.Horneffer@astro.ru.nl)                           *
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sstream>

#include <boost/thread.hpp>

#include "TBBraw.h"

/*!
  \file TBBraw2h5.cpp

  \ingroup DAL
  \ingroup dal_apps

  \brief Read TBB time-series data from a socket or file and generate an HDF5 file.

  \author Andreas Horneffer

  \date 2009/07/02

  <h3>Prerequisite</h3>

  - DAL::TBBraw -- Class to generate a TBB time-series hdf5 file from raw TBB data-frames.

  <h3>History</h3>

  This application and the corresponding \t TBBraw class is a rewrite of the \t tbb2h5
  application and its \t TBB class, reusing much of the original source code.

  <h3>Usage</h3>

  \t TBBraw2h5 supports reading in the raw TBB time-series data from files or
  an udp socket. It does not add data to an existing hdf5 file.

  <table border="0">
    <tr>
    <td class="indexkey">Command line</td>
    <td class="indexkey">Decription</td>
    </tr>
    <tr>
      <td>-H [--help]</td>
      <td>Show help messages</td>
    </tr>
    <tr>
      <td>-O [--outfile] arg</td>
      <td>Name of the output dataset. If the file already exists the programm terminates.</td>
    </tr>
    <tr>
      <td>-I [--infile] arg</td>
      <td>Name of the input file. Mutually exclusive to the -P option.</td>
    </tr>
    <tr>
      <td>-P [--port] arg</td>
      <td>Port number to accept data from. Mutually exclusive to the -I option.</td>
    </tr>
    <tr>
      <td>--ip arg</td>
      <td>Hostname/IP address from which to accept the data (currently not used)</td>
    </tr>
    <tr>
      <td>-S [--timeoutStart] arg</td>
      <td>Time-out, [sec], when opening socket connection, before dropping the
      conection. If the provided value is smaller but zero (which is the default)
      the connection to the port is kept open indefinitely.</td>
    </tr>
    <tr>
      <td>-R [--timeoutRead] arg</td>
      <td>Time-out, [sec], when opening socket connection, before dropping the
      conection. If the provided value is smaller but zero (which is the default)
      the connection to the port is kept open indefinitely.</td>
    </tr>
    <tr>
      <td>-F [--fixTimes] arg</td>
      <td>Fix broken time stamps (generated by the RSPs and passed through by the TBBs).
      (0): do not change time stamps
      (1): change old style time stamps: substract 1 from the second-counter of the last
      frame of a second and add 512 to the sample_nr of (true) odd-numbered second frames
      in 200MHz mode
      (2): change new style time stamps: add 512 to the sample_nr of odd-numbered second
      frames in 200MHz mode (default)
      </td>
    </tr>
    <tr>
      <td>-C [--doCheckCRC] arg</td>
      <td> Check the CRCs of the frames:
      (0): do not check the CRCs
      (1): check the header CRCs and discard broken frames (default)
      </td>
      <tr>
    <td>-B [--bufferSize] arg</td>
      <td> Size of the input buffer (in frames) when reading from a socket. The default is 
      50000, which is about 100MByte. </td>
    </tr>
    <tr>
      <td>-K [--keepRunning]</td>
      <td>Keep running, i.e. process more than one event by restarting the procedure.</td>
    </tr>
    <tr>
      <td>-V [--verbose]</td>
      <td>Enable verbose mode, showing status messages during processing.</td>
    </tr>
  </table>


*/

//includes for networking
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
//includes for threading
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
//includes for the commandline options
#include <boost/program_options.hpp>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/detail/cmdline.hpp>
namespace bpo = boost::program_options;

using namespace DAL;

//Global variables
//! (pointer to) the TBBraw object we are writing to.
TBBraw *tbb;

//!size of the buffer for the UDP-datagram
// 1 byte larger than the frame size
#define UDP_PACKET_BUFFER_SIZE 2141
//!number of frames in the input buffer (50000 is ca. 100MB)
// (the vBuf of the system on the storage nodes can store ca. 800 frames)
//#define INPUT_BUFFER_SIZE 50000
int input_buffer_size;

//!pointers (array indices) for the last buffer processed and the last buffer written
int inBufProcessID,inBufStorID;
//!the Input Buffer
char * inputBuffer_p;
//!maximum number of frames waiting in the vBuf while reading
int maxWaitingFrames;
//!maximum number of frames in the buffer
int maxCachedFrames;
//!number of frames dropped due to buffer overflow
int noFramesDropped;
//!number of running reader-threads
int noRunning;
//!mutex for writing into the buffer
boost::mutex writeMutex;

// -----------------------------------------------------------------
/*!
  \brief Thread that creates and then reads from a socket into the buffer

  \param port -- UDP port number to read data from
  \param ip -- Hostname (ip-address) to read data from (not used)
  \param startTimeout -- Timeout when opening socket connection [in sec]
  \param readTimeout -- Timeout while reading from the socket [in sec]
  \param verbose -- Produce more output

  \return \t true if successful
*/
void socketReaderThread(int port, string ip, double startTimeout,
                        double readTimeout, bool verbose)
{
  // Create the main socket
  int main_socket;
  fd_set readSet;
  struct timeval TimeoutWait, TimeoutRead, NullTimeout;
  TimeoutRead.tv_sec = floor(readTimeout);
  TimeoutRead.tv_usec = (readTimeout-TimeoutRead.tv_sec)*1e6;
  NullTimeout.tv_sec = NullTimeout.tv_usec =0;
  main_socket = socket(PF_INET, SOCK_DGRAM, 0);
  if (main_socket<0)
    {
      cerr << "TBBraw2h5::socketReaderThread:"<<port<<": Failed to create the main socket."<<endl;
      boost::mutex::scoped_lock lock(writeMutex);
      noRunning--;
      return;
    };
  //Create a sockaddr_in to describe the local port
  sockaddr_in local_info;
  local_info.sin_family = AF_INET;
  local_info.sin_addr.s_addr = htonl(INADDR_ANY);
  local_info.sin_port = htons(port);
  //Bind the socket to the port
  int erg = bind(main_socket, (sockaddr *) &local_info, sizeof(local_info));
  if (erg<0)
    {
      cerr << "TBBraw2h5::socketReaderThread:"<<port<<": Failed to bind to port"
           << "(with ip: " << ip <<")"<< endl;
      boost::mutex::scoped_lock lock(writeMutex);
      noRunning--;
      return;
    };
  //Wait for the first data to arrive
  cout << "TBBraw2h5::socketReaderThread:"<<port<<": Waiting for data." << endl;
  FD_ZERO(&readSet);
  FD_SET(main_socket, &readSet);
  if (startTimeout > 0)
    {
      TimeoutWait.tv_sec = floor(startTimeout);
      TimeoutWait.tv_usec = (startTimeout-TimeoutWait.tv_sec)*1e6;
      select( main_socket + 1, &readSet, NULL, NULL, &TimeoutWait );
    }
  else
    {
      select( main_socket + 1, &readSet, NULL, NULL, NULL );
    };
  bool ImRunning=true;
  int status, numWaiting=0, newBufID;
  struct sockaddr_in incoming_addr;
  socklen_t socklen = sizeof(incoming_addr);
  while (ImRunning)
    {
      if (verbose)
        {
          FD_ZERO(&readSet);
          FD_SET(main_socket, &readSet);
          if ((status = select(main_socket + 1, &readSet, NULL, NULL, &NullTimeout)) )
            {
              numWaiting++;
            }
          else
            {
              boost::mutex::scoped_lock lock(writeMutex);
              if (numWaiting > maxWaitingFrames)
                {
                  maxWaitingFrames=numWaiting;
                };
              numWaiting=0;
            };
        };
      FD_ZERO(&readSet);
      FD_SET(main_socket, &readSet);
      TimeoutWait = TimeoutRead;
      if ((status = select(main_socket + 1, &readSet, NULL, NULL, &TimeoutWait)) )
        {
          //there is a frame waiting in the vBuffer
          {
            //  make sure that we are the only ones writing to the input buffer
            //  (concurrent reading is O.K.)
            boost::mutex::scoped_lock lock(writeMutex);
            newBufID = inBufStorID+1;
            if (newBufID >= input_buffer_size)
              {
                newBufID =0;
              }
            if (newBufID == inBufProcessID)
              {
                noFramesDropped++;
                newBufID = inBufStorID;
              };
            //perform the actual read
            erg = recvfrom( main_socket, (inputBuffer_p + (newBufID*UDP_PACKET_BUFFER_SIZE)),
                            UDP_PACKET_BUFFER_SIZE, 0, (sockaddr *) &incoming_addr, &socklen);
            inBufStorID = newBufID;
          }
          ;// writeMutex lock is released here
          if (verbose)
            {
              if (erg != 2140)
                {
                  cout << "TBBraw2h5::socketReaderThread:"<<port
                       << ": Received strange packet size: " << erg <<endl;
                };
            };
        }
      else
        {
          if (verbose)
            {
              cout << "TBBraw2h5::socketReaderThread:"<<port<<": Data stopped coming." << endl;
            };
          ImRunning=false;
        };
    };
  {
    boost::mutex::scoped_lock lock(writeMutex);
    noRunning--;
  };
  close(main_socket);
  return;
};


// -----------------------------------------------------------------
/*!
  \brief Read the TBB-data from an udp socket

  \param port -- UDP port number to read data from
  \param ip -- Hostname (ip-address) to read data from (not used)
  \param startTimeout -- Timeout when opening socket connection [in sec]
  \param readTimeout -- Timeout while reading from the socket [in sec]
  \param verbose -- Produce more output

  \return \t true if successful
*/
bool readFromSocket(int port, string ip, float startTimeout,
                    float readTimeout, bool verbose=false)
{
  // initialize the buffer
  maxCachedFrames = maxWaitingFrames = 0;
  inBufProcessID = inBufStorID =0;
  noRunning = 0;
  inputBuffer_p = new char[(input_buffer_size*UDP_PACKET_BUFFER_SIZE)];
  if (inputBuffer_p == NULL)
    {
      cerr << "TBBraw2h5::readFromSocket: Failed to allocate input buffer!" <<endl;
      return false;
    }
  else if (verbose)
    {
      cout << "TBBraw2h5::readFromSocket: Allocated " << input_buffer_size*UDP_PACKET_BUFFER_SIZE
           << " bytes for the input buffer." << endl;
    };
  // start the reader-thread
  boost::thread readerTread(boost::bind(socketReaderThread, port, ip,
                                        startTimeout, readTimeout, verbose));
  noRunning++;
  int processingID, tmpint;
  int amWaiting=0;
  while ((noRunning>0) || (inBufStorID != inBufProcessID) )
    {
      if (inBufStorID == inBufProcessID)
        {
          if (verbose && ((amWaiting%100)==1) )
            {
              cout << "TBBraw2h5::readFromSocket: Buffer is empty! waiting." << endl;
              cout << "  Status: inBufStorID:" << inBufStorID << " inBufProcessID: " << inBufProcessID
                   << " noRunning: " << noRunning << " waiting for: " << amWaiting*0.11 << " sec."<< endl;
            };
          amWaiting++;
          usleep(100000);
          continue;
        };
      amWaiting=0;
      tmpint = (inBufStorID-inBufProcessID+input_buffer_size)%input_buffer_size;
      if (tmpint > maxCachedFrames)
        {
          maxCachedFrames = tmpint;
        };
      processingID = inBufProcessID+1;
      if (processingID >= input_buffer_size)
        {
          processingID -= input_buffer_size;
        };
      tbb->processTBBrawBlock( (inputBuffer_p + (processingID*UDP_PACKET_BUFFER_SIZE)),
                               UDP_PACKET_BUFFER_SIZE);
      inBufProcessID = processingID;
    };
  readerTread.join();
  delete inputBuffer_p;
  if (verbose)
    {
      cout << "Socket and Buffer Stats: Maximum # of waiting frames:" << maxWaitingFrames << endl;
      cout << "                        Maximum # of frames in cache:" << maxCachedFrames << endl;
      cout << "     Number of frames dropped due to buffer overflow:" << noFramesDropped << endl;
    };
  return true;
};


// -----------------------------------------------------------------
/*!
  \brief Read the TBB-data from a file

  \param infile -- Path to the input file.
  \param verbose -- Produce more output

  \return \t true if successful
*/
bool readFromFile(string infile, bool verbose=false)
{
  struct stat filestat;
  int i, numblocks, size;
  char buffer[TBB_FRAME_SIZE];

  stat(infile.c_str(), &filestat);
  numblocks = filestat.st_size / TBB_FRAME_SIZE;
  if (numblocks < 1)
    {
      cerr << "TBBraw2h5::readFromFile " << infile << " too small (smaller than one blocksize)." <<endl;
      return false;
    };
  if (verbose)
    {
      cout << "TBBraw2h5::readFromFile reading in " << numblocks << " blocks." << endl;
    };
  FILE *fd = fopen(infile.c_str(),"r");
  if (fd == NULL)
    {
      cerr << "TBBraw2h5::readFromFile: Can't open file: " << infile << endl;
      return false;
    };
  for (i=0; i<numblocks; i++)
    {
      size = fread(buffer, 1, TBB_FRAME_SIZE, fd);
      if ( (size != TBB_FRAME_SIZE )  )
        {
          if (verbose)
            {
              cout << "TBBraw2h5::readFromFile Cannot read in block: " << i << endl;
            };
          fclose(fd);
          return false;
        };
      tbb->processTBBrawBlock(buffer, size);
    };
  fclose(fd);
  return true;
};

// -----------------------------------------------------------------
// Main routine
int main(int argc, char *argv[])
{
  std::string infile;
  std::string outfile("test-TBBraw"),outfileOrig;
  std::string ip("All Hosts");
  int port;
  bool verboseMode(false);
  float timeoutStart(0);
  float timeoutRead(.5);
  int fixTransientTimes(2);
  int doCheckCRC(1);
  int socketmode(-1);
  bool keepRunning(false);
  int runNumber(0);

  input_buffer_size = 50000;
  

  bpo::options_description desc ("[TBBraw2h5] Available command line options");

  desc.add_options ()
  ("help,H", "Show help messages")
  ("outfile,O",bpo::value<std::string>(), "Name of the output dataset")
  ("infile,I", bpo::value<std::string>(), "Name of the input file, Mutually exclusive to -P")
  ("port,P", bpo::value<int>(), "Port number to accept data from, Mutually exclusive to -I")
  ("ip", bpo::value<std::string>(), "Hostname/IP address from which to accept the data")
  ("timeoutStart,S", bpo::value<float>(), "Time-out when opening socket connection, [sec].")
  ("timeoutRead,R", bpo::value<float>(), "Time-out when while reading from socket, [sec].")
  ("fixTimes,F", bpo::value<int>(), "Fix broken time-stamps old style (1), new style (2, default), or not (0)")
  ("doCheckCRC,C", bpo::value<int>(), "Check the CRCs: (0) no check, (1,default) check header.")
  ("bufferSize,B", bpo::value<int>(), "Size of the input buffer, [frames] (default=50000, about 100MB).")
  ("keepRunning,K", "Keep running, i.e. process more than one event by restarting the procedure.")
  ("verbose,V", "Verbose mode on")
  ;


  bpo::variables_map vm;
  bpo::store (bpo::parse_command_line(argc,argv,desc), vm);

  if (vm.count("help") || argc == 1)
    {
      cout << "\n" << desc << endl;
      return 0;
    }

  if (vm.count("verbose"))
    {
      verboseMode=true;
    }

  if (vm.count("keepRunning"))
    {
      keepRunning=true;
    }

  if (vm.count("infile"))
    {
      infile     = vm["infile"].as<std::string>();
      socketmode = 0;
    };

  if (vm.count("outfile"))
    {
      outfile = vm["outfile"].as<std::string>();
    }

  if (vm.count("ip"))
    {
      ip = vm["ip"].as<std::string>();
    }

  if (vm.count("port"))
    {
      port = vm["port"].as<int>();
      socketmode = 1;
    }

  if (vm.count("timeoutStart"))
    {
      timeoutStart = vm["timeoutStart"].as<float>();
    }

  if (vm.count("timeoutRead"))
    {
      timeoutRead = vm["timeoutRead"].as<float>();
    }

  if (vm.count("fixTimes"))
    {
      fixTransientTimes = vm["fixTimes"].as<int>();
    }

  if (vm.count("doCheckCRC"))
    {
      doCheckCRC = vm["doCheckCRC"].as<int>();
    }
  
  if (vm.count("bufferSize"))
    {
      input_buffer_size = vm["bufferSize"].as<int>();
    }
  

  // -----------------------------------------------------------------
  // Check the provided input

  if (vm.count("infile") && vm.count("port"))
    {
      cout << "[TBBraw2h5] Both input file and port number given, chose one of the two!" << endl;
      cout << endl << desc << endl;
      return 1;
    };

  if (socketmode == -1)
    {
      cout << "[TBBraw2h5] Neither input file nor port number given, chose one of the two!" << endl;
      cout << endl << desc << endl;
      return 1;
    };

  if (input_buffer_size < 100) 
    {
      cout << "[TBBraw2h5] Buffer Size too small ("<< input_buffer_size << "<100), setting to default value" << endl;
      input_buffer_size = 50000;
    };

  if (keepRunning && !socketmode)
    {
      cout << "[TBBraw2h5] KeepRunning only usefull in socketmode, option disabled!" << endl;
      keepRunning = false;
    };

  // -----------------------------------------------------------------
  // Feedback on the settings

  if (verboseMode)
    {
      std::cout << "[TBBraw2h5] Summary of parameters."        << std::endl;
      std::cout << "-- Socket mode  = " << socketmode          << std::endl;
      std::cout << "-- Output file  = " << outfile             << std::endl;
      std::cout << "-- CRC checking = " << doCheckCRC          << std::endl;
      std::cout << "-- Fix Times    = " << fixTransientTimes   << std::endl;
      if (socketmode) {
	std::cout << "-- IP address      = " << ip             << std::endl;
	std::cout << "-- Port number     = " << port           << std::endl;
	std::cout << "-- Timeout (start) = " << timeoutStart   << std::endl;
	std::cout << "-- Timeout (read)  = " << timeoutRead    << std::endl;
      }
      else {
	std::cout << "-- Input file   = " << infile  << std::endl;
      }
    }
  

  if (keepRunning) { 
      outfileOrig = outfile ; 
    };
  // -----------------------------------------------------------------
  // Begin of "keepRunning" loop
  do 
    {
    if (keepRunning) 
      {
	std::ostringstream temp;
	temp << outfileOrig << "-" << runNumber;
	outfile = temp.str();
	runNumber++;
      };
    
    // -----------------------------------------------------------------
    // Generate TBBraw object and open output file
    
    tbb = new TBBraw(outfile);
    if ( !tbb->isConnected() )
      {
	cout << "[TBBraw2h5] Failed to open output file." << endl;
	return 1;
      };
    
    // -----------------------------------------------------------------
    // Set the options in the TBBraw object
    
    if (doCheckCRC>0) {
      tbb->doHeaderCRC(true);
    }
    else {
      tbb->doHeaderCRC(false);
    };
    tbb->setFixTimes(fixTransientTimes);
    
    // -----------------------------------------------------------------
    // call the conversion routines
    
    if (socketmode) {
      readFromSocket(port, ip, timeoutStart, timeoutRead, verboseMode);
    }
    else {
      readFromFile(infile, verboseMode);
    };
    
    // -----------------------------------------------------------------
    //finish up, print some statistics.
    tbb->summary();

    // and free the memory:
    delete tbb;

    
    // -----------------------------------------------------------------
    // End of "keepRunning" loop
    } 
  while (keepRunning);
  
  return 0;
};
