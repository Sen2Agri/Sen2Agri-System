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
import re
import fnmatch
import os, errno
from os.path import isfile, isdir, join
import glob
import sys
import time, datetime
import gdal
import Queue
from osgeo import ogr
from multiprocessing import Pool
from sen2agri_common_db import *
from threading import Thread
import threading
from bs4 import BeautifulSoup as Soup
import zipfile
import tarfile
import tempfile
import ntpath

ARCHIVES = "archives"
general_log_path = "/tmp/"
general_log_filename = "demmaccs.log"
maccs_text_to_stop_retrying = ["The number of cloudy pixel is too high", "algorithm processing is stopped", "The dark surface reflectance associated to the value of AOT index min is lower than the dark surface reflectance threshold", "The number of NoData pixel in the output L2 composite product is too high", "PersistentStreamingConditionalStatisticsImageFilter::Synthetize.No pixel is valid. Return null statistics"]

def get_envelope(footprints):
    geomCol = ogr.Geometry(ogr.wkbGeometryCollection)

    for footprint in footprints:
        #ring = ogr.Geometry(ogr.wkbLinearRing)
        for pt in footprint:
            #ring.AddPoint(pt[0], pt[1])
            point = ogr.Geometry(ogr.wkbPoint)
            point.AddPoint_2D(pt[0], pt[1])
            geomCol.AddGeometry(point)

        #poly = ogr.Geometry(ogr.wkbPolygon)
        #poly

    hull = geomCol.ConvexHull()
    return hull.ExportToWkt()

class L1CContext(object):
    def __init__(self, l1c_list, processor_short_name, base_output_path, skip_dem = None):
        self.l1c_list = l1c_list
        self.processor_short_name = processor_short_name
        self.base_output_path = base_output_path
        self.skip_dem = skip_dem

class DEMMACCSLogExtract(object):
    def __init__(self):
        self.should_retry = True
        self.error_message = ""
        self.cloud_coverage = None
        self.snow_coverage = None

def get_previous_l2a_tiles_paths(satellite_id, l1c_product_path, l1c_date, l1c_orbit_id, l1c_db, site_id):
    #get all the tiles for the input. they will be used to find if there is a previous L2A product
    l1c_tiles = []
    if satellite_id == SENTINEL2_SATELLITE_ID:
        #sentinel, all the tiles are to be found as directories in product_name/GRANULE
        l1c_tiles_dir_list = (glob.glob("{}/GRANULE/*".format(l1c_product_path)))
        for tile_dir in l1c_tiles_dir_list:
            tile = re.search(r"_T(\d\d[a-zA-Z]{3})_", tile_dir)
            if tile is not None:
                l1c_tiles.append(tile.group(1))
    elif satellite_id == LANDSAT8_SATELLITE_ID:
        #for landsat, there is only 1 tile which can be taken from the l1c product name
        tile = re.search(r"LC8(\d{6})\d{7}[A-Z]{3}", l1c_product_path)
        if tile is not None:
            l1c_tiles.append(tile.group(1))
        else :
            tile = re.search(r"LC08_L1[a-zA-Z]{2}_(\d{6})_\d{8}_\d{8}_\d{2}_[a-zA-Z0-9]{2}", l1c_product_path)
            if tile is not None:
                l1c_tiles.append(tile.group(1))
    else:
        print("Unkown satellite id :{}".format(satellite_id))
        return [] and ([], [])
    l2a_tiles = []
    l2a_tiles_paths = []
    for l1c_tile in l1c_tiles:
        l2a_tile = l1c_db.get_previous_l2a_tile_path(satellite_id, l1c_tile, l1c_date, l1c_orbit_id, site_id)
        if len(l2a_tile) > 0:
            l2a_tiles.append(l1c_tile)
            l2a_tiles_paths.append(l2a_tile)
    return (l2a_tiles, l2a_tiles_paths)

def validate_L1C_product_dir(l1cDir):
    print('--\nChecking ROOT for valid symlink = ' + l1cDir)
    for root, subdirs, files in os.walk(l1cDir):
#        print('--\nChecking ROOT for valid symlink = ' + root)

        for subdir in subdirs:
            subdir_path = os.path.join(root, subdir)
#            print('\t- subdirectory ' + subdir_path)
            try:
                os.stat(subdir_path)
            except OSError, e:
                    print ("###################################")
                    print ("Cannot check if dir path {} exists or it is a valid symlink. Error was: {}".format(subdir_path, e.errno))
                    print ("###################################")
                    return False

        for filename in files:
            file_path = os.path.join(root, filename)
            #print('\t- file %s (full path: %s)' % (filename, file_path))
            try:
                os.stat(file_path)
            except OSError, e:
                print ("###################################")
                print ("Cannot check if file path {} exists or is a valid symlink. Error was: {}".format(subdir_path, e.errno))
                print ("###################################")
                return False

    return True

def post_process_maccs_product(demmaccs_config, output_path) : 
    print ("Postprocessing product {} ...".format(output_path))
    for root, dirnames, filenames in os.walk(output_path):
        for filename in fnmatch.filter(filenames, "*.TIF"):
            tifFilePath = os.path.join(root, filename)
            print("Processing {}".format(filename))
            if (demmaccs_config.removeSreFiles == True) :
                isSre = re.match(".*SRE.*\.DBL\.TIF", filename)
                if isSre is not None:
                    print("Deleting SRE file {}".format(tifFilePath))
                    os.remove(tifFilePath)
            elif (demmaccs_config.removeFreFiles == True) :
                isFre = re.match(".*FRE.*\.DBL\.TIF", filename)
                if isFre is not None:
                    print("Deleting FRE file {}".format(tifFilePath))
                    os.remove(tifFilePath)
            if ((demmaccs_config.compressTiffs == True) or (demmaccs_config.cogTiffs == True)) :
                optgtiffArgs = ""
                if (demmaccs_config.compressTiffs == True) :
                    optgtiffArgs += " --compress"
                    optgtiffArgs += " DEFLATE"
                else:
                    optgtiffArgs += " --no-compress"
                    
                if (demmaccs_config.cogTiffs == True) :
                    isMask = re.match(".*[_MSK|_QLT]?\.DBL\.TIF", filename)
                    if isMask is not None :
                        optgtiffArgs += " --resampler"
                        optgtiffArgs += " nearest"
                    else :
                        optgtiffArgs += " --resampler"
                        optgtiffArgs += " average"
                    optgtiffArgs += " --overviews"
                    optgtiffArgs += " --tiled"
                else:
                    optgtiffArgs += " --no-overviews"
                    optgtiffArgs += " --strippped"
                optgtiffArgs += " "
                optgtiffArgs += tifFilePath
                print("Running optimize_gtiff.py with params {}".format(optgtiffArgs))                 
                os.system("optimize_gtiff.py" + optgtiffArgs)
    
