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
        self.country = args.country
        self.ogr2ogr_path = args.ogr2ogr_path

def get_export_table_command(ogr2ogr_path, destination, source, *options):
    command = []
    command += [ogr2ogr_path]
    command += [destination, source]
    command += options
    return command
    
def get_srid(conn, lpis_table):
    with conn.cursor() as cursor:
        query = SQL("select Find_SRID('public', {}, 'wkb_geometry')")
        query = query.format(Literal(lpis_table))
        print(query.as_string(conn))

        cursor.execute(query)
        srid = cursor.fetchone()[0]
        conn.commit()
        return srid
   
def run_command(args, env=None):
    args = list(map(str, args))
    cmd_line = " ".join(map(pipes.quote, args))
    print(cmd_line)
    subprocess.call(args, env=env)

def exportPracticesFile(config, conn, pg_path, practice, site_id, out_file):
    lpis_table = "decl_{}_{}".format(config.site_short_name, config.year)
    lut_table = "lut_{}_{}".format(config.site_short_name, config.year)
    
    srid = get_srid(conn, lpis_table)
    
    with conn.cursor() as cursor:
        query = SQL(
            """
                select
                lpis."NewID" as "SEQ_ID",
                lpis.ori_id as "FIELD_ID",
                ap.country as "COUNTRY",
                ap.year as "YEAR",
                ap.main_crop as "MAIN_CROP",
                ap.veg_start as "VEG_START",
                ap.h_start as "H_START",
                ap.h_end as "H_END",
                ap.practice as "PRACTICE",
                ap.p_type as "P_TYPE",
                ap.p_start as "P_START",
                ap.p_end as "P_END",
                lpis."GeomValid",
                lpis."Duplic",
                lpis."Overlap",
                lpis."Area_meters" as "Area_meter",
                lpis."ShapeInd",
                lut.lc as "LC",
                lut.ctnum as "CTnum",
                lut.ct as "CT",
                lpis."S1Pix",
                lpis."S2Pix"
            from l4c_practices ap
            inner join {} lpis on lpis.ori_id = ap.orig_id
            natural join {} lut
            where ap.site_id = {} and ap.year = {} and ap.practice_short_name = {}
            and not lpis.is_deleted order by lpis."NewID" """
        )
        query = query.format(Identifier(lpis_table), Identifier(lut_table), Literal(site_id), Literal(config.year), Literal(practice))
        query_str = query.as_string(conn)
        print(query_str)

# TODO: change natural join with inner join lut_nld_2019_2019 lut using (ori_crop) or something like this

        name = os.path.basename(out_file)
        table_name = os.path.splitext(name)[0].lower()
        command = get_export_table_command(config.ogr2ogr_path, out_file, pg_path, "-nln", table_name, "-sql", query_str, "-a_srs", "EPSG:" + str(srid), "-gt", 100000)
        run_command(command)

def exportFilterIdsFile(config, conn, pg_path, site_id, out_file) :
    lpis_table = "decl_{}_{}".format(config.site_short_name, config.year)
    
    srid = get_srid(conn, lpis_table)
    
    with conn.cursor() as cursor:
        query = SQL(
            """
            select
                lpis."NewID" as "SEQ_ID"
            from l4c_practices ap
            inner join {} lpis on lpis.ori_id = ap.orig_id
            where ap.site_id = {} and ap.year = {}
            and not lpis.is_deleted order by lpis."NewID" """
        )
        query = query.format(Identifier(lpis_table), Literal(site_id), Literal(config.year))
        query_str = query.as_string(conn)
        print(query_str)

        name = os.path.basename(out_file)
        table_name = os.path.splitext(name)[0].lower()
        command = get_export_table_command(config.ogr2ogr_path, out_file, pg_path, "-nln", table_name, "-sql", query_str, "-a_srs", "EPSG:" + str(srid), "-gt", 100000)
        run_command(command)

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

