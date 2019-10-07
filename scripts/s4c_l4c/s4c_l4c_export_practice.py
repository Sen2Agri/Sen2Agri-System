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
        self.practice = args.practice
        self.out_file = args.out_file


def get_export_table_command(destination, source, *options):
    command = []
    command += ["ogr2ogr"]
    command += options
    command += [destination, source]
    return command
    
   
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
  

def exportPracticesFile(config, conn, pg_path, practice, site_id, out_file):
    lpis_table = "decl_{}_{}".format(config.site_short_name, config.year)
    lut_table = "lut_{}_{}".format(config.site_short_name, config.year)
    
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


        command = get_export_table_command(out_file, pg_path, "-sql", query_str, "-gt", 100000)
        run_command(command)

#        cursor.execute(query)
#        rowsDict = {}
#        for row in cursor:
#            print(row)
#        conn.commit()        
#        print("Rows are: {}".format(rowsDict))
    
    

def main():
    parser = argparse.ArgumentParser(description="Handles the upload of the agricultural practices input tables file")
    parser.add_argument('-c', '--config-file', default='/etc/sen2agri/sen2agri.conf', help="Configuration file location")
    parser.add_argument('-s', '--site-short-name', help="Site short name for which the file was uploaded")
    parser.add_argument('-y', '--year', type=int, help="The year")
    parser.add_argument('-t', '--country', help="The country short name")
    parser.add_argument('-p', '--practice', help="The practice for which this file corresponds")
    parser.add_argument('-o', '--out-file', default='.', help="Output path")
    args = parser.parse_args()

#    if args.out_dir :
#        if not os.path.exists(args.out_dir):
#            os.makedirs(args.out_dir)
    
    config = Config(args)
    
    pg_path = 'PG:dbname={} host={} port={} user={} password={}'.format(config.dbname, config.host,
                                                                        config.port, config.user, config.password)

    with psycopg2.connect(host=config.host, dbname=config.dbname, user=config.user, password=config.password) as conn:
        site_id = getSiteId(conn, config.site_short_name)
        exportPracticesFile(config, conn, pg_path, config.practice, site_id, args.out_file)
        
if __name__ == "__main__":
    main()
