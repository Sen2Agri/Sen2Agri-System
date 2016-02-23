#!/usr/bin/env python
from __future__ import print_function
import argparse
import glob
import os
from os.path import isfile, isdir, join
import sys
import time, datetime
import pipes
import shutil
from common import *


def create_sym_links(filenames, target_directory):
    for file_to_sym_link in filenames:
        #target name
        if file_to_sym_link.endswith("/"):
            basename = os.path.basename(file_to_sym_link[:len(file_to_sym_link) - 1])
        else:
            basename = os.path.basename(file_to_sym_link)
        target = target_directory+"/" + basename
        #does it already exist?
        if os.path.isfile(target) or os.path.isdir(target):
            print("Path exists")
            #skip it
            continue
        #create it
        if run_command(["ln", "-s", file_to_sym_link, target_directory]) != 0:
            return False
    return True


def remove_sym_links(filenames, target_directory):
    for sym_link in filenames:
        if sym_link.endswith("/"):
            basename = os.path.basename(sym_link[:len(sym_link) - 1])
        else:
            basename = os.path.basename(sym_link)
        if run_command(["rm", "{}/{}".format(target_directory, basename)]) != 0:
            continue
    return True


def get_dem_hdr_info(dem_hdr):
    basename = os.path.basename(dem_hdr)
    name = basename[0:len(basename) - 4]
    tile_id = re.match("\w+_REFDE2_(\w{5})\w+", name)
    return name and (name, tile_id.group(1))
        

parser = argparse.ArgumentParser(
    description="Launches DEM and MACCS for L2A product creation")
parser.add_argument('input', help="input L1C directory")
parser.add_argument('-w', '--working-dir', required=True,
                    help="working directory")
parser.add_argument('-d', '--dem-working-dir', required=True,
                    help="working directory for DEM")
parser.add_argument('--srtm', required=True, help="SRTM dataset path")
parser.add_argument('--swbd', required=True, help="SWBD dataset path")
parser.add_argument('-p', '--processes-number-dem', required=False,
                        help="number of processed to run", default="3")
parser.add_argument('--dem-output-dir', required=True, help="output directory for DEM")
parser.add_argument('--gip-dir', required=True, help="directory where gip are to be found")
parser.add_argument('--maccs-address', required=True, help="the ip address of the pc where MACCS is to be found")
parser.add_argument('--maccs-launcher', required=True, help="the shell from the remote pc which launches the MACCS")
parser.add_argument('output', help="output location")

args = parser.parse_args()

if not create_recursive_dirs(args.output):
    print("Could not use the output directory")
    sys.exit(-1)

if not create_recursive_dirs(args.working_dir):
    print("Could not use the output directory")
    sys.exit(-1)

if run_command(["./dem.py", "--srtm", args.srtm, "--swbd", args.swbd, "-p", args.processes_number_dem, "-w", args.dem_working_dir, args.input, args.dem_output_dir]) != 0:
    print("DEM failed")
    sys.exit(-1)

if not create_sym_links([args.input], args.working_dir):
    print("Could not create sym links for {}".format(args.input))
    sys.exit(-1)

gips = glob.glob("{}/*.*".format(args.gip_dir))
if not create_sym_links(gips, args.working_dir):
    print("Symbolic links for GIP files could not be created in the output directory")
    sys.exit(-1)

print("args.dem_working_dir = {}".format(args.dem_output_dir))
dem_hdrs = glob.glob("{}/*.HDR".format(args.dem_output_dir))
print("dem_hdrs = {}".format(dem_hdrs))
for dem_hdr in dem_hdrs:    
    basename, tile_id = get_dem_hdr_info(dem_hdr)
    dem_dir = glob.glob("{0}/{1}.DBL.DIR".format(args.dem_output_dir, basename))
    if len(dem_dir) != 1:
        print("Could not find the DEM directory tile for {}".format(dem_hdr))
        continue
    if not create_sym_links([dem_hdr, dem_dir[0]], args.working_dir):
        print("Could not create sym links for {0} and {1}".format(dem_hdr, dem_dir[0]))
        continue
    if run_command(["ssh", "sen2agri-service@{}".format(args.maccs_address), args.maccs_launcher, args.working_dir, tile_id, args.output]) != 0:
        print("MACCS didn't work !")
    remove_sym_links([dem_hdr, dem_dir[0]], args.working_dir)

remove_sym_links(gips, args.working_dir)
remove_sym_links([args.input], args.working_dir)
try:
    os.rmdir(args.working_dir)
except:
    print("Couldn't remove the temp dir {}".format(args.working_dir))

