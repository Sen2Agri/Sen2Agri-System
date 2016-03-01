#! /usr/bin/env python
import glob,os,sys
import time, datetime
import pipes
import subprocess
import psycopg2
import psycopg2.errorcodes
import optparse

DEBUG = True
FAKE_COMMAND = False
NUMBER_OF_CONFIG_PARAMS_FROM_DB = int(6)
SENTINEL2_SATELLITE_ID = int(1)
LANDSAT8_SATELLITE_ID = int(2)

def runCmd(cmdArray, useShell=True):
    start = time.time()
    print(" ".join(map(pipes.quote, cmdArray)))
    res = 0
    if not FAKE_COMMAND:
        res = subprocess.call(cmdArray, shell=useShell)
    print("App finished in: {}".format(datetime.timedelta(seconds=(time.time() - start))))
    if res != 0:
        print("Application error")
    return res

def createRecursiveDirs(dirName):
    #check if it already exists.... otherwise the makedirs function will raise an exception
    if os.path.exists(dirName):
        if not os.path.isdir(dirName):
            print("Can't create the output directory because there is a file with the same name")
            print("Remove: " + dirName)
            return False
    else:
        try:
            #create recursive dir
            os.makedirs(dirName)
        except:
            #for sure, the problem is with access rights
            print("Can't create the output directory due to access rights")
            return False
    return True

def log(location,info):
    logfile = os.path.join(location,'log.txt')
    if DEBUG:
        print("{}".format(info))
    log = open(logfile, 'a')
    log.write(str(datetime.datetime.now())+':'+str(info)+'\n')
    log.close()

def checkIfSeason(startSeason, endSeason, numberOfMonthsAfterEndSeason, yearArray, logDir):
    currentYear = datetime.date.today().year
    currentMonth = datetime.date.today().month
    print("{} | {}".format(startSeason, endSeason))
    yearArray.append(currentYear)
#    startSeasonYear = int(currentYear)
    yearArray.append(currentYear)
#    endSeasonYear = int(currentYear)
    startSeasonMonth = int(startSeason[0:2])
    startSeasonDay = int(startSeason[2:4])
    endSeasonMonth = int(endSeason[0:2])
    endSeasonDay = int(endSeason[2:4])
    if startSeasonMonth < 1 or startSeasonMonth > 12 or startSeasonDay < 1 or startSeasonDay > 31 or endSeasonMonth < 1 or endSeasonMonth > 12 or endSeasonDay < 1 or endSeasonDay > 31:
        return False
    log(logDir, "CurrentYear:{} | CurrentMonth:{} | StartSeasonMonth:{} | StartSeasonDay:{} | EndSeasonMonth:{} | EndSeasonDay:{}".format(currentYear, currentMonth, startSeasonMonth, startSeasonDay, endSeasonMonth, endSeasonDay))
    #check if the season comprises 2 consecutive years (e.q. from october to march next year)
    if startSeasonMonth > endSeasonMonth:
        if currentMonth >= startSeasonMonth and currentMonth <= 12:
            #endSeasonYear = currentYear + 1
            yearArray[1] = currentYear + 1
        else:
            if currentMonth >= 1:
                yearArray[0] = currentYear - 1
    log(logDir, "StartSeasonYear:{} | EndSeasonYear:{}".format(yearArray[0], yearArray[1]))
    currentDate = datetime.date.today()
    if currentDate < datetime.date(int(yearArray[0]), int(startSeasonMonth), int(startSeasonDay)) or currentDate > datetime.date(int(yearArray[1]), int(endSeasonMonth) + numberOfMonthsAfterEndSeason, int(endSeasonDay)):
        log(logDir, "Current date is not inside or near the season")
        return False
    return True

###########################################################################
class OptionParser (optparse.OptionParser):

    def check_required (self, opt):
      option = self.get_option(opt)
      # Assumes the option's 'default' is set to None!
      if getattr(self.values, option.dest) is None:
          self.error("{} option not supplied".format(option))


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

