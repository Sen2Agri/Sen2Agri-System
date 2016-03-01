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

general_log_path = "/tmp/"
general_log_filename = "demmaccs.log"

def create_sym_links(filenames, target_directory):
    global general_log_path
    global general_log_filename
    for file_to_sym_link in filenames:
        #target name
        if file_to_sym_link.endswith("/"):
            basename = os.path.basename(file_to_sym_link[:len(file_to_sym_link) - 1])
        else:
            basename = os.path.basename(file_to_sym_link)
        target = target_directory+"/" + basename
        #does it already exist?
        if os.path.isfile(target) or os.path.isdir(target):
            log(general_log_path, "Path exists", general_log_filename)
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
parser.add_argument('--srtm', required=True, help="SRTM dataset path")
parser.add_argument('--swbd', required=True, help="SWBD dataset path")
parser.add_argument('-p', '--processes-number-dem', required=False,
                        help="number of processed to run", default="3")
parser.add_argument('--gip-dir', required=True, help="directory where gip are to be found")
parser.add_argument('--maccs-address', required=True, help="the ip address of the pc where MACCS is to be found")
parser.add_argument('--maccs-launcher', required=True, help="the shell from the remote pc which launches the MACCS")
parser.add_argument('--dem-launcher', required=True, help="executable for DEM")
parser.add_argument('output', help="output location")

args = parser.parse_args()

if not create_recursive_dirs(args.output):
    log(general_log_path, "Could not use the output directory", general_log_filename)
    sys.exit(-1)

general_log_path = args.output
own_pid = os.getpid()
working_dir = "{}/{}".format(args.working_dir,own_pid)
dem_working_dir = "{}_DEM_TMP".format(working_dir)
dem_output_dir = "{}_DEM_OUT".format(working_dir)

log(general_log_path,"working_dir = {}".format(working_dir), general_log_filename)
log(general_log_path,"dem_working_dir = {}".format(dem_working_dir), general_log_filename)
log(general_log_path,"dem_output_dir = {}".format(dem_output_dir), general_log_filename)
log(general_log_path, "Started at {}:".format(time.time()), general_log_filename)
general_start = time.time()

if not create_recursive_dirs(dem_output_dir):
    log(general_log_path, "Could not create the output directory for DEM", general_log_filename)
    sys.exit(-1)

if not create_recursive_dirs(working_dir):
    log(general_log_path, "Could not use the temporary directory", general_log_filename)
    sys.exit(-1)

start = time.time()
if run_command([args.dem_launcher, "--srtm", args.srtm, "--swbd", args.swbd, "-p", args.processes_number_dem, "-w", dem_working_dir, args.input, dem_output_dir]) != 0:
    log(general_log_path, "DEM failed", general_log_filename)
    sys.exit(-1)

log(general_log_path, "DEM finished in: {}".format(datetime.timedelta(seconds=(time.time() - start))), general_log_filename)

if not create_sym_links([args.input], working_dir):
    log(general_log_path, "Could not create sym links for {}".format(args.input), general_log_filename)
    sys.exit(-1)

gips = glob.glob("{}/*.*".format(args.gip_dir))
if not create_sym_links(gips, working_dir):
    log(general_log_path, "Symbolic links for GIP files could not be created in the output directory", general_log_filename)
    sys.exit(-1)

log(general_log_path, "dem_output_dir = {}".format(dem_output_dir), general_log_filename)
dem_hdrs = glob.glob("{}/*.HDR".format(dem_output_dir))
log(general_log_path, "dem_hdrs = {}".format(dem_hdrs), general_log_filename)
number_of_processed_tiles = 0
for dem_hdr in dem_hdrs:    
    basename, tile_id = get_dem_hdr_info(dem_hdr)
    dem_dir = glob.glob("{0}/{1}.DBL.DIR".format(dem_output_dir, basename))
    if len(dem_dir) != 1:
        log(general_log_path, "Could not find the DEM directory tile for {}".format(dem_hdr), general_log_filename)
        continue
    if not create_sym_links([dem_hdr, dem_dir[0]], working_dir):
        log(general_log_path, "Could not create sym links for {0} and {1}".format(dem_hdr, dem_dir[0]), general_log_filename)
        continue
    start = time.time()
    if run_command(["ssh", "sen2agri-service@{}".format(args.maccs_address), 
                    args.maccs_launcher, 
                    "--input", working_dir, 
                    "--TileId", tile_id, 
                    "--output", args.output,
                    "--mode", "L2INIT", 
                    "--loglevel", "DEBUG",
                    "--enableTest", "false",
                    "--CheckXMLFilesWithSchema", "false",
                    "--conf", "UserConfiguration"]) != 0:
        log(general_log_path, "MACCS didn't work for {} | TileID: {}!".format(args.input, tile_id), general_log_filename)
    else:
        number_of_processed_tiles += 1
    log(general_log_path, "MACCS finished in: {}".format(datetime.timedelta(seconds=(time.time() - start))), general_log_filename)
    remove_sym_links([dem_hdr, dem_dir[0]], working_dir)
'''
try:
    shutil.rmtree(dem_working_dir)
except:
    log(general_log_path, "Couldn't remove the temp dir {}".format(dem_working_dir), general_log_filename)
try:
    shutil.rmtree(dem_output_dir)
except:
    log(general_log_path, "Couldn't remove the temp dir {}".format(dem_output_dir), general_log_filename)
try:
    shutil.rmtree(working_dir)
except:
    log(general_log_path, "Couldn't remove the temp dir {}".format(working_dir), general_log_filename)
'''
log(general_log_path, "Ended at {}:".format(time.time()), general_log_filename)
log(general_log_path, "Total execution {}:".format(datetime.timedelta(seconds=(time.time() - general_start))), general_log_filename)
if number_of_processed_tiles == 0:
    sys.exit(1)
