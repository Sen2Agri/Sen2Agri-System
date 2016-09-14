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

import glob,os,sys,math,urllib2,urllib,time,math,shutil
import subprocess
import datetime
import csv
import signal
import osgeo.ogr as ogr
import osgeo.osr as osr
from sen2agri_common_db import *
from multiprocessing import Pool

signal.signal(signal.SIGINT, signal_handler)

general_log_filename = "offline_l1_handler.log"
general_log_path = "/tmp/"

parser = argparse.ArgumentParser(
    description="It handles the L1C (sentinel2) and / or L1T (landsat8) products from an offline path")

parser.add_argument("-d", "--directory", dest="directory", action="store", required=True, \
                  help="MANDATORY: Path where the L1C and/or L1T products are to be found")
parser.add_argument("-s", "--site-name", dest="site_name", action="store", required=True, \
                  help="MANDATORY: Short site name (can be found using the website)")
parser.add_argument("-o", "--overwrite", dest="overwrite", action="store", required=False, \
                  help="Overwrites a product if it already exists. Possible values: True / False. Default: False", default = "False")
parser.add_argument("-c","--config", dest="config", action="store", required=False, \
                      help="File with credentials for the database. Default: /etc/sen2agri/sen2agri.conf",default="/etc/sen2agri/sen2agri.conf")

options = parser.parse_args()
if not os.path.exists(options.directory) or not os.path.isdir(options.directory):
    log(general_log_path, "The path {} where L1C/L1T products are to be found does not exist".format(options.directory), general_log_filename)
    sys.exit(-1)

config = Config()
if not config.loadConfig(options.config):
    log(general_log_path, "Could not load the config file", general_log_filename)
    sys.exit(-1)
sentinel_aoi_context = None
landsat_aoi_context = None

date_past = datetime.datetime.now() - datetime.timedelta(days=1)
fake_season_start = "{0:02d}{1:02d}".format(date_past.month, date_past.day)
date_future = datetime.datetime.now() + datetime.timedelta(days=1)
fake_season_end = "{0:02d}{1:02d}".format(date_future.month, date_future.day)

# Sentinel2 information from DB
sentinel_db_info = SentinelAOIInfo(config.host, config.database, config.user, config.password)
sentinel_aoi_contexts_aoi_database = sentinel_db_info.getSentinelAOI(-1, fake_season_start, fake_season_end)
for site in sentinel_aoi_contexts_aoi_database:
    if site.siteName == options.site_name:
        sentinel_aoi_context = site
        break

# Landsat8 information from DB
landsat_db_info = LandsatAOIInfo(config.host, config.database, config.user, config.password)
landsat_aoi_contexts_aoi_database = landsat_db_info.getLandsatAOI(-1, fake_season_start, fake_season_end)
for site in landsat_aoi_contexts_aoi_database :
    if site.siteName == options.site_name:
        landsat_aoi_context = site
        break

if sentinel_aoi_context is None or landsat_aoi_context is None:
    log(general_log_path, "Provided site name {} does not match to any site name found in database. The following is a list with site names found in database:".format(options.site_name), general_log_filename)
    for site in landsat_aoi_contexts_aoi_database :
        log(general_log_path, "Short site name: {}".format(site.siteName), general_log_filename)
    sys.exit(-1)

l1_list = glob.glob("{}/*".format(options.directory))
for l1 in l1_list:
    if os.path.isdir(l1):
        #get satellite id and acquisition date for the current product
        if l1.endswith("/"):
            l1_basename = os.path.basename(l1[:len(l1) - 1])
        else:
            l1_basename = os.path.basename(l1)
        print("===== Start processing for {}".format(l1))
        sat_id, acquisition_date = get_product_info(str(l1_basename))
        orbit_id = -1
        if sat_id == SENTINEL2_SATELLITE_ID:
            write_dir = sentinel_aoi_context.writeDir
            #get the orbit
            orbit_id_search = re.search(r"_R(\d{3})_", l1_basename)
            if orbit_id_search == None:                
                log(write_dir, "Could not take the orbit id from {}. Continue...".format(l1_basename), general_log_filename)
                continue
            orbit_id = orbit_id_search.group(1)
        elif sat_id == LANDSAT8_SATELLITE_ID:
            write_dir = landsat_aoi_context.writeDir
        else:
            log(general_log_path, "Unknown satellite id {} found for {}. Continue...".format(sat_id, l1), general_log_filename)
            continue
        new_dir = "{}/{}".format(write_dir, l1_basename)
        #check if there is already a product with the same name
        if os.path.isdir(new_dir):
            #check if overwrite flag is applied. otherwise, no action will be taken
            if options.overwrite == "False":
                log(write_dir, "There is already a product with the same name {} . The overwrite flag is not set, thus it will not be overwritten".format(new_dir), general_log_filename)
                continue
            log(write_dir, "There is already a product with the same name {} . The overwrite flag is set, thus it will be overwritten".format(new_dir), general_log_filename)
            #delete the old directory
            try:
                log(write_dir, "Deleting old product {} ".format(l1), general_log_filename)
                shutil.rmtree(new_dir)
            except:
                log(write_dir, "Couldn't remove the old dir {}".format(new_dir), general_log_filename)
        #create the dir structure
