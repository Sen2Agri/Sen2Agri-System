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
import re
import os
from os.path import isfile, isdir, join
import sys
import time, datetime
import pipes
import shutil
from multiprocessing import Pool
from sen2agri_common_db import *
from threading import Thread
import threading
import tempfile

general_log_path = "/tmp/"
general_log_filename = "demmaccs.log"

def create_sym_links(filenames, target_directory, log_path, log_filename):

    for file_to_sym_link in filenames:
        #target name
        if file_to_sym_link.endswith("/"):
            basename = os.path.basename(file_to_sym_link[:len(file_to_sym_link) - 1])
        else:
            basename = os.path.basename(file_to_sym_link)
        target = os.path.join(target_directory, basename)
        #does it already exist?
        if os.path.isfile(target) or os.path.isdir(target):
            log(log_path, "The path {} does exist already".format(target), log_filename)
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
        if run_command(["rm", os.path.join(target_directory, basename)]) != 0:
            continue
    return True


def get_prev_l2a_tile_path(tile_id, prev_l2a_product_path):
    tile_files = []
    print("START get_prev_l2a_tile_path")
    print("Tile_id = {} | prev_l2a_product_path = {}".format(tile_id, prev_l2a_product_path))
    if os.path.exists(prev_l2a_product_path) and os.path.isdir(prev_l2a_product_path):
        all_files = glob.glob("{}/*".format(prev_l2a_product_path))
        print("all_files = {}".format(all_files))
        for filename in all_files:
            print("Checking {}".format(filename))
            if filename.rfind(tile_id, len(prev_l2a_product_path)) > 0:
                print("added: {}".format(filename))
                tile_files.append(filename)
            else:
                print("Ignoring {}".format(filename))
    else:
        print("The dir {} does not exist or is not a dir".format(prev_l2a_product_path))
    print("STOP get_prev_l2a_tile_path")
    return tile_files


def copy_common_gipp_file(working_dir, gipp_base_dir, gipp_sat_dir, gipp_sat_prefix, full_gipp_sat_prefix, gipp_tile_type, gipp_tile_prefix, tile_id, common_tile_id):
    #take the common one
    tmp_tile_gipp = glob.glob("{}/{}/{}*{}_S_{}{}*.EEF".format(gipp_base_dir, gipp_sat_dir, gipp_sat_prefix, gipp_tile_type, gipp_tile_prefix, common_tile_id))
    print ("copy_common_gipp_file: found the following files {}".format(tmp_tile_gipp))
    #if found, copy it (not sym link it)
    if len(tmp_tile_gipp) > 0:
        common_gipp_file = tmp_tile_gipp[0];
        basename_tile_gipp_file = os.path.basename(common_gipp_file[:len(common_gipp_file) - 1]) if common_gipp_file.endswith("/") else os.path.basename(common_gipp_file)
        basename_tile_gipp_file = basename_tile_gipp_file.replace(common_tile_id, tile_id)
        
        for cmn_gipp_file_tmp in tmp_tile_gipp :
            tmpFile1 = os.path.basename(cmn_gipp_file_tmp[:len(cmn_gipp_file_tmp) - 1]) if cmn_gipp_file_tmp.endswith("/") else os.path.basename(cmn_gipp_file_tmp)
            tmpFile1 = tmpFile1.replace(common_tile_id, tile_id)
            if tmpFile1.startswith(full_gipp_sat_prefix) :
                print ("Selecting the file {}".format(tmpFile1))
                common_gipp_file = cmn_gipp_file_tmp
                basename_tile_gipp_file = tmpFile1
                break
            
        try:
            tile_gipp_file = "{}/{}".format(working_dir, basename_tile_gipp_file)
            with open(common_gipp_file, 'r') as handler_common_gipp_file, open(tile_gipp_file, 'w') as handler_tile_gipp_file:
                for line in handler_common_gipp_file:
                    if "<File_Name>" in line or "<Applicability_NickName>" in line or "<Applicable_SiteDefinition_Id" in line:
                        line = line.replace(common_tile_id, tile_id)
                    handler_tile_gipp_file.write(line)
        except EnvironmentError:
            return False, "Could not transform / copy the common GIPP tile file {} to {} ".format(common_gipp_file, tile_gipp_file)
    else:
        return False, "Could not find common gip tile id {}".format(common_tile_id)
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
        self.l1c_processor = L1C_UNKNOWN_PROCESSOR_OUTPUT_FORMAT 
        if re.search("maccs", maccs_launcher, re.IGNORECASE):
            self.l1c_processor = L1C_MACCS_PROCESSOR_OUTPUT_FORMAT
        elif re.search("maja", maccs_launcher, re.IGNORECASE):
            self.l1c_processor = L1C_MAJA_PROCESSOR_OUTPUT_FORMAT
        self.input = l1c_input
        self.output = l2a_output

