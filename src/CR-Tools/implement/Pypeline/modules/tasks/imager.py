"""
Imager documentation
====================

"""

from pycrtools.tasks import Task
from pycrtools.grid import CoordinateGrid
import pycrtools as cr
import pytmf
import numpy as np
import time

class Imager(Task):
    """Imager task documentation.
    """

    parameters = {
        'image' : { "default" : None, "positional" : 1 },
        'data' : { "default" : None, "positional" : 2 },
        'startblock' : { "default" : 0 },
        'nblocks' : { "default" : 16 },
        'ntimesteps' : { "default" : 1 },
        'obstime' : { "default" : 0 },
        'L' : { "default" : pytmf.deg2rad(6.869837540) },
        'phi' : { "default" : pytmf.deg2rad(52.915122495) },
        'NAXIS' : { "default" : 2 },
        'NAXIS1' : { "default" : 90 },
        'NAXIS2' : { "default" : 90 },
        'CTYPE1' : { "default" : 'ALON-STG' },
        'CTYPE2' : { "default" : 'ALAT-STG' },
        'LONPOLE' : { "default" : 0. },
        'LATPOLE' : { "default" : 90. },
        'CRVAL1' : { "default" : 180. },
        'CRVAL2' : { "default" : 90. },
        'CRPIX1' : { "default" : 45.5 },
        'CRPIX2' : { "default" : 45.5 },
        'CDELT1' : { "default" : 2.566666603088E+00 },
        'CDELT2' : { "default" : 2.566666603088E+00 },
        'CUNIT1' : { "default" : 'deg' },
        'CUNIT2' : { "default" : 'deg' },
        'PC001001' : { "default" : 1.000000000000E+00 },
        'PC002001' : { "default" : 0.000000000000E+00 },
        'PC001002' : { "default" : 0.000000000000E+00 },
        'PC002002' : { "default" : 1.000000000000E+00 }
    }

    def init(self):
        """Initialize imager.
        """

        # Generate coordinate grid
        print "Generating grid"
        self.grid=CoordinateGrid(obstime=self.obstime,
                                 L=self.L,
                                 phi=self.phi,
                                 NAXIS=self.NAXIS,
                                 NAXIS1=self.NAXIS1,
                                 NAXIS2=self.NAXIS2,
                                 CTYPE1=self.CTYPE1,
                                 CTYPE2=self.CTYPE2,
                                 LONPOLE=self.LONPOLE,
                                 LATPOLE=self.LATPOLE,
                                 CRVAL1=self.CRVAL1,
                                 CRVAL2=self.CRVAL2,
                                 CRPIX1=self.CRPIX1,
                                 CRPIX2=self.CRPIX2,
                                 CDELT1=self.CDELT1,
                                 CDELT2=self.CDELT2,
                                 CUNIT1=self.CUNIT1,
                                 CUNIT2=self.CUNIT2,
                                 PC001001=self.PC001001,
                                 PC002001=self.PC002001,
                                 PC001002=self.PC001002,
                                 PC002002=self.PC002002)
        print "Grid generation finished"
        print self.grid

        # Get frequencies
        self.frequencies=self.data.getFrequencies()
        self.nfreq = len(self.frequencies)

        print "Frequency range:", self.frequencies[0], self.frequencies[-1], "Hz"

        # Get antenna positions
        self.antpos=self.data.getRelativeAntennaPositions()
        self.nantennas=int(self.antpos.getDim()[0])

#        # Calculate geometric delays for all sky positions for all antennas
#        self.delays = cr.hArray(float, dimensions=(self.NAXIS1, self.NAXIS2, self.nantennas))
#        cr.hGeometricDelays(self.delays, self.antpos, self.grid.cartesian, True)

        # Initialize empty arrays
        self.fftdata=cr.hArray(complex, dimensions=(self.nantennas, self.nfreq))
        self.t_image=cr.hArray(complex, dimensions=(self.NAXIS1, self.NAXIS2, self.nfreq), fill=0.)

    def run(self):
        """Run the imager.
        """

        start = time.time()
        for step in range(self.ntimesteps):
            for block in range(self.startblock, self.startblock+self.nblocks):

                print "processing block:", block

                self.data.getFFTData(self.fftdata, block)

                print "reading done"

                self.t_image.fill(0.)

                print "beamforming started"
#                cr.hBeamformImage(self.t_image, self.fftdata, self.frequencies, self.delays)
                cr.hBeamformImage(self.t_image, self.fftdata, self.frequencies, self.antpos, self.grid.cartesian)
                print "beamforming done"

                cr.hAbsSquareAdd(self.image[step], self.t_image)

            self.startblock += self.nblocks

        end = time.time()
        print "total runtime:", end-start, "s"

