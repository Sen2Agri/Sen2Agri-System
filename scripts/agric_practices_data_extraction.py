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
from functools import partial

try:
    from configparser import ConfigParser
except ImportError:
    from ConfigParser import ConfigParser

def parse_date(str):
    return datetime.strptime(str, "%Y-%m-%d").date()

def parse_date2(str):
    return datetime.strptime(str, "%Y%m%d").date()
    
def parse_date_iso(str):
    return datetime.strptime(str, "%Y%m%dT%H%M%S").date()
    
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
        self.lpis_shp = args.lpis_shp
        if not args.lpis_shp :
            print("LPIS shape not provided. Exiting ...")
            #sys.exit(0)

        if args.season_start:
            self.season_start = parse_date(args.season_start)
        
        if args.season_end:        
            self.season_end = parse_date(args.season_end)

        self.inputs_file = args.inputs_file
        self.file_type = args.file_type
        self.polarisation = args.polarisation
        self.nf_2017 = args.nf_2017
        self.use_shapefile_only = args.use_shapefile_only
        self.gen_minmax = args.gen_minmax
        self.csvcompact = args.csvcompact
        self.filter_ids = args.filter_ids
        
        self.prds_per_group = args.prds_per_group
        
        self.uid_field = args.uid_field
        self.seq_field = args.seq_field
        
        self.pool_size = args.pool_size
        
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

def run_command2(args, allCmds, env=None): 
    print("=================================================")
    curIdx = allCmds.index(args) + 1
    allCmdsLen = len(allCmds)
    percentage = int(100 * float(curIdx)/float(allCmdsLen))
    print ("Processing group idx {} from a total of {} - Global progress: {}%".format(curIdx, allCmdsLen, percentage))
    print("=================================================")    
    
def run_command(args, allCmds, env=None):
    print("=================================================")
    curIdx = allCmds.index(args) + 1
    allCmdsLen = len(allCmds)
    percentage = int(100 * float(curIdx)/float(allCmdsLen))
    print ("Processing group idx {} from a total of {} - Global progress: {}%".format(curIdx, allCmdsLen, percentage))
    print("=================================================")    
    args = list(map(str, args))
    cmd_line = " ".join(map(pipes.quote, args))
    print(cmd_line)

    subprocess.call(args, env=env)
        
def get_products_from_file(config) :
    with open(config.inputs_file) as f:
        results = f.readlines()
    results = [x.strip() for x in results] 
    products = []
    isNdvi = False
    for (full_path) in results:
        fileName = os.path.basename(full_path)
        if "SNDVI" in fileName:
            isNdvi = True
            searchObj = re.search( r'S2AGRI_L3B_SNDVI_A(\d{8}T\d{6})_T(.+)\.TIF', fileName)
            if searchObj:
                products.append(NdviProduct(parse_date_iso(searchObj.group(1)), searchObj.group(2), full_path))
        elif "AMP" in fileName or "COHE" in fileName:
            searchObj = re.search( r'SEN4CAP_L2A.+_V(\d{8}T\d{6})_(\d{8}T\d{6})_(VV|VH)_(\d.+)_(AMP|COHE).*', fileName)
            if searchObj:
                products.append(RadarProduct(parse_date_iso(searchObj.group(1)), "", int(searchObj.group(4)), searchObj.group(3), searchObj.group(5), full_path))
        elif "amp" in fileName:
            searchObj = re.search( r'(\d{8})_slc_coreg_amp_calib_(asc|desc)_(\d.+)_(VV|VH)_geocoded.tiff', fileName)
            if searchObj:
                products.append(RadarProduct(parse_date2(searchObj.group(1)), "", int(searchObj.group(3)), searchObj.group(4), "AMP", full_path))
        elif "cohe" in fileName:
            searchObj = re.search( r'(\d{8})-(\d{8})_cohe_(asc|desc)_(\d.+)_(VV|VH)_geocoded.tiff', fileName)
            if searchObj:
                products.append(RadarProduct(parse_date2(searchObj.group(1)), "", int(searchObj.group(4)), searchObj.group(5), "COHE", full_path))

    if isNdvi == True :
        sortedProducts = sorted(products, key=lambda x: x.tile_id, reverse=False)
    else :
        sortedProducts = sorted(products, key=lambda x: x.orbit_type_id, reverse=False)
    
    # write also to a file to have it as reference
    writeSortedProductsToFile(config.inputs_file, sortedProducts)
     
    return sortedProducts