def maccs_launcher(demmaccs_context):
    if not os.path.isfile(demmaccs_context.dem_hdr_file):
        log(demmaccs_context.output, "General failure: There is no such DEM file {}".format(demmaccs_context.dem_hdr_file), log_filename)
        return ""
    product_name = os.path.basename(demmaccs_context.input[:len(demmaccs_context.input) - 1]) if demmaccs_context.input.endswith("/") else os.path.basename(demmaccs_context.input)
    sat_id, acquistion_date = get_product_info(product_name)
    gipp_sat_prefix = ""
    basename = os.path.basename(demmaccs_context.dem_hdr_file)
    dem_dir_list = glob.glob("{0}/{1}.DBL.DIR".format(dem_output_dir, basename[0:len(basename) - 4]))

    if len(dem_dir_list) != 1 or not os.path.isdir(dem_dir_list[0]):
        log(demmaccs_context.output, "General failure: No {}.DBL.DIR found for DEM ".format(demmaccs_context.dem_hdr_file[0:len(demmaccs_context.dem_hdr_file) - 4]), log_filename)
        return ""
    dem_dir = dem_dir_list[0]
    tile_id = ""
    gipp_sat_dir = ""
    gipp_tile_prefix = ""
    if sat_id == SENTINEL2_SATELLITE_ID:
        gipp_sat_prefix = "S2"
        full_gipp_sat_prefix = gipp_sat_prefix
        m = re.match("(S2[A-D])_\w+_V\d{8}T\d{6}_\w+.SAFE", product_name)
        # check if the new convention naming aplies
        if m == None:
            m = re.match("(S2[A-D])_\w+_\d{8}T\d{6}_\w+.SAFE", product_name)
        if m != None:
            full_gipp_sat_prefix = m.group(1)
        
        print ("full_gipp_sat_prefix is {}".format(full_gipp_sat_prefix))
        
        common_tile_id = "CMN00"
        #no prefix for sentinel
        gipp_tile_prefix = ""
        gipp_sat_dir = "SENTINEL2"
        tile = re.match("S2\w+_REFDE2_(\w{5})\w+", basename[0:len(basename) - 4])
        if tile is not None:
            tile_id = tile.group(1)
    elif sat_id == LANDSAT8_SATELLITE_ID:
        gipp_sat_prefix = "L8"
        full_gipp_sat_prefix = "L8" 
        common_tile_id = "CMN000"
        gipp_tile_prefix = "EU"
        gipp_sat_dir = "LANDSAT8"
        tile = re.match("L8\w+_REFDE2_(\w{6})\w+", basename[0:len(basename) - 4])
        if tile is not None:
            tile_id = tile.group(1)
    else:
        log(demmaccs_context.output, "General failure: Unknown satellite id {} found for {}".format(sat_id, demmaccs_context.input), log_filename)
        return ""

    if len(tile_id) == 0:
        log(demmaccs_context.output, "General failure: Could not get the tile id from DEM file {}".format(demmaccs_context.dem_hdr_file), log_filename)
        return ""

    tile_log_filename = "demmaccs_{}.log".format(tile_id)
    if suffix_log_name is not None:
        tile_log_filename = "demmaccs_{}_{}.log".format(tile_id, suffix_log_name)
    working_dir = os.path.join(demmaccs_context.base_working_dir, tile_id)
    maccs_working_dir = "{}/maccs_{}".format(demmaccs_context.output[:len(demmaccs_context.output) - 1] if demmaccs_context.output.endswith("/") else demmaccs_context.output, tile_id)
    if not create_recursive_dirs(working_dir):
        log(demmaccs_context.output, "Tile failure: Could not create the working directory {}".format(working_dir), tile_log_filename)
        return ""

    if not create_recursive_dirs(maccs_working_dir):
        log(demmaccs_context.output, "Tile failure: Could not create the MACCS/MAJA working directory {}".format(maccs_working_dir), tile_log_filename)
        return ""

    if not create_sym_links([demmaccs_context.input], working_dir, demmaccs_context.output, tile_log_filename):
        log(demmaccs_context.output, "Tile failure: Could not create sym links for {}".format(demmaccs_context.input), tile_log_filename)
        return ""

    common_gipps = glob.glob("{}/{}/{}*_L_*.*".format(demmaccs_context.gipp_base_dir, gipp_sat_dir, full_gipp_sat_prefix))
    if len(common_gipps) == 0:
        common_gipps = glob.glob("{}/{}/{}*_L_*.*".format(demmaccs_context.gipp_base_dir, gipp_sat_dir, gipp_sat_prefix))
    
    print ("common_gipps is {}".format(common_gipps))
    if not create_sym_links(common_gipps, working_dir, demmaccs_context.output, tile_log_filename):
        log(demmaccs_context.output, "Tile failure: Symbolic links for GIPP files could not be created in the output directory", tile_log_filename)
        return ""

    gipp_tile_types = ["L2SITE", "CKEXTL", "CKQLTL"]

    for gipp_tile_type in gipp_tile_types:
        #search for the specific gipp tile file. if it will not be found, the common one (if exists) will be used
        tmp_tile_gipp = glob.glob("{}/{}/{}*{}_S_{}{}*.EEF".format(demmaccs_context.gipp_base_dir, gipp_sat_dir, full_gipp_sat_prefix, gipp_tile_type, gipp_tile_prefix, tile_id))
        if len(tmp_tile_gipp) == 0:
            tmp_tile_gipp = glob.glob("{}/{}/{}*{}_S_{}{}*.EEF".format(demmaccs_context.gipp_base_dir, gipp_sat_dir, gipp_sat_prefix, gipp_tile_type, gipp_tile_prefix, tile_id))
        
        print ("tmp_tile_gipp is {}".format(tmp_tile_gipp))
        if len(tmp_tile_gipp) > 0:
            if not create_sym_links(tmp_tile_gipp, working_dir, demmaccs_context.output, tile_log_filename):
                log(demmaccs_context.output, "Tile failure: Symbolic links for tile id {} GIPP files could not be created in the output directory".format(tile_id), tile_log_filename)
                return ""
        else:
            #search for the gipp common tile file
            log(demmaccs_context.output, "Symbolic link {} for tile id {} GIPP file could not be found. Searching for the common one ".format(gipp_tile_type, tile_id), tile_log_filename)
            ret, log_gipp = copy_common_gipp_file(working_dir, demmaccs_context.gipp_base_dir, gipp_sat_dir, gipp_sat_prefix, full_gipp_sat_prefix, gipp_tile_type, gipp_tile_prefix, tile_id, common_tile_id)
            if len(log_gipp) > 0:
                log(demmaccs_context.output, log_gipp, tile_log_filename)
            if not ret:
                log(demmaccs_context.output, "Tile failure: {}".format(log_gipp), tile_log_filename)
                return ""

    if not create_sym_links([demmaccs_context.dem_hdr_file, dem_dir], working_dir, demmaccs_context.output, tile_log_filename):
        log(demmaccs_context.output, "Tile failure: Could not create symbolic links for {0} and {1}".format(dem_hdr_file, dem_dir), tile_log_filename)
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

        log(demmaccs_context.output, "Creating sym links for NOMINAL MACCS/MAJA mode: l2a prev tiles {}".format(prev_l2a_tile_path), tile_log_filename)
        if len(prev_l2a_tile_path) > 0 and create_sym_links(prev_l2a_tile_path, working_dir, demmaccs_context.output, tile_log_filename):
            #set MACCS mode to NOMINAL
            log(demmaccs_context.output, "Created sym links for NOMINAL MACCS/MAJA mode for {}".format(prev_l2a_tile_path), tile_log_filename)
            maccs_mode = "L2NOMINAL"
        else:
            # something went wrong. shall this be an exit point?
            # shall the mode remain to L2INIT? This behavior may as well hide a bug in a previous demmaccs run (it's possible)...
            log(demmaccs_context.output, "Tile failure: Could not create sym links for NOMINAL MACCS/MAJA mode for {}. Exit".format(prev_l2a_tile_path), tile_log_filename)
            return ""
    except SystemExit:
        log(demmaccs_context.output, "Tile failure: SystemExit caught when trying to create sym links for NOMINAL MACCS/MAJA mode, product {}. Exit!".format(demmaccs_context.input), tile_log_filename)
        return ""
    except:
        log(demmaccs_context.output, "No previous processed l2a tile found for {} in product {}. Running MACCS/MAJA in L2INIT mode".format(tile_id, product_name), tile_log_filename)
        pass
    #MACCS bug. In case of setting the file status from VALD to NOTV, MACCS will try to create a directory LTC in the current running directory
    #which is / Of course, it will fail. That's why we have to move the current running directory to the MACCS temporary directory
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
        #UserConfiguration has to be added for SENTINEL in cmd_array (don't know why, but I saw this is the only way to make it working)
        cmd_array += ["--conf", "/usr/share/sen2agri/sen2agri-demmaccs/UserConfiguration"]
    log(demmaccs_context.output, "sat_id = {} | acq_date = {}".format(sat_id, acquistion_date), tile_log_filename)
    log(demmaccs_context.output, "Starting MACCS/MAJA in {} for {} | TileID: {}".format(maccs_mode, demmaccs_context.input, tile_id), tile_log_filename)
    log(demmaccs_context.output, "MACCS_COMMAND: {}".format(cmd_array), tile_log_filename)
    if run_command(cmd_array, demmaccs_context.output, tile_log_filename) != 0:
        log(demmaccs_context.output, "MACCS/MAJA mode {} didn't work for {} | TileID: {}. Location {}".format(maccs_mode, demmaccs_context.input, tile_id, demmaccs_context.output), tile_log_filename)
    else:
        log(demmaccs_context.output, "MACCS/MAJA mode {} for {} tile {} finished in: {}. Location: {}".format(maccs_mode, demmaccs_context.input, tile_id, datetime.timedelta(seconds=(time.time() - start)), demmaccs_context.output), tile_log_filename)
    # move the maccs output to the output directory.
    # only the valid files should be moved
    maccs_report_file = glob.glob("{}/*_L*REPT*.EEF".format(maccs_working_dir))
    new_maccs_report_file = ""
    return_tile_id = ""
    try:
        # First, move the report log that maccs created it. Take care, first maccs creates a report file named PMC_LxREPT.EEF.
        # When it finishes, maccs will rename this file to something like S2A_OPER_PMC_L2REPT_{tile_id}____{dateofl1cproduct}.EEF
        # Sometimes (usually when it crashes or for different reasons stops at the beginning), this renaming does not take place,
        # so the report file will remain PMC_LxREPT.EEF
        # This report file (doesn't matter the name) will be kept and save to the working_dir with the name MACCS_L2REPT_{tile_id}.EEF
        log(demmaccs_context.output, "Searching for report maccs file (REPT) in: {}".format(maccs_working_dir), tile_log_filename)
        if len(maccs_report_file) >= 1:
            if len(maccs_report_file) > 1:
                log(demmaccs_context.output, "WARNING: More than one report maccs file (REPT) found in {}. Only the first one will be kept. Report files list: {}.".format(maccs_working_dir, maccs_report_file), tile_log_filename)
            log(demmaccs_context.output, "Report maccs file (REPT) found in: {} : {}".format(maccs_working_dir, maccs_report_file[0]), tile_log_filename)
            new_maccs_report_file = "{}/MACCS_L2REPT_{}.EEF".format(demmaccs_context.output[:len(demmaccs_context.output) - 1] if demmaccs_context.output.endswith("/") else demmaccs_context.output, tile_id)
            if os.path.isdir(new_maccs_report_file):
                log(demmaccs_context.output, "The directory {} already exists. Trying to delete it in order to move the new created directory by MACCS/MAJA".format(new_maccs_report_file), tile_log_filename)
                shutil.rmtree(new_maccs_report_file)
            elif os.path.isfile(new_maccs_report_file):
                log(demmaccs_context.output, "The file {} already exists. Trying to delete it in order to move the new created file by MACCS/MAJA".format(new_maccs_report_file), tile_log_filename)
                os.remove(new_maccs_report_file)
            else: #the destination does not exist, so move the files
                pass
            log(demmaccs_context.output, "Moving {} to {}".format(maccs_report_file[0], new_maccs_report_file), tile_log_filename)
            shutil.move(maccs_report_file[0], new_maccs_report_file)
        else:
            log(demmaccs_context.output, "No report maccs files (REPT) found in: {}.".format(maccs_working_dir), tile_log_filename)
        working_dir_content = glob.glob("{}/*".format(maccs_working_dir))
        log(demmaccs_context.output, "Searching for valid products in working dir: {}. Following is the content of this dir: {}".format(maccs_working_dir, working_dir_content), tile_log_filename)
        #check for MACCS format
        output_format, maja_dir = get_l1c_processor_output_format(maccs_working_dir, tile_id)
        print("output_format = {}, maja_dir = {}".format(output_format, maja_dir))
        if output_format == L1C_MACCS_PROCESSOR_OUTPUT_FORMAT:
            log(demmaccs_context.output, "MACCS output format found. Searching output for valid results", tile_log_filename)
            return_tile_id = "{}".format(tile_id)
            log(demmaccs_context.output, "Found valid tile id {} in {}. Moving all the files to destination".format(tile_id, maccs_working_dir), tile_log_filename)
            for maccs_out in working_dir_content:
                new_file = os.path.join(demmaccs_context.output, os.path.basename(maccs_out))
                if os.path.isdir(new_file):
                    log(demmaccs_context.output, "The directory {} already exists. Trying to delete it in order to move the new created directory by MACCS".format(new_file), tile_log_filename)
                    shutil.rmtree(new_file)
                elif os.path.isfile(new_file):
                    log(demmaccs_context.output, "The file {} already exists. Trying to delete it in order to move the new created file by MACCS".format(new_file), tile_log_filename)
                    os.remove(new_file)
                else: #the dest does not exist, so it will be moved without problems
                    pass
                log(demmaccs_context.output, "Moving {} to {}".format(maccs_out, new_file), tile_log_filename)
                shutil.move(maccs_out, new_file)
        elif output_format == L1C_MAJA_PROCESSOR_OUTPUT_FORMAT and maja_dir is not None:
            #check for THEIA/MUSCATE format
            log(demmaccs_context.output, "THEIA/MUSCATE ouput format found. Searching output for valid results", tile_log_filename)            
            new_file = os.path.join(demmaccs_context.output, os.path.basename(maja_dir))
            if os.path.isdir(new_file):
                log(demmaccs_context.output, "The directory {} already exists. Trying to delete it in order to move the new created directory by MAJA".format(new_file), tile_log_filename)
                shutil.rmtree(new_file)
            elif os.path.isfile(new_file):
                log(demmaccs_context.output, "The file {} already exists. Trying to delete it in order to move the new created file by MAJA".format(new_file), tile_log_filename)
                os.remove(new_file)
            else: #the dest does not exist, so it will be moved without problems
                pass
            log(demmaccs_context.output, "Moving {} to {}".format(maja_dir, new_file), tile_log_filename)
            shutil.move(maja_dir, new_file)
            return_tile_id = "{}".format(tile_id)
        else:
            log(demmaccs_context.output, "No valid products (MACCS VALD status or THEIA/MUSCATE formats) found in: {}.".format(maccs_working_dir), tile_log_filename)
        log(demmaccs_context.output, "Erasing the MACCS/MAJA working directory: rmtree: {}".format(maccs_working_dir), tile_log_filename)
        shutil.rmtree(maccs_working_dir)
    except Exception, e:
        return_tile_id = ""
        log(demmaccs_context.output, "Tile failure: Exception caught when moving maccs files for tile {} to the output directory {}: {}".format(tile_id, demmaccs_context.output, e), tile_log_filename)
 
    return return_tile_id


