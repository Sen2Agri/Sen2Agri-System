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

class RadarProduct(object):
    def __init__(self, dt, orbit_type_id, polarization, radar_product_type, path):
        self.orbit_type_id = orbit_type_id
        self.polarization = polarization
        self.radar_product_type = radar_product_type
        self.path = path

        (year, week, _) = dt.isocalendar()
        self.year = year
        self.week = week

class NdviProduct(object):
    def __init__(self, dt, tile_id, path):
        self.tile_id = tile_id
        self.path = path

        (year, week, _) = dt.isocalendar()
        self.year = year
        self.week = week

def get_ndvi_products_from_db(config, conn, site_id):
    with conn.cursor() as cursor:
        query = SQL(
            """
            with products as (
                select product.site_id,
                    product.name,
                    product.created_timestamp as date,
                    product.processor_id,
                    product.product_type_id,
                    product.full_path,
                    product.tiles
                from product
                where product.site_id = {}
                    and product.product_type_id = 3
            )
            select products.date,
                    products.tiles,
                    products.full_path
            from products
            where date between {} and {}
            order by date;
            """
        )

        site_id_filter = Literal(site_id)
        start_date_filter = Literal(config.season_start)
        end_date_filter = Literal(config.season_end)
        query = query.format(site_id_filter, start_date_filter, end_date_filter)
        # print(query.as_string(conn))
        cursor.execute(query)

        results = cursor.fetchall()
        conn.commit()

        products = []
        for (dt, tiles, full_path) in results:
            for tile in tiles :
                ndviTilePath = os.path.join(full_path, "TILES")
                acq_date = dt.strftime("%Y%m%dT%H%M%S")
                ndviTilePath = os.path.join(ndviTilePath, "S2AGRI_L3B_A{}_T{}".format(acq_date, tile))
                ndviTilePath = os.path.join(ndviTilePath, "IMG_DATA")
                ndviTilePath = os.path.join(ndviTilePath, "S2AGRI_L3B_SNDVI_A{}_T{}.TIF".format(acq_date, tile))
                products.append(NdviProduct(dt, tile, ndviTilePath))
                #if not os.path.exists(ndviTilePath) :
                #    print ("FILE DOES NOT EXISTS: ".format(ndviTilePath))

        return products
        
def get_radar_products(config, conn, site_id):
    with conn.cursor() as cursor:
        query = SQL(
            """
            with products as (
                select product.site_id,
                    product.name,
                    case
                        when substr(product.name, position('_V' in product.name) + 2, 8) > substr(product.name, position('_V' in product.name) + 18, 8) 
                        then substr(product.name, position('_V' in product.name) + 2, 8)
                        else substr(product.name, position('_V' in product.name) + 18, 8)
                    end :: date as date,
                    coalesce(product.orbit_type_id, 1) as orbit_type_id,
                    substr(product.name, position('_V' in product.name) + 34, 2) as polarization,
                    product.processor_id,
                    product.product_type_id,
                    substr(product.name, length(product.name) - strpos(reverse(product.name), '_') + 2) as radar_product_type,
                    product.orbit_id,
                    product.full_path
                from product
                where product.satellite_id = 3
                    and product.site_id = {}
            )
            select products.date,
                products.orbit_type_id,
                products.polarization,
                products.radar_product_type,
                products.full_path
            from products
            where date between {} and {}
            order by date;
            """
        )

        site_id_filter = Literal(site_id)
        start_date_filter = Literal(config.season_start)
        end_date_filter = Literal(config.season_end)
        query = query.format(site_id_filter, start_date_filter, end_date_filter)
        # print(query.as_string(conn))
        cursor.execute(query)

        results = cursor.fetchall()
        conn.commit()

        products = []
        for (dt, orbit_type_id, polarization, radar_product_type, full_path) in results:
            if config.file_type == radar_product_type : 
                if config.polarisation == "" or config.polarisation == polarization : 
                    products.append(RadarProduct(dt, orbit_type_id, polarization, radar_product_type, full_path))

        return products

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
    parser.add_argument('-b', '--start-date', help="The begining of the interval to be considered ")
    parser.add_argument('-e', '--end-date', help="The end of the interval to be considered ")
    parser.add_argument('-t', '--product-type', help="The end of the interval to be considered ")
    
    args = parser.parse_args()

    if not os.path.exists(os.path.dirname(args.output_file)):
        try:
            os.makedirs(os.path.dirname(args.output_file))
        except OSError as exc: # Guard against race condition
            if exc.errno != errno.EEXIST:
                raise
            
    #config = Config(args)
#    with psycopg2.connect(host=config.host, dbname=config.dbname, user=config.user, password=config.password) as conn:
#        productsNdvi = process_ndvi(config, conn)
#        productsRadar = process_radar(config, conn)

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
