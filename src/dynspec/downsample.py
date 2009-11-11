#!/usr/bin/env ipython

"""Program for downsampling beam formed HDF5 files."""

import pydal, pylab, numpy, time, sys, warnings, pyfits, rebin, os

display=True

if (len(sys.argv) < 2): sys.exit("Usage: downsample.py path")

#os.listdir('.') #sys.argv[1]
filename_h5=sys.argv[1]
filename_fits=filename_h5.replace('.h5','.fits')
if(os.path.exists(filename_fits)):
    print filename_fits, 'already exists, exitting now'
    os._exit(0)
 
print 'creating '+filename_fits
warnings.simplefilter("ignore",DeprecationWarning)
File = pydal.BeamFormed(filename_h5)
Beam = File.getBeam(0)
DDS=pydal.dalDataset(filename_h5,"HDF5")
samples=DDS.openTable("beam000/SB000").getNumberOfRows()
duration=samples*DDS.getAttribute_int("DOWNSAMPLE_RATE") \
          /DDS.getAttribute_double("SAMPLE_RATE")
ds=1000
ns=int(samples/ds)*ds
dt=duration*ns/samples
Subbands=numpy.empty((Beam.nofSubbands(),int(samples/ds)))
nsub=Beam.nofSubbands()

print "samples: ",samples," ns:",ns," ds: ",ds

if display:    pylab.draw()

for s in range(nsub):
    tstart=time.time()
    subband=rebin.rebin(Beam.getIntensitySquared(s,0,ns), ds)
    Subbands[s,]=subband
    print 'rebinning subband',s,' (',time.time()-tstart,'s)'
    if display:
        sp=pylab.imshow(numpy.log(Subbands),extent=(0,dt,nsub,0),
                        aspect='auto', interpolation='nearest')
        canvas=sp.figure.canvas
        canvas.blit(sp.figure.bbox)
        
pylab.colorbar(fraction=0.07, pad=0)
pylab.xlabel('time [s]')
pylab.ylabel('frequency [MHz]')
pylab.title(filename_h5)
#numpy.savez("preview.npz",Subbands=Subbands, dt=dt)
#Subbands=numpy.load("Subbands.npz")
#pylab.savefig(filename+".png")
HDU = pyfits.PrimaryHDU(Subbands)
HDU.writeto(filename_fits)
print time.time()-tstart,"s"

