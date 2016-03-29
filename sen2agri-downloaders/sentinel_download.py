#! /usr/bin/env python
# -*- coding: iso-8859-1 -*-

import glob,os,sys
import optparse
from xml.dom import minidom
import datetime
import subprocess
import pipes
import time
import signal
from sen2agri_common_db import *

#############################################################################
# CONSTANTS
MONTHS_FOR_REQUESTING_AFTER_SEASON_FINSIHED = int(2)
general_log_path = "/tmp/"
general_log_filename = "sentinel_download.log"

def unzipFile(outputDir, fileToUnzip):
    global general_log_path
    global general_log_filename
    retValue = False
    log(general_log_path, "fileToUnzip={}".format(fileToUnzip), general_log_filename)
    if (os.path.isfile(fileToUnzip)):
        if run_command(["zip", "--test", fileToUnzip], general_log_path, general_log_filename):
            log(general_log_path, "zip failed for {}".format(fileToUnzip), general_log_filename)
        else:
            create_recursive_dirs(outputDir)
            log(general_log_path, "zip OK for {}. Start to unzip".format(fileToUnzip), general_log_filename)
            if run_command(["unzip", "-d", outputDir,fileToUnzip], general_log_path, general_log_filename):
                log(general_log_path, "unziping failed for {}".format(fileToUnzip), general_log_filename)
            else:
                log(general_log_path, "unziping ok for {}".format(fileToUnzip), general_log_filename)
                retValue = True
    return retValue


###########################################################################
class Sentinel2Obj(object):
    def __init__(self, filename, link, product_date_as_string, cloud):
        self.filename = filename
        self.productname = filename
        self.link = link
        safeStringPos = filename.find(".SAFE")
        if safeStringPos != -1:
            self.productname = filename[:safeStringPos]
        self.product_date_as_string = product_date_as_string
        self.product_date = datetime.datetime.strptime(self.product_date_as_string, "%Y%m%dT%H%M%S")
#int(datatakeid.split("_")[1].split("T")[0])
        self.cloud = float(cloud)
    def __cmp__ (self, other):
        if hasattr(other, 'product_date'):
            return self.product_date.__cmp__(other.product_date)
    def __gt__ (self, other):
        return (isinstance(other, self.__class__)) and (self.product_date > other.product_date)
    def __lt__ (self, other):
        return (isinstance(other, self.__class__)) and (self.product_date < other.product_date)
    def __eq__ (self, other):
        return (isinstance(other, self.__class__)) and (self.product_date == other.product_date)
    def __ne__ (self, other):
        return not self.__eq__(other)


##########################################################################