#        if not create_recursive_dirs(new_dir):
#            log(new_dir, "Could not create the destination directory {}".format(new_dir), general_log_filename)
#            continue
        #log(write_dir, "Set downloading status for {}".format(new_dir), general_log_filename)
        #if sat_id == SENTINEL2_SATELLITE_ID:
        #    if not sentinel_db_info.upsertSentinelProductHistory(sentinel_aoi_context.siteId, l1_basename, DATABASE_DOWNLOADER_STATUS_DOWNLOADING_VALUE, acquisition_date, new_dir, orbit_id, sentinel_aoi_context.maxRetries):
        #        log(write_dir, "Couldn't upsert into database with status DOWNLOADED for {}. Continue...".format(new_dir), general_log_filename)
        #        continue    
        #elif sat_id == LANDSAT8_SATELLITE_ID:
        #    if not landsat_db_info.upsertLandsatProductHistory(landsat_aoi_context.siteId, l1_basename, DATABASE_DOWNLOADER_STATUS_DOWNLOADING_VALUE, acquisition_date, new_dir, landsat_aoi_context.maxRetries):
        #        log(write_dir, "Couldn't upsert into database with status DOWNLOADED for {}".format(new_dir), general_log_filename)
        #        continue
        #else:
        #    #impossible to happen, but for the sake of flow
        #    log(write_dir, "Unknown satellite id {} found for {}. Continue...".format(sat_id, l1), general_log_filename)
        #    continue    

        # phisically copy the l1 product
        log(write_dir, "Copying {} to {}".format(l1, new_dir), general_log_filename)
        if copy_directory(l1, new_dir):
            if sat_id == SENTINEL2_SATELLITE_ID:
                #applay angles correction if necessary
                cmd_angles_correction = ["java", "-jar", os.path.dirname(os.path.abspath(__file__)) + "/S2ProductDownloader-1.0.jar", "--input", write_dir, "--ma", "NAN", "--products", l1_basename]
                if run_command(cmd_angles_correction, write_dir, general_log_filename) != 0:
                    log(write_dir, "Couldn't apply the angles correction for {}".format(new_dir), general_log_filename)
                    continue
                print("orbit_id = {}".format(orbit_id))
                if not sentinel_db_info.upsertSentinelProductHistory(sentinel_aoi_context.siteId, l1_basename, DATABASE_DOWNLOADER_STATUS_DOWNLOADED_VALUE, acquisition_date, new_dir, int(orbit_id), sentinel_aoi_context.maxRetries):
                    log(write_dir, "Couldn't upsert into database with status DOWNLOADED for {}".format(new_dir), general_log_filename)
                    continue
            elif sat_id == LANDSAT8_SATELLITE_ID: 
                if not landsat_db_info.upsertLandsatProductHistory(landsat_aoi_context.siteId, l1_basename, DATABASE_DOWNLOADER_STATUS_DOWNLOADED_VALUE, acquisition_date, new_dir, landsat_aoi_context.maxRetries):
                    log(write_dir, "Couldn't upsert into database with status DOWNLOADED for {}".format(new_dir), general_log_filename)
                    continue
            else:
                #impossible to happen, but for the sake of flow
                log(write_dir, "Unknown satellite id {} found for {}. Continue...".format(sat_id, l1), general_log_filename)
                continue
            log(write_dir, "Database downloader_history tabe updated with status DOWNLOADED for {}".format(new_dir), general_log_filename)
                
            
