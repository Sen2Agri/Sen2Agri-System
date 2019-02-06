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
import ntpath
import time
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
from functools import partial

try:
    from configparser import ConfigParser
except ImportError:
    from ConfigParser import ConfigParser

def parse_date(str):
    return datetime.strptime(str, "%Y-%m-%d").date()

class Config(object):
    def __init__(self, args):
        parser = ConfigParser()
        parser.read([args.config_file])

        self.host = parser.get("Database", "HostName")
        self.dbname = parser.get("Database", "DatabaseName")
        self.user = parser.get("Database", "UserName")
        self.password = parser.get("Database", "Password")

        self.site_id = args.site_id
        self.path = args.path
        self.lpis_name = args.lpis_name
        self.lpis_path = args.lpis_path

        if args.season_start:
            self.season_start = parse_date(args.season_start)
        
        if args.season_end:        
            self.season_end = parse_date(args.season_end)

        self.file_type = args.file_type
        self.polarisation = args.polarisation
        self.prd_only = args.prd_only
        self.cmp_file = args.cmp_file
       
        self.tiles_filter = []
        if args.tiles_filter:
            self.tiles_filter = [tile.strip() for tile in args.tiles_filter.split(',')]
        print ("Tiles filter is : {}".format(self.tiles_filter))

class RadarProduct(object):
    def __init__(self, dt, tile_id, orbit_type_id, polarization, radar_product_type, path):
        self.tile_id = tile_id
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
        
class RadarGroup(object):
    def __init__(self, year, week, tile_id, orbit_type_id, polarization, radar_product_type):
        self.year = year
        self.week = week
        self.tile_id = tile_id
        self.orbit_type_id = orbit_type_id
        self.polarization = polarization
        self.radar_product_type = radar_product_type

    def __lt__(self, other):
        return self.__key() < other.__key()

    def __le__(self, other):
        return self.__key() <= other.__key()

    def __eq__(self, other):
        return self.__key() == other.__key()

    def __ne__(self, other):
        return self.__key() != other.__key()

    def __ge__(self, other):
        return self.__key() >= other.__key()

    def __gt__(self, other):
        return self.__key() > other.__key()

    def __hash__(self):
        return hash(self.__key())

    def __key(self):
        return (self.year, self.week, self.tile_id, self.orbit_type_id, self.polarization, self.radar_product_type)

    def format(self, site_id, dt):
        orbit_type = get_orbit_type(self.orbit_type_id)
        return "SEN4CAP_L2A_PRD_S{}_W{:04}{:02}_T{}_{}_{}_{}_{}.tif".format(site_id, self.year, self.week, self.tile_id, dt.strftime("%Y%m%dT%H%M%S"), orbit_type, self.polarization, self.radar_product_type)

def writeProductsToFile(config, groups) :
    filter_files = []
    if config.cmp_file != '' :
        with open(config.cmp_file) as f:
            tmp_filter_files = f.readlines()
        # remove whitespace characters like `\n` at the end of each line
        tmp_filter_files = [x.strip() for x in tmp_filter_files] 
        for filter_file in tmp_filter_files:
            filter_files.append(filter_file.split(';', 1)[0])

        if config.cmp_file == config.path:
            # make a backup to the existing file
            inFileName = ntpath.basename(config.path)
            parentDirName = os.path.dirname(config.path)
            file_prefix = time.strftime("%Y%m%d_%H%M%S_", time.gmtime(os.path.getmtime(config.path)))
            #file_prefix = datetime.now().strftime("%Y%m%d_%H%M%S_")
            newFileName = file_prefix + inFileName
            newPath = os.path.join(parentDirName, newFileName)
            os.rename(config.path, newPath)
            config.cmp_file = newPath
            
    outFileName = config.path
    with open(outFileName, 'w') as f:
        for (product, tile_refs) in groups.iteritems():
            tile_refs.sort()
            if len(filter_files) > 0 and str(product) in filter_files:
                continue
            print("S1 product: {}".format(product))
            f.write(product)
            if config.prd_only == 0:
                for tile_ref in tile_refs:
                    f.write(';' + tile_ref)
                    print("\t\tRaster: {}".format(tile_ref))
            f.write('\n')
        
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
                    site_tiles.tile_id,
                    product.name,
                    case
                        when substr(product.name, 17, 8) > substr(product.name, 33, 8) then substr(product.name, 17, 8)
                        else substr(product.name, 33, 8)
                    end :: date as date,
                    coalesce(product.orbit_type_id, 1) as orbit_type_id,
                    substr(product.name, 49, 2) as polarization,
                    product.processor_id,
                    product.product_type_id,
                    substr(product.name, length(product.name) - strpos(reverse(product.name), '_') + 2) as radar_product_type,
                    product.orbit_id,
                    product.full_path
                from sp_get_site_tiles({} :: smallint, 1 :: smallint) as site_tiles
                inner join shape_tiles_s2 on shape_tiles_s2.tile_id = site_tiles.tile_id
                inner join product on ST_Intersects(product.geog, shape_tiles_s2.geog)
                where product.satellite_id = 3
                    and product.site_id = {}
            )
            select products.date,
                    products.tile_id,
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
        query = query.format(site_id_filter, site_id_filter, start_date_filter, end_date_filter)
        # print(query.as_string(conn))
        cursor.execute(query)

        results = cursor.fetchall()
        conn.commit()

        products = []
        for (dt, tile_id, orbit_type_id, polarization, radar_product_type, full_path) in results:
            if config.file_type == radar_product_type : 
                if config.polarisation == "" or config.polarisation == polarization : 
                    products.append(RadarProduct(dt, tile_id, orbit_type_id, polarization, radar_product_type, full_path))

        return products

