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
from psycopg2.sql import SQL, Literal
import psycopg2.extras
import subprocess
import re
import itertools

try:
    from configparser import ConfigParser
except ImportError:
    from ConfigParser import ConfigParser

def parse_date(str):
    return datetime.strptime(str, "%Y-%m-%d").date()

def parse_date_iso(str):
    return datetime.strptime(str, "%Y%m%dT%H%M%S").date()
    
class Config(object):
    def __init__(self, args):
        parser = ConfigParser()
        parser.read([args.config_file])

#        self.host = parser.get("Database", "HostName")
#        self.dbname = parser.get("Database", "DatabaseName")
#        self.user = parser.get("Database", "UserName")
#        self.password = parser.get("Database", "Password")
#
#        self.site_id = args.site_id
#        self.path = args.path
#        self.lpis_name = args.lpis_name
#        self.lpis_path = args.lpis_path
#        self.lpis_shp = args.lpis_shp
#        if not args.lpis_shp :
#            print("LPIS shape not provided. Exiting ...")
#            #sys.exit(0)
#
#        self.season_start = parse_date(args.season_start)
#        self.season_end = parse_date(args.season_end)
#
#        self.inputs_file = args.inputs_file
#        self.file_type = args.file_type
#        self.use_shapefile_only = args.use_shapefile_only
#        
#        self.prds_per_group = args.prds_per_group
#        
#        self.uid_field = args.uid_field
#        self.seq_field = args.seq_field
#        
#        self.pool_size = args.pool_size
#        
#        self.tiles_filter = []
#        if args.tiles_filter:
#            self.tiles_filter = [tile.strip() for tile in args.tiles_filter.split(',')]
#        print ("Tiles filter is : {}".format(self.tiles_filter))

def run_command(args, env=None):
    args = list(map(str, args))
    cmd_line = " ".join(map(pipes.quote, args))
    print(cmd_line)

    subprocess.call(args, env=env)
        
def main():
    parser = argparse.ArgumentParser(description="Executes agricultural monitoring data extraction")
    parser.add_argument('-i', '--input-dir', default='.', help="Input directory containing the files for merging")
    parser.add_argument('-o', '--output-file', help="Output file for merging")
    parser.add_argument('--in-file-type', default="csv", help="The type of the files for merging")
    parser.add_argument('--out-file-type', default="csv", help="The type of the output file")
    parser.add_argument('--csvcompact', type=int, default=1, help="Compact CSV file")
    args = parser.parse_args()

    if not os.path.exists(os.path.dirname(args.output_file)):
        try:
            os.makedirs(os.path.dirname(args.output_file))
        except OSError as exc: # Guard against race condition
            if exc.errno != errno.EEXIST:
                raise
            
    #config = Config(args)

    fileExt = ".{}".format(args.in_file_type)
    listFilePaths = []
    files = os.listdir(args.input_dir)
    for file in files:
        if fileExt in file:
            fullFilePath = os.path.join(args.input_dir, file)
            listFilePaths.append(fullFilePath)
    
    command = []
    command += ["otbcli", "AgricPractMergeDataExtractionFiles", "./sen2agri-processors-build/"]
    command += ["-csvcompact", args.csvcompact]
    command += ["-outformat", args.out_file_type]
    command += ["-out", args.output_file]
    command += ["-il"] + listFilePaths
    
    run_command(command)
    
if __name__ == "__main__":
    main()
