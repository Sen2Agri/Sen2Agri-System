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
from common_download import *


#############################################################################
# CONSTANTS
MONTHS_FOR_REQUESTING_AFTER_SEASON_FINSIHED = int(2)
global exitFlag

def unzipFile(outputDir, fileToUnzip):
    retValue = False
    print("fileToUnzip={}".format(fileToUnzip))
    if (os.path.isfile(fileToUnzip)):
        if runCmd(["zip", "--test", fileToUnzip], False):
            print("zip failed for {}".format(fileToUnzip))
        else:
            createRecursiveDirs(outputDir)
            print("zip OK for {}. Start to unzip".format(fileToUnzip))
            if runCmd(["unzip", "-d", outputDir,fileToUnzip], False):
                print("unziping failed for {}".format(fileToUnzip))
            else:
                print("unziping ok for {}".format(fileToUnzip))
                retValue = True
    return retValue

def signal_handler(signal, frame):
    global exitFlag
    print("SIGINT caught")
    exitFlag = True
    sys.exit(0)

###########################################################################
class Sentinel2Obj(object):
    def __init__(self, filename, link, datatakeid, cloud):
        self.filename = filename
        self.productname = filename
        self.link = link
        safeStringPos = filename.find(".SAFE")
        if safeStringPos != -1:
            self.productname = filename[:safeStringPos]
        self.datatakeid = datatakeid
        self.dateAsInt = int(datatakeid.split("_")[1].split("T")[0])
        self.cloud = float(cloud)
    def __cmp__ (self, other):
        if hasattr(other, 'dateAsInt'):
            return self.dateAsInt.__cmp__(other.dateAsInt)
    def __gt__ (self, other):
        return (isinstance(other, self.__class__)) and (self.dateAsInt > other.dateAsInt)
    def __lt__ (self, other):
        return (isinstance(other, self.__class__)) and (self.dateAsInt < other.dateAsInt)
    def __eq__ (self, other):
        return (isinstance(other, self.__class__)) and (self.dateAsInt == other.dateAsInt)
    def __ne__ (self, other):
        return not self.__eq__(other)

###########################################################################

def downloadFromAmazon(s2Obj, aoiFile, db):
    if aoiFile.fileExists(s2Obj.filename):
        log(aoiFile.writeDir, "FILE ALREADY DOWNLOADED, SKIP IT! {}".format(s2Obj.filename))
        return
    log(aoiFile.writeDir, "Downloading from amazon ")

    if float(s2Obj.cloud) < float(aoiFile.maxCloudCoverage) and len(aoiFile.aoiTiles) > 0:
        commandArray = ["java", "-jar", "AWSS2ProductDownload-0.1.jar", "--out", aoiFile.writeDir, "--tiles"]
        for tile in aoiFile.aoiTiles:
            commandArray.append(tile)
        commandArray.append("-p")
        commandArray.append(s2Obj.productname)
        if runCmd(commandArray, False) != 0:
            log(aoiFile.writeDir, "the java donwloader command didn't work for {}".format(s2Obj.filename))
            return
        print ("startUPDATE")
        if not db.updateSentinelHistory(aoiFile.siteId, s2Obj.filename, "{}/{}".format(aoiFile.writeDir, s2Obj.filename)):
            log(aoiFile.writeDir, "Could not insert into database site_id {} the product name {}".format(aoiFile.siteId, s2Obj.filename))
        print ("stopUPDATE")
    else:
        log(aoiFile.writeDir, "Too many clouds to download this product or no tiles to download".format(s2Obj.filename))

def downloadFromScihub(s2Obj, aoiFile, db):
    unzipped_file_exists= os.path.exists(("%s/%s")%(aoiFile.writeDir, s2Obj.filename))
    fullFilename = aoiFile.writeDir+"/"+s2Obj.filename+".zip"
    if aoiFile.fileExists(s2Obj.filename):
        log(aoiFile.writeDir, "FILE ALREADY DOWNLOADED, SKIP IT! {}".format(s2Obj.filename))
        if unzipped_file_exists == False and os.path.isfile(fullFilename) and options.no_download==False:
            outputDir = aoiFile.writeDir+"/"+s2Obj.filename
            unzipFile(outputDir, fullFilename)
        return
    if not createRecursiveDirs(aoiFile.writeDir):
        log(aoiFile.writeDir, "Could not create the output directory for {}".format(aoiFile.writeDir))
        return

    #==================================download product

    if float(s2Obj.cloud) < float(aoiFile.maxCloudCoverage) :
        #commande_wget='%s %s --continue --tries=0 --output-document=%s/%s "%s"'%(wg, auth, aoiFile.writeDir, s2Obj.filename + ".zip", s2Obj.link)
        commande_wget=wg + auth + ["--continue", "--tries", "0", "--output-document", aoiFile.writeDir+s2Obj.filename+".zip", s2Obj.link]
        #do not download the product if it was already downloaded and unzipped, or if no_download option was selected.

        log(aoiFile.writeDir, "Command wget:{}".format(commande_wget))
        if unzipped_file_exists==False and options.no_download==False:
            log(aoiFile.writeDir, "Start download:")

            if runCmd(commande_wget, False) != 0:
                #sys.exit(-1)
                log(aoiFile.writeDir, "wget command didn't work for {}".format(s2Obj.filename))
                return
            #write the filename in history
            if not db.updateSentinelHistory(aoiFile.siteId, s2Obj.filename,  "{}/{}".format(aoiFile.writeDir, s2Obj.filename)):
                log(aoiFile.writeDir, "Could not insert into database site_id {} the product name {}".format(aoiFile.siteId, s2Obj.filename))
            else:
                unzipFile(aoiFile.writeDir, fullFilename)
        else:
            log(aoiFile.writeDir, "No wget command because either it's already downloaded and unzipped, either the no_download option was used")
    else:
        log(aoiFile.writeDir, "Too many clouds to download this product".format(s2Obj.filename))