parser = argparse.ArgumentParser(
    description="Launches DEM and MACCS/MAJA for L2A product creation")
parser.add_argument('input', help="input L1C directory")
parser.add_argument('-t', '--tiles-to-process', required=False, nargs='+', help="only this tiles shall be processed from the whole product", default=None)
parser.add_argument('-w', '--working-dir', required=True,
                    help="working directory")
parser.add_argument('--srtm', required=True, help="SRTM dataset path")
parser.add_argument('--swbd', required=True, help="SWBD dataset path")
parser.add_argument('--gipp-dir', required=True, help="directory where gip are to be found")
parser.add_argument('--maccs-launcher', required=True, help="MACCS or MAJA binary path in localhost (or remote host if maccs-address is set)")
parser.add_argument('--processes-number-dem', required=False,
                        help="number of processes to run DEM in parallel", default="3")
parser.add_argument('--processes-number-maccs', required=False,
                        help="number of processes to run MACCS/MAJA in parallel", default="2")
parser.add_argument('--maccs-address', required=False, help="MACCS/MAJA has to be run from a remote host. This should be the ip address of the pc where MACCS/MAJA is to be found")
parser.add_argument('--skip-dem', required=False,
                        help="skip DEM if a directory with previous work of DEM is given", default=None)
parser.add_argument('--prev-l2a-tiles', required=False,
                        help="Previous processed tiles from L2A product", default=[], nargs="+")