def writeSortedProductsToFile(inputFile, sortedProducts) :
    outFileName = inputFile + "_sorted"
    with open(outFileName, 'w') as f:
        for s in sortedProducts:
            f.write(s.path + '\n')

def writeProductsToFile(config, file_idx, groups) :
    targetTempPath = os.path.join(config.path, "product_to_rasterize_files")
    if not os.path.exists(targetTempPath):
        os.makedirs(targetTempPath)
    
    fileName = "product_to_rasterize_files_" + config.file_type
    if config.polarisation != "" :
        fileName += ("_" + config.polarisation)
    if file_idx == -1 :
        fileName += "_ALL"
    else :
        fileName += ("_" + str(file_idx))
    fileName += ".csv"
    outFileName = os.path.join(targetTempPath, fileName)
    print("Writing to file {}".format(outFileName))
    
    with open(outFileName, 'w') as f:
        for (product, tile_refs) in groups.iteritems():
            tile_refs.sort()
            print("S1 product: {}".format(product))
            f.write(product)
            for tile_ref in tile_refs:
                f.write(';' + tile_ref)
                print("\t\tRaster: {}".format(tile_ref))
            f.write('\n')
    return outFileName
    
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
    products = []
    if not config.inputs_file : 
        products = get_ndvi_products_from_db(config, conn, config.site_id)

        groups = defaultdict(list)
        for product in products:
            tile_ref = os.path.join(config.lpis_path, "{}_{}_S2.tif".format(config.lpis_name, product.tile_id))
            groups[product.path].append(tile_ref)
    
        writeProductsToFile(config, -1, groups)
        return groups

    else :
        products = get_products_from_file(config)
    return products
    
def process_radar(config, conn):
    products = []
    if not config.inputs_file : 
        products = get_radar_products(config, conn, config.site_id)

        groups = defaultdict(list)
        for product in products:
            if product.path.endswith('.nc'):
                continue
            tile_ref = os.path.join(config.lpis_path, "{}_{}_S1.tif".format(config.lpis_name, product.tile_id))
            #print("S1 Product: {}, Raster: {}".format(product.path, tile_ref))
            groups[product.path].append(tile_ref)
    
        writeProductsToFile(config, -1, groups)
        return groups

    else :
        products = get_products_from_file(config)
    return products        
#        for product in products:
#            tile_ref = os.path.join(config.lpis_path, "{}_{}.tif".format(config.lpis_name, product.tile_id))
#            print("Ndvi Product: {} Tile {}, Date {}".format(product.path, product.tile_id, product.year))
#            #print("{}".format(product.path))
#    execute_data_extraction(config, products)
    
#    groups = defaultdict(list)
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
#    execute_data_extraction(config, products)

def grouper(n, iterable):
    it = iter(iterable)
    while True:
       chunk = tuple(itertools.islice(it, n))
       if not chunk:
           return
       yield chunk
       
def execute_data_extraction(config, products) :
    prds_per_group = config.prds_per_group
    if prds_per_group <= 0 :
        prds_per_group = len(products)
    prdsChunks = grouper(prds_per_group, products)
    commands = []
    i = 0;
    for prdsChunk in prdsChunks:
        command = []
        command += ["otbcli", "AgricPractDataExtraction", "./sen2agri-processors-build/"]
