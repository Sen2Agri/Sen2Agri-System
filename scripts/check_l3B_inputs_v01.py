#!/usr/bin/env python

import argparse
import os
import pipes
import subprocess
import sys
import re
import shutil
import getopt
import datetime
from datetime import timedelta, date
import fnmatch
import psycopg2
from psycopg2.sql import SQL, Literal
import psycopg2.extras

from dateutil.relativedelta import relativedelta

import xml.etree.ElementTree
import logging

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
        self.path = args.input
        self.start_date = args.start_date
        self.end_date = args.stop_date
#        if args.start_date:
#            self.start_date = parse_date(args.start_date)
            
#        if args.stop_date:
#            self.stop_date = parse_date(args.stop_date)
            
        print ("Start date is {}".format(self.start_date))
        print ("Stop date is {}".format(self.end_date))
        
        log_dir=os.path.dirname(os.path.realpath(__file__))
        log_file=os.path.basename(__file__)+".log"
        
        logFilePath = log_dir+'/'+ log_file
        if os.path.exists(logFilePath):
            os.remove(logFilePath)
        logging.basicConfig(filename=logFilePath,level=logging.INFO)
        self.logging = logging

def list_1d(inFolder):
    return os.listdir(inFolder)

def run_command(args, env=None):
    args = list(map(str, args))
    cmd_line = " ".join(map(pipes.quote, args))
    print(cmd_line)
    subprocess.call(args, env=env)
    return

def get_granules_from_xml(xml_in):
    granules=[]
    e = xml.etree.ElementTree.parse(xml_in).getroot()
    for child in e.iter("Granules"):
        #print child.attrib.get("granuleIdentifier")
        granules.append(child.attrib.get("granuleIdentifier"))
    return granules

def get_inputs_from_xml(xml_in):
    granules=[]
    e = xml.etree.ElementTree.parse(xml_in).getroot()
    for child in e.iter("XML_files"):
        for child2 in child:
            #print child2.text
            granules.append(child2.text)
    return granules

def get_aux_files_from_ndvi_products_from_db(config, conn):
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

        site_id_filter = Literal(config.site_id)
        start_date_filter = Literal(config.start_date)
        end_date_filter = Literal(config.end_date)
        query = query.format(site_id_filter, start_date_filter, end_date_filter)
        # print(query.as_string(conn))
        cursor.execute(query)

        results = cursor.fetchall()
        conn.commit()

        products = []
        for (dt, tiles, full_path) in results:
            ndviTilePath = os.path.join(full_path, "AUX_DATA")
            acq_date = dt.strftime("%Y%m%dT%H%M%S")
            ndviTilePath = os.path.join(ndviTilePath, "S2AGRI_L3B_IPP_A{}.xml".format(acq_date))
            s2HdrFiles = getFilesFromIPPFile(config, ndviTilePath)
            products += s2HdrFiles

        return products

