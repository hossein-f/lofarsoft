from __future__ import with_statement
import os
import tempfile
import subprocess
import shutil
import threading

from lofarpipe.support.lofarrecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.ipython import LOFARTask
from lofarpipe.support.clusterlogger import clusterlogger
from lofarpipe.support.remotecommand import ProcessLimiter
from lofarpipe.support.group_data import load_data_map
from lofarpipe.support.pipelinelogging import log_process_output
import lofarpipe.support.utilities as utilities
import lofarpipe.support.lofaringredient as ingredient

template = """
create tablename="%s"
adddef Gain:0:0:Ampl  values=1.0
adddef Gain:1:1:Ampl  values=1.0
adddef Gain:0:0:Real  values=1.0
adddef Gain:1:1:Real  values=1.0
adddef DirectionalGain:0:0:Ampl  values=1.0
adddef DirectionalGain:1:1:Ampl  values=1.0
adddef DirectionalGain:0:0:Real  values=1.0
adddef DirectionalGain:1:1:Real  values=1.0
adddef AntennaOrientation values=5.497787144
quit
"""

class parmdb(BaseRecipe, RemoteCommandRecipeMixIn):
    inputs = {
        'executable': ingredient.ExecField(
            '--executable',
            help="Executable for parmdbm",
            default="/opt/LofIm/daily/lofar/bin/parmdbm"
        ),
        'nproc': ingredient.IntField(
            '--nproc',
            help="Maximum number of simultaneous processes per compute node",
            default=8
        )
    }

    outputs = {
        'mapfile': ingredient.FileField()
    }

    def go(self):
        self.logger.info("Starting parmdb run")
        super(parmdb, self).go()

        self.logger.info("Generating template parmdb")
        pdbdir = tempfile.mkdtemp(
            dir=self.config.get("layout", "parset_directory")
        )
        pdbfile = os.path.join(pdbdir, 'instrument')

        parmdbm_process = subprocess.Popen(
            [self.inputs['executable']],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        sout, serr = parmdbm_process.communicate(template % pdbfile)
        log_process_output("parmdbm", sout, serr, self.logger)

        #                           Load file <-> compute node mapping from disk
        # ----------------------------------------------------------------------
        self.logger.debug("Loading map from %s" % self.inputs['args'][0])
        data = load_data_map(self.inputs['args'][0])

        #                               Limit number of process per compute node
        # ----------------------------------------------------------------------
        self.logger.debug("Limit to %s processes/node" % self.inputs['nproc'])
        compute_nodes_lock = ProcessLimiter(self.inputs['nproc'])

        command = "python %s" % (self.__file__.replace('master', 'nodes'))
        with clusterlogger(self.logger) as (loghost, logport):
            self.logger.debug("Logging to %s:%d" % (loghost, logport))
            with utilities.log_time(self.logger):
                parmdb_threads = []
                for host, ms in data:
                    parmdb_threads.append(
                        threading.Thread(
                            target=self._dispatch_compute_job,
                            args=(host, command, compute_nodes_lock[host],
                                loghost, str(logport),
                                ms, pdbfile
                            )
                        )
                    )
                [thread.start() for thread in parmdb_threads]
                self.logger.info("Waiting for parmdb threads")
                [thread.join() for thread in parmdb_threads]

        self.logger.debug("Removing template parmdb")
        shutil.rmtree(pdbdir, ignore_errors=True)

        if self.error.isSet():
            return 1
        else:
            self.outputs['mapfile'] = self.inputs['args'][0]
            return 0

if __name__ == '__main__':
    sys.exit(parmdb().main())
