#!/usr/bin/env python
from __future__ import print_function

import argparse
import csv
from collections import defaultdict
from datetime import date
from glob import glob
import multiprocessing.dummy
import os
import os.path
import sys
import pipes
import subprocess

import psycopg2
from psycopg2.sql import SQL, Literal, Identifier
import psycopg2.extras

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

def get_practice(name):
    if name == "NA":
        return PRACTICE_NA
    elif name == "CatchCrop":
        return PRACTICE_CATCH_CROP
    elif name == "NFC":
        return PRACTICE_NFC
    elif name == "Fallow":
        return PRACTICE_FALLOW
    else:
        return None

def is_int(s):
    try: 
        int(s)
        return True
    except ValueError:
        return False
        
def get_product_id_by_full_path(conn, prd_full_path):
    product_id = -1
    with conn.cursor() as cursor:
        query = SQL(
            """ select id from product where full_path = {} """
        )
        query = query.format(Literal(prd_full_path))
        print(query.as_string(conn))

        cursor.execute(query)
        for row in cursor:
            product_id = row[0]
        conn.commit()
    return product_id

def get_product_id_by_name(conn, prd_name):
    product_id = -1
    with conn.cursor() as cursor:
        query = SQL(
            """ select id from product where name = {} """
        )
        query = query.format(Literal(prd_name))
        print(query.as_string(conn))

        cursor.execute(query)
        for row in cursor:
            product_id = row[0]
        conn.commit()
    return product_id

def get_product_id(conn, prd_descr):
    if is_int(prd_descr):
        return prd_descr
    if os.path.isdir(prd_descr) : 
        return get_product_id_by_full_path(conn, prd_descr)
    else :
        return get_product_id_by_name(conn, prd_descr)

def get_product_path(conn, prd_id):
    product_path = ''
    with conn.cursor() as cursor:
        query = SQL(
            """ select full_path from product where id = {} """
        )
        query = query.format(Literal(prd_id))
        print(query.as_string(conn))

        cursor.execute(query)
        for row in cursor:
            product_path = row[0]
        conn.commit()
    return product_path

def clean_product_details_l4c(conn, id):
    with conn.cursor() as cursor:
        query = SQL(
            """ delete from product_details_l4c where product_id = {} """
        ).format(Literal(id))
        print(query.as_string(conn))
        cursor.execute(query)
        conn.commit()    

def run_command(args, env=None):
    args = list(map(str, args))
    cmd_line = " ".join(map(pipes.quote, args))
    print(cmd_line)
    subprocess.call(args, env=env)
    #os.system(cmd_line)

def run_import_details_cmd(prd_id):
    command = []
    command += ["import-product-details.py", "-p"]
    command += [prd_id]
    run_command(command)

def run_export_product_cmd(prd_id, out_file_path):
    command = []
    command += ["export-product.py", "-p"]
    command += [prd_id]
    command += [out_file_path]
    run_command(command)
    
def create_prd_shp_files(vectDataDirPath):    
    print ("Checking files in {}".format(vectDataDirPath))        
    for f in glob(os.path.join(vectDataDirPath,'*.gpkg')):  
        gpkgFileName = os.path.splitext(os.path.basename(f))[0]
        ogr2ogr_cmd = ["ogr2ogr"]
        ogr2ogr_cmd += [os.path.join(vectDataDirPath, gpkgFileName + '.shp'),
                        os.path.join(vectDataDirPath, gpkgFileName + '.gpkg')]
        ogr2ogr_cmd += ["-lco", "ENCODING=UTF-8"]
        run_command(ogr2ogr_cmd)

def delete_existing_gpkg_files(vectDataDirPath):
    for file in glob(os.path.join(vectDataDirPath,'*.gpkg')):
        print("Deleting gpkg file: {}".format(file))
        os.remove(file) 

def move_shp_files(vectDataDirPath):
    vectShpDataDirPath = os.path.join(vectDataDirPath, "SHP")
    extensions = ["shp", "shx", "prj", "cpg", "dbf"]
    try:
        os.mkdir(vectShpDataDirPath)
    except OSError:
        print ("Creation of the directory %s failed" % vectShpDataDirPath)
    else:
        print ("Successfully created the directory %s" % vectShpDataDirPath)
        
    for ext in extensions:
        for file in glob(os.path.join(vectDataDirPath,'*.' + ext)):
            print("Moving product shp file: {}".format(file))
            os.rename(file, vectShpDataDirPath)        

def main():
    parser = argparse.ArgumentParser(description="Imports product contents into the database")
    parser.add_argument('-c', '--config-file', default='/etc/sen2agri/sen2agri.conf', help="Configuration file location")
    parser.add_argument('-f', '--product-ids-file', help="product ids file")
    parser.add_argument('-o', '--out', default="", help="Out file name")

    args = parser.parse_args()
    
    config = Config(args)
    
    file=open( args.product_ids_file, "r")
    reader = csv.reader(file, delimiter=";")
    for line in reader:
        if len(line) > 0 :
            with psycopg2.connect(host=config.host, dbname=config.dbname, user=config.user, password=config.password) as conn:
                prdId = get_product_id(conn, line[0])
                prdPath = get_product_path(conn, prdId)
                vectDataDirPath = os.path.join(prdPath, "VECTOR_DATA")
                
                # Cleanup the product_details_l4c table if the product was previusly added
                clean_product_details_l4c(conn, prdId)
                
                # Delete existing gpkg files in the directory
                delete_existing_gpkg_files(vectDataDirPath)
                
                # Import (again) the product into the product_details_l4c table
                run_import_details_cmd(prdId)
                
                # Export the product as gpkg files
                vectDataPath = os.path.join(vectDataDirPath, args.out)
                run_export_product_cmd(prdId, vectDataPath)
                
                # create shp files from gpkg files
                create_prd_shp_files(vectDataDirPath)
                
                # move the shp files to the SHP subfolder
                move_shp_files(vectDataDirPath)

        else :
            print ("Error reading product id and path from the file {}".format(args.product_ids_file))
            exit(1)
                
if __name__ == "__main__":
    sys.exit(main())