# def get_product_footprint(tiles_dir_list, satellite_id):
#     wgs84_extent_list = []
#     for tile_dir in tiles_dir_list:
#         if satellite_id == SENTINEL2_SATELLITE_ID:
#             tile_img = (glob.glob("{}/*_FRE_R1.DBL.TIF".format(tile_dir)))
#         else: #satellite_id is LANDSAT8_SATELLITE_ID:
#             tile_img = (glob.glob("{}/*_FRE.DBL.TIF".format(tile_dir)))

#         if len(tile_img) > 0:
#             wgs84_extent_list.append(get_footprint(tile_img[0])[0])
#     wkt = get_envelope(wgs84_extent_list)
#     return wkt

def get_product_footprint(output_path, output_format, maja_dir, satellite_id, tile_log_filename):
    wgs84_extent_list = []
    if output_format == L1C_MACCS_PROCESSOR_OUTPUT_FORMAT:
        tiles_dir_list = (glob.glob("{}*.DBL.DIR".format(output_path)))
        log(output_path, "{}: Creating common footprint for MACCS output DBL.DIR: [{}]".format(threading.currentThread().getName(), tiles_dir_list), tile_log_filename)
        for tile_dir in tiles_dir_list:
            if satellite_id == SENTINEL2_SATELLITE_ID:
                tile_img = (glob.glob("{}/*_FRE_R1.DBL.TIF".format(tile_dir)))
            else: #satellite_id is LANDSAT8_SATELLITE_ID:
                tile_img = (glob.glob("{}/*_FRE.DBL.TIF".format(tile_dir)))

            if len(tile_img) > 0:
                wgs84_extent_list.append(get_footprint(tile_img[0])[0])
    elif output_format == L1C_MAJA_PROCESSOR_OUTPUT_FORMAT and maja_dir is not None:
        log(output_path, "{}: Creating common footprint for MAJA".format(threading.currentThread().getName()), tile_log_filename)
        tile_img = ""
        for root, dirs, filenames in os.walk(output_path):
            for filename in fnmatch.filter(filenames, "*_FRE_B2.tif"):
                tile_img = os.path.join(root, filename)
                log(output_path, "{}: MAJA common footprint tif file: {}".format(threading.currentThread().getName(), tile_img), tile_log_filename)
                wgs84_extent_list.append(get_footprint(tile_img)[0])
                break
            if len(tile_img) > 0:
                break
        
    wkt = get_envelope(wgs84_extent_list)
    return wkt
