import sys
sys.path.append('')

import bdsm


bdsm.execute(bdsm.fits_chain,
             {'fits_name': 'WN65341H.fits',
              'beam': (.015, .015, 0)})