parser.add_argument('--prev-l2a-products-paths', required=False,
                        help="Path of the previous processed tiles from L2A product", default=[], nargs="+")
parser.add_argument('--delete-temp', required=False,
                        help="if set to True, it will delete all the temporary files and directories. Default: True", default="True")
parser.add_argument('--suffix-log-name', required=False,
                        help="if set, the string will be part of the log filename . Default: null", default=None)
parser.add_argument('output', help="output location")

args = parser.parse_args()
log_filename = "demmaccs.log"
suffix_log_name = None
print("args.suffix_log_name = {}".format(args.suffix_log_name))
if args.suffix_log_name is not None:
    suffix_log_name = args.suffix_log_name
    log_filename = "demmaccs_{}.log".format(args.suffix_log_name)
if not create_recursive_dirs(args.output):
    log(general_log_path, "Could not create the output directory", log_filename)
    sys.exit(-1)

if len(args.prev_l2a_tiles) != len(args.prev_l2a_products_paths):
    log(general_log_path, "The number of previous l2a tiles is not the same as for paths for these tiles. Check args: --prev-l2-tiles and --prev-l2a-products-paths, the length should be equal", log_filename)
    sys.exit(-1)

general_log_path = args.output
working_dir = tempfile.mkdtemp(dir = args.working_dir)
#working_dir = "{}/{}".format(args.working_dir[:len(args.working_dir) - 1] if args.working_dir.endswith("/") else args.working_dir, os.getpid())
if args.skip_dem is not None:
    working_dir = "{}".format(args.skip_dem[:len(args.skip_dem) - 1]) if args.skip_dem.endswith("/") else "{}".format(args.skip_dem)
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

