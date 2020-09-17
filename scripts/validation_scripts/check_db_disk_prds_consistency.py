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

class L2AProduct(object):
    def __init__(self, prdFullPath, prdRelPath, prdMetadataFullPath):
        self.prdFullPath = prdFullPath
        self.prdRelPath =  prdRelPath;
        self.prdMetadataFullPath = prdMetadataFullPath
    
class Config(object):
    def __init__(self, args):
        parser = ConfigParser()
        parser.read([args.config_file])

        self.host = parser.get("Database", "HostName")
        self.dbname = parser.get("Database", "DatabaseName")
        self.user = parser.get("Database", "UserName")
        self.password = parser.get("Database", "Password")

        self.site_id = args.site_id
        self.start_date = args.start_date
        self.end_date = args.stop_date
        self.dwn_hist_check = args.dwn_hist_check
            
        print ("Start date is {}".format(self.start_date))
        print ("Stop date is {}".format(self.end_date))

def run_command(args, env=None):
    args = list(map(str, args))
    cmd_line = " ".join(map(pipes.quote, args))
    print(cmd_line)
    subprocess.call(args, env=env)
    return

def check_products_from_db(config, conn):
    with conn.cursor() as cursor:
        query = SQL(
            """
            select full_path from product
            where site_id = {} and created_timestamp between {} and {}
            order by created_timestamp asc;
            """
        )
        query = query.format(Literal(config.site_id), Literal(config.start_date), Literal(config.end_date))
        print(query.as_string(conn))
        cursor.execute(query)

        results = cursor.fetchall()
        conn.commit()

        for (full_path,) in results:
            prdFullPath = full_path
            if (not os.path.isdir(prdFullPath)) and (not os.path.isfile(prdFullPath)) : 
                print("Product {} does not exist on disk!".format(prdFullPath))        

def check_dwn_history_prds(config, conn):
    with conn.cursor() as cursor:
        query = SQL(
            """
            select full_path, status_id from downloader_history
            where site_id = {} and product_date between {} and {}
            order by product_date asc;
            """
        )
        query = query.format(Literal(config.site_id), Literal(config.start_date), Literal(config.end_date))
        print(query.as_string(conn))
        cursor.execute(query)

        results = cursor.fetchall()
        conn.commit()

        for (full_path,status_id) in results:
            prdFullPath = full_path
            if ((not os.path.isdir(prdFullPath))) and (not os.path.isfile(prdFullPath)) : 
                print("Product {} with status id = {} does not exist on disk!".format(prdFullPath, status_id))        

def main():

    parser = argparse.ArgumentParser(description='Check if the products in the DB for a site exist on disk')

    parser.add_argument('-c', '--config-file', default='/etc/sen2agri/sen2agri.conf', help="Configuration file location")
    parser.add_argument('-s', '--site-id', default=0, type=int, help="Site ID to filter by")
    
    parser.add_argument('-b', '--start_date', help='start-date: yyyy-mm-dd', default=(datetime.datetime.now() - relativedelta(years=5)).strftime('%Y-%m-%d'))
    parser.add_argument('-f', '--stop_date', help='stop_date: yyyy-mm-dd', default=datetime.datetime.now().strftime( "%Y-%m-%d"))
    parser.add_argument('-d', '--dwn-hist-check', default=0, type=int, help='Downloader history products check. 0 - do not check, 1 - check')

    args = parser.parse_args()
    
    config = Config(args)
    
    if config.site_id == 0 :
        print("Please provide either site name of folders path!!!")
        return

    with psycopg2.connect(host=config.host, dbname=config.dbname, user=config.user, password=config.password) as conn:
        check_products_from_db(config, conn)
        if config.dwn_hist_check == 1 :
            check_dwn_history_prds(config, conn)
                
###################################################################################################
if __name__ == '__main__':
    sys.exit(main())