def launch_demmaccs(l1c_context, l1c_db_thread):
    global general_log_path
    global general_log_filename
    if len(l1c_context.l1c_list) == 0:
        return
    tiles_dir_list = []
    #get site short name
    site_short_name = l1c_db_thread.get_short_name("site", l1c_context.l1c_list[0][1])
    #create the output_path. it will hold all the tiles found inside the l1c
    #for sentinel, this output path will be something like /path/to/poduct/site_name/processor_name/....MSIL2A.../
    #for landsat, this output path will be something like /path/to/poduct/site_name/processor_name/LC8...._L2A/
    site_output_path = l1c_context.base_output_path.replace("{site}", site_short_name)
    if not site_output_path.endswith("/"):
        site_output_path += "/"

    for l1c in l1c_context.l1c_list:
        file_path = l1c[3]
        site_id = l1c[1]
        satellite_id = l1c[2]
        if not l1c_db_thread.is_site_enabled(site_id):
            print("Aborting processing for site {} because it's disabled".format(site_id))
            return

        if not l1c_db_thread.is_sensor_enabled(site_id, satellite_id):
            print("Aborting processing for site {} because sensor downloading for {} is disabled".format(site_id, satellite_id))
            return

        if not validate_L1C_product_dir(file_path):
            print("The product {} is not valid or temporary unavailable!!!".format(file_path))
            return

        # l1c is a record from downloader_history table. the indexes are :
        # 0  | 1       | 2            | 3         | 4            | 5
        # id | site_id | satellite_id | full_path | product_date | orbit_id
        l2a_basename = os.path.basename(file_path[:len(file_path) - 1]) if file_path.endswith("/") else os.path.basename(file_path)
        satellite_id = int(l1c[2])
        if satellite_id != SENTINEL2_SATELLITE_ID and satellite_id != LANDSAT8_SATELLITE_ID:
            log(general_log_path, "Unkown satellite id :{}".format(satellite_id), general_log_filename)
            continue
        if l2a_basename.startswith("S2"):
            l2a_basename = l2a_basename.replace("L1C", "L2A")
        elif l2a_basename.startswith("LC8"):
            l2a_basename += "_L2A"
        elif l2a_basename.startswith("LC08"):
            if l2a_basename.find("_L1TP_") > 0 :
                l2a_basename = l2a_basename.replace("_L1TP_", "_L2A_")
            elif l2a_basename.find("_L1GS_") > 0 :
                l2a_basename = l2a_basename.replace("_L1GS_", "_L2A_")
            elif l2a_basename.find("_L1GT_") > 0 :
                l2a_basename = l2a_basename.replace("_L1GT_", "_L2A_")
            else:
                log(general_log_path, "The L1C product name is bad - L2A cannot be filled: {}".format(l2a_basename), general_log_filename)
                continue
        else:
            log(general_log_path, "The L1C product name is bad: {}".format(l2a_basename), general_log_filename)
            continue

        output_path = site_output_path + l2a_basename + "/"
        # normally, the output_path should be created by the demmaccs.py script itself, but for log reason it has to be created here
        if not create_recursive_dirs(output_path):
            log(general_log_path, "Could not create the output directory", general_log_filename)
            continue

        l2a_tiles, l2a_tiles_paths = get_previous_l2a_tiles_paths(satellite_id, l1c[3], l1c[4], l1c[5], l1c_db_thread, site_id)

        if len(l2a_tiles) != len(l2a_tiles_paths):
            log(output_path, "The lengths of lists l1c tiles and previous l2a tiles are different for {}".format(l2a_basename), general_log_filename)
            continue

        l2a_processed_tiles = []
        wkt = []
        sat_id = 0
        acquisition_date = ""
        base_abs_path = os.path.dirname(os.path.abspath(__file__)) + "/"
        demmaccs_command = [base_abs_path + "demmaccs.py", "--srtm", demmaccs_config.srtm_path, "--swbd", demmaccs_config.swbd_path, "--processes-number-dem", "20", "--processes-number-maccs", "2", "--gipp-dir", demmaccs_config.gips_path, "--working-dir", demmaccs_config.working_dir, "--maccs-launcher", demmaccs_config.maccs_launcher, "--delete-temp", "True", l1c[3], output_path]
        if len(demmaccs_config.maccs_ip_address) > 0:
            demmaccs_command += ["--maccs-address", demmaccs_config.maccs_ip_address]
        if l1c_context.skip_dem != None:
            demmaccs_command += ["--skip-dem", l1c_context.skip_dem]
        if len(l2a_tiles) > 0:
            demmaccs_command.append("--prev-l2a-tiles")
            demmaccs_command += l2a_tiles
            demmaccs_command.append("--prev-l2a-products-paths")
            demmaccs_command += l2a_tiles_paths
        log(output_path, "Starting demmaccs", general_log_filename)
        if run_command(demmaccs_command, output_path, general_log_filename) == 0 and os.path.exists(output_path) and os.path.isdir(output_path):
            tiles_dir_list = (glob.glob("{}*.DBL.DIR".format(output_path)))
            log(output_path, "Creating common footprint for tiles: DBL.DIR List: {}".format(tiles_dir_list), general_log_filename)
            wgs84_extent_list = []
            for tile_dir in tiles_dir_list:
                if satellite_id == SENTINEL2_SATELLITE_ID:
                    tile_img = (glob.glob("{}/*_FRE_R1.DBL.TIF".format(tile_dir)))
                else: #satellite_id is LANDSAT8_SATELLITE_ID:
                    tile_img = (glob.glob("{}/*_FRE.DBL.TIF".format(tile_dir)))

                if len(tile_img) > 0:
                    wgs84_extent_list.append(get_footprint(tile_img[0])[0])
            wkt = get_envelope(wgs84_extent_list)

            if len(wkt) == 0:
                log(output_path, "Could not create the footprint", general_log_filename)
            else:
                sat_id, acquisition_date = get_product_info(os.path.basename(output_path[:len(output_path) - 1]))
                if sat_id > 0 and acquisition_date != None:
                    #check for MACCS tiles output. If none was processed, only the record from
                    #downloader_history table will be updated. No l2a product will be added into product table
                    for tile_dbl_dir in tiles_dir_list:
                        tile = None
                        print("tile_dbl_dir {}".format(tile_dbl_dir))
                        if satellite_id == SENTINEL2_SATELLITE_ID:
                            tile = re.search(r"_L2VALD_(\d\d[a-zA-Z]{3})____[\w\.]+$", tile_dbl_dir)
                        else:
                            tile = re.search(r"_L2VALD_([\d]{6})_[\w\.]+$", tile_dbl_dir)
                        if tile is not None and not tile.group(1) in l2a_processed_tiles:
                            l2a_processed_tiles.append(tile.group(1))
                    log(output_path, "Processed tiles: {}  to path: {}".format(l2a_processed_tiles, output_path), general_log_filename)
                else:
                    log(output_path,"Could not get the acquisition date from the product name {}".format(output_path), general_log_filename)
        else:
            log(output_path, "demmaccs.py script didn't work!", general_log_filename)
        if len(l2a_processed_tiles) > 0:
            # post process the valid maccs products
            post_process_maccs_product(demmaccs_config, output_path);
            log(output_path, "Insert info in product table and set state as processed in downloader_history table for product {}".format(output_path), general_log_filename)
        else:
            log(output_path, "Only set the state as processed in downloader_history (no l2a tiles found after maccs) for product {}".format(output_path), general_log_filename)
        l1c_db.set_processed_product(1, site_id, l1c[0], l2a_processed_tiles, output_path, os.path.basename(output_path[:len(output_path) - 1]), wkt, sat_id, acquisition_date, l1c[5])

def path_filename(path):
    head, filename = ntpath.split(path)
    return filename or ntpath.basename(head)

def check_if_flat_archive(output_dir, archive_filename):
    dir_content = os.listdir(output_dir)
    print("check_if_flat_archive dir_content = {} / len = {}".format(dir_content, len(dir_content)))
    if len(dir_content) == 1 and os.path.isdir(os.path.join(output_dir, dir_content[0])):
        return os.path.join(output_dir, dir_content[0])
    if len(dir_content) > 1:
        #use the archive filename, strip it from extension
        product_name, file_ext = os.path.splitext(path_filename(archive_filename))
        #handle .tar.gz case
        if product_name.endswith(".tar"):
            product_name, file_ext = os.path.splitext(product_name)        
        product_path = os.path.join(output_dir, product_name)
        if create_recursive_dirs(product_path):            
            #move the list to this directory:
            for name in dir_content:
                shutil.move(os.path.join(output_dir, name), os.path.join(product_path, name))
            return product_path
    print("Checking if the archive is flat: returns None")
    return None