###########################################################################

url_search="https://scihub.esa.int/apihub/search?q="
#url_search="https://131.176.236.19//apihub/search?q="

#==================
#parse command line
#==================
aoiFiles = []
apihubFile = ""
exitFlag = False
signal.signal(signal.SIGINT, signal_handler)
if len(sys.argv) == 1:
    prog = os.path.basename(sys.argv[0])
    print '      '+sys.argv[0]+' [options]'
    print "     Help : ", prog, " --help"
    print "        or : ", prog, " -h"
    print ("example {} -i /path/to/input/directory -a apihub.txt (scene)".format(sys.argv[0]))
    sys.exit(-1)
else:
    usage = "usage: %prog [options] "
    parser = OptionParser(usage=usage)

    parser.add_option("-p","--proxy_passwd", dest="proxy", action="store", type="string", \
            help="Proxy account and password file",default=None)
    parser.add_option("-n","--no_download", dest="no_download", action="store_true",  \
            help="Do not download products, just print wget command",default=False)
    parser.add_option("-a","--apihub", dest="apihub", action="store",type="string",  \
            help="File with credentials for the scihub server",default='./apihub.txt')
    parser.add_option("-c","--config", dest="config", action="store",type="string",  \
                      help="File with credentials for the database",default="/etc/sen2agri/sen2agri.conf")
    parser.add_option("-l","--location", dest="location", action="store",type="string",  \
            help="The location from where the product should be donwloaded: scihub or amazon",default='None')

    (options, args) = parser.parse_args()

    parser.check_required("-c")
    configFile = options.config
    apihubFile = options.apihub
    fullFilename = os.path.realpath(__file__)
    dirname = fullFilename[0:fullFilename.rfind('/') + 1]
    config = Config()
    if not config.loadConfig(options.config):
        print("Could not load the config")
        sys.exit(-1)
    db = SentinelAOIInfo(config.host, config.database, config.user, config.password)
    aoiDatabase = db.getSentinelAOI()
    print("------------------------")
    for aoi in aoiDatabase:
        aoi.printInfo()
        print("------------------------")

    if len(aoiDatabase) <= 0:
        print("Could not get DB info")
        sys.exit(-1)


#====================
# read password file
#====================
try:
    f=file(apihubFile)
    (account,passwd)=f.readline().split(' ')
    if passwd.endswith('\n'):
        passwd=passwd[:-1]
    f.close()
except :
    print("error with password file ".format(str(apihubFile)))
    sys.exit(-2)


#==================================================
#      prepare wget command line to search catalog
#==================================================

wg = ["wget", "--no-check-certificate"]
auth = ["--user",account, "--password", passwd]