def product_download(s2Obj, aoiContext, db):
    global general_log_filename
    if aoiContext.fileExists(s2Obj.filename):
        log(aoiContext.writeDir, "FILE ALREADY DOWNLOADED, SKIP IT! {}".format(s2Obj.filename), general_log_filename)
        return False
    log(aoiContext.writeDir, "Downloading from {} ".format(aoiContext.sentinelLocation), general_log_filename)

    if float(s2Obj.cloud) < float(aoiContext.maxCloudCoverage) and len(aoiContext.aoiTiles) > 0:
        commandArray = ["java", "-jar", os.path.dirname(os.path.abspath(__file__)) + "/S2ProductDownload-0.2.jar", "--out", aoiContext.writeDir, "--tiles"]
        for tile in aoiContext.aoiTiles:
            commandArray.append(tile)
        commandArray += ["-p", s2Obj.productname]
        if aoiContext.sentinelLocation == "scihub":
            commandArray.append("-u")
            uid = re.search(r"\('([\w-]+)'\)", s2Obj.link)
            if uid is None:
                print ("The provided link did not match in order to take the uid. Link: {}".format(s2Obj.link))
                return False
            commandArray.append(uid.group(1))
        elif aoiContext.sentinelLocation == "amazon":
            commandArray += ["-s", "AWS"]
        else:
            log(aoiContext.writeDir, "product_download: The location is not an expected one (scihub or amazon) for product {}".format(s2Obj.filename), general_log_filename)
            return False
        #TODO: insert the product name into the downloader_history
        abs_filename = "{}/{}".format(aoiContext.writeDir, s2Obj.filename)
        if not db.upsertSentinelProductHistory(aoiContext.siteId, s2Obj.filename, DATABASE_DOWNLOADER_STATUS_DOWNLOADING_VALUE, s2Obj.product_date_as_string, abs_filename, aoiContext.maxRetries):
            log(aoiContext.writeDir, "Couldn't upsert into database with status DOWNLOADING for {}".format(s2Obj.filename), general_log_filename)
            return False        
        if run_command(commandArray, aoiContext.writeDir, general_log_filename) != 0:
            #get the product name and check the no_of_retries. 
            #if it's greater than aoiContext.maxRetries, update the product name status with ABORTED (4)
            #else update the product name in the downloader_history with status FAILED (3) and increment the no_of_retries
            #this logic is performed by db.upsertSentinelProductHistory
            log(aoiContext.writeDir, "the java donwloader command didn't work for {}".format(s2Obj.filename), general_log_filename)
            if not db.upsertSentinelProductHistory(aoiContext.siteId, s2Obj.filename, DATABASE_DOWNLOADER_STATUS_FAILED_VALUE, s2Obj.product_date_as_string, abs_filename, aoiContext.maxRetries):
                log(aoiContext.writeDir, "Couldn't upsert into database with status FAILED for {}".format(s2Obj.filename), general_log_filename)
            return False
        #update the product name in the downloader_history with status DOWNLOADED (2)
        if not db.upsertSentinelProductHistory(aoiContext.siteId, s2Obj.filename, DATABASE_DOWNLOADER_STATUS_DOWNLOADED_VALUE, s2Obj.product_date_as_string, abs_filename, aoiContext.maxRetries):
            log(aoiContext.writeDir, "Couldn't upsert into database with status DOWNLOADED for {}".format(s2Obj.filename), general_log_filename)
        return False
    else:
        log(aoiContext.writeDir, "Too many clouds to download this product or no tiles to download".format(s2Obj.filename), general_log_filename)
    return True


###########################################################################

def downloadFromAmazon(s2Obj, aoiFile, db):
    global general_log_filename
    if aoiFile.fileExists(s2Obj.filename):
        log(aoiFile.writeDir, "FILE ALREADY DOWNLOADED, SKIP IT! {}".format(s2Obj.filename), general_log_filename)
        return
    log(aoiFile.writeDir, "Downloading from amazon ", general_log_filename)

    if float(s2Obj.cloud) < float(aoiFile.maxCloudCoverage) and len(aoiFile.aoiTiles) > 0:
        commandArray = ["java", "-jar", os.path.dirname(os.path.abspath(__file__)) + "/AWSS2ProductDownload-0.1.jar", "--out", aoiFile.writeDir, "--tiles"]
        for tile in aoiFile.aoiTiles:
            commandArray.append(tile)
        commandArray.append("-p")
        commandArray.append(s2Obj.productname)
        if run_command(commandArray, aoiFile.writeDir, general_log_filename) != 0:
            log(aoiFile.writeDir, "the java donwloader command didn't work for {}".format(s2Obj.filename), general_log_filename)
            return
        if not db.updateSentinelHistory(aoiFile.siteId, s2Obj.filename, "{}/{}".format(aoiFile.writeDir, s2Obj.filename)):
            log(aoiFile.writeDir, "Could not insert into database site_id {} the product name {}".format(aoiFile.siteId, s2Obj.filename), general_log_filename)
    else:
        log(aoiFile.writeDir, "Too many clouds to download this product or no tiles to download".format(s2Obj.filename), general_log_filename)

