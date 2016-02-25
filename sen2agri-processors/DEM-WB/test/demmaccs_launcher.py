#!/usr/bin/env python
from __future__ import print_function
import argparse
import re
import os
from os.path import isfile, isdir, join
import glob
import sys
import time, datetime
from common import *

parser = argparse.ArgumentParser(
    description="Launcher for DEM MACCS script")
parser.add_argument('-c', '--config', required=True,  help="configuration file")

args = parser.parse_args()


config = Config("Demmaccs")
if not config.loadConfig(args.config):
    print("Could not load the config")
    sys.exit(-1)

l1c_db = L1CInfo(config.host, config.database, config.user, config.password)
l1c_list = l1c_db.get_unprocessed_l1c()
if len(l1c_list) == 0:
    print("No unprocessed L1C found in DB")
    sys.exit(0)
print("Start the work for:")
print("{}".format(l1c_list))


