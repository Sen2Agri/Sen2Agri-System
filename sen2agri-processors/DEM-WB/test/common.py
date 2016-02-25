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
#import logging
import math
import os
from os.path import isfile, isdir, join
import glob
import sys
import time, datetime
import pipes
import shutil
import psycopg2
import psycopg2.errorcodes

FAKE_COMMAND = 0

DATABASE_DEMMACCS_GIPS_PATH = "demmaccs.gips-path"
DATABASE_DEMMACCS_OUTPUT_PATH = "demmaccs.output-path"
DATABASE_DEMMACCS_SRTM_PATH = "demmaccs.srtm-path"
DATABASE_DEMMACCS_SWBD_PATH = "demmaccs.swbd-path"
DATABASE_DEMMACCS_MACCS_IP_ADDRESS = "demmaccs.maccs-ip-address"
DATABASE_DEMMACCS_MACCS_LAUNCHER = "demmaccs.maccs-launcher"
DATABASE_DEMMACCS_LAUNCHER = "demmaccs.launcher"
DATABASE_DEMMACCS_WORKING_DIR = "demmacs.working-path"
def run_command(cmd_array, use_shell=False):
    start = time.time()
    print(" ".join(map(pipes.quote, cmd_array)))    
    res = 0
    if not FAKE_COMMAND:
        res = subprocess.call(cmd_array, shell=use_shell)
    print("App finished in: {}".format(datetime.timedelta(seconds=(time.time() - start))))
    if res != 0:
        print("Application error")        
    return res


def create_recursive_dirs(dir_name):
    try:
        #create recursive dir
        os.makedirs(dir_name)
    except:
        pass
    #check if it already exists.... otherwise the makedirs function will raise an exception 
    if os.path.exists(dir_name):
        if not os.path.isdir(dir_name):
            print("Can't create the directory because there is a file with the same name: {}".format(dir_name))
            print("Remove: {}".format(dir_name))
            return False
    else:
        #for sure, the problem is with access rights
        print("Can't create the directory due to access rights {}".format(dir_name))
        return False
    return True


class Config(object):
    def __init__(self, section):
        self.section="[{}]".format(section)
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
                            if elements[0].lower() == "host":
                                self.host = elements[1]
                            elif elements[0].lower() == "database" or elements[0].lower() == "db":
                                self.database = elements[1]
                            elif elements[0].lower() == "user":                            
                                self.user = elements[1]
                            elif elements[0].lower() == "pass" or elements[0].lower() == "password":                            
                                self.password = elements[1]
                            else:
                                print("Unkown key for {} section".format(self.section))
                        else:
                            print("Error in config file, found more than on keys, line: {}".format(line))
                    elif line == self.section:
                        found_section = True
        except:
            print("Error in opening the config file ".format(str(configFile)))
            return False
        if len(self.host) <= 0 or len(self.database) <= 0:
            return False
        return True

class DEMMACSConfig(object):
    def __init__(self, output_path, gips_path, srtm_path, swbd_path, maccs_ip_address, maccs_launcher, launcher, working_dir):
        self.output_path = output_path
        self.gips_path = gips_path
        self.srtm_path = srtm_path
        self.swbd_path = swbd_path
        self.maccs_ip_address = maccs_ip_address
        self.maccs_launcher = maccs_launcher
        self.launcher = launcher
        self.working_dir = working_dir

class L1CInfo(object):
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
        print("connectString:={}".format(connectString))
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

    def get_demmacs_config(self):
        if not self.database_connect():
            return None
        try:
            self.cursor.execute("select * from sp_get_parameters('demmaccs')")
            rows = self.cursor.fetchall()
        except:
            self.database_disconnect()
            return None        
        output_path = ""
        gips_path = ""
        srtm_path = ""
        swbd_path = ""
        maccs_ip_address = ""
        maccs_launcher = ""
        launcher = ""
        working_dir = ""

        for row in rows:
            if len(row) != 3:
                continue            
            if row[0] == DATABASE_DEMMACCS_OUTPUT_PATH:
                output_path = row[2]
            if row[0] == DATABASE_DEMMACCS_GIPS_PATH:
                gips_path = row[2]
            elif row[0] == DATABASE_DEMMACCS_SRTM_PATH:
                srtm_path = row[2]
            elif row[0] == DATABASE_DEMMACCS_SWBD_PATH:
                swbd_path = row[2]
            elif row[0] == DATABASE_DEMMACCS_MACCS_IP_ADDRESS:
                maccs_ip_address = row[2]
            elif row[0] == DATABASE_DEMMACCS_MACCS_LAUNCHER:
                maccs_launcher = row[2]
            elif row[0] == DATABASE_DEMMACCS_LAUNCHER:
                launcher = row[2]
            elif row[0] == DATABASE_DEMMACCS_WORKING_DIR:
                working_dir = row[2]
            
        self.database_disconnect()
        if len(output_path) == 0 or len(gips_path) == 0 or len(srtm_path) == 0 or len(swbd_path) == 0 or len(maccs_ip_address) == 0 or len(maccs_launcher) == 0 or len(launcher) == 0 or len(working_dir) == 0:
            return None
        return DEMMACSConfig(output_path, gips_path, srtm_path, swbd_path, maccs_ip_address, maccs_launcher, launcher, working_dir)
        
    def get_short_name(self, table, use_id):
        if not self.database_connect():
            return ""
        if table != "site" and table != "processor":
            return ""
        try:
            self.cursor.execute("select short_name from {} where id={}".format(table, use_id))
            rows = self.cursor.fetchall()
        except:
            self.database_disconnect()
            return ""
        self.database_disconnect()
        print("rows[0][0] = {}".format(rows[0][0]))
        return rows[0][0]

    def get_unprocessed_l1c(self):
        if not self.database_connect():
            return []
        try:
            self.cursor.execute("select id, site_id, full_path from downloader_history where processed=false")
            rows = self.cursor.fetchall()
        except:
            self.database_disconnect()
            return []
        retArray = []
        for row in rows:
            retArray.append(row)
        self.database_disconnect()
        return retArray

    def mark_as_processed(self, l1c_list_ids):
        if not self.database_connect():
            return False
        if len(l1c_list_ids) == 0:
            return True
        try:
            condition = "id={}".format(l1c_list_ids[0])
            for l1c_id in l1c_list_ids[1:]:
                conditon += " or id={}".format(l1c_id)
            print("condition={}".format(condition))
            self.cursor.execute("update downloader_history set processed=true where {}".format(condition))
            self.conn.commit()
        except:
            print("Database update query FAILED!!!!!")
            self.database_disconnect()
            return False
        self.database_disconnect()
        return True