def unzip(output_dir, input_file):
    global general_log_path
    global general_log_filename
    log(general_log_path, "Unzip archive = {} to {}".format(input_file, output_dir), general_log_filename)
    try:
        with zipfile.ZipFile(input_file) as zip_archive:
            zip_archive.extractall(output_dir)
            return check_if_flat_archive(output_dir, path_filename(input_file))
    except Exception, e:
        log(general_log_path, "Exception when trying to unzip file {}:  {} ".format(input_file, e), general_log_filename)
    return None

def untar(output_dir, input_file):
    global general_log_path
    global general_log_filename
    log(general_log_path, "Untar archive = {} to {}".format(input_file, output_dir), general_log_filename)
    try:
        tar_archive = tarfile.open(input_file)
        tar_archive.extractall(output_dir)
        tar_archive.close()
        return check_if_flat_archive(output_dir, path_filename(input_file))
    except Exception, e:
        log(general_log_path, "Exception when trying to untar file {}:  {} ".format(input_file, e), general_log_filename)
    return None

def extract_from_archive_if_needed(archive_file):
    #create the temporary path where the archive will be extracted
    extracted_archive_dir = tempfile.mkdtemp(dir = os.path.join(demmaccs_config.working_dir, ARCHIVES))
    print("ARCHIVES DIRECTORY = {}".format(extracted_archive_dir))
    extracted_file_path = None
    # check if the file is indeed an archive
    # exception raised only if the archive_file does not exist
    try:
        if zipfile.is_zipfile(archive_file):
            extracted_file_path = unzip(extracted_archive_dir, archive_file)
    except Exception, e:
        print("is_zipfile: The object (directory or file) {} is not an archive: {} !".format(archive_file, e))
        extracted_file_path = None            
    try:
        if tarfile.is_tarfile(archive_file):
            extracted_file_path = untar(extracted_archive_dir, archive_file)
    except Exception, e:
        print("is_tarfile: The object (directory or file) {} is not an archive: {} !".format(archive_file, e))
        extracted_file_path = None            
    if extracted_file_path is not None:
        print("Archive extracted to: {}".format(extracted_file_path))
        return True, extracted_file_path
    # this isn't and archive, so no need for the temporary directory
    print("This wasn't an archive, so continue as is. Deleting {}".format(extracted_archive_dir))
    remove_dir(extracted_archive_dir)
    return False, archive_file
    
def get_maccs_log_extract(maccs_report_file):
    demmaccs_log_extract = DEMMACCSLogExtract()
    try:
        xml_handler = open(maccs_report_file).read()
        soup = Soup(xml_handler)
        for message in soup.find_all('message'):
            msg_type = message.find('type').get_text()
            msg_text = message.find('text').get_text()
            if msg_type == 'W' or msg_type == 'E':
                demmaccs_log_extract.error_message =  demmaccs_log_extract.error_message + msg_text + "\n"
            if msg_type == 'I' and re.search("code return: 0", msg_text, re.IGNORECASE):
                demmaccs_log_extract.should_retry = False 
            if demmaccs_log_extract.cloud_coverage == None and re.search("cloud percentage computed is", msg_text, re.IGNORECASE):
                numbers = [int(s) for s in re.findall(r'\d+', msg_text)]
                print(msg_text)
                print("{}".format(numbers))
                if len(numbers) > 0:
                    demmaccs_log_extract.cloud_coverage = numbers[0]
            if demmaccs_log_extract.snow_coverage == None and re.search("snow percentage computed is", msg_text, re.IGNORECASE):
                numbers = [int(s) for s in re.findall(r'\d+', msg_text)]
                print("{}".format(numbers))
                if len(numbers) > 0:
                    demmaccs_log_extract.snow_coverage = numbers[0]
            
    except Exception, e:
        print("Exception received when trying to read the MACCS/MAJA error text from file {}: {}".format(maccs_report_file, e))
        pass
    return demmaccs_log_extract

def get_log_info(path, tile_id):
    path_to_use = path[:len(path) - 1] if path.endswith("/") else path
    maccs_report_file = "{}/MACCS_L2REPT_{}.EEF".format(path_to_use, tile_id)
    demmaccs_log_extract = get_maccs_log_extract(maccs_report_file)
    tile_log_filename = "{}/demmaccs_{}.log".format(path_to_use, tile_id)
    if len(demmaccs_log_extract.error_message) > 0:
        demmaccs_log_extract.error_message = "MACCS/MAJA: \n" + demmaccs_log_extract.error_message
        log(path, "MACCS/MAJA error / warning text found. should retry: {}".format(demmaccs_log_extract.should_retry), tile_log_filename)

    try:
        with open(tile_log_filename) as in_file:
            contents = in_file.readlines()
            for line in contents:
                index = line.find("Tile failure: ")
                if index != -1:
                    demmaccs_log_extract.error_message = demmaccs_log_extract.error_message + "DEMMACCS: \n" + line[index + 14:]
                    break
    except IOError as e:
        print("No log file {} to get the reason".format(tile_log_filename))
    if len(demmaccs_log_extract.error_message) == 0:
        demmaccs_log_extract.error_message = None
    return demmaccs_log_extract

def set_l1_tile_status(l1c_db_thread, product_id, tile_id, cloud_coverage, snow_coverage, reason = None, should_retry = None):
    retries = 0
    max_number_of_retries = 3
    while True:
        if reason is not None and should_retry is not None:
            is_product_finished = l1c_db_thread.mark_l1_tile_failed(product_id, tile_id, reason, should_retry, cloud_coverage, snow_coverage)
        else:
            is_product_finished = l1c_db_thread.mark_l1_tile_done(product_id, tile_id, cloud_coverage, snow_coverage)
        if is_product_finished == False:
            serialization_failure, commit_result = l1c_db_thread.sql_commit()
            if commit_result == False:
                l1c_db_thread.sql_rollback()
            if serialization_failure and retries < max_number_of_retries:
                time.sleep(2)
                retries += 1
                continue
            l1c_db_thread.database_disconnect()
        return is_product_finished

