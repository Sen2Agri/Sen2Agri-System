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

def get_bool_value(value):
    boolStr = str(value).lower()
    if (boolStr == "true") or (boolStr == "yes") or (boolStr == "y") or (boolStr == "t") or (boolStr == "1"):
        return True
    return False
        
def delete_product_files(location):
    print("Deleting products from location {} ...".format(location))
    
def get_products_folder(configKey, siteName, processor) :
    configVal = l2a_db.get_config_key(configKey)
    if configVal == "":
        return ""
    configVal = configVal.replace("{site}", siteName)
    configVal = configVal.replace("{processor}", processor)
    
    return configVal
        
def delete_dwn_product_files(siteName):    
    s2PrdFolder = get_products_folder("downloader.s2.write-dir", "", "")
    if s2PrdFolder != "" :
        s2PrdFolder += "/{}".format(siteName)
        delete_product_files(s2PrdFolder)
        
    l8PrdFolder = get_products_folder("downloader.l8.write-dir", "", "")
    if l8PrdFolder != "" :
        l8PrdFolder += "/{}".format(siteName)
        delete_product_files(l8PrdFolder)

def delete_product_files_by_type(configKey, siteName, processor):
    prdFolder = get_products_folder(configKey, siteName, processor)
    if prdFolder != "" :
        delete_product_files(prdFolder)
        
def delete_site(siteId, siteName, keepDwn, keepL2A, keepL3A, keepL3B, keepL4A, keepL4B) :
    # cleanup the database
    l2a_db.delete_site_from_db(siteId)
    
    # now remove the files from disk if required
    if (not keepDwn) :
        delete_dwn_product_files(siteName)
        
    if (not keepL2A) :
        delete_product_files_by_type("demmaccs.output-path", siteName, "l2a")
        
    if (not keepL3A) :
        delete_product_files_by_type("archiver.archive_path", siteName, "l3a")
        
    if (not keepL3B) :
        delete_product_files_by_type("archiver.archive_path", siteName, "l3b_lai")
        delete_product_files_by_type("archiver.archive_path", siteName, "l3e_pheno")
        
    if (not keepL4A) :
        delete_product_files_by_type("archiver.archive_path", siteName, "l4a")
        
    if (not keepL4B) :
        delete_product_files_by_type("archiver.archive_path", siteName, "l4b")
    
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

    def get_config_key(self, key):
        if not self.database_connect():
            return ""
        cmd = "select value from config where key = '{}'".format(key)
        try:
            self.cursor.execute(cmd)
            rows = self.cursor.fetchall()
        except:
            print("Unable to execute command: {}".format(cmd))
            self.database_disconnect()
            return ""
        self.database_disconnect()
        return rows[0][0]
            
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

    def executeSqlDeleteCmd(self, cmd, disconnectOnExit=False):
        if not self.database_connect():
            return False
        try:
            print("Executing SQL command: {}".format(cmd))
            #self.cursor.execute(cmd)
            #self.conn.commit()
        except Exception, e:
            print("Database update query failed: {}".format(e))
            self.database_disconnect()
            return False
        
        if disconnectOnExit :
            print("All done OK. Disconnecting from database.")
            self.database_disconnect()
        
        return True
    
    def delete_site_from_db(self, siteId):
        # deleting from site, site tiles and config
        self.executeSqlDeleteCmd("delete from site where (id = {})".format(siteId))
        self.executeSqlDeleteCmd("delete from site_tiles where (site_id = {})".format(siteId))
        self.executeSqlDeleteCmd("delete from config where (site_id = {})".format(siteId))

        # delete the steps for this site
        self.executeSqlDeleteCmd("delete from step where task_id in (select id from task where job_id in (select id from job where site_id = {}))".format(siteId))
        
        # delete the steps ressource logs for tasks in jobs for this site
        self.executeSqlDeleteCmd("delete from step_resource_log where task_id in (select id from task where job_id in (select id from job where site_id = {}))".format(siteId))
        
        # delete the tasks for this site
        self.executeSqlDeleteCmd("delete from task where job_id in (select id from job where site_id = {})".format(siteId))
        
        # delete the config job the entries for the jobs of this site
        self.executeSqlDeleteCmd("delete from config_job where job_id in (select id from job where site_id = {})".format(siteId))
        
        # now finally remove the jobs
        self.executeSqlDeleteCmd("delete from job where (site_id = {})".format(siteId))
        
        # First get the scheduled job ids and remove them also from the scheduled_task_status
        self.executeSqlDeleteCmd("delete from scheduled_task_status where task_id in (select id from scheduled_task where site_id = {})".format(siteId))
        self.executeSqlDeleteCmd("delete from scheduled_task where (site_id = {})".format(siteId))
        
        # now delete the L1C and other products
        self.executeSqlDeleteCmd("delete from downloader_history where (site_id = {})".format(siteId))
        self.executeSqlDeleteCmd("delete from product where (site_id = {})".format(siteId))
        
        #deleting also from season table (supported only from version 1.6)
        self.executeSqlDeleteCmd("delete from season where (site_id = {})".format(siteId), True)

        return True
    
parser = argparse.ArgumentParser(
    description="Script for filtering the downloaded tiles for a site")
parser.add_argument('-c', '--config', default="/etc/sen2agri/sen2agri.conf", help="configuration file")
parser.add_argument('-s', '--site_name', help="The site name for which the enable/disable is performed")
parser.add_argument('-d', '--keep_dwn', required=False, default="True", help="If false, the downloaded L1C products will be removed")
parser.add_argument('-e', '--keep_l2a', required=False, default="True", help="If false, the created L2A products will be removed")
parser.add_argument('-a', '--keep_l3a', required=False, default="True", help="If false, the created L3A products will be removed")
parser.add_argument('-l', '--keep_lai', required=False, default="True", help="If false, the created LAI products will be removed")
parser.add_argument('-m', '--keep_crop_mask', required=False, default="True", help="If false, the created CropMask will be removed")
parser.add_argument('-t', '--keep_crop_type', required=False, default="True", help="If false, the created CropType will be removed")

args = parser.parse_args()

config = Config()
if not config.loadConfig(args.config):
    log(general_log_path, "Could not load the config from configuration file", general_log_filename)
    sys.exit(-1)

l2a_db = L2AInfo(config.host, config.database, config.user, config.password)

if (not args.site_name):
    sys.exit("Please provide the site short name using -s or --site_name. Available options: {}".format(l2a_db.get_site_names()))
    
siteNames = l2a_db.get_site_names()
siteId = l2a_db.get_site_id(args.site_name)
if (siteId == '') or (args.site_name not in siteNames) :
    sys.exit("The site with name {} does not exists. Available options: {}".format(args.site_name, siteNames))

keepDwn = get_bool_value(args.keep_dwn)
keepL2A = get_bool_value(args.keep_l2a)
keepL3A = get_bool_value(args.keep_l3a)
keepL3B = get_bool_value(args.keep_lai)
keepL4A = get_bool_value(args.keep_crop_mask)
keepL4B = get_bool_value(args.keep_crop_type)
    
delete_site(siteId, args.site_name, keepDwn, keepL2A, keepL3A, keepL3B, keepL4A, keepL4B)
