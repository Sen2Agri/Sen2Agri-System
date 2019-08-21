import os, glob
import sys

import re

import dateutil.parser
import datetime

import argparse
try:
    from configparser import ConfigParser
except ImportError:
    from ConfigParser import ConfigParser
    
import psycopg2
from psycopg2.sql import SQL, Literal
import psycopg2.extras

from datetime import date
from datetime import datetime
from datetime import timedelta
import fnmatch
import pipes
import subprocess

def parse_date(str):
    return datetime.strptime(str, "%Y-%m-%d").date()

def get_bool_value(value):
    boolStr = str(value).lower()
    if (boolStr == "true") or (boolStr == "yes") or (boolStr == "y") or (boolStr == "t") or (boolStr == "1"):
        return True
    return False
    
def run_command(config,args, env=None):
    args = list(map(str, args))
    cmd_line = " ".join(map(pipes.quote, args))
    print(cmd_line)
    ret = True
    if config.verify_only==True:
        return ret
    try:
        ret1 = subprocess.call(args, env=env)
        if ret1 != 0 :
            print "bad subprocess.call"
            ret=False
    except subprocess.CalledProcessError:
        print "run_cmd error"
        ret = False # handle errors in the called executable
    except OSError:
        print "os error"
        ret = False # executable not found    
    
    return ret
    
def movePath(config, srcPath, destPath) :
    if config.cp_rm == True : 
        ret = run_command(config,["cp", "-fR", srcPath, destPath])
        if ret == True:
            run_command(config, ["rm", "-fR", srcPath])
    else :
        run_command(config, ["mv", srcPath, destPath])

class S4CConfig(object):
    def __init__(self, args):
        parser = ConfigParser()
        parser.read([args.config_file])

        self.host = parser.get("Database", "HostName")
        self.dbname = parser.get("Database", "DatabaseName")
        self.user = parser.get("Database", "UserName")
        self.password = parser.get("Database", "Password")

        self.site_id = args.site_id

        if args.season_start:
            self.season_start = parse_date(args.season_start)
        else : 
            self.season_start = parse_date('2015-01-01')
        
        if args.season_end:        
            self.season_end = parse_date(args.season_end)
        else:
            self.season_end = parse_date('2050-12-31')
            
        if args.cp_rm :
            self.cp_rm = get_bool_value(args.cp_rm)
        else :
            self.cp_rm = False
            
        if args.verify_only :
            self.verify_only = get_bool_value(args.verify_only)
        else :
            self.verify_only = True

def checkTileDir(config, tilesPath, subPath, tile, tile_dir_acq_date) :
    fullTilePath = os.path.join(tilesPath, subPath)
    tileImgDataPath = os.path.join(fullTilePath, "IMG_DATA")
    try:
        tilesDirFiles = os.listdir(tileImgDataPath)
    except:
        print("Expected L3B product structure found but the path {} does not exists".format(tileImgDataPath))
        return
    # normally would be enough to take only the first file, no matter is SNDVI, SLAI, SFAPAR or SFCOVER
    prdFiles = fnmatch.filter(tilesDirFiles, "S2AGRI_L3B_S*_A*_T{}.TIF".format(tile))
    if len (prdFiles) > 0:
        prdFile = prdFiles[0]
        m = re.match("S2AGRI_L3B_S.*_A(\d{{8}}T\d{{6}})_T{}.TIF".format(tile), prdFile)
        tif_acq_date = m.group(1)
        print ("##############################################")
        if tif_acq_date == tile_dir_acq_date : 
            print (" *** File {} and parent dir have the same acq date. No operation needed.".format(os.path.join(tileImgDataPath, prdFile)))
            pass
        else :
            newSubPath = os.path.join(tilesPath, "S2AGRI_L3B_A{}_T{}".format(tif_acq_date, tile))
            #print (">> Renaming tile directory from {} to {} ...".format(fullTilePath, newSubPath))

            # moving the folder MTD file
            movePath(config, os.path.join(fullTilePath, "S2AGRI_L3B_MTD_A{}_T{}.xml".format(tile_dir_acq_date, tile)),
                     os.path.join(fullTilePath, "S2AGRI_L3B_MTD_A{}_T{}.xml".format(tif_acq_date, tile)))

            # moving the folder PVI file
            movePath(config, os.path.join(fullTilePath, "S2AGRI_L3B_PVI_A{}_T{}.jpg".format(tile_dir_acq_date, tile)),
                        os.path.join(fullTilePath, "S2AGRI_L3B_PVI_A{}_T{}.jpg".format(tif_acq_date, tile)))
            
            # Finally, rename the folder
            movePath(config, fullTilePath, newSubPath)
    else :
        print ("WARNING: Directory {} contains no TIF file!!!".format(tileImgDataPath))
            