def new_launch_demmaccs(l1c_db_thread):
    global general_log_path
    global general_log_filename
    print("Starting thread {}".format(threading.currentThread().getName()))
    while True:
        # get the tile to process. The object from the queue is L1CContext
        l1c_context = l1c_queue.get()
        thread_finished_queue.put("started")
        print("{} will consume: {} | {} | {}".format(threading.currentThread().getName(), l1c_context.l1c_list, l1c_context.processor_short_name, l1c_context.base_output_path))
        if l1c_context.l1c_list == None:
            # no more tiles to process in db, so exit from thread
            log(general_log_path, "{}: No tile to process. Gracefully closing...".format(threading.currentThread().getName()), general_log_filename)
            thread_finished_queue.get()
            l1c_queue.task_done()
            log(general_log_path, "{}: Exit thread".format(threading.currentThread().getName()), general_log_filename)
            return
        if len(l1c_context.l1c_list) != 1:
            # input error, length of the list has to be 1, this is in fact the result from database, 
            # query: select * from sp_start_l1_tile_processing()
            log(general_log_path, "{}: Input error from database".format(threading.currentThread().getName()), general_log_filename)
            thread_finished_queue.get()
            l1c_queue.task_done()
            continue

        l1c = l1c_context.l1c_list[0]
        # l1c is the only record from sp_start_l1_tile_processing() function. the cells are :
        # 0       | 1            | 2        | 3       | 4                     | 5    | 6
        # site_id | satellite_id | orbit_id | tile_id | downloader_history_id | path | previous_l2a_path
        site_id = l1c[0][0]
        satellite_id = l1c[0][1]
        orbit_id = l1c[0][2]
        tile_id = l1c[0][3]
        product_id = l1c[0][4]        
        log(general_log_path, "{}: Starting extract_from_archive_if_needed for tile {}".format(threading.currentThread().getName(), tile_id), general_log_filename)
        l1c_was_archived, full_path = extract_from_archive_if_needed(l1c[0][5])
        log(general_log_path, "{}: Ended extract_from_archive_if_needed for tile {}".format(threading.currentThread().getName(), tile_id), general_log_filename)
        previous_l2a_path = l1c[0][6]
        print("{}: site_id = {}".format(threading.currentThread().getName(), site_id))
        print("{}: satellite_id = {}".format(threading.currentThread().getName(), satellite_id))
        print("{}: orbit_id = {}".format(threading.currentThread().getName(), orbit_id))
        print("{}: tile_id = {}".format(threading.currentThread().getName(), tile_id))
        print("{}: dh_id = {}".format(threading.currentThread().getName(), product_id))
        print("{}: path = {}".format(threading.currentThread().getName(), full_path))
        print("{}: prev_path = {}".format(threading.currentThread().getName(), previous_l2a_path))