for aoiFile in aoiDatabase:
    query_geom = 'footprint:"Intersects({})"'.format(aoiFile.polygon)
    queryResultsFilename = aoiFile.writeDir+"/"+str(aoiFile.siteName)+"_query_results.xml"
    search_output = ["--output-document", queryResultsFilename]
    query='%s filename:S2A*'%(query_geom)
    createRecursiveDirs(aoiFile.writeDir)

    currentMonth = datetime.date.today().month
    startSeasonMonth = int(0)
    startSeasonDay = int(0)
    endSeasonMonth = int(0)
    endSeasonDay = int(0)
    # first position is the startSeasonYear, the second is the endPositionYear
    currentYearArray = []
    print("SITE NAME:{}".format(aoiFile.siteName))
    if checkIfSeason(aoiFile.startSummerSeason, aoiFile.endSummerSeason, MONTHS_FOR_REQUESTING_AFTER_SEASON_FINSIHED, currentYearArray, aoiFile.writeDir):
        startSeasonMonth = int(aoiFile.startSummerSeason[0:2])
        startSeasonDay = int(aoiFile.startSummerSeason[2:4])
        endSeasonMonth = int(aoiFile.endSummerSeason[0:2])
        endSeasonDay = int(aoiFile.endSummerSeason[2:4])
    elif checkIfSeason(aoiFile.startWinterSeason, aoiFile.endWinterSeason, MONTHS_FOR_REQUESTING_AFTER_SEASON_FINSIHED, currentYearArray, aoiFile.writeDir):
        startSeasonMonth = int(aoiFile.startWinterSeason[0:2])
        startSeasonDay = int(aoiFile.startWinterSeason[2:4])
        endSeasonMonth = int(aoiFile.endWinterSeason[0:2])
        endSeasonDay = int(aoiFile.endWinterSeason[2:4])
    else:
        log(aoiFile.writeDir, "OUT OF SEASON !!!!! No request will be made for {}".format(aoiFile.siteName))
        continue
    if len(currentYearArray) == 0:
        log(aoiFile.writeDir, "Something went wrong in checkIfSeason function")
        continue

    start_date=str(currentYearArray[0])+"-"+str(startSeasonMonth)+"-"+str(startSeasonDay)+"T00:00:00.000Z"
    end_date=str(currentYearArray[1])+"-"+str(endSeasonMonth)+"-"+str(endSeasonDay)+"T23:59:50.000Z"

    query_date = " ingestiondate:[{} TO {}]".format(start_date, end_date)
    query = "{}{}".format(query, query_date)

    #commande_wget = '%s %s %s "%s%s&rows=1000"'%(wg,auth,search_output,url_search,query)
    commande_wget = wg + auth + search_output
    commande_wget.append(url_search+query+"&rows=1000")
#+ [url_search+query+"&rows=1000"]
    log(aoiFile.writeDir, commande_wget)

    if runCmd(commande_wget, False) != 0:
    #if runCmd([wg, auth, search_output, url_search+query+"&rows=1000"], False) != 0:
        log(aoiFile.writeDir, "Could not get the catalog output for {}".format(query_geom))
        continue
    if exitFlag:
        sys.exit(0)

    #=======================
    # parse catalog output
    #=======================
    xml=minidom.parse(queryResultsFilename)

    #check if all the results are returned
    subtitle=xml.getElementsByTagName("subtitle")[0].firstChild.data
    log(aoiFile.writeDir, "Subtitle = {}".format(subtitle))
    if len(subtitle) > 0:
        words=subtitle.split(" ")
        log(aoiFile.writeDir, "WORDS[2]: {}".format(words[2]))
        if len(words) > 2 and words[2] != "results." and len(words) > 5:
            try:
                totalRes=int(words[5]) + 1
                query = query + "&rows=" + str(totalRes)
                #commande_wget='%s %s %s "%s%s"'%(wg,auth,search_output,url_search,query)
                commande_wget = wg + auth + search_output + [url_search+query]
                log(aoiFile.writeDir, "Changing the pagination to {0} to get all of the existing files, command: {1}".format(totalRes, commande_wget))
                if runCmd(commande_wget, False) != 0:
                    log(aoiFile.writeDir, "Could not get the catalog output (re-pagination) for {}".format(query_geom))
                    continue
                xml=minidom.parse("query_results.xml")
            except ValueError:
                log(aoiFile.writeDir, "Exception: it was expected for the word in position 5 (starting from 0) to be an int. It ain't. The checked string is: {}".format(subtitle))
                log(aoiFile.writeDir, "Could not get the catalog output (exception for re-pagination) for {}".format(query_geom))
                continue

    products=xml.getElementsByTagName("entry")
    s2Objs = []
    for prod in products:
        ident=prod.getElementsByTagName("id")[0].firstChild.data
        link=prod.getElementsByTagName("link")[0].attributes.items()[0][1]
        #to avoid wget to remove $ special character
        link=link.replace('$','\\$')

        for node in prod.getElementsByTagName("str"):
            (name,value)=node.attributes.items()[0]
            if value=="filename":
                filename= str(node.toxml()).split('>')[1].split('<')[0]   #ugly, but minidom is not straightforward
            elif value=="s2datatakeid":
                datatakeid=str(node.toxml()).split('>')[1].split('<')[0]

        for node in prod.getElementsByTagName("double"):
            (name,value)=node.attributes.items()[0]
            if value=="cloudcoverpercentage":
                cloud=float((node.toxml()).split('>')[1].split('<')[0])
        s2Objs.append(Sentinel2Obj(filename, link, datatakeid, cloud))

    s2Objs.sort()

    for s2Obj in s2Objs:
        print("===============================================")
        print(s2Obj.filename)
        print("Date:{}".format(s2Obj.dateAsInt))
        print(s2Obj.link)
        print "cloud percentage = %5.2f %%"%s2Obj.cloud
        print("cloud percentage in aoiFile = {} %".format(aoiFile.maxCloudCoverage))
        print("date de prise de vue {}".format(s2Obj.datatakeid))
        print("product name {}".format(s2Obj.productname))
        print("===============================================")

        if exitFlag:
            sys.exit(0)
        if options.location != None and options.location == "amazon":
            downloadFromAmazon(s2Obj, aoiFile, db)
        else:
            downloadFromScihub(s2Obj, aoiFile, db)
