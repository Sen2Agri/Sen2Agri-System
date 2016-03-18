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
from sen2agri_common_db import *

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
parser.add_argument('--skip-dem', required=False,
                        help="skip DEM if a directory with previous work of DEM is given", default=None)
parser.add_argument('--delete-temp', required=False,
                        help="if set to True, it will delete all the temporary files and directories. Default: True", default="True")
parser.add_argument('--prev-l2a-tiles', required=False,
                        help="Previous processed tiles from L2A product", default=[], nargs="+")
parser.add_argument('--prev-l2a-tiles-paths', required=False,
                        help="Path of the previous processed tiles from L2A product", default=[], nargs="+")
parser.add_argument('output', help="output location")

args = parser.parse_args()

if not create_recursive_dirs(args.output):
    log(general_log_path, "Could not use the output directory", general_log_filename)
    sys.exit(-1)

if len(args.prev_l2a_tiles) != len(args.prev_l2a_tiles_paths):
    log(general_log_path, "The number of previous l2a tiles is not the same as for paths for these tiles. Check args: --prev-l2-tiles and --prev-l2a-tiles-paths, the length should be equal", general_log_filename)
    sys.exit(-1)

general_log_path = args.output

working_dir = "{}/{}".format(args.working_dir[:len(args.working_dir) - 1] if args.working_dir.endswith("/") else args.working_dir, os.getpid())
if args.skip_dem is not None:
    working_dir = "{}".format(args.skip_dem)
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
if args.skip_dem is None:
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

dem_hdrs = glob.glob("{}/*.HDR".format(dem_output_dir))
log(general_log_path, "DEM output directory {} has DEM hdrs = {}".format(dem_output_dir, dem_hdrs), general_log_filename)
processed_tiles = []

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
    product_name = ""
    if args.input.endswith("/"):
        product_name = os.path.basename(args.input[:len(args.input) - 1])
    else:
        product_name = os.path.basename(args.input)
    sat_id, acquistion_date = get_product_info(product_name)
    maccs_mode = "L2INIT"
    try:
        idx = args.prev_l2a_tiles.index(tile_id)
        print("!!!!!!!!!!!!!!! IDX = {}".format(idx))
        prev_l2a_tile_path = args.prev_l2a_tiles_paths[idx]
        print("!!!!!!!!!!!!!!! prev_l2a_tile_path = {}".format(prev_l2a_tile_path))
        maccs_mode = "L2NOMINAL"
        #sys.exit(0)
        if not create_sym_links(prev_l2a_tile_path, working_dir):
            log(general_log_path, "Could not create sym links for NOMINAL MACCS mode for {}".format(prev_l2a_tile_path), general_log_filename)
            sys.exit(-1)
    except:
        print("No previous processed l2a tile found for {} in product {}".format(tile_id, product_name))
        pass        
    cmd_array = ["ssh", args.maccs_address, 
                    args.maccs_launcher, 
                    "--input", working_dir, 
                    "--TileId", tile_id, 
                    "--output", args.output,
                    "--mode", maccs_mode, 
                    "--loglevel", "DEBUG",
                    "--enableTest", "false",
                    "--CheckXMLFilesWithSchema", "false"]
    if sat_id == 1:
        #UserConfiguration has to be added for SENTINEL in cmd_array
        cmd_array += ["--conf", "UserConfiguration"]
    elif sat_id == 2:
        #the cmd_array it's enough for LANDSAT8
        pass
    else:
        #unknown product name !
        log(general_log_path, "Unknown product name: {}".format(product_name), general_log_filename)
        log(general_log_path, "Continue to the next tile (if present) ", general_log_filename)
        remove_sym_links([dem_hdr, dem_dir[0]], working_dir)
        continue
    log(general_log_path, "sat_id = {} | acq_date = {}".format(sat_id, acquistion_date), general_log_filename)
    log(general_log_path, "Starting MACCS for {} | TileID: {}!".format(args.input, tile_id), general_log_filename)

    if run_command(cmd_array) != 0:
        log(general_log_path, "MACCS didn't work for {} | TileID: {}!".format(args.input, tile_id), general_log_filename)
    else:
        processed_tiles.append(tile_id)
        log(general_log_path, "MACCS finished in: {}".format(datetime.timedelta(seconds=(time.time() - start))), general_log_filename)
        if run_command([os.path.dirname(os.path.abspath(__file__)) + "/mosaic_l2a.py", "-i", args.output, "-w", working_dir]) != 0:
            log(general_log_path, "Mosaic didn't work")    
    remove_sym_links([dem_hdr, dem_dir[0]], working_dir)

if args.delete_temp == "True":
    log(general_log_path, "Remove all the temporary files and directory", general_log_filename)
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

log(general_log_path, "Ended at {}:".format(time.time()), general_log_filename)
log(general_log_path, "Total execution {}:".format(datetime.timedelta(seconds=(time.time() - general_start))), general_log_filename)
if len(processed_tiles) == 0:
    sys.exit(1)
else:
    with open((args.output[:len(args.output) - 1] if args.output.endswith("/") else args.output) + "/processed_tiles", 'w') as result_processed_tiles:
        for tile in processed_tiles:
            result_processed_tiles.write(" {}".format(tile))
        result_processed_tiles.write("\n")            
    sys.exit(0)
