"""
This is a model imaging pipeline definition.

Although it should be runnable as it stands, the user is encouraged to copy it
to a job directory and customise it as appropriate for the particular task at
hand.
"""
from __future__ import with_statement
from contextlib import closing
from itertools import repeat
import sys
import os

from pyrap.quanta import quantity

from lofarpipe.support.control import control
from lofarpipe.support.utilities import log_time, patch_parset

class sip(control):
    def pipeline_logic(self):
        from to_process import datafiles # datafiles is a list of MS paths.
        temporary_files = [] # will be cleaned up at end of run.
        with log_time(self.logger):
            try:
                # Build a map of compute node <-> data location on storage nodes.
                storage_mapfile = self.run_task(
                    "datamapper", datafiles
                )['mapfile']

                # Produce a GVDS file describing the data on the storage nodes.
                self.run_task('vdsmaker', storage_mapfile)
                self._save_state()

                # Read metadata (start, end times, pointing direction) from GVDS.
                vdsinfo = self.run_task("vdsreader")
                self._save_state()

                # NDPPP reads the data from the storage nodes, according to the
                # map. It returns a new map, describing the location of data on
                # the compute nodes.
                compute_mapfile = self.run_task(
                    "ndppp",
                    storage_mapfile,
                    parset=os.path.join(
                        self.config.get("layout", "parset_directory"),
                        "ndppp.1.initial.parset"
                    ),
                    data_start_time=vdsinfo['start_time'],
                    data_end_time=vdsinfo['end_time']
                )['mapfile']
                self._save_state()

                # Build a sky model ready for BBS & return the name of the
                # central source.
                ra = quantity(vdsinfo['pointing']['ra']).get_value('deg')
                dec = quantity(vdsinfo['pointing']['dec']).get_value('deg')
                source_name = self.run_task(
                    "skymodel", ra=ra, dec=dec, search_size=2.5
                )['source_name']

                # Patch the name of the central source into the BBS parset for
                # subtraction.
                bbs_parset = patch_parset(
                    self.task_definitions.get("bbs", "parset"),
                    {
                        'Step.correct.Model.Sources': "[ \"%s\" ]" % (source_name),
                        'Step.subtract.Model.Sources': "[ \"%s\" ]" % (source_name)
                    }
                )
                self.logger.info("BBS parset is %s" % bbs_parset)
                temporary_files.append(bbs_parset)

                # BBS modifies data in place, so the map produced by NDPPP
                # remains valid.
                self.run_task("bbs", compute_mapfile, parset=bbs_parset)
                self._save_state()

                # Now, run DPPP three times on the output of BBS. We'll run
                # this twice: once on CORRECTED_DATA, and once on
                # SUBTRACTED_DATA.
                for i in repeat(None, 3):
                    self.run_task(
                        "ndppp",
                        compute_mapfile,
                        parset=os.path.join(
                            self.config.get("layout", "parset_directory"),
                            "ndppp.1.postbbs.parset"
                        ),
                        data_start_time=vdsinfo['start_time'],
                        data_end_time=vdsinfo['end_time'],
                        suffix=""
                    )
                    self._save_state()

                subtracted_ndppp_parset = patch_parset(
                    os.path.join(
                        self.config.get("layout", "parset_directory"),
                        "ndppp.1.postbbs.parset"
                    ),
                    {
                        "msin.datacolumn": "SUBTRACTED_DATA",
                        "msout.datacolumn": "SUBTRACTED_DATA"
                    },
                    output_dir=self.config.get("layout", "parset_directory")
                )
                temporary_files.append(subtracted_ndppp_parset)

                for i in repeat(None, 3):
                    self.run_task(
                        "ndppp",
                        compute_mapfile,
                        parset=subtracted_ndppp_parset,
                        data_start_time=vdsinfo['start_time'],
                        data_end_time=vdsinfo['end_time'],
                        suffix=""
                    )
                    self._save_state()

                # Image CORRECTED_DATA
                self.logger.info("Imaging CORRECTED_DATA")

                # Patch the pointing direction recorded in the VDS file into
                # the parset for the cimager.
                imager_parset = patch_parset(
                    self.task_definitions.get("cimager", "parset"),
                    {
                        'Images.ra': quantity(vdsinfo['pointing']['ra']).formatted("time"),
                        'Images.dec': quantity(vdsinfo['pointing']['dec']).formatted("angle")
                    },
                    output_dir=self.config.get("layout", "parset_directory")

                )
                temporary_files.append(imager_parset)

                # And run cimager.
                self.outputs['images'] = self.run_task(
                    "cimager", compute_mapfile,
                    parset=imager_parset,
                    results_dir=os.path.join(
                        self.config.get("layout", "results_directory"),
                        "corrected"
                    )
                )['images']

                # Image SUBTRACTED_DATA
                self.logger.info("Imaging SUBTRACTED_DATA")

                # Patch the pointing direction recorded in the VDS file into
                # the parset for the cimager, and change the column to be
                # imaged.
                subtracted_imager_parset = patch_parset(
                    self.task_definitions.get("cimager", "parset"),
                    {
                        'Images.ra': quantity(vdsinfo['pointing']['ra']).formatted("time"),
                        'Images.dec': quantity(vdsinfo['pointing']['dec']).formatted("angle"),
                        'datacolumn': "SUBTRACTED_DATA"

                    },
                    output_dir=self.config.get("layout", "parset_directory")

                )
                temporary_files.append(subtracted_imager_parset)

                # And run cimager.
                self.outputs['images'] = self.run_task(
                    "cimager", compute_mapfile,
                    parset=subtracted_imager_parset,
                    results_dir=os.path.join(
                        self.config.get("layout", "results_directory"),
                        "subtracted"
                    )
                )['images']

            finally:
                # Clean up temporary files.
                [os.unlink(file) for file in temporary_files if os.path.exists(file)]

if __name__ == '__main__':
    sys.exit(sip().main())