def downloadFromScihub(s2Obj, aoiFile, db):
    global general_log_filename
    unzipped_file_exists= os.path.exists(("%s/%s")%(aoiFile.writeDir, s2Obj.filename))
    fullFilename = aoiFile.writeDir+"/"+s2Obj.filename+".zip"
    if aoiFile.fileExists(s2Obj.filename):
        log(aoiFile.writeDir, "FILE ALREADY DOWNLOADED, SKIP IT! {}".format(s2Obj.filename), general_log_filename)
        if unzipped_file_exists == False and os.path.isfile(fullFilename) and options.no_download==False:
            outputDir = aoiFile.writeDir+"/"+s2Obj.filename
            unzipFile(outputDir, fullFilename)
        return
    if not create_recursive_dirs(aoiFile.writeDir):
        log(aoiFile.writeDir, "Could not create the output directory for {}".format(aoiFile.writeDir), general_log_filename)
        return

    #================================== download product

    if float(s2Obj.cloud) < float(aoiFile.maxCloudCoverage) :
        #commande_wget='%s %s --continue --tries=0 --output-document=%s/%s "%s"'%(wg, auth, aoiFile.writeDir, s2Obj.filename + ".zip", s2Obj.link)
        commande_wget=wg + auth + ["--continue", "--tries", "0", "--output-document", aoiFile.writeDir+s2Obj.filename+".zip", s2Obj.link]
        #do not download the product if it was already downloaded and unzipped, or if no_download option was selected.

        log(aoiFile.writeDir, "Command wget:{}".format(commande_wget), general_log_filename)
        if unzipped_file_exists==False and options.no_download==False:
            log(aoiFile.writeDir, "Start download:", general_log_filename)

            if run_command(commande_wget) != 0:
                #sys.exit(-1)
                log(aoiFile.writeDir, "wget command didn't work for {}".format(s2Obj.filename), general_log_filename)
                return
            #write the filename in history
            if not db.updateSentinelHistory(aoiFile.siteId, s2Obj.filename,  "{}/{}".format(aoiFile.writeDir, s2Obj.filename)):
                log(aoiFile.writeDir, "Could not insert into database site_id {} the product name {}".format(aoiFile.siteId, s2Obj.filename), general_log_filename)
            else:
                unzipFile(aoiFile.writeDir, fullFilename)
        else:
            log(aoiFile.writeDir, "No wget command because either it's already downloaded and unzipped, either the no_download option was used", general_log_filename)
    else:
        log(aoiFile.writeDir, "Too many clouds to download this product".format(s2Obj.filename), general_log_filename)


###########################################################################

signal.signal(signal.SIGINT, signal_handler)