#        demmaccs_log_extract = get_log_info("/mnt/archive/maccs_def/belgium_test_alex/l2a/LC08_L2A_196025_20160620_20170323_01_T1", 196025)
#        print("should_retry = {} \n error_message = {} \n cloud_coverage = {} \n snow_coverage = {} \n".format(demmaccs_log_extract.should_retry, demmaccs_log_extract.error_message, demmaccs_log_extract.cloud_coverage, demmaccs_log_extract.snow_coverage))
#        db_result_tile_processed = set_l1_tile_status(l1c_db_thread, product_id, "196025", demmaccs_log_extract.cloud_coverage, demmaccs_log_extract.snow_coverage)
#        l1c_db_thread.sql_rollback        
#        print("!!!!{}".format(db_result_tile_processed))
#        time.sleep(10) 
#        l1c_queue.task_done()        
#        thread_finished_queue.get()
#        continue

        # processing the tile
        #get site short name
        site_short_name = l1c_db_thread.get_short_name("site", site_id)
        #create the output_path. it will hold all the tiles found inside the l1c
        #for sentinel, this output path will be something like /path/to/poduct/site_name/processor_name/....MSIL2A.../
        #for landsat, this output path will be something like /path/to/poduct/site_name/processor_name/LC8...._L2A/
        site_output_path = l1c_context.base_output_path.replace("{site}", site_short_name)
        if not site_output_path.endswith("/"):
            site_output_path += "/"

        if not l1c_db_thread.is_site_enabled(site_id):
            log(general_log_path, "{}: Aborting processing for site {} because it is marked as being deactivated".format(threading.currentThread().getName(), site_id), general_log_filename)
            if l1c_was_archived:
                remove_dir(full_path)
            thread_finished_queue.get()
            l1c_queue.task_done()
            continue

        if not l1c_db_thread.is_sensor_enabled(site_id, satellite_id):
            log(general_log_path, "{}: Aborting processing for site {} because sensor downloading for {} is marked as being disabled".format(threading.currentThread().getName(), site_id, satellite_id), general_log_filename)
            if l1c_was_archived:
                remove_dir(full_path)
            thread_finished_queue.get()
            l1c_queue.task_done()
            continue

        if not validate_L1C_product_dir(full_path):
            log(general_log_path, "{}: The product {} is not valid or temporary unavailable...".format(threading.currentThread().getName(), full_path), general_log_filename)
            if l1c_was_archived:
                remove_dir(full_path)
            thread_finished_queue.get()
            l1c_queue.task_done()
            continue

        l2a_basename = os.path.basename(full_path[:len(full_path) - 1]) if full_path.endswith("/") else os.path.basename(full_path)
        satellite_id = int(satellite_id)
        orbit_id = int(orbit_id)
        if satellite_id != SENTINEL2_SATELLITE_ID and satellite_id != LANDSAT8_SATELLITE_ID:
            log(general_log_path, "{}: Unkown satellite id :{}".format(threading.currentThread().getName(), satellite_id), general_log_filename)
            if l1c_was_archived:
                remove_dir(full_path)
            thread_finished_queue.get()
            l1c_queue.task_done()            
            continue
        if l2a_basename.startswith("S2"):
            l2a_basename = l2a_basename.replace("L1C", "L2A")
        elif l2a_basename.startswith("LC8"):
            l2a_basename += "_L2A"
        elif l2a_basename.startswith("LC08"):
            if l2a_basename.find("_L1TP_") > 0 :
                l2a_basename = l2a_basename.replace("_L1TP_", "_L2A_")
            elif l2a_basename.find("_L1GS_") > 0 :
                l2a_basename = l2a_basename.replace("_L1GS_", "_L2A_")
            elif l2a_basename.find("_L1GT_") > 0 :
                l2a_basename = l2a_basename.replace("_L1GT_", "_L2A_")
            else:
                log(general_log_path, "{}: The L1C product name is wrong - L2A cannot be filled: {}".format(threading.currentThread().getName(), l2a_basename), general_log_filename)
                if l1c_was_archived:
                    remove_dir(full_path)
                thread_finished_queue.get()
                l1c_queue.task_done()
                continue
        else:
            log(general_log_path, "{}: The L1C product name is wrong: {}".format(threading.currentThread().getName(), l2a_basename), general_log_filename)
            if l1c_was_archived:
                remove_dir(full_path)
            thread_finished_queue.get()
            l1c_queue.task_done()            
            continue

        output_path = site_output_path + l2a_basename + "/"
        tile_log_filename = "demmaccs_{}.log".format(tile_id)
        log(output_path, "{}: Starting the process for tile {}".format(threading.currentThread().getName(), tile_id), tile_log_filename)
        # the output_path should be created by the demmaccs.py script itself, but for log reason it will be created here
        if not create_recursive_dirs(output_path):
            log(general_log_path, "{}: Could not create the output directory".format(threading.currentThread().getName()), general_log_filename)
            if l1c_was_archived:
                remove_dir(full_path)
            thread_finished_queue.get()
            l1c_queue.task_done()
            continue            

        l2a_tiles = []
        l2a_tiles_paths = []
        if previous_l2a_path != None:
            l2a_tiles.append(tile_id)
            l2a_tiles_paths.append(previous_l2a_path)

        l2a_processed_tiles = []
        wkt = []
        sat_id = 0
        acquisition_date = ""
        base_abs_path = os.path.dirname(os.path.abspath(__file__)) + "/"
        #demmaccs_command = [base_abs_path + "demmaccs.py", "--srtm", demmaccs_config.srtm_path, "--swbd", demmaccs_config.swbd_path, "--processes-number-dem", "1", "--processes-number-maccs", "1", "--gipp-dir", "/mnt/archive/gipp_maja", "--working-dir", demmaccs_config.working_dir, "--maccs-launcher", "/opt/maja/3.1.1/bin/maja", "--delete-temp", "False", full_path, output_path]
        demmaccs_command = [base_abs_path + "demmaccs.py", "--srtm", demmaccs_config.srtm_path, "--swbd", demmaccs_config.swbd_path, "--processes-number-dem", "1", "--processes-number-maccs", "1", "--gipp-dir", demmaccs_config.gips_path, "--working-dir", demmaccs_config.working_dir, "--maccs-launcher", demmaccs_config.maccs_launcher, "--delete-temp", "True", full_path, output_path]
        if len(demmaccs_config.maccs_ip_address) > 0:
            demmaccs_command += ["--maccs-address", demmaccs_config.maccs_ip_address]
        if l1c_context.skip_dem != None:
            demmaccs_command += ["--skip-dem", l1c_context.skip_dem]
        if len(tile_id) > 0:
            demmaccs_command.append("--tiles-to-process")
            tiles = []
            tiles.append(tile_id)
            demmaccs_command += tiles
        if len(l2a_tiles) > 0:
            demmaccs_command.append("--prev-l2a-tiles")
            demmaccs_command += l2a_tiles
            demmaccs_command.append("--prev-l2a-products-paths")
            demmaccs_command += l2a_tiles_paths
        create_footprint = False
        should_retry = None
        reason = None
        log(output_path, "{}: Starting demmaccs".format(threading.currentThread().getName()), tile_log_filename)
        if run_command(demmaccs_command, output_path, tile_log_filename) == 0 and os.path.exists(output_path) and os.path.isdir(output_path):
            # mark the tile as done
            # if there are still tiles to be processed within this product, the commit call for database will be performed 
            # inside mark_l1_tile_done or mark_l1_tile_failed. If this was the last tile within this product (the mark_l1_tile_done or 
            # mark_l1_tile_failed functions return the true) the footprint of the product has to be processed 
            # before calling set_l2a_product function. The set_l2a_product function will also close the transaction by calling commit 
            # for database. In this case the mark_l1_tile_done or mark_l1_tile_failed functions will not call the commit for database            
            demmaccs_log_extract = get_log_info(output_path, tile_id)
            db_result_tile_processed = set_l1_tile_status(l1c_db_thread, product_id, tile_id, demmaccs_log_extract.cloud_coverage, demmaccs_log_extract.snow_coverage)
            log(output_path, "{}: Tile {} marked as DONE, with db_result_tile_processed = {}".format(threading.currentThread().getName(), tile_id, db_result_tile_processed), tile_log_filename)
        else:
            log(output_path, "{}: demmaccs.py script failed!".format(threading.currentThread().getName()), tile_log_filename)
            demmaccs_log_extract = get_log_info(output_path, tile_id)
            should_retry = demmaccs_log_extract.should_retry
            reason = demmaccs_log_extract.error_message
            db_result_tile_processed = set_l1_tile_status(l1c_db_thread, product_id, tile_id, demmaccs_log_extract.cloud_coverage, demmaccs_log_extract.snow_coverage, reason, should_retry)
            log(output_path, "{}: Tile {} marked as FAILED (should the process be retried: {}). The L1C product {} finished: {}. Reason for failure: {}".format(threading.currentThread().getName(), tile_id, demmaccs_log_extract.should_retry, l2a_basename, db_result_tile_processed, reason), tile_log_filename)
        #handle the MAJA processo case
        #elif l1c_processor = L1C_MAJA_PROCESSOR:
        #                        tile = re.search(r"_L2A_T(\d\d[a-zA-Z]{3})_[\w]+$", tile_dbl_dir)
        #                        print("MAJA: tile = {}".format(tile))
        #elif l1c_processor = L1C_MAJA_PROCESSOR:
        #                        tile = re.search(r"_L2A_T([\d]{6})_[\w]+$", tile_dbl_dir)
        #                        print("MAJA: tile = {}".format(tile))
        if db_result_tile_processed:
            # to end the transaction started in mark_l1_tile_done or mark_l1_tile_failed functions,
            # the sql commit for database has to be called within set_l2a_product function below
            
                        
            # create the footprint for the whole product
            #wkt = get_product_footprint(tiles_dir_list, satellite_id)
            output_format, maja_dir = get_l1c_processor_output_format(output_path, tile_id)
            wkt = get_product_footprint(output_path, output_format, maja_dir, satellite_id, tile_log_filename)

            if len(wkt) == 0:
                log(output_path, "{}: Could not create the footprint".format(threading.currentThread().getName()), tile_log_filename)
            else:
                sat_id, acquisition_date = get_product_info(os.path.basename(output_path[:len(output_path) - 1]))
                if sat_id > 0 and acquisition_date != None:
                    #check for MACCS / MAJA tiles output. If none was processed, only the record from
                    #downloader_history table will be updated. No l2a product info will be added into the product table                    
                    if output_format == L1C_MACCS_PROCESSOR_OUTPUT_FORMAT:
                        tiles_dir_list = (glob.glob("{}*.DBL.DIR".format(output_path)))
                        for tile_dbl_dir in tiles_dir_list:
                            tile = None
                            print("tile_dbl_dir = {}".format(tile_dbl_dir))
                            if satellite_id == SENTINEL2_SATELLITE_ID:
                                tile = re.search(r"_L2VALD_(\d\d[a-zA-Z]{3})____[\w\.]+$", tile_dbl_dir)                            
                            elif satellite_id == LANDSAT8_SATELLITE_ID:
                                tile = re.search(r"_L2VALD_([\d]{6})_[\w\.]+$", tile_dbl_dir)                            
                            if tile is not None and not tile.group(1) in l2a_processed_tiles:
                                l2a_processed_tiles.append(tile.group(1))
                        log(output_path, "{}: Processed tiles: {}  to path: {}".format(threading.currentThread().getName(), l2a_processed_tiles, output_path), tile_log_filename)
                    elif output_format == L1C_MAJA_PROCESSOR_OUTPUT_FORMAT and maja_dir is not None:
                        # MAJA case: only the tile that has been processed should exist. Each product should be mono-tile
                        tiles_dir_list = (glob.glob("{}*_T{}_[CHD]_V*".format(output_path, tile_id)))
                        print("tiles_dir_list = {}".format(tiles_dir_list))
                        if len(tiles_dir_list) == 1:
                            l2a_processed_tiles.append(tile_id)
                    else:
                        log(output_path, "{}: Unknown format found in output directory".format(threading.currentThread().getName()), tile_log_filename)
                else:
                    log(output_path,"{}: Could not get the acquisition date from the product name {}".format(threading.currentThread().getName(), output_path), tile_log_filename)
            if len(l2a_processed_tiles) > 0:
                # post process the valid maccs products
                post_process_maccs_product(demmaccs_config, output_path);
                log(output_path, "{}: Processing for tile {} finished. Insert info in product table for {}. Also, set the state as processed in downloader_history table ".format(threading.currentThread().getName(), tile_id, output_path), tile_log_filename)
            else:
                log(output_path, "{}: Processing for tile {} finished. Set the state as processed in downloader_history (no l2a tiles found after maccs finished) for product {}".format(threading.currentThread().getName(), tile_id, output_path), tile_log_filename)
            retries = 0
            max_number_of_retries = 3
            # the postgres SERIALIZATION_FAILURE exception has to be handled
            # this has to be done somehow here at the higher level instead of the database level l1c_db_thread
            while True:
                serialization_failure, commit_result = l1c_db_thread.set_l2a_product(1, site_id, product_id, l2a_processed_tiles, output_path, os.path.basename(output_path[:len(output_path) - 1]), wkt, sat_id, acquisition_date, orbit_id)
                if commit_result == False:                    
                    l1c_db_thread.sql_rollback()
                    log(output_path, "{}: Rolling back for {}".format(threading.currentThread().getName(), output_path), tile_log_filename)
                    if serialization_failure == True and retries < max_number_of_retries and set_l1_tile_status(l1c_db_thread, product_id, tile_id, reason, should_retry):
                        log(output_path, "{}: Exception when inserting to product table: SERIALIZATION_FAILURE, rolling back and will retry".format(threading.currentThread().getName()), tile_log_filename)
                        time.sleep(2)
                        retries += 1
                        continue                
                    log(output_path, "{}: Couldn't insert the product {}".format(threading.currentThread().getName(), output_path), tile_log_filename)
                    l1c_db_thread.database_disconnect()
                    break
                retries = 0
                serialization_failure, commit_result = l1c_db_thread.sql_commit()
                if commit_result == False:                    
                    l1c_db_thread.sql_rollback()
                    log(output_path, "{}: Commit returned false, rolling back for {}".format(threading.currentThread().getName(), output_path), tile_log_filename)
                else:
                    log(output_path, "{}: Product {} inserted".format(threading.currentThread().getName(), output_path), tile_log_filename)
                if serialization_failure == True and retries < max_number_of_retries and set_l1_tile_status(l1c_db_thread, product_id, tile_id, reason, should_retry):
                    log(output_path, "{}: Exception when inserting to product table: SERIALIZATION_FAILURE, rolling back and will retry".format(threading.currentThread().getName()), tile_log_filename)
                    time.sleep(2)
                    retries += 1
                    continue
                l1c_db_thread.database_disconnect()
                break
            # create mosaic 
            if len(l2a_processed_tiles) > 0:
                # MACCS case, the mosaic script is called
                if output_format == L1C_MACCS_PROCESSOR_OUTPUT_FORMAT:
                    if run_command([os.path.dirname(os.path.abspath(__file__)) + "/mosaic_l2a.py", "-i", output_path, "-w", demmaccs_config.working_dir], output_path, tile_log_filename) != 0:
                        log(output_path, "{}: Mosaic didn't work".format(threading.currentThread().getName()), tile_log_filename)
                # MAJA case, just copy the QKL file as "output_path/mosaic.jpg"
                elif output_format == L1C_MAJA_PROCESSOR_OUTPUT_FORMAT and maja_dir is not None:
                    mosaic = ""
                    for root, dirs, filenames in os.walk(output_path):
                        for filename in filenames:
                            if re.search("QKL(.*)\.jpg$", filename, re.IGNORECASE) is not None:
                                mosaic = os.path.join(output_path, "mosaic.jpg")
                                shutil.copy(os.path.join(root, filename), mosaic)
                                break
                        if len(mosaic) > 0:
                            break                        
            if l1c_was_archived:
                remove_dir(full_path)
        # end of tile processing
        # remove the tile from queue
        thread_finished_queue.get()
        l1c_queue.task_done()