tiles_to_process = []
if args.skip_dem is None:
    print("Creating DEMs for {}".format(args.input))
    if args.tiles_to_process is not None :
        tiles_to_process = args.tiles_to_process
        args.processes_number_dem = str(len(tiles_to_process))
        args.processes_number_maccs = str(len(tiles_to_process))
    print("tiles_to_process = {}".format(tiles_to_process))
    dem_command = [base_abs_path + "dem.py", "--srtm", args.srtm, "--swbd", args.swbd, "-p", args.processes_number_dem, "-w", dem_working_dir, args.input, dem_output_dir]
    if len(tiles_to_process) > 0:
        dem_command.append("-l")
        dem_command += tiles_to_process
    print("dem_command = {}".format(dem_command))
    if run_command(dem_command , general_log_path, log_filename) != 0:
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

if len(tiles_to_process) > 0 and len(tiles_to_process) != len(dem_hdrs):
    log(general_log_path, "The number of hdr DEM files in {} is not equal with the number of the received tiles to process !!".format(dem_output_dir), log_filename)
demmaccs_contexts = []
print("Creating demmaccs contexts with: input: {} | output {}".format(args.input, args.output))
for dem_hdr in dem_hdrs:
    print("DEM_HDR: {}".format(dem_hdr))
    demmaccs_contexts.append(DEMMACCSContext(working_dir, dem_hdr, args.gipp_dir, args.prev_l2a_tiles, args.prev_l2a_products_paths, args.maccs_address, args.maccs_launcher, args.input, args.output))