def get_l2a_products_from_db(config, conn):
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
                    and product.product_type_id = 1
            )
            select products.date,
                    products.tiles,
                    products.full_path
            from products
            where date between {} and {}
            order by date;
            """
        )

        site_id_filter = Literal(config.site_id)
        start_date_filter = Literal(config.start_date)
        end_date_filter = Literal(config.end_date)
        query = query.format(site_id_filter, start_date_filter, end_date_filter)
        # print(query.as_string(conn))
        cursor.execute(query)

        results = cursor.fetchall()
        conn.commit()

        products = []
        log_time=datetime.datetime.now().strftime("%Y-%m-%d %H-%M-%S")  
        for (dt, tiles, full_path) in results:
            #print("Full path: {}".format(full_path))
            files = os.listdir(full_path)
            l2aTilePaths = fnmatch.filter(files, "S2*_OPER_SSC_L2VALD_*.HDR")
            if len(l2aTilePaths) == 0:
                l2aTilePaths = fnmatch.filter(files, "L8_TEST_L8C_L2VALD_*.HDR")
                
            for file in l2aTilePaths:
                fullHdrFilePath = os.path.join(full_path, file)
                #print("Full HDR file path: {}".format(fullHdrFilePath))
                #config.logging.info('[%s] L2A HDR PATH from PRODUCT:[ %s ]',log_time,fullHdrFilePath)
                products.append(fullHdrFilePath)
            
        return products
        
def getFilesFromIPPFile(config, path) :
    #print ("IPP XML File: {}".format(path))
    prdName = path.split("/")[-3]

    l2aHdrPaths = []
    log_time=datetime.datetime.now().strftime("%Y-%m-%d %H-%M-%S")       
    #config.logging.info('[%s] prodName:[ %s ]',log_time,prdName)
    #config.logging.info('[%s] IPP XML File:[ %s ]',log_time,path)    
    for gDir in get_inputs_from_xml(path):
        
        if not os.path.isdir(config.path+"/"+gDir.split('/')[-2]):
            #print("L2A Product Name: {}".format(config.path + "/" + gDir.split('/')[-2]))
            #config.logging.info('[%s] InputName:[ %s ]',log_time,gDir.split('/')[-2])
            #config.logging.info('[%s] L3B IPP L2A InputPath:[ %s ]',log_time,gDir)
            #print("Full HDR file path from IPP: {}".format(gDir))
            l2aHdrPaths.append(gDir)
    
    return l2aHdrPaths
    
def getMissingL2AProducts(config, allL2AHdrFiles, l3bInputs) :  
    log_time=datetime.datetime.now().strftime("%Y-%m-%d %H-%M-%S")         
    missingS2Hdrs = [item for item in allL2AHdrFiles if item not in l3bInputs]
    missingProductPaths = []
    config.logging.info('[%s] MISSING PRODUCT PATHS : ',log_time)
    for missingHdr in missingS2Hdrs:
        elements = missingHdr.split('/')
        missingPrdPath = "/".join(elements[0:-1])
        missingPrdPath += "/"
        config.logging.info('%s',missingPrdPath)
        missingProductPaths.append(missingPrdPath)
    return missingProductPaths
    
def main():

    parser = argparse.ArgumentParser(description='Extracts the L2A for the processed NDVIs and returns also a list of missing L2A in NDVIs')
    parser.add_argument('--input', default="", help='input Folder')

    parser.add_argument('-c', '--config-file', default='/etc/sen2agri/sen2agri.conf', help="Configuration file location")
    parser.add_argument('-s', '--site-id', default=0, type=int, help="Site ID to filter by")
    
    parser.add_argument('-b', '--start_date', help='start-date: yyyy-mm-dd', default=(datetime.datetime.now() - relativedelta(years=5)).strftime('%Y-%m-%d'))
    parser.add_argument('-f', '--stop_date', help='stop_date: yyyy-mm-dd', default=datetime.datetime.now().strftime( "%Y-%m-%d"))

    args = parser.parse_args()
    
    config = Config(args)
    
    if config.site_id == 0 and config.path == "" :
        print("Please provide either site name of folders path!!!")
        return

    log_time=datetime.datetime.now().strftime("%Y-%m-%d %H-%M-%S")       

    if config.site_id == 0:
        config.logging.info('[%s]: Start check for %s',log_time,config.path)

        for (dirpath, _, files) in os.walk(config.path, followlinks=True):
            for element in fnmatch.filter(files, 'S2AGRI_L3B'+'*'):
                (base,ext)=os.path.splitext(element)
                base=base.lower()
                path=os.path.join(dirpath,element)
                if (re.search(r"AUX_DATA", path) != None) and ext.endswith(".xml"):
                    getFilesFromIPPFile(config, path)
    else :
        config.logging.info('[%s]: Start check for site with id %s',log_time,config.site_id)
        with psycopg2.connect(host=config.host, dbname=config.dbname, user=config.user, password=config.password) as conn:
            l2aFromNdvis = get_aux_files_from_ndvi_products_from_db(config, conn)
#            for l2aFromNdvi in l2aFromNdvis:
#                config.logging.info('[%s] Aux file:[ %s ]',log_time,l2aFromNdvi)
            allL2AFromDB = get_l2a_products_from_db(config, conn)
#            for l2aFromDB in allL2AFromDB:
#                config.logging.info('[%s] L2A prd file:[ %s ]',log_time,l2aFromDB)
                
            missingS2Hdrs = getMissingL2AProducts(config, allL2AFromDB, l2aFromNdvis)
#            for missingHdr in missingS2Hdrs:
#                config.logging.info('[%s] MISSING L2A HDR file:[ %s ]',log_time,missingHdr)
                
###################################################################################################
if __name__ == '__main__':
    sys.exit(main())
