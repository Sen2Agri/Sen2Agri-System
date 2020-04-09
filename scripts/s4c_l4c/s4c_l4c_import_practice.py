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
        if args.country : 
            self.country = args.country
        self.practice = args.practice
        self.input_file = args.input_file
        self.ogr2ogr_path = args.ogr2ogr_path

expected_columns = ['FIELD_ID', 'MAIN_CROP', 'VEG_START', 'H_START', 'H_END', 'PRACTICE', 'P_TYPE', 'P_START', 'P_END']

def checkInputFileColumnNames(cols) :
    if len(cols) < len(expected_columns) :
        return False
        
    newCols = [x.upper() for x in cols]
    print (newCols)
    for col in expected_columns :
        if not col in newCols:
            print("Column {} was not found in the file columns!".format(col))
            return False
    return True
    
def get_import_export_table_command(ogr2ogr_path, destination, source, *options):
    command = []
    command += [ogr2ogr_path]
    command += options
    command += [destination, source]
    return command
    
   
def run_command(args, env=None):
    args = list(map(str, args))
    cmd_line = " ".join(map(pipes.quote, args))
    print(cmd_line)
    subprocess.call(args, env=env)


def drop_table(conn, name):
    with conn.cursor() as cursor:
        query = SQL(
            """ drop table if exists {} """
        ).format(Identifier(name))
        print(query.as_string(conn))
        cursor.execute(query)
        conn.commit()
    
def detectDelimiter(csvFile):
    with open(csvFile, 'r') as myCsvfile:
        header = myCsvfile.readline()
        if header.find(";") != -1:
            return ";"
        if header.find(",") != -1:
            return ","
        if header.find("\t") != -1:
            return "\t"
    return ","
    
def validateUploadedFileStruct(input_file) : 
    ret = False
    # Check if the input file exists and has the expected structure (at least at header level and number of columns for each line)
    with open(input_file) as csv_file:
        delim = detectDelimiter(input_file)
        print("Detected delimiter is: {}".format(delim))
        csv_reader = csv.reader(csv_file, delimiter=delim)
        for row in csv_reader:
            print("Column names are: {}".format(", ".join(row)))
            if not checkInputFileColumnNames(row) :
                sys.exit(1)
            ret = True
            break
    
    return ret
                
def getSiteId(conn, siteShortName):
    site_id = -1
    with conn.cursor() as cursor:
        query = SQL(
            """ select id from site where short_name = {} """
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
            """ select value from config where key = {} and site_id = {} """
        )
        query = query.format(Literal(key), Literal(site_id))
        print(query.as_string(conn))

        cursor.execute(query)
        for row in cursor:
            value = row[0]
        conn.commit()
    return value

def getCountry(conn, site_id):
    return getSiteConfigKey(conn, 'processor.s4c_l4c.country', site_id)  

def import_agricultural_practices(config, conn, pg_path, file, site_id):
    table_name = "pd_ap_staging_practices_{}_{}".format(site_id, config.practice.lower())
    drop_table(conn, table_name)
    command = get_import_export_table_command(config.ogr2ogr_path, pg_path, file, "-nln", table_name, "-gt", 100000, "-lco", "UNLOGGED=YES")
    run_command(command)
    
    with conn.cursor() as cursor:
        query = SQL(
            """ alter table {} add column site_id integer, add column practice_short_name varchar, add column year integer, add column country varchar"""
        ).format(Identifier(table_name))
        print(query.as_string(conn))
        cursor.execute(query)
        conn.commit()    

    with conn.cursor() as cursor:
        query = SQL(
            """ update {} set site_id = {}, practice_short_name = {}, year = {}, country = {} """
        ).format(Identifier(table_name), Literal(site_id), Literal(config.practice), Literal(config.year), Literal(config.country))
        print(query.as_string(conn))
        cursor.execute(query)
        conn.commit()    

    with conn.cursor() as cursor:
        query = SQL(
            """ CREATE TABLE IF NOT EXISTS l4c_practices
                (
                  site_id smallint not null,
                  year integer NOT NULL,
                  practice_short_name character varying NOT NULL,
                  country character varying NOT NULL,
                  orig_id character varying DEFAULT NULL,
                  main_crop character varying NOT NULL,
                  veg_start character varying NOT NULL,
                  h_start character varying NOT NULL,
                  h_end character varying NOT NULL,
                  practice character varying NOT NULL,
                  p_type character varying NOT NULL,
                  p_start character varying NOT NULL,
                  p_end character varying NOT NULL
                )
            """
        )
        print(query.as_string(conn))
        cursor.execute(query)
        conn.commit()    

    with conn.cursor() as cursor:
        query = SQL(
            """ DO
                $$
                BEGIN
                   IF to_regclass('public.idx_l4c_practices') IS NULL THEN
                      CREATE INDEX idx_l4c_practices ON public.l4c_practices (site_id, year, practice_short_name);
                   END IF;

                END
                $$; 
            """
        )
        print(query.as_string(conn))
        cursor.execute(query)
        conn.commit()    
        
    with conn.cursor() as cursor:
        query = SQL(
            """ delete from  l4c_practices where site_id = {} and practice_short_name = {} and year = {} """
        ).format(Literal(site_id), Literal(config.practice), Literal(config.year), Literal(config.country))
        print(query.as_string(conn))
        cursor.execute(query)
        conn.commit()    

    with conn.cursor() as cursor:
        query = SQL(
            """
                insert into l4c_practices(
                    site_id,
                    practice_short_name,
                    orig_id,
                    country,
                    year,
                    main_crop,
                    veg_start,
                    h_start,
                    h_end,
                    practice,
                    p_type,
                    p_start,
                    p_end
                )
                select
                    site_id,
                    practice_short_name,
                    field_id,
                    country,
                    year :: int,
                    main_crop,
                    veg_start,
                    h_start,
                    h_end,
                    practice,
                    p_type,
                    p_start,
                    p_end
                from {}
                """
        ).format(Identifier(table_name))
        print(query.as_string(conn))
        cursor.execute(query)
        conn.commit()    
    
    drop_table(conn, table_name)
    
def main():
    parser = argparse.ArgumentParser(description="Handles the upload of the agricultural practices input tables file")
    parser.add_argument('-c', '--config-file', default='/etc/sen2agri/sen2agri.conf', help="Configuration file location")
    parser.add_argument('-s', '--site-short-name', required=True, help="Site short name for which the file was uploaded")
    parser.add_argument('-y', '--year', required=True, type=int, help="The year")
    parser.add_argument('-t', '--country', help="The country short name")
    parser.add_argument('-p', '--practice', required=True, help="The practice for which this file corresponds")
    parser.add_argument('-i', '--input-file', required=True, help="The uploaded input file")
    parser.add_argument('-g', '--ogr2ogr-path', default='ogr2ogr', help="The path to ogr2ogr")
    args = parser.parse_args()

    config = Config(args)
    
    pg_path = 'PG:dbname={} host={} port={} user={} password={}'.format(config.dbname, config.host,
                                                                        config.port, config.user, config.password)

    # Check that the columns are the expected ones
    validateUploadedFileStruct(args.input_file)
    
    with psycopg2.connect(host=config.host, dbname=config.dbname, user=config.user, password=config.password) as conn:
        site_id = getSiteId(conn, config.site_short_name)
        if not args.country :
            config.country = getCountry(conn, site_id)
        import_agricultural_practices(config, conn, pg_path, args.input_file, site_id)
        
if __name__ == "__main__":
    main()
