#!/usr/bin/env python
from __future__ import print_function

import argparse
from collections import defaultdict
from datetime import date
from datetime import datetime
from datetime import timedelta
from glob import glob
import multiprocessing.dummy
import os
import os.path
from osgeo import osr
from osgeo import gdal
from gdal import gdalconst
import pipes
import psycopg2
from psycopg2.sql import SQL, Literal, Identifier
import psycopg2.extras
import subprocess
import re
import sys
import csv
from shutil import copyfile


try:
    from configparser import ConfigParser
except ImportError:
    from ConfigParser import ConfigParser

class Config(object):
    def __init__(self, args):
        parser = ConfigParser()
        parser.read([args.config_file])

        self.host = parser.get("Database", "HostName")
        self.port = int(parser.get("Database", "Port", vars={"Port": "5432"}))
        self.dbname = parser.get("Database", "DatabaseName")
        self.user = parser.get("Database", "UserName")
        self.password = parser.get("Database", "Password")

        self.site_short_name = args.site_short_name
        self.year = args.year
        if args.mowing_start_date:
            self.mowing_start_date = args.mowing_start_date

def run_command(args, env=None):
    args = list(map(str, args))
    cmd_line = " ".join(map(pipes.quote, args))
    print(cmd_line)
    subprocess.call(args, env=env)


def getSiteId(conn, siteShortName):
    site_id = -1
    with conn.cursor() as cursor:
        query = SQL(
            """
            select id from site
            where short_name = {}
            """
        )
        query = query.format(Literal(siteShortName))
        print(query.as_string(conn))

        cursor.execute(query)
        for row in cursor:
            site_id = row[0]
        conn.commit()
    return site_id
  
def setConfigValue(conn, site_id, key, value):
    with conn.cursor() as cursor:
        id = -1
        if not site_id :
            query = SQL(""" select id from config where key = {} and site_id is null""").format(Literal(key), Literal(site_id))
        else :
            query = SQL(""" select id from config where key = {} and site_id = {}""").format(Literal(key), Literal(site_id))
        print(query.as_string(conn))
        cursor.execute(query)
        for row in cursor:
            id = row[0]
        conn.commit()
        
        if id == -1 :
            if not site_id :
                query = SQL(""" insert into config (key, value) values ({}, {}) """).format(Literal(key), Literal(value))
            else :
                query = SQL(""" insert into config (key, site_id, value) values ({}, {}, {}) """).format(Literal(key), Literal(site_id), Literal(value))
            print(query.as_string(conn))
            cursor.execute(query)
            conn.commit() 
        else :
            if not site_id :
                query = SQL(""" update config set value = {} where key = {} and site_id is null """).format(Literal(value), Literal(key), Literal(site_id))
            else:
                query = SQL(""" update config set value = {} where key = {} and site_id = {} """).format(Literal(value), Literal(key), Literal(site_id))
            print(query.as_string(conn))
            cursor.execute(query)
            conn.commit() 

        if not site_id :
            query = SQL(""" select value from config where key = {} and site_id is null""").format(Literal(key), Literal(site_id))
        else :
            query = SQL(""" select value from config where key = {} and site_id = {}""").format(Literal(key), Literal(site_id))
        print(query.as_string(conn))
        cursor.execute(query)
        read_value = ''
        for row in cursor:
            read_value = row[0]
        conn.commit()
        
        print ("========")
        if str(value) == str(read_value) : 
            print ("Key {} succesfuly updated for site id {} with value {}".format(key, site_id, value))
        else :
            print ("Error updating key {} for site id {} with value {}. The read value was: {}".format(key, site_id, value, read_value))
        print ("========")

def getSiteConfigKey(conn, key, site_id):
    value = ''
    with conn.cursor() as cursor:
        query = SQL(
            """ select value from config where key = {} and site_id = {} """
        )
        query = query.format(Literal(key), Literal(site_id))
        print(query.as_string(conn))

        cursor.execute(query)
        for row in cursor:
            value = row[0]
        conn.commit()
        
        # get the default value if not found
        if value == '' :
            query = SQL(
                """ select value from config where key = {} and site_id is null """
            )
            query = query.format(Literal(key))
            print(query.as_string(conn))

            cursor.execute(query)
            for row in cursor:
                value = row[0]
            conn.commit()
            
    return value

def copyConfigFile(config, conn, site_id, config_file) :
    # target file example. S4C_L4B_Config_2019.cfg
    targetFileName = "S4C_L4B_Config_" + str(config.year) + ".cfg"
    cfgDir = getSiteConfigKey(conn, 'processor.s4c_l4b.cfg_dir', site_id)
    if cfgDir == '' :
        print("The processor.s4c_l4b.cfg_dir key is not configured in the database. Creating it ...")
        setConfigValue(conn, None, 'processor.s4c_l4b.cfg_dir', '/mnt/archive/grassland_mowing/{site}/{year}/config/')
        cfgDir = '/mnt/archive/grassland_mowing/{site}/{year}/config/'
    
    cfgDir = cfgDir.replace("{site}", config.site_short_name)
    cfgDir = cfgDir.replace("{year}", str(config.year))
    if not os.path.exists(cfgDir):
        os.makedirs(cfgDir)    
    destFilePath = os.path.join(cfgDir, targetFileName)
    print("Copying the L4B config file from {} to {}".format(config_file, destFilePath))
    copyfile(config_file, destFilePath)
    
    cfgYear = getSiteConfigKey(conn, 'processor.s4c_l4b.year', site_id)
    if cfgYear == '' :
        # add the general key
        print("The processor.s4c_l4b.year key is not configured in the database. Creating it ...")
        setConfigValue(conn, None, 'processor.s4c_l4b.year', '')
    # set also the value specific for the site
    setConfigValue(conn, site_id, 'processor.s4c_l4b.year', config.year)

def main():
    parser = argparse.ArgumentParser(description="Handles the upload of the grassland mowing config file")
    parser.add_argument('-c', '--config-file', default='/etc/sen2agri/sen2agri.conf', help="Configuration file location")
    parser.add_argument('-s', '--site-short-name', required=True, help="Site short name for which the file was uploaded")
    parser.add_argument('-y', '--year', type=int, required=True, help="The year")
    parser.add_argument('-i', '--input-file', required=True, help="The uploaded config file")
    parser.add_argument('-d', '--mowing-start-date', required=False, help="The mowing start date")
    args = parser.parse_args()

    config = Config(args)
    
    with psycopg2.connect(host=config.host, dbname=config.dbname, user=config.user, password=config.password) as conn:
        site_id = getSiteId(conn, config.site_short_name)
        copyConfigFile(config, conn, site_id, args.input_file)
        if hasattr(config, 'mowing_start_date') and config.mowing_start_date:
            setConfigValue(conn, site_id, 'processor.s4c_l4b.start_date', config.mowing_start_date)
        
if __name__ == "__main__":
    main()
