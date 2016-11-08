#!/usr/bin/env python
from __future__ import print_function
import argparse
import re
import glob
import gdal
import osr
import subprocess
import lxml.etree
from lxml.builder import E
import math
import os
from os.path import isfile, isdir, join
import glob
import sys
import time, datetime
from time import gmtime, strftime
import pipes
import shutil
import psycopg2
import psycopg2.errorcodes
import optparse
from osgeo import ogr

SENTINEL2_SATELLITE_ID = int(1)
LANDSAT8_SATELLITE_ID = int(2)
UNKNOWN_SATELLITE_ID = int(-1)
general_log_filename = "log.log"

DEBUG = 1

def log(location, info, log_filename = None):
    if log_filename == None:
        log_filename = "log.txt"
    try:
        logfile = os.path.join(location, log_filename)
        if DEBUG:
            #print("logfile: {}".format(logfile))
            print("{}:{}".format(str(datetime.datetime.now()), str(info)))
            sys.stdout.flush()
        log = open(logfile, 'a')
        log.write("{}:{}\n".format(str(datetime.datetime.now()),str(info)))
        log.close()
    except:
        print("Could NOT write inside the log file {}".format(logfile))
        sys.stdout.flush()

def GetExtent(gt, cols, rows):
    ext = []
    xarr = [0, cols]
    yarr = [0, rows]

    for px in xarr:
        for py in yarr:
            x = gt[0] + px * gt[1] + py * gt[2]
            y = gt[3] + px * gt[4] + py * gt[5]
            ext.append([x, y])
        yarr.reverse()
    return ext


def ReprojectCoords(coords, src_srs, tgt_srs):
    trans_coords = []
    transform = osr.CoordinateTransformation(src_srs, tgt_srs)
    for x, y in coords:
        x, y, z = transform.TransformPoint(x, y)
        trans_coords.append([x, y])
    return trans_coords

def get_footprint(image_filename):
    dataset = gdal.Open(image_filename, gdal.gdalconst.GA_ReadOnly)

    size_x = dataset.RasterXSize
    size_y = dataset.RasterYSize

    geo_transform = dataset.GetGeoTransform()

    spacing_x = geo_transform[1]
    spacing_y = geo_transform[5]

    extent = GetExtent(geo_transform, size_x, size_y)

    source_srs = osr.SpatialReference()
    source_srs.ImportFromWkt(dataset.GetProjection())
    epsg_code = source_srs.GetAttrValue("AUTHORITY", 1)
    target_srs = osr.SpatialReference()
    target_srs.ImportFromEPSG(4326)

    wgs84_extent = ReprojectCoords(extent, source_srs, target_srs)
    return wgs84_extent

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
       
def get_product_info(product_name):
    acquisition_date = None
    sat_id = UNKNOWN_SATELLITE_ID
    if args.processor_name == "l2a" :
        if product_name.startswith("S2"):
            m = re.match("\w+_V(\d{8}T\d{6})_\w+.SAFE", product_name)
            if m != None:
                sat_id = SENTINEL2_SATELLITE_ID
                acquisition_date = m.group(1)
        elif product_name.startswith("LC8"):
            m = re.match("LC8\d{6}(\d{7})[A-Z]{3}\d{2}", product_name)
            if m != None:
                sat_id = LANDSAT8_SATELLITE_ID
                acquisition_date = datetime.datetime.strptime("{} {}".format(m.group(1)[0:4],m.group(1)[4:]), '%Y %j').strftime("%Y%m%dT%H%M%S")
                print("Acquisition date: {}".format(acquisition_date))
    else :
        m = re.match("\w+(_A|_V)(\w+)", product_name)
        if m != None:
            acquisition_date = m.group(2)
            words = acquisition_date.split('_')
            if len(words) == 1 :
                acquisition_date = words[0]
            else :
                if len(words) == 2:
                    acquisition_date = words[1]    
                else:
                    acquisition_date = ""  
            if acquisition_date != "" :
                acquisition_date = acquisition_date + "T000000"
            
    return sat_id and (sat_id, acquisition_date)
    