def getSiteConfigKey(conn, key, site_id):
    value = ''
    with conn.cursor() as cursor:
        query = SQL(
            """
            select value from config
            where key = {} and site_id = {}
            """
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

def getCountry(conn, site_id):
    return getSiteConfigKey(conn, 'processor.s4c_l4c.country', site_id)

def getFilterIdsFilePath( config,conn, site_id):
    filter_ids_file = getSiteConfigKey(conn, 'processor.s4c_l4c.filter_ids_path', site_id)
    filter_ids_file = filter_ids_file.replace("{site}", config.site_short_name)
    filter_ids_file = filter_ids_file.replace("{year}", str(config.year))
    if not os.path.exists(os.path.dirname(filter_ids_file)):
        os.makedirs(os.path.dirname(filter_ids_file)) 
    return filter_ids_file

def getConfiguredPractices(conn, site_id):
    practicesVal = getSiteConfigKey(conn, 'processor.s4c_l4c.practices', site_id)
    return [x.strip() for x in practicesVal.split(',')]

def getPracticeOutFile(config, conn, site_id, practice):
    practicesDir = getSiteConfigKey(conn, 'processor.s4c_l4c.ts_input_tables_dir', site_id)
    # the key might be something like "/mnt/archive/agric_practices_files/{site}/ts_input_tables/{practice}/"
    practicesDir = practicesDir.replace("{site}", config.site_short_name)
    practicesDir = practicesDir.replace("{practice}", practice)
    practicesDir = practicesDir.replace("{year}", str(config.year))
    if not os.path.exists(practicesDir):
        os.makedirs(practicesDir) 

    practiceFileName = "Sen4CAP_L4C_" + practice + "_" + config.country + "_" + str(config.year) + ".csv"
    return os.path.join(practicesDir, practiceFileName)

def hasPracticeImported(conn, site_id, practice, year):
    count_parcels = 0
    with conn.cursor() as cursor:
        query = SQL(
            """
            select count(*) from l4c_practices
            where site_id = {} and practice_short_name = {} and year = {}
            """
        )
        query = query.format(Literal(site_id), Literal(practice), Literal(year))
        print(query.as_string(conn))

        cursor.execute(query)
        for row in cursor:
            count_parcels = row[0]
        conn.commit()
    return count_parcels != 0

def getLpisProduct(conn, site_id):
    lpis_prod = ''
    with conn.cursor() as cursor:
        query = SQL(
            """
            select full_path from product
            where product_type_id in (select id from product_type where name = 'lpis') and site_id = {}
            """
        )
        query = query.format(Literal(site_id))
        print(query.as_string(conn))

        cursor.execute(query)
        for row in cursor:
            lpis_prod = row[0]
        conn.commit()
    return lpis_prod

    
def checkTableExists(conn, tablename):
    cursor = conn.cursor()
    query = SQL(
            """
            SELECT COUNT(*)
            FROM information_schema.tables
            WHERE table_name = {}
            """
        )
    query = query.format(Literal(tablename))
    print(query.as_string(conn))
    
    cursor.execute(query)
    if cursor.fetchone()[0] == 1:
        cursor.close()
        return True

    cursor.close()
    return False

def checkLpisAndLutExists(config, conn, site_id) :
    lpis_table = "decl_{}_{}".format(config.site_short_name, config.year)
    lut_table = "lut_{}_{}".format(config.site_short_name, config.year)

    if (getLpisProduct(conn, site_id) == '') or (not checkTableExists(conn, lpis_table)) or (not checkTableExists(conn, lut_table)):
        return False
        
    return True

    
def generatePracticesFiles(config, conn, pg_path, site_id) :
    if not checkLpisAndLutExists(config, conn, site_id) : 
        # No LPIS product available yet
        print("Cannot extract practices. The L4C practices files will not be generated as no LPIS/GSAA product yet!")
        return
    practices = getConfiguredPractices(conn, site_id)
    if len(practices) == 0 : 
        # No configuration file available yet, practices cannot be extracted
        print("Cannot extract practices. The configuration might not have been uploaded!")
        return
    for practice in practices:
        # Check if we have the practice imported into the database
        if not hasPracticeImported(conn, site_id, practice, config.year) :
            print("Cannot extract practices. Not all practices were imported. Missing practice is {}".format(practice))
            return 
    # Now iterate again through all the practices and export the files
    for practice in practices:
        # Generate the file - build the file path
        practiceFilePath = getPracticeOutFile(config, conn, site_id, practice)
        # export the file
        exportPracticesFile(config, conn, pg_path, practice, site_id, practiceFilePath)
        
    # Generate also the filter ids file
    filterIdsPath = getFilterIdsFilePath(config, conn, site_id)
    
    print ("Filter ids path is : ".format(filterIdsPath))
    exportFilterIdsFile(config, conn, pg_path, site_id, filterIdsPath)

def main():
    parser = argparse.ArgumentParser(description="Handles the change of an LPIS, LUT or practices import")
    parser.add_argument('-c', '--config-file', default='/etc/sen2agri/sen2agri.conf', help="Configuration file location")
    parser.add_argument('-s', '--site-short-name', help="Site short name for which the file was uploaded")
    parser.add_argument('-y', '--year', type=int, help="The year")
    parser.add_argument('-t', '--country', help="The country short name")
    parser.add_argument('-g', '--ogr2ogr-path', default='ogr2ogr', help="The path to ogr2ogr")
    args = parser.parse_args()

    config = Config(args)
    
    pg_path = 'PG:dbname={} host={} port={} user={} password={}'.format(config.dbname, config.host,
                                                                        config.port, config.user, config.password)

    with psycopg2.connect(host=config.host, dbname=config.dbname, user=config.user, password=config.password) as conn:
        site_id = getSiteId(conn, config.site_short_name)
        if not args.country :
            config.country = getCountry(conn, site_id)
        
        # generate the practices files
        generatePracticesFiles(config, conn, pg_path, site_id)

        
if __name__ == "__main__":
    main()
