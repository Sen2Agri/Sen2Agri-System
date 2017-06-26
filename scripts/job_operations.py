#!/usr/bin/env python
from __future__ import print_function
import argparse
import re
import glob
import gdal
import osr
import subprocess
import lxml.etree
from lxml.builder import E
import math
import os
from os.path import isfile, isdir, join
import glob
import sys
import time, datetime
from time import gmtime, strftime
import pipes
import shutil
import psycopg2
import psycopg2.errorcodes
import optparse
from osgeo import ogr

SENTINEL2_SATELLITE_ID = int(1)
LANDSAT8_SATELLITE_ID = int(2)
UNKNOWN_SATELLITE_ID = int(-1)
general_log_filename = "log.log"

DEBUG = 1

def log(location, info, log_filename = None):
    if log_filename == None:
        log_filename = "log.txt"
    try:
        logfile = os.path.join(location, log_filename)
        if DEBUG:
            #print("logfile: {}".format(logfile))
            print("{}:{}".format(str(datetime.datetime.now()), str(info)))
            sys.stdout.flush()
        log = open(logfile, 'a')
        log.write("{}:{}\n".format(str(datetime.datetime.now()),str(info)))
        log.close()
    except:
        print("Could NOT write inside the log file {}".format(logfile))
        sys.stdout.flush()

def get_bool_value(value):
    boolStr = str(value).lower()
    if (boolStr == "true") or (boolStr == "yes") or (boolStr == "y") or (boolStr == "t") or (boolStr == "1"):
        return True
    return False
        
def delete_jobs(job_ids) :
    # cleanup the database
    l2a_db.delete_jobs(job_ids)

def cancel_jobs(job_ids) :
    # cleanup the database
    l2a_db.cancel_jobs(job_ids)

def pause_jobs(job_ids) :
    # cleanup the database
    l2a_db.pause_jobs(job_ids)

def resume_jobs(job_ids) :
    # cleanup the database
    l2a_db.resume_jobs(job_ids)
    
###########################################################################
class Config(object):
    def __init__(self):
        self.host = ""
        self.database = ""
        self.user = ""
        self.password = ""
    def loadConfig(self, configFile):
        try:
            with open(configFile, 'r') as config:
                found_section = False
                for line in config:
                    line = line.strip(" \n\t\r")
                    if found_section and line.startswith('['):
                        break
                    elif found_section:
                        elements = line.split('=')
                        if len(elements) == 2:
                            if elements[0].lower() == "hostname":
                                self.host = elements[1]
                            elif elements[0].lower() == "databasename":
                                self.database = elements[1]
                            elif elements[0].lower() == "username":
                                self.user = elements[1]
                            elif elements[0].lower() == "password":
                                self.password = elements[1]
                            else:
                                print("Unkown key for [Database] section")
                        else:
                            print("Error in config file, found more than on keys, line: {}".format(line))
                    elif line == "[Database]":
                        found_section = True
        except:
            print("Error in opening the config file ".format(str(configFile)))
            return False
        if len(self.host) <= 0 or len(self.database) <= 0:
            return False
        return True

###########################################################################
class L2AInfo(object):
    def __init__(self, server_ip, database_name, user, password, log_file=None):
        self.server_ip = server_ip
        self.database_name = database_name
        self.user = user
        self.password = password
        self.is_connected = False;
        self.log_file = log_file

    def database_connect(self):
        if self.is_connected:
            return True
        connectString = "dbname='{}' user='{}' host='{}' password='{}'".format(self.database_name, self.user, self.server_ip, self.password)
        try:
            self.conn = psycopg2.connect(connectString)
            self.cursor = self.conn.cursor()
            self.is_connected = True
        except:
            print("Unable to connect to the database")
            exceptionType, exceptionValue, exceptionTraceback = sys.exc_info()
            # Exit the script and print an error telling what happened.
            print("Database connection failed!\n ->{}".format(exceptionValue))
            self.is_connected = False
            return False
        return True

    def database_disconnect(self):
        if self.conn:
            self.conn.close()
            self.is_connected = False

    def get_config_key(self, key):
        if not self.database_connect():
            return ""
        cmd = "select value from config where key = '{}'".format(key)
        try:
            self.cursor.execute(cmd)
            rows = self.cursor.fetchall()
        except:
            print("Unable to execute command: {}".format(cmd))
            self.database_disconnect()
            return ""
        self.database_disconnect()
        return rows[0][0]
            
    def get_site_names(self):
        if not self.database_connect():
            return ""
        try:
            self.cursor.execute("select short_name from site")
            rows = self.cursor.fetchall()
        except:
            print("Unable to execute select short_name from site")
            self.database_disconnect()
            return ""
        self.database_disconnect()
        return [item[0] for item in rows]

    def get_site_id(self, short_name):
        if not self.database_connect():
            return ""
        try:
            self.cursor.execute("select id from site where short_name='{}'".format(short_name))
            rows = self.cursor.fetchall()
        except:
            print("Unable to execute select id from site")
            self.database_disconnect()
            return ""
        self.database_disconnect()
        return rows[0][0]

    def executeSqlSelectCmd(self, cmd):
        if not self.database_connect():
            return ""
        try:
            self.cursor.execute(cmd)
            rows = self.cursor.fetchall()
        except:
            print("Unable to execute command {}".format(cmd))
            self.database_disconnect()
            return ""
        self.database_disconnect()
        return [item[0] for item in rows]
        
    
    def executeSqlDeleteCmd(self, cmd, disconnectOnExit=False):
        if not self.database_connect():
            return False
        try:
            print("Executing SQL command: {}".format(cmd))
            self.cursor.execute(cmd)
            self.conn.commit()
        except Exception, e:
            print("Database update query failed: {}".format(e))
            self.database_disconnect()
            return False
        
        if disconnectOnExit :
            print("All done OK. Disconnecting from database.")
            self.database_disconnect()
        
        return True
    
    def delete_jobs(self, jobIds):
        jobsStr = ','.join(map(str, jobIds))