parser = argparse.ArgumentParser(
    description="Launcher for DEM MACCS/MAJA script")
parser.add_argument('-c', '--config', default="/etc/sen2agri/sen2agri.conf", help="configuration file")
parser.add_argument('-p', '--processes-number', default=2, help="Number of tiles to be processed at the same time. This number is also applying for the number of MACCS/MAJA processes which may run at the same time")
parser.add_argument('--skip-dem', required=False,
                        help="skip DEM if a directory with previous work of DEM is given", default=None)

args = parser.parse_args()
manage_log_file(general_log_path, general_log_filename)

# get the db configuration from cfg file
config = Config()
if not config.loadConfig(args.config):
    log(general_log_path, "Could not load the config from configuration file", general_log_filename)
    sys.exit(-1)

#load configuration from db for demmaccs processor
l1c_db = L1CInfo(config.host, config.database, config.user, config.password)
demmaccs_config = l1c_db.get_demmaccs_config()
if demmaccs_config is None:
    log(general_log_path, "Could not load the config from database", general_log_filename)
    sys.exit(-1)

if not os.path.isdir(demmaccs_config.working_dir) and not create_recursive_dirs(demmaccs_config.working_dir):
    log(general_log_path, "Could not create the work base directory {}".format(demmaccs_config.working_dir), general_log_filename)
    sys.exit(-1)

