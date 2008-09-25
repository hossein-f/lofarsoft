/*-------------------------------------------------------------------------*
 | $Id::                                                                 $ |
 *-------------------------------------------------------------------------*
 ***************************************************************************
 *   Copyright (C) 2008                                                  *
 *   Andreas Horneffer (<mail>)                                                     *
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

#include <Analysis/tbbTools.h>

#define PI 3.1415926536

namespace CR { // Namespace CR -- begin
  
  // ============================================================================
  //
  //  Construction
  //
  // ============================================================================
  
  tbbTools::tbbTools ()
  {;}
    
  // ============================================================================
  //
  //  Destruction
  //
  // ============================================================================
  
  tbbTools::~tbbTools ()
  {
    destroy();
  }
  
  void tbbTools::destroy ()
  {;}  
  
  // ============================================================================
  //
  //  Methods
  //
  // ============================================================================
  
  Bool tbbTools::meanFPGAtrigger(Vector<Double> inary, int level, int start, int stop, int window,
				 int afterwindow,
				 Vector<Int> &index, Vector<Int> &sum, Vector<Int> &width, 
				 Vector<Int> &peak, Vector<Int> &meanval, Vector<Int> &afterval,
				 Bool reset){
    try {
      int i,numPulses=0,outsize=100;
      static int pos1=0, pos2=0, pos3=0, pos4=0, pos5=0, opos;
      static int apos2=0, apos3=0, apos4=0;
      static Bool pos6=False, isTrigger=False, doAfterTrigger=False, isAfterTrigger=False;
      static int startcount=0, stopcount=0, startindex=0, startmean=0, psum=0, pmax=0, aftercount=0;
      

      doAfterTrigger=False;
      if (reset) {
	// start a new dataset, reset all internal variables.
	pos1=0; pos2=0; pos3=0; pos4=0; pos5=0; opos=0; 
	apos2=0; apos3=0; apos4=0;
	pos6=False; isTrigger=False; isAfterTrigger=False;
	startcount=0; stopcount=0; startindex=0; startmean=0; psum=0; pmax=0; aftercount=0;
//       } else {
// 	//input data is the next block after the previeous processed.
      };

      index.resize(outsize);
      sum.resize(outsize);
      width.resize(outsize);
      peak.resize(outsize);
      meanval.resize(outsize);
      afterval.resize(outsize);
      if (afterwindow > 0) { doAfterTrigger=True; };
      
      for (i=0; i<(int)inary.nelements(); i++){
	opos = pos1;
	pos6 = pos1>pos5;
	pos5 = pos3*level;
	pos4 = pos2-pos3;
	pos3 = pos2/window;
	pos2 = pos1+pos4;
	if (doAfterTrigger){
	  apos4 = apos2-apos3;
	  apos3 = apos2/afterwindow;
	  apos2 = pos1+apos4;
	};
	pos1 = (int)abs(inary(i));
	if (reset && (i<window)) continue;
	if (isTrigger){
	  if (opos>pmax) pmax = opos;
	  psum += opos;
	  if (!pos6) {
	    stopcount++;
	  } else {
	    stopcount=0;
	  };
	  if (stopcount>=stop){
	    isTrigger=False;
	    index(numPulses) = startindex;
	    sum(numPulses) = psum;
	    peak(numPulses) = pmax;
	    width(numPulses) = i-startindex;
	    meanval(numPulses) = startmean;
	    afterval(numPulses) = -1;
	    numPulses++;
	    startcount=0;
	    stopcount=0;
	    if (doAfterTrigger) { isAfterTrigger = True; };
	  };	  
	} else if(isAfterTrigger) {
	  aftercount++;
	  if (aftercount>=afterwindow){
	    isAfterTrigger=False;
	    aftercount = 0;
	    if (numPulses>0) {afterval(numPulses-1) = apos3;}
	  };
	} else {
	  if (pos6) {
	    startcount += 4;
	    if (startcount>=start){
	      isTrigger=True;
	      startindex = i;
	      startmean = pos3;
	      pmax = opos;
	      psum = opos;
	    };
	  } else {
	    if (startcount>0) startcount -= 1;
	  };
	};
	if (numPulses>=outsize){
	  outsize += 100;
	  index.resize(outsize,True);
	  sum.resize(outsize,True);
	  width.resize(outsize,True);
	  peak.resize(outsize,True);
	  meanval.resize(outsize,True);
	  afterval.resize(outsize,True);
	};
      };    
      //save the values for the next block
      if (isTrigger) { startindex = startindex - i; };

      index.resize(numPulses,True);
      sum.resize(numPulses,True);
      width.resize(numPulses,True);
      peak.resize(numPulses,True);
      meanval.resize(numPulses,True);
      afterval.resize(numPulses,True);
    } catch (AipsError x) {
      cerr << "tbbTools::meanFPGAtrigger: " << x.getMesg() << endl;
      return False;
    }; 
    return True;    
  };
  
  Bool tbbTools::meanFPGAtrigger(Vector<Double> inary, int level, int start, int stop, int window,
				 Vector<Int> &index, Vector<Int> &sum, Vector<Int> &width, 
				 Vector<Int> &peak){
    Vector<Int> tmpvec;
    return meanFPGAtrigger(inary, level, start, stop, window, 0,
			   index, sum, width, peak, tmpvec, tmpvec);
  };


  Vector<Double> tbbTools::FPGAfilter(Vector<Double> &inary, int B0B2, int B1, int A1, int A2, 
				      Double resolution, Bool reset){
    Vector<Double> outdata;
    try {
      static Double oldin1=0., oldin2=0., oldout1=0., oldout2=0.;
      int len=inary.nelements();
      int i;
      
      if (len < 10) {
	cerr << "tbbTools::FPGAfilter: " << "input array too small (len<10)!";
	return inary;
      };
      
      outdata.resize(len);
      if (reset) {
	outdata(0) = outdata(1) = 0.;
      } else {	
	outdata(0) = floor((inary(0)*B0B2 + oldin1*B1 + oldin2*B0B2 
			    - oldout1*A1 - oldout2*A2)/resolution);
	outdata(1) = floor((inary(1)*B0B2 + inary(0)*B1 + oldin1*B0B2 
			    - outdata(0)*A1 - oldout1*A2)/resolution);
      };
      for (i=2; i<len; i++){
	outdata(i) = floor((inary(i)*B0B2 + inary(i-1)*B1 + inary(i-2)*B0B2 
			    - outdata(i-1)*A1 - outdata(i-2)*A2)/resolution);
      };
      oldin1 = inary(len-1);
      oldin2 = inary(len-2);
      oldout1 = outdata(len-1);
      oldout2 = outdata(len-2);

    } catch (AipsError x) {
      cerr << "tbbTools::FPGAfilter: " << x.getMesg() << endl;
      return Vector<Double>();
    }; 
    return outdata;
  }

  Vector<Double> tbbTools::FPGAfilterNotch(Vector<Double> &inary, Double F0, Double BW, Double SR, 
					   Double resolution, Bool reset){
    Double B0B2,B1A1,A0,A2;
    try {
      Double Q, w0, alpha;

      Q = F0/(2.*PI*BW);
      w0 = 2*PI*F0/SR;
      alpha = sin(w0)/(2*Q);
      
      B0B2 = 1;
      B1A1 = -2.*cos(w0);
      A0 = 1+alpha;
      A2 = 1-alpha;

      B0B2 = B0B2/A0*resolution;
      B1A1 = B1A1/A0*resolution;
      A2 = A2/A0*resolution;
      
#ifdef DEBUGGING_MESSAGES
      cout <<  "tbbTools::FPGAfilterNotch: " << "Set parameters: B0B2: " << (int)B0B2 
 	   << " B1A1: " << (int)B1A1 << " A2: " << (int)A2 << endl;
#endif      

    } catch (AipsError x) {
      cerr << "tbbTools::FPGAfilterNotch: " << x.getMesg() << endl;
      return Vector<Double>();
    }; 
    return FPGAfilter(inary, (int)B0B2, (int)B1A1, (int)B1A1, (int)A2, resolution, reset);
  }

  Vector<Double> tbbTools::FPGAfilterLPF(Vector<Double> &inary, Double F0, Double BW, Double SR, 
					 Double resolution, Bool reset){
    Double B0B2,B1,A1,A0,A2;
    try {
      Double Q, w0, alpha;

      Q = F0/(2.*PI*BW);
      w0 = 2*PI*F0/SR;
      alpha = sin(w0)/(2*Q);
      
      B0B2 = (1-cos(w0))/2.;
      B1 = 1-cos(w0);
      A1 = -2.*cos(w0);
      A0 = 1+alpha;
      A2 = 1-alpha;

      B0B2 = B0B2/A0*resolution;
      B1 = B1/A0*resolution;
      A1 = A1/A0*resolution;
      A2 = A2/A0*resolution;
      
#ifdef DEBUGGING_MESSAGES
      cout <<  "tbbTools::FPGAfilterLPF: " << "Set parameters: B0B2: " << (int)B0B2 
	   << " B1: " << (int)B1 << " A1: " << (int)A1 << " A2: " << (int)A2 << endl;
#endif      
 
    } catch (AipsError x) {
      cerr << "tbbTools::FPGAfilterLPF: " << x.getMesg() << endl;
      return Vector<Double>();
    }; 
    return FPGAfilter(inary, (int)B0B2, (int)B1, (int)A1, (int)A2, resolution, reset);
  }

  Vector<Double> tbbTools::FPGAfilterHPF(Vector<Double> &inary, Double F0, Double BW, Double SR, 
					 Double resolution, Bool reset){
    Double B0B2,B1,A1,A0,A2;
    try {
      Double Q, w0, alpha;

      Q = F0/(2.*PI*BW);
      w0 = 2*PI*F0/SR;
      alpha = sin(w0)/(2*Q);
      
      B0B2 = (1+cos(w0))/2.;
      B1 = -(1+cos(w0));
      A1 = -2.*cos(w0);
      A0 = 1+alpha;
      A2 = 1-alpha;

      B0B2 = B0B2/A0*resolution;
      B1 = B1/A0*resolution;
      A1 = A1/A0*resolution;
      A2 = A2/A0*resolution;
      
#ifdef DEBUGGING_MESSAGES
      cout <<  "tbbTools::FPGAfilterHPF: " << "Set parameters: B0B2: " << (int)B0B2 
	   << " B1: " << (int)B1 << " A1: " << (int)A1 << " A2: " << (int)A2 << endl;
#endif      
 
    } catch (AipsError x) {
      cerr << "tbbTools::FPGAfilterHPF: " << x.getMesg() << endl;
      return Vector<Double>();
    }; 
    return FPGAfilter(inary, (int)B0B2, (int)B1, (int)A1, (int)A2, resolution, reset);
  }
  

} // Namespace CR -- end