def process_ndvi_products_list(config, prds_list):
        for full_path in prds_list:
            print ("Checking dir: {}".format(full_path))
            tilesPath = os.path.join(full_path, "TILES")
            try:
                tilesDirs = os.listdir(tilesPath)
            except:
                print("Product {} found in DB but not on disk".format(full_path))
                continue       
                
            for tileDir in tilesDirs :     
                m = re.match("S2AGRI_L3B_A(\d{8}T\d{6})_T(.*)", tileDir)
                if m:
                    checkTileDir(config, tilesPath, tileDir, m.group(2), m.group(1))
                else :
                    print ("Tile dir {} did not matched expression".format(tileDir))

def process_db_ndvi_products(config, conn, site_id):
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
        #print(query.as_string(conn))
        # execute the query
        cursor.execute(query)            
        results = cursor.fetchall()
        conn.commit()

        print ("Extracted a number of {} products to check".format(len(results)))
        
        for (dt, tiles, full_path) in results:
            tilesPath = os.path.join(full_path, "TILES")
            try:
                tilesDirs = os.listdir(tilesPath)
            except:
                print("Product {} found in DB but not on disk".format(full_path))
                continue       
                
            for tile in tiles :                
                tilePaths = fnmatch.filter(tilesDirs, "S2AGRI_L3B_A*_T{}".format(tile))
                if len(tilePaths) == 1:
                    subPath = tilePaths[0]
                    m = re.match("S2AGRI_L3B_A(\d{{8}}T\d{{6}})_T{}".format(tile), subPath)
                    checkTileDir(config, tilesPath, subPath, tile, m.group(1))

def process_ndvi_products(config, conn, site_id, prds_list):
    if prds_list is None or len(prds_list) == 0 :
        print ("Handling products in DB ...")
        process_db_ndvi_products(config, conn, site_id)
    else :
        print ("Handling products list ...")
        process_ndvi_products_list(config, prds_list)

