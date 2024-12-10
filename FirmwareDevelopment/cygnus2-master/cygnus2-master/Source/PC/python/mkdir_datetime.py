#
# Script to create a directory with a name derived from the current date / time
# The date format is selected so that fields are arranged in order of significance
# Intended to be used for collecting logs
#
# Written in Python 3.6
#

import datetime
import os

now = datetime.datetime.now()

logdir = now.strftime("%Y-%m-%d_%H-%M-%S")
if not os.path.exists(logdir):
    os.makedirs(logdir)