class AOIDatabase(object):
    def __init__(self):
        self.siteId = int(0)
        self.siteName = ""
        self.polygon = ""
        self.startSummerSeason = ""
        self.endSummerSeason = ""
        self.startWinterSeason = ""
        self.endWinterSeason = ""
        self.maxCloudCoverage = int(100)
        self.writeDir = ""
        self.aoiHistoryFiles = []
        self.aoiTiles = []
    def addHistoryFiles(self, historyFiles):
        self.aoiHistoryFiles = historyFiles
    def appendHistoryFile(self, historyFile):
        self.aoiHistoryFiles.append(historyFile)
    def appendTile(self, tile):
        self.aoiTiles.append(tile)
    def setConfigParams(self, configParams):
        if len(configParams) != NUMBER_OF_CONFIG_PARAMS_FROM_DB:
            return
        self.startSummerSeason = configParams[0]
        self.endSummerSeason = configParams[1]
        self.startWinterSeason = configParams[2]
        self.endWinterSeason = configParams[3]
        self.maxCloudCoverage = int(configParams[4])
        self.writeDir = configParams[5]


    def fillHistory(self, dbInfo):
        self.aoiHistoryFiles = dbInfo
    def fileExists(self, filename):
        for historyFilename in self.aoiHistoryFiles:
            if filename == historyFilename:
                return True
        return False
    def printInfo(self):
        print("SiteID  : {}".format(self.siteId))
        print("SiteName: {}".format(self.siteName))
        print("Polygon : {}".format(self.polygon))
        print("startSS : {}".format(self.startSummerSeason))
        print("endSS   : {}".format(self.endSummerSeason))
        print("startWS : {}".format(self.startWinterSeason))
        print("endWS   : {}".format(self.endWinterSeason))
        print("CloudCov: {}".format(self.maxCloudCoverage))
        print("writeDir: {}".format(self.writeDir))

        if len(self.aoiTiles) <= 0:
            print("tiles: NONE")
        else:
            print("tiles:")
            print(" ".join(self.aoiTiles))

        if len(self.aoiHistoryFiles) <= 0:
            print("historyFiles: NONE")
        else:
            print("historyFiles:")
            print(" ".join(self.aoiHistoryFiles))