def insert_product(product_dir) :
    l2a_processed_tiles = []
    wkt = []
    sat_id = 0
    acquisition_date = ""
    mosaic_img = "mosaic.jpg"
    
    if not product_dir.endswith(os.path.sep):
        product_dir += os.path.sep
    print("Output path: {}".format(product_dir))
    
    product_name = os.path.basename(product_dir[:len(product_dir) - 1]) if product_dir.endswith("/") else os.path.basename(product_dir)
    print("Product dir is: {}".format(product_name))
    wgs84_extent_list = []
    if args.processor_name == "l2a" :
        if product_name.startswith("S2"):
            satellite_id = SENTINEL2_SATELLITE_ID
        else :
            satellite_id = LANDSAT8_SATELLITE_ID    
        tiles_dir_list = (glob.glob("{}*.DBL.DIR".format(product_dir)))
        log(product_dir, "Creating common footprint for tiles: DBL.DIR List: {}".format(tiles_dir_list), general_log_filename)
        for tile_dir in tiles_dir_list:
            if satellite_id == SENTINEL2_SATELLITE_ID:
                tile_img = (glob.glob("{}/*_FRE_R1.DBL.TIF".format(tile_dir)))
            else: #satellite_id is LANDSAT8_SATELLITE_ID:
                tile_img = (glob.glob("{}/*_FRE.DBL.TIF".format(tile_dir)))                
            
            if len(tile_img) > 0:
                wgs84_extent_list.append(get_footprint(tile_img[0]))
    else :
        if product_name.startswith("S2AGRI_"):
            mosaic_files_list = (glob.glob("{}*_PVI_*.jpg".format(product_dir)))
            if len(mosaic_files_list) > 0:
                mosaic_img = os.path.basename(mosaic_files_list[0])
                print ("mosaic image is {}".format(mosaic_img))
            
            tiles_dir_list = (glob.glob("{}TILES/S2AGRI_*".format(product_dir)))
            log(product_dir, "Creating common footprint for tiles: {}".format(tiles_dir_list), general_log_filename)
            for tile_dir in tiles_dir_list:
                tile_img = (glob.glob("{}/IMG_DATA/S2AGRI_*.TIF".format(tile_dir)))
                if len(tile_img) > 0:
                    wgs84_extent_list.append(get_footprint(tile_img[0]))
            
    wkt = get_envelope(wgs84_extent_list)

    if len(wkt) == 0:
        log(product_dir, "Could not create the footprint", general_log_filename)
    else:
        sat_id, acquisition_date = get_product_info(product_name)
        if args.processor_name == "l2a" :
            if sat_id > 0 and acquisition_date != None:                    
                #check for MACCS tiles output. If none was processed, only the record from
                #product table will be updated. No l2a product will be added into product table                    
                for tile_dbl_dir in tiles_dir_list:
                    tile = None
                    print("tile_dbl_dir {}".format(tile_dbl_dir))
                    if satellite_id == SENTINEL2_SATELLITE_ID:
                        tile = re.search(r"_L2VALD_(\d\d[a-zA-Z]{3})____[\w\.]+$", tile_dbl_dir)
                    else:
                        tile = re.search(r"_L2VALD_([\d]{6})_[\w\.]+$", tile_dbl_dir)
                    if tile is not None and not tile.group(1) in l2a_processed_tiles:
                        l2a_processed_tiles.append(tile.group(1))
                log(product_dir, "Processed tiles: {}  to path: {}".format(l2a_processed_tiles, product_dir), general_log_filename)
            else:
                log(product_dir,"Could not get the acquisition date from the product name {}".format(product_dir), general_log_filename)
        else :
            for tile_dbl_dir in tiles_dir_list:
                tile = re.search("\w+_T(\w+)", tile_dbl_dir)
                if tile is not None and not tile.group(1) in l2a_processed_tiles:
                    l2a_processed_tiles.append(tile.group(1))                
                
    if len(l2a_processed_tiles) > 0:
        log(product_dir, "Insert info in product table and set state as processed in product table for product {}".format(product_dir), general_log_filename)
    else:
        log(product_dir, "Only set the state as processed in product (no l2a tiles found after maccs) for product {}".format(product_dir), general_log_filename)
          
    processor_id = l2a_db.get_processor_id(args.processor_name)      
    product_type_id = l2a_db.get_product_type_id(args.product_type)  
    site_id = l2a_db.get_site_id(args.site_name)
    if(site_id == ''):
        sys.exit('Cannot find in the database the provided site name!!!')

    l2a_db.set_processed_product(processor_id, product_type_id, site_id, l2a_processed_tiles, product_dir, os.path.basename(product_dir[:len(product_dir) - 1]), wkt, sat_id, acquisition_date, 0, mosaic_img)
    
