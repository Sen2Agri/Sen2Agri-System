#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
_____________________________________________________________________________

   Program:      Sen2Agri-Processors
   Language:     Python
   Copyright:    2015-2016, CS Romania, office@c-s.ro
   See COPYRIGHT file for details.
   
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
_____________________________________________________________________________

"""
from __future__ import print_function
import argparse
import glob
import os
from os.path import isfile, isdir, join
import sys
import time, datetime
import pipes
import shutil
from multiprocessing import Pool
from sen2agri_common_db import *

general_log_path = "/tmp/"
general_log_filename = "demmaccs.log"

def create_sym_links(filenames, target_directory, log_path, log_filename):

    for file_to_sym_link in filenames:
        #target name
        if file_to_sym_link.endswith("/"):
            basename = os.path.basename(file_to_sym_link[:len(file_to_sym_link) - 1])
        else:
            basename = os.path.basename(file_to_sym_link)
        target = target_directory+"/" + basename
        #does it already exist?
        if os.path.isfile(target) or os.path.isdir(target):
            log(log_path, "Path exists", log_filename)
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


def get_prev_l2a_tile_path(tile_id, prev_l2a_product_path):
    tile_files = []
    if os.path.exists(prev_l2a_product_path) and os.path.isdir(prev_l2a_product_path):
        all_files = glob.glob("{}*.*".format(prev_l2a_product_path))
        for filename in all_files:
            if filename.rfind(tile_id) > 0:
                print("added: {}".format(filename))
                tile_files.append(filename)
    return tile_files


def copy_common_gipp_file(working_dir, gipp_base_dir, gipp_sat_dir, gipp_sat_prefix, gipp_tile_type, gipp_tile_prefix, tile_id, common_tile_id):
    #take the common one
    tmp_tile_gipp = glob.glob("{}/{}/{}*{}_S_{}{}*.EEF".format(gipp_base_dir, gipp_sat_dir, gipp_sat_prefix, gipp_tile_type, gipp_tile_prefix, common_tile_id))
    #if found, copy it (not sym link it)
    if len(tmp_tile_gipp) == 1:
        common_gipp_file = tmp_tile_gipp[0];
        basename_tile_gipp_file = os.path.basename(common_gipp_file[:len(common_gipp_file) - 1]) if common_gipp_file.endswith("/") else os.path.basename(common_gipp_file)
        basename_tile_gipp_file = basename_tile_gipp_file.replace(common_tile_id, tile_id)
        try:
            tile_gipp_file = "{}/{}".format(working_dir, basename_tile_gipp_file)
            with open(common_gipp_file, 'r') as handler_common_gipp_file, open(tile_gipp_file, 'w') as handler_tile_gipp_file:
                for line in handler_common_gipp_file:
                    if "<File_Name>" in line or "<Applicability_NickName>" in line or "<Applicable_SiteDefinition_Id" in line:
                        line = line.replace(common_tile_id, tile_id)
                    handler_tile_gipp_file.write(line)
        except EnvironmentError:
            return False, "Could not transform / copy the common GIPP tile file {} to {} ".format(common_gipp_file, tile_gipp_file)
    return True, "Copied {} to {}".format(common_gipp_file, tile_gipp_file)


class DEMMACCSContext(object):
    def __init__(self, base_working_dir, dem_hdr_file, gipp_base_dir, prev_l2a_tiles, prev_l2a_products_paths, maccs_address, maccs_launcher, l1c_input, l2a_output):
        self.base_working_dir = base_working_dir
        self.dem_hdr_file = dem_hdr_file
        self.dem_output_dir = dem_output_dir
        self.gipp_base_dir = gipp_base_dir
        self.prev_l2a_tiles = prev_l2a_tiles
        self.prev_l2a_products_paths = prev_l2a_products_paths
        self.maccs_address = maccs_address
        self.maccs_launcher = maccs_launcher
        self.input = l1c_input        
        self.output = l2a_output


def maccs_launcher(demmaccs_context):
    if not os.path.isfile(demmaccs_context.dem_hdr_file):
        log(demmaccs_context.output, "There is no such DEM file {}".format(demmaccs_context.dem_hdr_file), "demmaccs.log")
        return ""
    product_name = os.path.basename(demmaccs_context.input[:len(demmaccs_context.input) - 1]) if demmaccs_context.input.endswith("/") else os.path.basename(demmaccs_context.input)
    sat_id, acquistion_date = get_product_info(product_name)
    gipp_sat_prefix = ""
    basename = os.path.basename(demmaccs_context.dem_hdr_file)    
    dem_dir_list = glob.glob("{0}/{1}.DBL.DIR".format(dem_output_dir, basename[0:len(basename) - 4]))

    if len(dem_dir_list) != 1 or not os.path.isdir(dem_dir_list[0]):
        log(demmaccs_context.output, "No {}.DBL.DIR found for DEM ".format(demmaccs_context.dem_hdr_file[0:len(demmaccs_context.dem_hdr_file) - 4]), "demmaccs.log")
        return ""
    dem_dir = dem_dir_list[0]
    tile_id = ""
    gipp_sat_dir = ""
    gipp_tile_prefix = ""
    if sat_id == SENTINEL2_SATELLITE_ID:
        gipp_sat_prefix = "S2"        
        common_tile_id = "CMN00"
        #no prefix for sentinel
        gipp_tile_prefix = ""
        gipp_sat_dir = "SENTINEL2"
        tile = re.match("S2\w+_REFDE2_(\w{5})\w+", basename[0:len(basename) - 4])
        if tile is not None:
            tile_id = tile.group(1)
    elif sat_id == LANDSAT8_SATELLITE_ID:
        gipp_sat_prefix = "L8"
        common_tile_id = "CMN000"
        gipp_tile_prefix = "EU"
        gipp_sat_dir = "LANDSAT8"
        tile = re.match("L8\w+_REFDE2_(\w{6})\w+", basename[0:len(basename) - 4])
        if tile is not None:
            tile_id = tile.group(1)            
    else:
        log(demmaccs_context.output, "Unknown satellite id {} found for {}".format(sat_id, demmaccs_context.input), "demmaccs.log")
        return ""

    if len(tile_id) == 0:
        log(demmaccs_context.output, "Could not get the tile id from DEM file {}".format(demmaccs_context.dem_hdr_file), "demmaccs.log")
        return ""
    tile_log_filename = "demmaccs_{}.log".format(tile_id)
    working_dir = "{}/{}".format(demmaccs_context.base_working_dir, tile_id)
    maccs_working_dir = "{}/maccs_{}".format(demmaccs_context.output, tile_id)

    if not create_recursive_dirs(working_dir):
        log(demmaccs_context.output, "Could not create the working directory {}".format(working_dir), tile_log_filename)
        return ""

    if not create_recursive_dirs(maccs_working_dir):
        log(demmaccs_context.output, "Could not create the MACCS working directory {}".format(maccs_working_dir), tile_log_filename)
        return ""

    if not create_sym_links([demmaccs_context.input], working_dir, demmaccs_context.output, tile_log_filename):
        log(demmaccs_context.output, "Could not create sym links for {}".format(demmaccs_context.input), tile_log_filename)
        return ""

    common_gipps = glob.glob("{}/{}/{}*_L_*.*".format(demmaccs_context.gipp_base_dir, gipp_sat_dir, gipp_sat_prefix))
    if not create_sym_links(common_gipps, working_dir, demmaccs_context.output, tile_log_filename):
        log(demmaccs_context.output, "Symbolic links for GIPP files could not be created in the output directory", tile_log_filename)
        return ""
        
    gipp_tile_types = ["L2SITE", "CKEXTL", "CKQLTL"]
    
    for gipp_tile_type in gipp_tile_types:
        #search for the specific gipp tile file. if it will not be found, the common one (if exists) will be used
        tmp_tile_gipp = glob.glob("{}/{}/{}*{}_S_{}{}*.EEF".format(demmaccs_context.gipp_base_dir, gipp_sat_dir, gipp_sat_prefix, gipp_tile_type, gipp_tile_prefix, tile_id))
        if len(tmp_tile_gipp) == 1:
            if not create_sym_links(tmp_tile_gipp, working_dir, demmaccs_context.output, tile_log_filename):
                log(demmaccs_context.output, "Symbolic links for tile id {} GIPP files could not be created in the output directory".format(tile_id), tile_log_filename)
                return ""
        else:
            #search for the gipp common tile file
            log(demmaccs_context.output, "Symbolic link {} for tile id {} GIPP file could not be found. Searching for the common one ".format(gipp_tile_type, tile_id), tile_log_filename)
            ret, log_gipp = copy_common_gipp_file(working_dir, demmaccs_context.gipp_base_dir, gipp_sat_dir, gipp_sat_prefix, gipp_tile_type, gipp_tile_prefix, tile_id, common_tile_id)
            if len(log_gipp) > 0:
                log(demmaccs_context.output, log_gipp, tile_log_filename)
            if not ret:
                return ""

    if not create_sym_links([demmaccs_context.dem_hdr_file, dem_dir], working_dir, demmaccs_context.output, tile_log_filename):
        log(demmaccs_context.output, "Could not create sym links for {0} and {1}".format(dem_hdr_file, dem_dir), tile_log_filename)
        return ""
    start = time.time()
    maccs_mode = "L2INIT"
    prev_l2a_tile_path = []
    try:
        #demmaccs_context.prev_l2a_tiles.index will throw an exception if it will not find the tile_id inside prev_l2a_tiles
        #so the maccs mode will not be set to nominal
        idx = demmaccs_context.prev_l2a_tiles.index(tile_id)
        product_path = demmaccs_context.prev_l2a_products_paths[idx]
        print("product_path = {}".format(product_path))

        prev_l2a_tile_path = get_prev_l2a_tile_path(tile_id, product_path)

        log(demmaccs_context.output, "Creating sym links for NOMINAL MACCS mode: l2a prev tiles {}".format(prev_l2a_tile_path), tile_log_filename)
        if len(prev_l2a_tile_path) > 0 and create_sym_links(prev_l2a_tile_path, working_dir, demmaccs_context.output, tile_log_filename):
            #set MACCS mode to NOMINAL
            log(demmaccs_context.output, "Created sym links for NOMINAL MACCS mode for {}".format(prev_l2a_tile_path), tile_log_filename)
            maccs_mode = "L2NOMINAL"
        else:
            # something went wrong. shall this be an exit point?
            # shall the mode remain to L2INIT? This behavior may as well hide a bug in a previous demmaccs run (it's possible)...
            log(demmaccs_context.output, "Could not create sym links for NOMINAL MACCS mode for {}. Exit".format(prev_l2a_tile_path), tile_log_filename)
            return ""
    except SystemExit:
        log(demmaccs_context.output, "SystemExit caught when trying to create sym links for NOMINAL MACCS mode, product {}. Exit!".format(demmaccs_context.input), tile_log_filename)
        return ""
    except:
        print("No previous processed l2a tile found for {} in product {}. Running MACCS in L2INIT mode".format(tile_id, product_name))
    #MACCS bug. In case of setting the file status from VALD to NOTV, MACCS will try to create a diretory LTC in the current running directory
    #which is /. Of course, it will fail
    os.chdir(maccs_working_dir)
    cmd_array = []
    if demmaccs_context.maccs_address is not None:
        cmd_array = ["ssh", demmaccs_context.maccs_address]
    cmd_array += [demmaccs_context.maccs_launcher, 
                    "--input", working_dir, 
                    "--TileId", tile_id, 
                    "--output", maccs_working_dir,
                    "--mode", maccs_mode, 
                    "--loglevel", "DEBUG",
                    "--enableTest", "false",
                    "--CheckXMLFilesWithSchema", "false"]
    if sat_id == SENTINEL2_SATELLITE_ID:
        #UserConfiguration has to be added for SENTINEL in cmd_array
        cmd_array += ["--conf", "/usr/share/sen2agri/sen2agri-demmaccs/UserConfiguration"]
    log(demmaccs_context.output, "sat_id = {} | acq_date = {}".format(sat_id, acquistion_date), tile_log_filename)
    log(demmaccs_context.output, "Starting MACCS in {} for {} | TileID: {}".format(maccs_mode, demmaccs_context.input, tile_id), tile_log_filename)
    log(demmaccs_context.output, "MACCS_COMMAND: {}".format(cmd_array), tile_log_filename)
    if run_command(cmd_array, demmaccs_context.output, tile_log_filename) != 0:
        log(demmaccs_context.output, "MACCS mode {} didn't work for {} | TileID: {}. Location {}".format(maccs_mode, demmaccs_context.input, tile_id, demmaccs_context.output), tile_log_filename)        
    else:
        log(demmaccs_context.output, "MACCS mode {} for {} tile {} finished in: {}. Location: {}".format(maccs_mode, demmaccs_context.input, tile_id, datetime.timedelta(seconds=(time.time() - start)), demmaccs_context.output), tile_log_filename)
    # move the maccs output to the output directory. 
    # only the valid files should be moved
    maccs_dbl_dir = glob.glob("{}/*_L2VALD_*.DBL.DIR".format(maccs_working_dir))
    maccs_hdr_file = glob.glob("{}/*_L2VALD_*.HDR".format(maccs_working_dir))
    return_tile_id = ""
    try:
        maccs_working_dir_content = glob.glob("{}/*".format(maccs_working_dir))
        log(demmaccs_context.output, "Searching for valid products in MACCS working dir: {}. Following is the content of this dir: {}".format(maccs_working_dir, maccs_working_dir_content), tile_log_filename)
        if len(maccs_dbl_dir) >= 1 and len(maccs_hdr_file) >= 1:
            return_tile_id = "{}".format(tile_id)
            log(demmaccs_context.output, "Found valid tile id {} in {}. Move the files to destination".format(tile_id, maccs_working_dir), tile_log_filename)
            for maccs_out in maccs_working_dir_content:
                new_file = "{}/{}".format(demmaccs_context.output, os.path.basename(maccs_out))
                if os.path.isdir(new_file):
                    log(demmaccs_context.output, "The directory {} already exists. Trying to delete it in order to move the new created directory by MACCS".format(new_file), tile_log_filename)
                    shutil.rmtree(new_file)
                elif os.path.isfile(new_file):
                    log(demmaccs_context.output, "The file {} already exists. Trying to delete it in order to move the new created file by MACCS".format(new_file), tile_log_filename)
                    os.remove(new_file)
                else: #the dest does not exist, so will move it without problems
                    pass
                log(demmaccs_context.output, "Moving {} to {}".format(maccs_out, demmaccs_context.output + "/" + os.path.basename(maccs_out)), tile_log_filename)
                shutil.move(maccs_out, new_file)
        else:
            log(demmaccs_context.output, "No valid products (VALD status) found in: {}.".format(maccs_working_dir), tile_log_filename)
        log(demmaccs_context.output, "Deleting MACCS working directory: rmtree: {}".format(maccs_working_dir), tile_log_filename)
        shutil.rmtree(maccs_working_dir)
    except:
        return_tile_id = ""        
        print("Exception caught when moving maccs files for tile {} to the output directory :( ".format(tile_id))
    return return_tile_id


parser = argparse.ArgumentParser(
    description="Launches DEM and MACCS for L2A product creation")
parser.add_argument('input', help="input L1C directory")
parser.add_argument('-w', '--working-dir', required=True,
                    help="working directory")
parser.add_argument('--srtm', required=True, help="SRTM dataset path")
parser.add_argument('--swbd', required=True, help="SWBD dataset path")
parser.add_argument('--processes-number-dem', required=False,
                        help="number of processes to run DEM in parallel", default="3")
parser.add_argument('--processes-number-maccs', required=False,
                        help="number of processes to run MACCS in parallel", default="2")
parser.add_argument('--gipp-dir', required=True, help="directory where gip are to be found")
parser.add_argument('--maccs-address', required=False, help="MACCS has to be run from a remote host. This should be the ip address of the pc where MACCS is to be found")
parser.add_argument('--maccs-launcher', required=True, help="MACCS binary path in localhost (or remote host if maccs-address is set)")
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
log_filename = "demmaccs.log"
if not create_recursive_dirs(args.output):
    log(general_log_path, "Could not create the output directory", log_filename)
    sys.exit(-1)

if len(args.prev_l2a_tiles) != len(args.prev_l2a_products_paths):
    log(general_log_path, "The number of previous l2a tiles is not the same as for paths for these tiles. Check args: --prev-l2-tiles and --prev-l2a-products-paths, the length should be equal", log_filename)
    sys.exit(-1)

general_log_path = args.output

working_dir = "{}/{}".format(args.working_dir[:len(args.working_dir) - 1] if args.working_dir.endswith("/") else args.working_dir, os.getpid())
if args.skip_dem is not None:
    working_dir = "{}".format(args.skip_dem)
dem_working_dir = "{}_DEM_TMP".format(working_dir)
dem_output_dir = "{}_DEM_OUT".format(working_dir)


log(general_log_path,"working_dir = {}".format(working_dir), log_filename)
log(general_log_path,"dem_working_dir = {}".format(dem_working_dir), log_filename)
log(general_log_path,"dem_output_dir = {}".format(dem_output_dir), log_filename)

general_start = time.time()

if not create_recursive_dirs(working_dir):
    log(general_log_path, "Could not create the temporary directory", log_filename)
    sys.exit(-1)

start = time.time()
base_abs_path = os.path.dirname(os.path.abspath(__file__)) + "/"
product_name = os.path.basename(args.input[:len(args.input) - 1]) if args.input.endswith("/") else os.path.basename(args.input)
sat_id, acquistion_date = get_product_info(product_name)

# crop the LANDSAT products for the alignment
if sat_id == LANDSAT8_SATELLITE_ID:
    base_l8align_abs_path = os.path.dirname(os.path.abspath(__file__)) + "/../l8_alignment/"
    if run_command([base_l8align_abs_path + "l8_align.py", "-i", args.input, "-o", working_dir, "-v", base_l8align_abs_path + "wrs2_descending/wrs2_descending.shp", "-w", working_dir + "/l8_align_tmp", "-t", product_name]) != 0:
        log(general_log_path, "The LANDSAT8 product could not be aligned {}".format(args.input), log_filename)
        if not remove_dir(working_dir):
            log(general_log_path, "Couldn't remove the temp dir {}".format(working_dir), log_filename)
        sys.exit(-1)
    #aligned_landsat_product_path, log_message = landsat_crop_to_cutline(args.input, working_dir)
    #if len(aligned_landsat_product_path) <= 0:
    #    log(general_log_path, log_message, log_filename)
    #    try:
    #        shutil.rmtree(working_dir)
    #    except:
    #        log(general_log_path, "Couldn't remove the temp dir {}".format(working_dir), log_filename)
    #    sys.exit(-1)
    #the l8.align.py outputs in the working_dir directory where creates a directory which has the product name
    args.input = working_dir + "/" + product_name
    log(general_log_path, "The LANDSAT8 product was aligned here: {}".format(args.input), log_filename)

if not create_recursive_dirs(dem_output_dir):
    log(general_log_path, "Could not create the output directory for DEM", log_filename)
    if not remove_dir(working_dir):
        log(general_log_path, "Couldn't remove the temp dir {}".format(working_dir), log_filename)
    sys.exit(-1)

if args.skip_dem is None:
    print("Creating DEMs for {}".format(args.input))
    if run_command([base_abs_path + "dem.py", "--srtm", args.srtm, "--swbd", args.swbd, "-p", args.processes_number_dem, "-w", dem_working_dir, args.input, dem_output_dir], general_log_path, log_filename) != 0:
        log(general_log_path, "DEM failed", log_filename)
        if not remove_dir(working_dir):
            log(general_log_path, "Couldn't remove the temp dir {}".format(working_dir), log_filename)
        sys.exit(-1)

log(general_log_path, "DEM finished in: {}".format(datetime.timedelta(seconds=(time.time() - start))), log_filename)

dem_hdrs = glob.glob("{}/*.HDR".format(dem_output_dir))
log(general_log_path, "DEM output directory {} has DEM hdrs = {}".format(dem_output_dir, dem_hdrs), log_filename)
if len(dem_hdrs) == 0:
    log(general_log_path, "There are no hdr DEM files in {}".format(dem_output_dir), log_filename)
    if not remove_dir(dem_working_dir):
        log(general_log_path, "Couldn't remove the temp dir {}".format(dem_working_dir), log_filename)
    if not remove_dir(dem_output_dir):
        log(general_log_path, "Couldn't remove the temp dir {}".format(dem_output_dir), log_filename)
    if not remove_dir(working_dir):
        log(general_log_path, "Couldn't remove the temp dir {}".format(working_dir), log_filename)
    sys.exit(-1)


demmaccs_contexts = []
print("Creating demmaccs contexts with: input: {} | output {}".format(args.input, args.output))
for dem_hdr in dem_hdrs:    
    print("DEM_HDR: {}".format(dem_hdr))
    demmaccs_contexts.append(DEMMACCSContext(working_dir, dem_hdr, args.gipp_dir, args.prev_l2a_tiles, args.prev_l2a_products_paths, args.maccs_address, args.maccs_launcher, args.input, args.output))

#RELEASE mode, it launches in parallel
pool = Pool(int(args.processes_number_maccs))
pool_outputs = pool.map(maccs_launcher, demmaccs_contexts)
pool.close()
pool.join()

#DEBUG mode only, it launches sequentially 
#pool_outputs = map(maccs_launcher, demmaccs_contexts)

processed_tiles = []
for out in pool_outputs:
    if len(out) >=5:
        processed_tiles.append(out)

sys_exit = int(0)
if len(processed_tiles) == 0:
    log(general_log_path, "MACCS did not processed any tiles for L1C product {}".format(args.input), log_filename)
    sys_exit = 1
else:
    log(general_log_path, "MACCS processed the following tiles for L1C product {} :".format(args.input), log_filename)
    log(general_log_path, "{}".format(processed_tiles), log_filename)
    if run_command([os.path.dirname(os.path.abspath(__file__)) + "/mosaic_l2a.py", "-i", args.output, "-w", working_dir], args.output, log_filename) != 0:
        log(general_log_path, "Mosaic didn't work", log_filename)    

if args.delete_temp == "True":
    log(general_log_path, "Remove all the temporary files and directory", log_filename)
    if not remove_dir(dem_working_dir):
        log(general_log_path, "Couldn't remove the temp dir {}".format(dem_working_dir), log_filename)
    if not remove_dir(dem_output_dir):
        log(general_log_path, "Couldn't remove the temp dir {}".format(dem_output_dir), log_filename)
    if not remove_dir(working_dir):
        log(general_log_path, "Couldn't remove the temp dir {}".format(working_dir), log_filename)

log(general_log_path, "Total execution {}:".format(datetime.timedelta(seconds=(time.time() - general_start))), log_filename)

sys.exit(sys_exit)
