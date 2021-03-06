#!/usr/bin/env perl

use lib $ENV{"DADA_ROOT"}."/bin";

use strict;
use warnings;
use File::Basename;
use Getopt::Std;
use Apsr;
use Dada::server_web_monitor qw(%cfg);


Dada::preventDuplicateDaemon(basename($0));

#
# Global Variable Declarations
#
%cfg = Apsr::getConfig();

#
# Initialize module variables
#
$Dada::server_web_monitor::dl = 1;
$Dada::server_web_monitor::daemon_name = Dada::daemonBaseName($0);


# Autoflush STDOUT
$| = 1;

my $result = 0;
$result = Dada::server_web_monitor->main();

exit($result);