###########################################################################
class Config(object):
    def __init__(self):
        self.host = ""
        self.database = ""
        self.user = ""
        self.password = ""
    def loadConfig(self, configFile):
        try:
            with open(configFile, 'r') as config:
                found_section = False
                for line in config:
                    line = line.strip(" \n\t\r")
                    if found_section and line.startswith('['):
                        break
                    elif found_section:
                        elements = line.split('=')
                        if len(elements) == 2:
                            if elements[0].lower() == "hostname":
                                self.host = elements[1]
                            elif elements[0].lower() == "databasename":
                                self.database = elements[1]
                            elif elements[0].lower() == "username":
                                self.user = elements[1]
                            elif elements[0].lower() == "password":
                                self.password = elements[1]
                            else:
                                print("Unkown key for [Database] section")
                        else:
                            print("Error in config file, found more than on keys, line: {}".format(line))
                    elif line == "[Database]":
                        found_section = True
        except:
            print("Error in opening the config file ".format(str(configFile)))
            return False
        if len(self.host) <= 0 or len(self.database) <= 0:
            return False
        return True
    
###########################################################################
class L2AInfo(object):
    def __init__(self, server_ip, database_name, user, password, log_file=None):
        self.server_ip = server_ip
        self.database_name = database_name
        self.user = user
        self.password = password
        self.is_connected = False;
        self.log_file = log_file

    def database_connect(self):
        if self.is_connected:
            return True
        connectString = "dbname='{}' user='{}' host='{}' password='{}'".format(self.database_name, self.user, self.server_ip, self.password)
        try:
            self.conn = psycopg2.connect(connectString)
            self.cursor = self.conn.cursor()
            self.is_connected = True
        except:
            print("Unable to connect to the database")
            exceptionType, exceptionValue, exceptionTraceback = sys.exc_info()
            # Exit the script and print an error telling what happened.
            print("Database connection failed!\n ->{}".format(exceptionValue))
            self.is_connected = False
            return False
        return True

    def database_disconnect(self):
        if self.conn:
            self.conn.close()
            self.is_connected = False

    def get_site_names(self):
        if not self.database_connect():
            return ""
        try:
            self.cursor.execute("select short_name from site")
            rows = self.cursor.fetchall()
        except:
            print("Unable to execute select short_name from site")
            self.database_disconnect()
            return ""
        self.database_disconnect()
        return [item[0] for item in rows]
        
    def get_site_id(self, short_name):
        if not self.database_connect():
            return ""
        try:
            self.cursor.execute("select id from site where short_name='{}'".format(short_name))
            rows = self.cursor.fetchall()
        except:
            print("Unable to execute select id from site")
            self.database_disconnect()
            return ""
        self.database_disconnect()
        return rows[0][0]

    def get_processor_names(self):
        if not self.database_connect():
            return ""
        try:
            self.cursor.execute("select short_name from processor")
            rows = self.cursor.fetchall()
        except:
            print("Unable to execute select short_name from processor")
            self.database_disconnect()
            return ""
        self.database_disconnect()
        return [item[0] for item in rows]
        
    def get_processor_id(self, short_name):
        if not self.database_connect():
            return ""
        try:
            self.cursor.execute("select id from processor where short_name='{}'".format(short_name))
            rows = self.cursor.fetchall()
        except:
            print("Unable to execute select id from processor")
            self.database_disconnect()
            return ""
        self.database_disconnect()
        return rows[0][0]

    def get_product_type_names(self):
        if not self.database_connect():
            return ""
        try:
            self.cursor.execute("select name from product_type")
            rows = self.cursor.fetchall()
        except:
            print("Unable to execute select name from product_type")
            self.database_disconnect()
            return ""
        self.database_disconnect()
        return [item[0] for item in rows]
        
    def get_product_type_id(self, short_name):
        if not self.database_connect():
            return ""
        try:
            self.cursor.execute("select id from product_type where name='{}'".format(short_name))
            rows = self.cursor.fetchall()
        except:
            print("Unable to execute select id from product_type")
            self.database_disconnect()
            return ""
        self.database_disconnect()
        return rows[0][0]
        
    def set_processed_product(self, processor_id, product_type_id, site_id, l2a_processed_tiles, full_path, product_name, footprint, sat_id, acquisition_date, orbit_id, mosaic_img):
        #input params:
        #product type by default is 1
        #processor id
        #site id
        #job id has to be NULL
        #full path is the whole path to the product including the name
        #created timestamp NULL
        #name product (basename from the full path)
        #quicklook image has to be NULL
        #footprint
        if not self.database_connect():
            return False
        try:
            if len(l2a_processed_tiles) > 0:
                #normally , sp_insert_product should upsert the record
                self.cursor.execute("""select * from sp_insert_product(%(product_type_id)s :: smallint,
                               %(processor_id)s :: smallint, 
                               %(satellite_id)s :: smallint, 
                               %(site_id)s :: smallint, 
                               %(job_id)s :: smallint, 
                               %(full_path)s :: character varying,
                               %(created_timestamp)s :: timestamp,
                               %(name)s :: character varying,
                               %(quicklook_image)s :: character varying,
                               %(footprint)s,
                               %(orbit_id)s :: integer,
                               %(tiles)s :: json)""",
                                {
                                    "product_type_id" : product_type_id,
                                    "processor_id" : processor_id,
                                    "satellite_id" : sat_id,
                                    "site_id" : site_id,
                                    "job_id" : None,
                                    "full_path" : full_path,
                                    "created_timestamp" : acquisition_date,
                                    "name" : product_name,
                                    "quicklook_image" : mosaic_img,
                                    "footprint" : footprint, 
                                    "orbit_id" : orbit_id,
                                    "tiles" : '[' + ', '.join(['"' + t + '"' for t in l2a_processed_tiles]) + ']' 
                                })
                self.conn.commit()
        except Exception, e:
            print("Database update query failed: {}".format(e))            
            self.database_disconnect()
            return False
        self.database_disconnect()
        return True

        