def sentinel_download(aoiContext):
    global g_exit_flag
    global general_log_filename

    url_search="https://scihub.esa.int/apihub/search?q="
    general_log_filename = "sentinel_download.log"
    general_log_path = aoiContext.writeDir    
    apihubFile = aoiContext.remoteSiteCredentials

    # read password file
    try:
        f = file(apihubFile)
        (account,passwd) = f.readline().split(' ')
        if passwd.endswith('\n'):
            passwd=passwd[:-1]
        f.close()
    except :
        log(aoiContext.writeDir, "error with password file ".format(str(apihubFile)), general_log_filename)
        return

    db = SentinelAOIInfo(aoiContext.configObj.host, aoiContext.configObj.database, aoiContext.configObj.user, aoiContext.configObj.password)

    #==================================================
    #      prepare wget command line to search catalog
    #==================================================

    wg = ["wget", "--no-check-certificate"]
    auth = ["--user",account, "--password", passwd]

    query_geom = 'footprint:"Intersects({})"'.format(aoiContext.polygon)
    queryResultsFilename = aoiContext.writeDir+"/"+str(aoiContext.siteName)+"_query_results.xml"
    print("queryResultsFilename = {}".format(queryResultsFilename))
    search_output = ["--output-document", queryResultsFilename]
    query = "{} filename:S2A*".format(query_geom)

    start_date=str(aoiContext.startSeasonYear)+"-"+str(aoiContext.startSeasonMonth)+"-"+str(aoiContext.startSeasonDay)+"T00:00:00.000Z"
    end_date=str(aoiContext.endSeasonYear)+"-"+str(aoiContext.endSeasonMonth)+"-"+str(aoiContext.endSeasonDay)+"T23:59:50.000Z"

    query_date = " ingestiondate:[{} TO {}]".format(start_date, end_date)
    query = "{}{}".format(query, query_date)

    commande_wget = wg + auth + search_output
    commande_wget.append(url_search + query + "&rows=1000")
    log(aoiContext.writeDir, commande_wget, general_log_filename)

    if run_command(commande_wget, aoiContext.writeDir, general_log_filename) != 0:
        log(aoiContext.writeDir, "Could not get the catalog output for {}".format(query_geom), general_log_filename)
        return
    if g_exit_flag:
        return

    #=======================
    # parse catalog output
    #=======================
    if not os.path.isfile(queryResultsFilename):
        log(aoiContext.writeDir, "Could not find the catalog output {} for {}".format(queryResultsFilename, aoiContext.siteName), general_log_filename)
        return
    xml=minidom.parse(queryResultsFilename)

    #check if all the results are returned
    subtitle=xml.getElementsByTagName("subtitle")[0].firstChild.data
    log(aoiContext.writeDir, "Subtitle = {}".format(subtitle), general_log_filename)
    if len(subtitle) > 0:
        words=subtitle.split(" ")
        log(aoiContext.writeDir, "WORDS[2]: {}".format(words[2]), general_log_filename)
        if len(words) > 2 and words[2] != "results." and len(words) > 5:
            try:
                totalRes=int(words[5]) + 1
                query = query + "&rows=" + str(totalRes)
                #commande_wget='%s %s %s "%s%s"'%(wg,auth,search_output,url_search,query)
                commande_wget = wg + auth + search_output + [url_search+query]
                log(aoiContext.writeDir, "Changing the pagination to {0} to get all of the existing files, command: {1}".format(totalRes, commande_wget), general_log_filename)
                if run_command(commande_wget, aoiContext.writeDir, general_log_filename) != 0:
                    log(aoiContext.writeDir, "Could not get the catalog output (re-pagination) for {}".format(query_geom), general_log_filename)
                    return
                xml=minidom.parse("query_results.xml")
            except ValueError:
                log(aoiContext.writeDir, "Exception: it was expected for the word in position 5 (starting from 0) to be an int. It ain't. The checked string is: {}".format(subtitle), general_log_filename)
                log(aoiContext.writeDir, "Could not get the catalog output (exception for re-pagination) for {}".format(query_geom), general_log_filename)
                return

    products=xml.getElementsByTagName("entry")
    s2Objs = []
    for prod in products:
        ident = prod.getElementsByTagName("id")[0].firstChild.data
        link = prod.getElementsByTagName("link")[0].attributes.items()[0][1]
        filename = ""
        cloud = float(200)
        product_date = None
        for node in prod.getElementsByTagName("str"):
            (name,value)=node.attributes.items()[0]
            if value=="filename":
                filename= str(node.toxml()).split('>')[1].split('<')[0]   #ugly, but minidom is not straightforward

        for node in prod.getElementsByTagName("double"):
            (name,value)=node.attributes.items()[0]
            if value=="cloudcoverpercentage":
                cloud=float((node.toxml()).split('>')[1].split('<')[0])
        
        product_date = re.search(r"_V(\d{8}T\d{6})_", filename)
        if len(filename) == 0 or cloud > 100 or product_date == None:
            continue
        
        s2Objs.append(Sentinel2Obj(filename, link, product_date.group(1), cloud))

    #sort the products using product date
    s2Objs.sort()

    #handle each object
    for s2Obj in s2Objs:
        print("===============================================")
        print(s2Obj.filename)
        print("Date:{}".format(s2Obj.product_date))
        print(s2Obj.link)
        print("cloud percentage in product  = {}".format(s2Obj.cloud))
        print("max allowed cloud percentage = {}".format(aoiContext.maxCloudCoverage))
        print("date as string {}".format(s2Obj.product_date_as_string))
        print("product name {}".format(s2Obj.productname))
        print("===============================================")

        if g_exit_flag:
            return
        product_download(s2Obj, aoiContext, db)
        print("Finished to download product: {}".format(s2Obj.productname))
        