class AOIInfo(object):
    def __init__(self, serverIP, databaseName, user, password, logFile):
        self.serverIP = serverIP
        self.databaseName = databaseName
        self.user = user
        self.password = password
        self.isConnected = False;
        self.logFile = logFile
    def databaseConnect(self):
        if self.isConnected:
            return True
        try:
            connectString = "dbname='{}' user='{}' host='{}' password='{}'".format(self.databaseName, self.user, self.serverIP, self.password)
            print("connectString:={}".format(connectString))
            self.conn = psycopg2.connect(connectString)
            self.cursor = self.conn.cursor()
            self.isConnected = True
        except:
            print "Unable to connect to the database"
            self.isConnected = False
            return False
        return True
    def databaseDisconnect(self):
        if self.conn:
            self.conn.close()
            self.isConnected = False
    def getAOI(self, satelliteId):
        writeDirSatelliteName = ""
        if satelliteId == SENTINEL2_SATELLITE_ID:
            writeDirSatelliteName = "s2."
        elif satelliteId == LANDSAT8_SATELLITE_ID:
            writeDirSatelliteName = "l8."
        else:
            return False
        if not self.databaseConnect():
            return False
        try:
            self.cursor.execute("select *,st_astext(geog) from site")
            rows = self.cursor.fetchall()
        except:
            self.databaseDisconnect()
            return []
        # retArray will have: [siteId, siteName, polygon, startSummerSeason, endSummerSeason, startWinterSeason, endWinterSeason, maxCloudCoverage, writeDir]
        retArray = []
        for row in rows:
            if len(row) == 5 and row[4] != None:
                # retry in case of disconnection
                if not self.databaseConnect():
                    return False
                currentAOI = AOIDatabase()
                currentAOI.siteId = int(row[0])
                currentAOI.siteName = row[2]
                currentAOI.polygon = row[4]
                currentAOI.printInfo()
                baseQuery = "select * from sp_get_parameters(\'downloader."
                whereQuery = "where \"site_id\"="
                suffixArray = ["summer-season.start\')", "summer-season.end\')", "winter-season.start\')", "winter-season.end\')", "max-cloud-coverage\')", "{}write-dir\')".format(writeDirSatelliteName)]
                dbHandler = True
                configArray = []
                for suffix in suffixArray:
                    baseQuerySite = "{}{}".format(baseQuery, suffix)
                    query = "{} {} {}".format(baseQuerySite, whereQuery, currentAOI.siteId)
                    print("query with where={}".format(query))
                    baseQuerySite += " where \"site_id\" is null"
                    try:
                        self.cursor.execute(query)
                        if self.cursor.rowcount <= 0:
                            print("query={}".format(baseQuerySite))
                            self.cursor.execute(baseQuerySite)
                            print("2.self.cursor.rowcount={}".format(self.cursor.rowcount))
                            if self.cursor.rowcount <= 0:
                                print("could not get event the default value for downloader.{}".format(suffix))
                                dbHandler = False
                                break
                        if self.cursor.rowcount != 1:
                            print("More than 1 result from the db for downloader.{}".format(suffix))
                            dbHandler = False
                            break
                        result = self.cursor.fetchall()
                        print("result={}".format(result))
                        configArray.append(result[0][2])
                    except Exception, e:
                        print("exception in query for downloader.{}:".format(suffix))
                        print("{}".format(e))
                        self.databaseDisconnect()
                        dbHandler = False
                        break
                print("-------------------------")
                if dbHandler:
                    currentAOI.setConfigParams(configArray)
                    try:
                        self.cursor.execute("select \"product_name\" from downloader_history where \"satellite_id\"={} and \"site_id\"={}".format(satelliteId, currentAOI.siteId))
                        if self.cursor.rowcount > 0:
                            result = self.cursor.fetchall()
                            for res in result:
                                if len(res) == 1:
                                    currentAOI.appendHistoryFile(res[0])
                        self.cursor.execute("select * from sp_get_site_tiles({} :: smallint, {})".format(currentAOI.siteId, satelliteId))
                        if self.cursor.rowcount > 0:
                            result = self.cursor.fetchall()
                            for res in result:
                                if len(res) == 1:
                                    currentAOI.appendTile(res[0])
                    except Exception, e:
                        print("exception = {}".format(e))
                        print("Error in getting the downloaded files")
                        dbHandler = False
                        self.databaseDisconnect()
                if dbHandler:
                    retArray.append(currentAOI)

        self.databaseDisconnect()
        return retArray
    def updateHistory(self, siteId, satelliteId, productName, fullPath):
        if not self.databaseConnect():
            return False
        try:
            print("UPDATING: insert into downloader_history (\"site_id\", \"satellite_id\", \"product_name\", \"full_path\") VALUES ({}, {}, '{}', '{}')".format(siteId, satelliteId, productName, fullPath))
            self.cursor.execute("insert into downloader_history (\"site_id\", \"satellite_id\", \"product_name\", \"full_path\") VALUES ({}, {}, '{}', '{}')".format(siteId, satelliteId, productName, fullPath))
            self.conn.commit()
        except:
            print("DATABASE INSERT query FAILED!!!!!")
            self.databaseDisconnect()
            return False
        self.databaseDisconnect()
        return True


class SentinelAOIInfo(AOIInfo):
    def __init__(self, serverIP, databaseName, user, password, logFile=None):
        AOIInfo.__init__(self, serverIP, databaseName, user, password, logFile)
    def getSentinelAOI(self):
        return self.getAOI(SENTINEL2_SATELLITE_ID)
    def updateSentinelHistory(self, siteId, productName, fullPath):
        return self.updateHistory(siteId, SENTINEL2_SATELLITE_ID, productName, fullPath)

class LandsatAOIInfo(AOIInfo):
    def __init__(self, serverIP, databaseName, user, password, logFile=None):
        AOIInfo.__init__(self, serverIP, databaseName, user, password, logFile)
    def getLandsatAOI(self):
        return self.getAOI(LANDSAT8_SATELLITE_ID)
    def updateLandsatHistory(self, siteId, productName, fullPath):
        return self.updateHistory(siteId, LANDSAT8_SATELLITE_ID, productName, fullPath)
