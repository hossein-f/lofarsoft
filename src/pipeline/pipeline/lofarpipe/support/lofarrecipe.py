import os, sys
import utilities
from lofarpipe.support.lofarexceptions import PipelineException
from lofarpipe.cuisine.WSRTrecipe import WSRTrecipe
from IPython.kernel import client as IPclient
from ConfigParser import SafeConfigParser as ConfigParser

class LOFARrecipe(WSRTrecipe):
    """
    Provides standard boiler-plate used in the various LOFAR pipeline recipes.
    """
    def __init__(self):
        super(LOFARrecipe, self).__init__()
        self.optionparser.add_option(
            '-j', '--job-name', 
            dest="job_name",
            help="Job name (required)"
        )
        self.optionparser.add_option(
            '-r', '--runtime-directory', 
            dest="runtime_directory",
            help="Runtime directory"
        )
        self.optionparser.add_option(
            '-c', '--config', 
            dest="config",
            help="Configuration file"
        )
        self.optionparser.add_option(
            '-n', '--dry-run',
            dest="dry_run",
            help="Dry run",
            action="store_true"
        )
        self.optionparser.add_option(
            '--start-time',
            dest="start_time",
            help="[Expert use] Pipeline start time"
        )

    def run_task(self, configblock, datafiles=[]):
        self.logger.info("Running task: %s" % (configblock,))
        recipe = self.config.get(configblock, "recipe")
        inputs = LOFARinput(self.inputs)
        inputs['args'] = datafiles
        inputs.update(self.config.items(configblock))
        # These inputs are never required:
        for inp in ('recipe', 'recipe_directories', 'lofarroot', 'default_working_directory'):
            del(inputs[inp])
        outputs = LOFARoutput()
        if self.cook_recipe(recipe, inputs, outputs):
            self.logger.warn(
                "%s reports failure (using %s recipe)" % (configblock, recipe)
            )
            raise PipelineRecipeFailed("%s failed", configblock)
        try:
            return outputs['data']
        except:
            return None

    def go(self):
        # Every recipe needs a job identifier
        if not self.inputs["job_name"]:
            raise PipelineException("Job undefined")

        if not self.inputs["start_time"]:
            import datetime
            self.inputs["start_time"] = datetime.datetime.utcnow().replace(microsecond=0).isoformat()

        # If a config file hasn't been specified, use the default
        if not self.inputs["config"]:
            # Possible config files, in order of preference:
            conf_locations = (
                os.path.join(sys.path[0], 'pipeline.cfg'),
                os.path.join(os.path.expanduser('~'), '.pipeline.cfg')
            )
            for path in conf_locations:
                if os.access(path, os.R_OK):
                    self.inputs["config"] = path
                    break
            if not self.inputs["config"]:
                raise PipelineException("Configuration file not found")

        self.logger.debug("Pipeline start time: %s" % self.inputs['start_time'])

        self.config = ConfigParser({
            "job_name": self.inputs["job_name"],
            "runtime_directory": self.inputs["runtime_directory"],
            "start_time": self.inputs["start_time"]
        })
        self.config.read(self.inputs["config"])

        self.recipe_path = utilities.string_to_list(
            self.config.get('DEFAULT', "recipe_directories")
        )

        if not self.inputs['runtime_directory']:
            self.inputs["runtime_directory"] = self.config.get(
                "DEFAULT", "runtime_directory"
            )
            if not os.access(self.inputs['runtime_directory'], os.F_OK):
                raise IOError, "Runtime directory doesn't exist"

    def _get_cluster(self):
        self.logger.info("Connecting to IPython cluster")
        try:
            tc  = IPclient.TaskClient(self.config.get('cluster', 'task_furl'))
            mec = IPclient.MultiEngineClient(self.config.get('cluster', 'multiengine_furl'))
        except:
            self.logger.error("Unable to initialise cluster")
            raise utilities.ClusterError
        self.logger.info("Cluster initialised")
        return tc, mec
