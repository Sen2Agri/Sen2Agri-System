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

def enable_site_download(siteName, satId, enableDownload, tilesList):
    l2a_db.update_site_tiles(siteName, satId, enableDownload, tilesList)

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

    def update_site_tiles(self, siteId, satId, enableDownload, tilesList):
        if not self.database_connect():
            return False
        try:
            # first delete any existing line (actually, enable the download)
            self.cursor.execute("""delete from site_tiles where 
                                   (site_id = %(site_id)s :: smallint and 
                                   satellite_id = %(satellite_id)s :: integer)""",
                            {
                                "site_id" : siteId,
                                "satellite_id" : satId
                            })
            if (enableDownload and len(tilesList) > 0) or (not enableDownload):
                try :
                    # download already enabled, perform filtering or disable if needed
                    self.cursor.execute("""insert into site_tiles (site_id, satellite_id, tiles) values (%(site_id)s :: smallint,
                               %(satellite_id)s :: integer,
                               %(tiles)s :: text[])""",
                                {
                                    "site_id" : siteId,
                                    "satellite_id" : satId,                                        
                                    "tiles" : '{' + ', '.join(['"' + t + '"' for t in tilesList]) + '}'
                                })
                except Exception, e:
                    self.database_disconnect()
                    if not self.database_connect():
                        return False
                    try :
                        # download already enabled, perform filtering or disable if needed
                        self.cursor.execute("""insert into site_tiles (site_id, satellite_id, tiles, id) values (%(site_id)s :: smallint,
                               %(satellite_id)s :: integer,
                               %(tiles)s :: text[],
                               %(id)s :: integer)""",
                                {
                                    "site_id" : siteId,
                                    "satellite_id" : satId,                                        
                                    "tiles" : '{' + ', '.join(['"' + t + '"' for t in tilesList]) + '}',
                                    "id" : 0
                                })
                    except Exception, e:
                        print("Database insert query failed: {}".format(e))
                        self.database_disconnect()
                        return False
            self.conn.commit()
        except Exception, e:
            print("Database update query failed: {}".format(e))
            self.database_disconnect()
            return False
        self.database_disconnect()
        return True

    def executeInsert(siteId, satId, tilesList):
        try :
                # download already enabled, perform filtering or disable if needed
                self.cursor.execute("""insert into site_tiles (site_id, satellite_id, tiles) values (%(site_id)s :: smallint,
                               %(satellite_id)s :: integer,
                               %(tiles)s :: text[])""",
                                {
                                    "site_id" : siteId,
                                    "satellite_id" : satId,                                        
                                    "tiles" : '{' + ', '.join(['"' + t + '"' for t in tilesList]) + '}'
                                })
        except Exception, e:
             try :
                 # download already enabled, perform filtering or disable if needed
                 self.cursor.execute("""insert into site_tiles (site_id, satellite_id, tiles) values (%(site_id)s :: smallint,
                               %(satellite_id)s :: integer,
                               %(tiles)s :: text[])""",
                                {
                                    "site_id" : siteId,
                                    "id" : 0,
                                    "satellite_id" : satId,                                        
                                    "tiles" : '{' + ', '.join(['"' + t + '"' for t in tilesList]) + '}'
                                })
             except Exception, e:
                 print("Database update query failed: {}".format(e))
                 self.database_disconnect()
                 return False
	
parser = argparse.ArgumentParser(
    description="Script for filtering the downloaded tiles for a site")
parser.add_argument('-c', '--config', default="/etc/sen2agri/sen2agri.conf", help="configuration file")
parser.add_argument('-t', '--sat_id', help="The processor satellite id for which the enable/disable is performed (S2 or L8)")
parser.add_argument('-s', '--site_name', help="The site name for which the enable/disable is performed")
parser.add_argument('-e', '--enable', help="If false, the download for the specified products will be disabled for this site")
parser.add_argument('-l', '--tiles', help="List of tiles comma separated")

args = parser.parse_args()

config = Config()
if not config.loadConfig(args.config):
    log(general_log_path, "Could not load the config from configuration file", general_log_filename)
    sys.exit(-1)

l2a_db = L2AInfo(config.host, config.database, config.user, config.password)

if (not args.sat_id):
    sys.exit("Please provide the satellite id using -t or --sat_id. Available options: 1 = S2, 2 = L8")

satId = int(args.sat_id)
if (satId != 1 and satId != 2) :
    sys.exit("Please provide a valid value for sattelite id. Available options: 1 = S2, 2 = L8")

if (not args.site_name):
    sys.exit("Please provide the site short name using -s or --site_name. Available options: {}".format(l2a_db.get_site_names()))
    
siteNames = l2a_db.get_site_names()
siteId = l2a_db.get_site_id(args.site_name)
if (siteId == '') or (args.site_name not in siteNames) :
    sys.exit("The site with name {} does not exists. Available options: {}".format(args.site_name, siteNames))

if (not args.enable):
    sys.exit("Please provide the enable flag using -e or --enable. Available options: (True/true/False/false)")

enableDownload = False
enableStr = str(args.enable).lower()
if (args.enable == "true") or (args.enable == "yes") or (args.enable == "y") or (args.enable == "t") or (args.enable == "1"):
    enableDownload = True

tilesList = []
if (args.tiles) :
    if (not enableDownload):
        print("The tiles option is applicable only with enable flag to True! It will be ignored!")
    else :
        tilesList = args.tiles.split(', ')
        tilesList = [x.upper() for x in tilesList]
        for tile in tilesList:
            if (satId == 1) and (len(tile) != 5) :
                sys.exit("Invalid tile id {} for S2 satellite id (1). Its length should be 5".format(tile))
            if (satId == 2) and (len(tile) != 6) :
                sys.exit("Invalid tile id {} for L8 satellite id (2). Its length should be 6".format(tile))

enable_site_download(siteId, satId, enableDownload, tilesList)