def process_ndvi(config, conn):
    products = get_ndvi_products_from_db(config, conn, config.site_id)
    
    groups = defaultdict(list)
    for product in products:
        tile_ref = os.path.join(config.lpis_path, "{}_{}_S2.tif".format(config.lpis_name, product.tile_id))
        groups[product.path].append(tile_ref)
    
    writeProductsToFile(config, groups)
    
def process_radar(config, conn):
    products = get_radar_products(config, conn, config.site_id)

    groups = defaultdict(list)
    for product in products:
        tile_ref = os.path.join(config.lpis_path, "{}_{}_S1.tif".format(config.lpis_name, product.tile_id))
        #print("S1 Product: {}, Raster: {}".format(product.path, tile_ref))
        groups[product.path].append(tile_ref)
    
    writeProductsToFile(config, groups)
            
#    for product in products:
#        group = RadarGroup(product.year, product.week, product.tile_id, product.orbit_type_id, product.polarization, product.radar_product_type)
#        #groups[group].append(product)
#        groups[product.path].append(product)
#
#    groups = sorted(list(groups.items()))
#    
#    for (group, products) in groups:
#        print("Group: {}".format(group))
#        for product in products:
#            tile_ref = os.path.join(config.lpis_path, "{}_{}.tif".format(config.lpis_name, product.tile_id))
#            #print("Tile Ref: {}".format(tile_ref))
#            #print("Tile: {}, TileRef: {}, Orbit: {}, Polarisation: {}, ProdType: {}, Path: {}".format(product.tile_id, tile_ref, product.orbit_type_id, product.polarization, product.radar_product_type, product.path))
#  


def filter_by_tiles(config, products) : 
    ret_prds = []
    if config.tiles_filter and len(config.tiles_filter):
        for product in products :
            if (not product.tile_id) or (product.tile_id in config.tiles_filter) :
                   ret_prds.append(product)
    else :
        ret_prds = products
    
    return ret_prds
                
        
def main():
    parser = argparse.ArgumentParser(description="Executes agricultural monitoring data extraction")
    parser.add_argument('-c', '--config-file', default='/etc/sen2agri/sen2agri.conf', help="Configuration file location")
    parser.add_argument('-s', '--site-id', type=int, help="Site ID to filter by")
    parser.add_argument('--season-start', help="Season start date")
    parser.add_argument('--season-end', help="Season end date")
    parser.add_argument('-p', '--path', default='./products_to_rasterized_files.csv', help="Output path")
    parser.add_argument('--tiles-filter', help="Input tiles filter")
    parser.add_argument('--lpis-name', help="Name of the LPIS dataset")
    parser.add_argument('--lpis-path', help="Path to the rasterized LPIS")
    parser.add_argument('--file-type', help="File type to be processed")
    parser.add_argument('--polarisation', help="Polarisation (optionally) for radar products")
    parser.add_argument('--prd-only', type=int, default=0, help="Export only the product path and not the full mapping")
    parser.add_argument('--cmp-file', default='', help="Only entries that are not in this file are outputted")
    args = parser.parse_args()

    config = Config(args)

    with psycopg2.connect(host=config.host, dbname=config.dbname, user=config.user, password=config.password) as conn:
        # process_optical(config, conn, pool, SATELLITE_ID_SENTINEL)
        products = []
        if args.file_type == "NDVI" : 
            products = process_ndvi(config, conn)
        else :
            products = process_radar(config, conn)
        
        #products = filter_by_tiles(config, products)
        

if __name__ == "__main__":
    main()