#        if config.file_type == "AMP" and config.gen_minmax == 0:
#            command += ["-convdb", 1]
        if config.nf_2017 == 1 : 
            command += ["-oldnf", 1]
        if config.gen_minmax == 1 : 
            command += ["-minmax", 1]
        if config.csvcompact == 1 : 
            command += ["-csvcompact", 1]
        command += ["-field", config.uid_field]
        command += ["-prdtype", config.file_type]
        command += ["-outdir", config.path]
        if config.filter_ids != "" : 
            command += ["-filterids", config.filter_ids]            
            
        tiffs = []            
        if not config.inputs_file : 
            chunk_groups = defaultdict(list)
            for prdInChunk in prdsChunk:
                tile_refs = products.get(prdInChunk)
                for tile_ref in tile_refs:
                    chunk_groups[prdInChunk].append(tile_ref)
                    
            matchingFile = writeProductsToFile(config, i, chunk_groups)
            tiffs.append(matchingFile)
            i += 1
        else :
            command += ["-vec", config.lpis_shp]
            for prdInChunk in prdsChunk:
                tiffs.append(prdInChunk.path)
                
        command += ["-il"] + tiffs
        commands.append(command)
        
        print ("Executing command: {}".format(command))
            
            # check if we should use S2 tiles as reference
    #        if not config.use_shapefile_only :
    #            groups = defaultdict(list)
    #            for product in prdInChunk:
    #                groups[product.path].append(product)
    #            groups = sorted(list(groups.items()))
    #            
    #            tile_refs = []
    #            tile_refs_cnts = []
    #            for (group, grpProducts) in groups:
    #                for product in grpProducts:
    #                    tile_ref = os.path.join(config.lpis_path, "{}_{}.tif".format(config.lpis_name, product.tile_id))
    #                    tile_refs.append(tile_ref)
    #                tile_refs_cnts.append(len(grpProducts)
    #                #print("Tile Ref: {}".format(tile_ref))
    #                #print("Tile: {}, TileRef: {}, Orbit: {}, Polarisation: {}, ProdType: {}, Path: {}".format(product.tile_id, tile_ref, product.orbit_type_id, product.polarization, product.radar_product_type, product.path))     
    #            #command += ["-s2il"] + tile_refs
    #            #command += ["-s2ilcnts"] + tile_refs_cnts


################## TODO: UNCOMMENT ##########################            
    pool = multiprocessing.Pool(config.pool_size)
    
    results = []
    run_command_x=partial(run_command, allCmds=commands)
    r = pool.map_async(run_command_x, commands, callback=results.append)
    r.wait() 
################## END: UNCOMMENT ##########################    
    
    #print(results)
#    for command in commands:
#        run_command(command)

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
    parser.add_argument('-p', '--path', default='.', help="Output path")
    parser.add_argument('--tiles-filter', help="Input tiles filter")
    parser.add_argument('--lpis-name', help="Name of the LPIS dataset")
    parser.add_argument('--lpis-path', help="Path to the rasterized LPIS")
    parser.add_argument('--lpis-shp', help="Input LPIS shapefiles")
    parser.add_argument('--file-type', help="File type to be processed")
    parser.add_argument('--polarisation', default='', help="Polarisation (optionally) for radar products")
    parser.add_argument('--prds-per-group', type=int, default=20, help="Number of products to be grouped during a data extraction execution")
    parser.add_argument('--inputs-file', help="Use this list of files instead of the using the site products")
    parser.add_argument('--use-shapefile-only', default=1, type=int, help="Use only shapefile without tiles images")
    parser.add_argument('--uid-field', help="Unique identifier string field")
    parser.add_argument('--seq-field', default="NewID", help="Unique identifier sequence integerr field")
    parser.add_argument('--pool-size', type=int, default=4, help="The number of parallel executions")
    parser.add_argument('--nf-2017', default=0, type=int, help="Use the 2017 naming format")
    parser.add_argument('--gen-minmax', default=0, type=int, help="Generates also the minimum and maximum for each parcel")
    parser.add_argument('--csvcompact', default=1, type=int, help="Generates the output CSV file in a compact form (each parcel on one line in CSV)")
    parser.add_argument('--filter-ids', default='', help="File containing the parcel ids filters")
    args = parser.parse_args()

    if args.path :
        if not os.path.exists(args.path):
            os.makedirs(args.path)
    
    config = Config(args)

    with psycopg2.connect(host=config.host, dbname=config.dbname, user=config.user, password=config.password) as conn:
        # process_optical(config, conn, pool, SATELLITE_ID_SENTINEL)
        products = []
        if args.file_type == "NDVI" : 
            products = process_ndvi(config, conn)
        else :
            products = process_radar(config, conn)
        
        products = filter_by_tiles(config, products)
        
#        for product in products:
#            print("{}".format(product.path))            
        execute_data_extraction(config, products)

if __name__ == "__main__":
    main()
