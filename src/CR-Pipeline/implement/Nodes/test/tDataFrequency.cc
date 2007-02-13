
/* $Id: tDataFrequency.cc,v 1.1 2005/07/15 07:16:47 bahren Exp $ */

#include <casa/aips.h>
#include <tasking/Glish.h>

#include <Nodes/DataFrequency.h>

#include <casa/namespace.h>

/*!
  \file tDataFrequency.cc

  \brief A collection of test routines for DataFrequency.

  \author Lars B&auml;hren

  \data 2005/03/14
*/

using CR::DataFrequency;

// =============================================================================

GlishRecord setupGlishRecord () {

  GlishRecord rec;
  
  Double FrequencyUnit  = 1e06;
  Double SamplerateUnit = 1e06;
  Double Samplerate     = 80;
  Int NyquistZone       = 2;
  Double FrequencyLow   = 0e06;
  Double FrequencyHigh  = 40e06;
  Int FFTSize           = 8191;
  Int FFTlen            = 4096;

  rec.add("FrequencyUnit",FrequencyUnit);
  rec.add("SamplerateUnit",SamplerateUnit);
  rec.add("Samplerate",Samplerate);
  rec.add("NyquistZone",NyquistZone);
  rec.add("FrequencyLow",FrequencyLow);
  rec.add("FrequencyHigh",FrequencyHigh);
  rec.add("FFTSize",FFTSize);
  rec.add("FFTlen",FFTlen);

  return rec;
}

// =============================================================================

void test_construction (GlishRecord& rec) {

  cout << "\n[tDataFrequency::test_construction]\n" << endl;

  // Empty Constructor
  {
    DataFrequency df = DataFrequency ();
    df.show(std::cout);
  }

  // Argumented constructor
  {
    DataFrequency df (rec);
    df.show(std::cout);
  }

}

// =============================================================================

void test_subBands (GlishRecord& rec)
{
  cout << "\n[tDataFrequency::test_subBands]\n" << endl;

  Int nofBands = 10;
  String division;
  Vector<Double> freqRange;
  Double bandwidth;
  Double frac;

  DataFrequency df (rec);

  freqRange = df.frequencyRange ();

  bandwidth = freqRange(1)-freqRange(0);
  frac = 0.1*bandwidth;

  for (int i=0; i<5; i++) {
    freqRange(0) = freqRange(1)-bandwidth+i*frac;
    //
    division = "lin";
    df.setFrequencyBands (freqRange,nofBands,division);
    //
    division = "log";
    df.setFrequencyBands (freqRange,nofBands,division);
  }

}

// =============================================================================
//
//  Main routine
//
// =============================================================================

int main () {

  GlishRecord record = setupGlishRecord ();

  test_construction (record);

  test_subBands (record);

  return 0;

}