#def process_ndvi_products(config, conn, site_id, prds_list):
#    with conn.cursor() as cursor:
#        if prds_list is None or len(prds_list) == 0 :
#            query = SQL(
#                """
#                with products as (
#                    select product.site_id,
#                        product.name,
#                        product.created_timestamp as date,
#                        product.processor_id,
#                        product.product_type_id,
#                        product.full_path,
#                        product.tiles
#                    from product
#                    where product.site_id = {}
#                        and product.product_type_id = 3
#                )
#                select products.date,
#                        products.tiles,
#                        products.full_path
#                from products
#                where date between {} and {}
#                order by date;
#                """
#            )
#
#            site_id_filter = Literal(site_id)
#            start_date_filter = Literal(config.season_start)
#            end_date_filter = Literal(config.season_end)
#            query = query.format(site_id_filter, start_date_filter, end_date_filter)
#            #print(query.as_string(conn))
#        else :
#            if len (prds_list) > 1:
#                prdsSubstr = tuple(prds_list)
#            else :
#                prdsSubstr = "('{}')".format(prds_list[0])
#            query= """
#                   with products as (
#                    select product.site_id,
#                        product.name,
#                        product.created_timestamp as date,
#                        product.processor_id,
#                        product.product_type_id,
#                        product.full_path,
#                        product.tiles
#                    from product
#                    where product.site_id = {} and product.product_type_id = 3 and product.name in {}
#                )
#                select products.date,
#                        products.tiles,
#                        products.full_path
#                from products
#                order by date;""".format(site_id, prdsSubstr)
#            #print(query)
#        
#        # execute the query
#        cursor.execute(query)            
#            
#        results = cursor.fetchall()
#        conn.commit()
#
#        print ("Extracted a number of {} products to check".format(len(results)))
#        
#        for (dt, tiles, full_path) in results:
#            tilesPath = os.path.join(full_path, "TILES")
#            try:
#                tilesDirs = os.listdir(tilesPath)
#            except:
#                print("Product {} found in DB but not on disk".format(full_path))
#                continue       
#                
#            for tile in tiles :                
#                tilePaths = fnmatch.filter(tilesDirs, "S2AGRI_L3B_A*_T{}".format(tile))
#                if len(tilePaths) == 1:
#                    subPath = tilePaths[0]
#                    m = re.match("S2AGRI_L3B_A(\d{{8}}T\d{{6}})_T{}".format(tile), subPath)
#                    tile_dir_acq_date = m.group(1)
#                    
#                    fullTilePath = os.path.join(tilesPath, subPath)
#                    tileImgDataPath = os.path.join(fullTilePath, "IMG_DATA")
#                    try:
#                        tilesDirFiles = os.listdir(tileImgDataPath)
#                    except:
#                        print("Expected L3B product structure found but the path {} does not exists".format(tileImgDataPath))
#                        continue
#                    # normally would be enough to take only the first file, no matter is SNDVI, SLAI, SFAPAR or SFCOVER
#                    prdFiles = fnmatch.filter(tilesDirFiles, "S2AGRI_L3B_S*_A*_T{}.TIF".format(tile))
#                    if len (prdFiles) > 0:
#                        prdFile = prdFiles[0]
#                        m = re.match("S2AGRI_L3B_S.*_A(\d{{8}}T\d{{6}})_T{}.TIF".format(tile), prdFile)
#                        tif_acq_date = m.group(1)
#                        print ("##############################################")
#                        if tif_acq_date == tile_dir_acq_date : 
#                            print (" *** File {} and parent dir have the same acq date. No operation needed.".format(os.path.join(tileImgDataPath, prdFile)))
#                            pass
#                        else :
#                            newSubPath = os.path.join(tilesPath, "S2AGRI_L3B_A{}_T{}".format(tif_acq_date, tile))
#                            #print (">> Renaming tile directory from {} to {} ...".format(fullTilePath, newSubPath))
#
#                            # moving the folder MTD file
#                            run_command(["mv", os.path.join(fullTilePath, "S2AGRI_L3B_MTD_A{}_T{}.xml".format(tile_dir_acq_date, tile)),
#                                               os.path.join(newSubPath, "S2AGRI_L3B_MTD_A{}_T{}.xml".format(tif_acq_date, tile))])
#
#                            # moving the folder PVI file
#                            run_command(["mv", os.path.join(fullTilePath, "S2AGRI_L3B_PVI_A{}_T{}.jpg".format(tile_dir_acq_date, tile)),
#                                               os.path.join(newSubPath, "S2AGRI_L3B_PVI_A{}_T{}.jpg".format(tif_acq_date, tile))])
#                            
#                            # Finally, rename the folder
#                            run_command(["mv", fullTilePath, newSubPath])
#                    else :
#                        print ("WARNING: Directory {} contains no TIF file!!!".format(tileImgDataPath))
#
                
def main() :
    parser = argparse.ArgumentParser(description="Executes grassland mowing S2 detection")
    parser.add_argument('-c', '--config-file', default='/etc/sen2agri/sen2agri.conf', help="Sen4CAP system configuration file location")
    parser.add_argument('-s', '--site-id', help="The site id")
    parser.add_argument('-b', '--season-start', help="Season start")
    parser.add_argument('-e', '--season-end', help="Season end")
    parser.add_argument('-l', '--input-products-list', nargs='+', help="The list of L3B products")   
    parser.add_argument('-m', '--cp-rm', help="Perform cp and then rm instead of simple move (for Object storage)")   
    parser.add_argument('-v', '--verify_only', help="Perform only chek for rename operation")   
    
    args = parser.parse_args()
    
    s4cConfig = S4CConfig(args)
    
    print("Season_start = ", s4cConfig.season_start)
    print("Season_end = ", s4cConfig.season_end)
    
    with psycopg2.connect(host=s4cConfig.host, dbname=s4cConfig.dbname, user=s4cConfig.user, password=s4cConfig.password) as conn:
        process_ndvi_products(s4cConfig, conn, s4cConfig.site_id, args.input_products_list)
                       
if __name__== "__main__":
    main()                       
    
    
# /mnt/archive/ltu_2019/l3b_lai/S2AGRI_L3B_PRD_S2_20190606T092305_A20190519T092524/TILES            
