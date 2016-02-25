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
    print("Could not load the config from configuration file")
    sys.exit(-1)

l1c_db = L1CInfo(config.host, config.database, config.user, config.password)
demmaccs_config = l1c_db.get_demmacs_config()
if demmaccs_config is None:
    print("Could not load the config from database")
    sys.exit(-1)
print(demmaccs_config.output_path)
print(demmaccs_config.gips_path)
print(demmaccs_config.srtm_path)
print(demmaccs_config.swbd_path)
print(demmaccs_config.maccs_ip_address)
print(demmaccs_config.maccs_launcher)
print(demmaccs_config.launcher)

l1c_list = l1c_db.get_unprocessed_l1c()

if len(l1c_list) == 0:
    print("No unprocessed L1C found in DB")
    sys.exit(0)
print("Start the work for:")
print("{}".format(l1c_list))
processor_short_name = l1c_db.get_short_name("processor", 1)
print("processor_short_name={}".format(processor_short_name))
output_path = demmaccs_config.output_path.replace("{processor}", processor_short_name)
processed_ids = []
for l1c in l1c_list:
    #get site short name
    site_short_name = l1c_db.get_short_name("site", l1c[1])
    tmp_output_path = output_path.replace("{site}", site_short_name)
    if run_command([demmaccs_config.launcher, "--srtm", demmaccs_config.srtm_path, "--swbd", demmaccs_config.swbd_path, "-p", "5", "--gip-dir", demmaccs_config.gips_path, "--working-dir", demmaccs_config.working_dir, "--maccs-address", demmaccs_config.maccs_ip_address, "--maccs-launcher", demmaccs_config.maccs_launcher, l1c[2], demmaccs_config.output_path]):
        processed_ids.append(l1c[0])


l1c_db.mark_as_processed(processed_ids)

