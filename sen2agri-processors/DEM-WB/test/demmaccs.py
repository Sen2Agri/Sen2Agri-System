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
        

def get_prev_l2a_tile_path(tile_id, prev_l2a_product_path):
    tile_files = []
    if os.path.exists(prev_l2a_product_path) and os.path.isdir(prev_l2a_product_path):
        all_files = glob.glob("{}*.*".format(prev_l2a_product_path))
        for filename in all_files:
            if filename.rfind(tile_id) > 0:
                print("added: {}".format(filename))
                tile_files.append(filename)
            #reg_exp = re.match("[\w+-]_L2VALD_\d\d[a-zA-Z]{3}____w+", filename)            
            #print("{}".format(filename))
            #if reg_exp is not None:
            #    print("added")
            #    tile_files.append(filename)
    return tile_files

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
parser.add_argument('--prev-l2a-products-paths', required=False,
                        help="Path of the previous processed tiles from L2A product", default=[], nargs="+")
parser.add_argument('output', help="output location")

args = parser.parse_args()

if not create_recursive_dirs(args.output):
    log(general_log_path, "Could not use the output directory", general_log_filename)
    sys.exit(-1)

if len(args.prev_l2a_tiles) != len(args.prev_l2a_products_paths):
    log(general_log_path, "The number of previous l2a tiles is not the same as for paths for these tiles. Check args: --prev-l2-tiles and --prev-l2a-products-paths, the length should be equal", general_log_filename)
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

product_name = os.path.basename(args.input[:len(args.input) - 1]) if args.input.endswith("/") else os.path.basename(args.input)
sat_id, acquistion_date = get_product_info(product_name)
gip_sat = ""
if sat_id == SENTINEL2_SATELLITE_ID:
    gip_sat = "S2"
elif sat_id == LANDSAT8_SATELLITE_ID:
    gip_sat = "L8"
else:
    log(general_log_path, "Unknown satellite id {} found for {}".format(sat_id, args.input), general_log_filename)
    sys.exit(-1)

gips = glob.glob("{}/{}*.*".format(args.gip_dir, gip_sat))

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
    maccs_mode = "L2INIT"
    try:
        #.index will throw an exception if it will not find the tile_id inside prev_l2a_tiles
        #so the maccs mode will not be set to nominal
        idx = args.prev_l2a_tiles.index(tile_id)
        product_path = args.prev_l2a_products_paths[idx]
        print("product_path = {}".format(product_path))

        prev_l2a_tile_path = get_prev_l2a_tile_path(tile_id, product_path)

        log(general_log_path, "Creating sym links for NOMINAL MACCS mode: l2a prev tiles {}".format(prev_l2a_tile_path), general_log_filename)
        if len(prev_l2a_tile_path) > 0 and create_sym_links(prev_l2a_tile_path, working_dir):
            #set MACCS mode to NOMINAL
            log(general_log_path, "Created sym links for NOMINAL MACCS mode for {}".format(prev_l2a_tile_path), general_log_filename)
            maccs_mode = "L2NOMINAL"
        else:
            # something went wrong. shall this be an exit point?
            # shall the mode remain to L2INIT? This behavior may as well hide a bug in a previous demmaccs run (it's possible)...
            log(general_log_path, "Could not create sym links for NOMINAL MACCS mode for {}".format(prev_l2a_tile_path), general_log_filename)
            #or generate sys.exit(-1) and catch it on except
    except SystemExit:
        print("exit")
        sys.exit(-1)
    except:
        print("No previous processed l2a tile found for {} in product {}".format(tile_id, product_name))
    
    cmd_array = ["ssh", args.maccs_address, 
                    args.maccs_launcher, 
                    "--input", working_dir, 
                    "--TileId", tile_id, 
                    "--output", args.output,
                    "--mode", maccs_mode, 
                    "--loglevel", "DEBUG",
                    "--enableTest", "false",
                    "--CheckXMLFilesWithSchema", "false"]
    if sat_id == SENTINEL2_SATELLITE_ID:
        #UserConfiguration has to be added for SENTINEL in cmd_array
        cmd_array += ["--conf", "UserConfiguration"]
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
    remove_sym_links(prev_l2a_tile_path, working_dir)

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
    log(general_log_path, "MACCS did not processed any tiles for L1C product {}".format(args.input), general_log_filename)
    sys.exit(1)
else:
    log(general_log_path, "MACCS processed the following tiles for L1C product {} :".format(args.input), general_log_filename)
    log(general_log_path, "{}".format(processed_tiles), general_log_filename)
    sys.exit(0)