processed_tiles = []
if len(demmaccs_contexts) == 1:
    print("One process will be started for this demmaccs")
    out = maccs_launcher(demmaccs_contexts[0])
    if len(out) >=5:
        processed_tiles.append(out)
else:
    #RELEASE mode, parallel launching
    # LE (august 2018): keeping parallel launching for compatibility. Now, demmaccs is launched for one tile only
    print("Parallel launching")
    pool = Pool(int(args.processes_number_maccs))
    pool_outputs = pool.map(maccs_launcher, demmaccs_contexts)
    pool.close()
    pool.join()

    #DEBUG mode only, sequentially launching
    #pool_outputs = map(maccs_launcher, demmaccs_contexts)

    for out in pool_outputs:
        if len(out) >=5:
            processed_tiles.append(out)

sys_exit = int(0)
if len(processed_tiles) == 0:
    log(general_log_path, "MACCS/MAJA did not processed any tiles for L1C product {}".format(args.input), log_filename)
    sys_exit = 1
else:
    log(general_log_path, "MACCS/MAJA processed the following tiles for L1C product {} :".format(args.input), log_filename)
    log(general_log_path, "{}".format(processed_tiles), log_filename)
#    if run_command([os.path.dirname(os.path.abspath(__file__)) + "/mosaic_l2a.py", "-i", args.output, "-w", working_dir], args.output, log_filename) != 0:
#        log(general_log_path, "Mosaic didn't work", log_filename)

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