parser = argparse.ArgumentParser(
    description="Script for inserting products into the database")
parser.add_argument('-d', '--dir', help="The directory of the product")
parser.add_argument('-c', '--config', default="/etc/sen2agri/sen2agri.conf", help="configuration file")
parser.add_argument('-p', '--processor_name', help="The processor short name of the product")
parser.add_argument('-t', '--product_type', help="The product type")
parser.add_argument('-s', '--site_name', help="The site name for the product")
parser.add_argument('-m', '--multi', required=False, type=bool, default=False, help="If false, dir is considered product folder otherwise it is considered a cotainer of products")

args = parser.parse_args()

config = Config()
if not config.loadConfig(args.config):
    log(general_log_path, "Could not load the config from configuration file", general_log_filename)
    sys.exit(-1)
      
l2a_db = L2AInfo(config.host, config.database, config.user, config.password)  

if (not args.dir):
    sys.exit("Please provide the product directory using -d or --dir")
if (not args.processor_name):    
    sys.exit("Please provide the processor name using -p or --processor_name. Available options: {}".format(l2a_db.get_processor_names()))
if (not args.product_type):    
    sys.exit("Please provide the product type using -t or --product_type. Available options: {}".format(l2a_db.get_product_type_names()))
if (not args.site_name):        
    sys.exit("Please provide the site short name using -s or --site_name. Available options: {}".format(l2a_db.get_site_names()))

root_dir = os.path.abspath(args.dir)
if not root_dir.endswith(os.path.sep):
    root_dir += os.path.sep
#print("Provided dir {} (using {})".format(args.dir, root_dir))
    
if args.multi :
    for name in os.listdir(root_dir):
        subdir_path = os.path.join(root_dir, name)
        if os.path.isdir(subdir_path) :
            print("Inserting multi product: {}".format(str(subdir_path)))
            insert_product(subdir_path)
else :
    print("Inserting single product: {}".format(root_dir))
    insert_product(root_dir)
        
        