#delete all the temporary content from a previous run
#remove_dir_content(demmaccs_config.working_dir)

#create directory for the eventual archives like l1c products
create_recursive_dirs(os.path.join(demmaccs_config.working_dir, ARCHIVES))
l1c_queue = Queue.Queue(maxsize=int(args.processes_number))
thread_finished_queue = Queue.Queue(maxsize=int(args.processes_number))

for i in range(int(args.processes_number)):
    t = Thread(name="dmworker_{}".format(i), target=new_launch_demmaccs, args=(L1CInfo(config.host, config.database, config.user, config.password), ))
    t.daemon = False
    t.start()

#by convention, the processor ID for demmaccs will always be 1 within the DB
processor_short_name = l1c_db.get_short_name("processor", 1)
base_output_path = demmaccs_config.output_path.replace("{processor}", processor_short_name)
l1c_db.clear_pending_l1_tiles()
l1c_tile_to_process = l1c_db.get_unprocessed_l1c_tile()
if(l1c_tile_to_process == None):
    sys.exit(1)
while True:
    if len(l1c_tile_to_process) > 0:
        prev_queue_size = thread_finished_queue.qsize()
        #print("{}: Feeding the queue ...")
        l1c_queue.put(L1CContext(l1c_tile_to_process, processor_short_name, base_output_path, args.skip_dem))
        print("Demmaccs main thread: feeding the queue for workers with: {}".format(l1c_tile_to_process))
        l1c_tile_to_process = []
        time.sleep(1)
        #print("{}: Queue feeded")
    else:
        #print("Main thread is sleeping....")
        time.sleep(5)
    if thread_finished_queue.qsize() < int(args.processes_number):
        l1c_tile_to_process = l1c_db.get_unprocessed_l1c_tile()    
    if (l1c_tile_to_process == None) or (len(l1c_tile_to_process) == 0 and thread_finished_queue.qsize() == 0):
        for i in range(int(args.processes_number)):
            l1c_queue.put(L1CContext(None, processor_short_name, base_output_path, args.skip_dem))
        print("Waiting for queue to join...")
        l1c_queue.join()
        print("All the workers finished their job. Exiting...")
        break

remove_dir_content("{}/".format(demmaccs_config.working_dir))
# following code applies to the old function launch_demmaccs. This launches in paralel products instead of tiles 
#load the unprocessed l1c products from db
#the products will come sorted by date in ascending
#l1c_list = l1c_db.get_unprocessed_l1c()

#do nothing if there is no unprocessed l1c products
#if len(l1c_list) == 0:
#    log(general_log_path, "No unprocessed L1C found in DB", general_log_filename)
#    sys.exit(0)

#by convention, the processor ID for demmaccs will always be 1 within the DB
#processor_short_name = l1c_db.get_short_name("processor", 1)
#base_output_path = demmaccs_config.output_path.replace("{processor}", processor_short_name)

#l1c_context_list = []
#for l1c_site in l1c_list:
#    l1c_context_list.append([L1CContext(l1c_site, processor_short_name, base_output_path, args.skip_dem), L1CInfo(config.host, config.database, config.user, config.password)])

#p = Pool(args.processes_number)
#p.map(launch_demmaccs, l1c_context_list)

#used for debug mode only
#for l1c_context in l1c_context_list:
#    launch_demmaccs(l1c_context)