#        print("JOBS1: {}".format(jobsStr))
#        jobs = self.executeSqlSelectCmd("select * from job where id in ({})".format(jobsStr))
#        print("JOBS: {}".format(jobs))
        
        #print("The following JOBS will be stopped: {}".format(jobsStr))
        
        # delete the steps for this site
        self.executeSqlDeleteCmd("delete from step where task_id in (select id from task where job_id in ({}))".format(jobsStr))
        
        # delete the steps ressource logs for tasks in jobs for this site
        self.executeSqlDeleteCmd("delete from step_resource_log where task_id in (select id from task where job_id in ({}))".format(jobsStr))
        
        # delete the tasks for this site
        self.executeSqlDeleteCmd("delete from task where job_id in ({})".format(jobsStr))
        
        # delete the config job the entries for the jobs of this site
        self.executeSqlDeleteCmd("delete from config_job where job_id in ({})".format(jobsStr))
        
        # now finally remove the jobs
        self.executeSqlDeleteCmd("delete from job where id in ({})".format(jobsStr))

    def cancel_jobs(self, jobIds):
        # delete the steps for this site
        for jobId in jobIds : 
            print("Cancelling job: {}".format(jobId))
            self.executeSqlDeleteCmd("set transaction isolation level repeatable read; select from sp_mark_job_canceled({})".format(jobId))

    def pause_jobs(self, jobIds):
        # delete the steps for this site
        for jobId in jobIds : 
            print("Pausing job: {}".format(jobId))
            self.executeSqlDeleteCmd("set transaction isolation level repeatable read; select from sp_mark_job_paused({})".format(jobId))
            
    def resume_jobs(self, jobIds):
        # delete the steps for this site
        for jobId in jobIds : 
            print("Resuming job: {}".format(jobId))
            self.executeSqlDeleteCmd("set transaction isolation level repeatable read; select from sp_mark_job_resumed({})".format(jobId))

parser = argparse.ArgumentParser(
    description="Script for stopping one or several jobs")
parser.add_argument('-c', '--config', default="/etc/sen2agri/sen2agri.conf", help="configuration file")
parser.add_argument('-j', '--job_ids', help="The Job id to be stopped")
parser.add_argument('-o', '--operation', help="The operation to be performed on the jobs (delete/cancel/pause/resume")

args = parser.parse_args()

config = Config()
if not config.loadConfig(args.config):
    sys.exit("Could not load the config from configuration file {}".format(args.config))

l2a_db = L2AInfo(config.host, config.database, config.user, config.password)

if (not args.job_ids):
    sys.exit("Please provide the job ids -j or --job_ids!")
    
if (not args.operation):
    sys.exit("Please provide the operation -o or --operation with one of the values: delete, cancel, pause or resume!")

jobIds = []    
jobStrIds = args.job_ids.split(', ')
for jobIdStr in jobStrIds:
    jobIds.append(int(jobIdStr))
    
jobsOperation = args.operation.lower
if args.operation == "delete" : 
    print("Deleting jobs: {}".format(jobIds))
    delete_jobs(jobIds)
    
elif args.operation == "cancel" : 
    print("Cancelling jobs: {}".format(jobIds))
    cancel_jobs(jobIds)

elif args.operation == "pause" : 
    print("Pausing jobs: {}".format(jobIds))
    pause_jobs(jobIds)

elif args.operation == "resume" : 
    print("Resuming jobs: {}".format(jobIds))
    resume_jobs(jobIds)
else:
    sys.exit("lease provide the operation -o or --operation with one of the values: delete, cancel, pause or resume!")



    
    
    
