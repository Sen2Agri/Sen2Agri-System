#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""
_____________________________________________________________________________

   Program:      Sen2Agri-Processors
   Language:     Python
   Copyright:    2015-2016, CS Romania, office@c-s.ro
   See COPYRIGHT file for details.

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
_____________________________________________________________________________

    Landsat Data download from earth explorer. Original source code from Olivier Hagolle
    https://github.com/olivierhagolle/LANDSAT-Download
    Incorporates jake-Brinkmann improvements
"""

import glob
import os
import sys
import math
import urllib2
import urllib
import time
import math
import shutil
import socket
import subprocess

import datetime
import csv
import signal
import osgeo.ogr as ogr
import osgeo.osr as osr
from sen2agri_common_db import *
from bs4 import BeautifulSoup
import traceback
general_log_path = "/tmp/"
general_log_filename = "landsat_download.log"

#############################################################################
# CONSTANTS
MONTHS_FOR_REQUESTING_AFTER_SEASON_FINSIHED = int(1)
DEBUG = True


def get_url_opener(proxy_info):
    cookies = urllib2.HTTPCookieProcessor()
    if proxy_info is None or len(proxy_info) < 2:
        proxy_support = None
    elif len(proxy_info) == 2:
        proxy_support = urllib2.ProxyHandler({"http": "http://%(host)s:%(port)s" % proxy_info,
                                              "https": "https://%(host)s:%(port)s" % proxy_info})
    elif len(proxy_info) == 4:
        proxy_support = urllib2.ProxyHandler({"http": "http://%(user)s:%(pass)s@%(host)s:%(port)s" % proxy_info,
                                              "https": "https://%(user)s:%(pass)s@%(host)s:%(port)s" % proxy_info})

    if proxy_support is None:
        return urllib2.build_opener(cookies)
    else:
        log(general_log_path, "Connected through proxy", general_log_filename)
        return urllib2.build_opener(cookies, proxy_support)

# "Connection to Earth explorer with proxy


def connect_earthexplorer(proxy_info, usgs):
    log(general_log_path, "Establishing connection to EarthExplorer...", general_log_filename)

    opener = get_url_opener(proxy_info)

    # installation
    urllib2.install_opener(opener)
    log(general_log_path, "Opener installed", general_log_filename)
    # deal with csrftoken required by USGS as of 7-20-2016
    soup = BeautifulSoup(urllib2.urlopen("https://ers.cr.usgs.gov", timeout=100).read(), "lxml")
    log(general_log_path, "Soup created", general_log_filename)
    token = soup.find('input', {'name': 'csrf_token'})
    log(general_log_path, "token = {}".format(token), general_log_filename)
    # parametres de connection
    params = urllib.urlencode(dict(username=usgs['account'], password=usgs['passwd'], csrf_token=token['value']))
    log(general_log_path, "params = {}".format(params), general_log_filename)
    # utilisation
    f = opener.open('https://ers.cr.usgs.gov/login', params)
    log(general_log_path, "Link opened", general_log_filename)
    data = f.read()
    log(general_log_path, "Data read", general_log_filename)
    f.close()
    log(general_log_path, "Link closed", general_log_filename)

    if data.find("You must sign in as a registered user to download data or place orders for USGS EROS products") > 0 or\
       data.find("Invalid username/password") > 0:
        print "Authentification failed"
        return False
    return True

#############################


def sizeof_fmt(num):
    for x in ['bytes', 'KB', 'MB', 'GB', 'TB']:
        if num < 1024.0:
            return "%3.1f %s" % (num, x)
        num /= 1024.0
#############################


def downloadChunks(url, prod_name, prod_date, abs_prod_path, aoiContext, db):
    """ Downloads large files in pieces
     inspired by http://josh.gourneau.com
    """
    nom_fic = prod_name + ".tgz"
    # the name in the old format LC817000....
    old_prod_name = prod_name
    print("INFO START")
    print("url: {}".format(url))
    print("aoiContext.writeDir: {}".format(aoiContext.writeDir))
    print("nom_fic: {}".format(nom_fic))
    print("prod_date: {}".format(prod_date))
    #print("abs_prod_path: {}".format(abs_prod_path))
    print("INFO STOP")

    log(aoiContext.writeDir, "Trying to download {0}".format(prod_name), general_log_filename)
    try:
        req = urllib2.urlopen(url, timeout=100)
        print("request performed for URL: {}".format(url))
        #print("request INFO: {}".format(req.info()))

        localName = nom_fic
        if req.info().has_key('Content-Disposition'):
            # If the response has Content-Disposition, we take file name from it
            localName = req.info()['Content-Disposition'].split('filename=')[1]
            if localName[0] == '"' or localName[0] == "'":
                localName = localName[1:-1]

        nom_fic = localName
        if nom_fic.endswith('.tgz'):
            prod_name = nom_fic[:-4]
        elif nom_fic.endswith('.tar.gz'):
            prod_name = nom_fic[:-7]

        print("Downloading FileName is: {}. Product name is: {}".format(nom_fic, prod_name))
        abs_prod_path = os.path.join(aoiContext.writeDir, prod_name)
        print("abs_prod_path recomputed is: {}".format(abs_prod_path))
            
        if prod_name.endswith('_T2') or prod_name.endswith('_RT') :
            log(aoiContext.writeDir, 'Ignoring product with name {} as it is not Tier 1...'.format(prod_name), general_log_filename)
            print("Ignoring product with name {} as it is not Tier 1...".format(prod_name))
            if not db.upsertLandsatProductHistory(aoiContext.siteId, prod_name, DATABASE_DOWNLOADER_STATUS_FAILED_VALUE, prod_date, abs_prod_path, aoiContext.maxRetries):
                log(aoiContext.writeDir, "Couldn't upsert into database with status FAILED for {}".format(prod_name), general_log_filename)
            return False

        # taille du fichier
        if (req.info().gettype() == 'text/html'):
            log(aoiContext.writeDir, 'Error: the file has a html format for '.format(nom_fic), general_log_filename)
            lignes = req.read()
            if lignes.find('Download Not Found') > 0:
                return False
            else:
                log(aoiContext.writeDir, lignes, general_log_filename)
                log(aoiContext.writeDir, 'Download not found for '.format(nom_fic), general_log_filename)
                return False
        total_size = int(req.info().getheader('Content-Length').strip())

        if (total_size < 50000):
            log(aoiContext.writeDir, "Error: The file is too small to be a Landsat Image: {0}".format(nom_fic), general_log_filename)
            log(aoiContext.writeDir, "The used url which generated this error was: {}".format(url), general_log_filename)
            return False

        log(aoiContext.writeDir, "The filename {} has a total size of {}".format(nom_fic, total_size), general_log_filename)

        total_size_fmt = sizeof_fmt(total_size)
        fullFilename = aoiContext.writeDir + '/' + prod_name
        fullFilenameOld = aoiContext.writeDir + '/' + old_prod_name
        log(aoiContext.writeDir, "Checking new file name {} having old file name {}".format(fullFilename, fullFilenameOld), general_log_filename)

        # if we have in the database the old format name or the new one and in the same time we have the folder on disk, do not download it anymore
        if (aoiContext.fileExists(prod_name) or ((old_prod_name != prod_name) and aoiContext.fileExists(old_prod_name))) :
            if os.path.isdir(fullFilename) or os.path.isdir(fullFilenameOld):
                #log(aoiContext.writeDir, "downloadChunks: File {} or {} found in history so it's already downloaded, returning true".format(prod_name, old_prod_name), general_log_filename)
                return True
            
        # insert the product name into the downloader_history
        if not db.upsertLandsatProductHistory(aoiContext.siteId, prod_name, DATABASE_DOWNLOADER_STATUS_DOWNLOADING_VALUE, prod_date, abs_prod_path, aoiContext.maxRetries):
            log(aoiContext.writeDir, "Couldn't upsert into database with status DOWNLOADING for {}".format(prod_name), general_log_filename)
            return False

        downloaded = 0
        CHUNK = 1024 * 1024 * 8
        with open(aoiContext.writeDir + '/' + nom_fic, 'wb') as fp:
            start = time.clock()
            log(aoiContext.writeDir, 'Downloading {0} ({1})'.format(nom_fic, total_size_fmt), general_log_filename)
            while True:
                chunk = req.read(CHUNK)
                downloaded += len(chunk)
                duration = time.clock() - start
                if sys.stdout.isatty() and duration > 0:
                    done = int(50 * downloaded / total_size)
                    sys.stdout.write('\r[{1}{2}]{0:3.0f}% {3}ps'
                                     .format(math.floor((float(downloaded)
                                                         / total_size) * 100),
                                             '=' * done,
                                             ' ' * (50 - done),
                                             sizeof_fmt((downloaded // (time.clock() - start)) / 8)))
                    sys.stdout.flush()
                if not chunk:
                    break
                fp.write(chunk)
    except socket.timeout, e:
        log(aoiContext.writeDir, "Timeout for file {0} ".format(nom_fic), general_log_filename)
        return False
    except socket.error, e:
        log(aoiContext.writeDir, "socket.error for file {0}. Error: {1}".format(nom_fic, e), general_log_filename)
        return False
    except urllib2.HTTPError, e:
        if e.code == 500:
            log(aoiContext.writeDir, "File doesn't exist: {0}".format(nom_fic), general_log_filename)
        else:
            log(aoiContext.writeDir, "HTTP Error for file {0}. Error code: {1}. Url: {2}".format(nom_fic, e.code, url), general_log_filename)
        return False

    except urllib2.URLError, e:
        log(aoiContext.writeDir, "URL Error for file {0} . Reason: {1}. Url: {2}".format(nom_fic, e.reason, url), general_log_filename)
        return False
    size = os.stat(aoiContext.writeDir + '/' + nom_fic).st_size
    log(aoiContext.writeDir, "File {0} downloaded with size {1} from a total size of {2}".format(nom_fic, str(size), str(total_size)), general_log_filename)
    if int(total_size) != int(size):
        log(aoiContext.writeDir, "File {0} has a different size {1} than the expected one {2}. Will not be marked as downloaded".format(nom_fic, str(size), str(total_size)), general_log_filename)
        if not db.upsertLandsatProductHistory(aoiContext.siteId, prod_name, DATABASE_DOWNLOADER_STATUS_FAILED_VALUE, prod_date, abs_prod_path, aoiContext.maxRetries):
            log(aoiContext.writeDir, "Couldn't upsert into database with status DOWNLOADING for {}".format(prod_name), general_log_filename)
            return False
        return False
    # Change the absolute product date to the real one
    unzipSuccess, abs_prod_path = unzipimage(prod_name, aoiContext.writeDir)
    if unzipSuccess:
        # apply north-south correction for all bands
        landsat_tif_files = glob.glob("{}/*_B*.TIF".format(abs_prod_path))
        base_path = os.path.dirname(os.path.abspath(__file__)) + "/../"
        if run_command([base_path + "fix_utm_proj.py"] + landsat_tif_files):
            log(aoiContext.writeDir, "Error: fix_utm_proj.py did not work for {}. This product will still be used as is (with north-south coordonates switched)".format(prod_name), general_log_filename)
        # write the filename in history
        if not db.upsertLandsatProductHistory(aoiContext.siteId, prod_name, DATABASE_DOWNLOADER_STATUS_DOWNLOADED_VALUE, prod_date, abs_prod_path, aoiContext.maxRetries):
            log(aoiContext.writeDir, "Couldn't upsert into database with status FAILED for {}".format(prod_name), general_log_filename)
    else:
        if not db.upsertLandsatProductHistory(aoiContext.siteId, prod_name, DATABASE_DOWNLOADER_STATUS_FAILED_VALUE, prod_date, abs_prod_path, aoiContext.maxRetries):
            log(aoiContext.writeDir, "Couldn't upsert into database with status DOWNLOADING for {}".format(prod_name), general_log_filename)
            return False
    # all went well...hopefully
    return True

##################


def cycle_day(path):
    """ provides the day in cycle given the path number
    """
    cycle_day_path1 = 5
    cycle_day_increment = 7
    nb_days_after_day1 = cycle_day_path1 + cycle_day_increment * (path - 1)

    cycle_day_path = math.fmod(nb_days_after_day1, 16)
    if path >= 98:  # change date line
        cycle_day_path += 1
    return(cycle_day_path)


###################
def next_overpass(date1, path, sat):
    """ provides the next overpass for path after date1
    """
    date0_L5 = datetime.datetime(1985, 5, 4)
    date0_L7 = datetime.datetime(1999, 1, 11)
    date0_L8 = datetime.datetime(2013, 5, 1)
    if sat == 'LT5':
        date0 = date0_L5
    elif sat == 'LE7':
        date0 = date0_L7
    elif sat == 'LC8':
        date0 = date0_L8
    next_day = math.fmod((date1 - date0).days - cycle_day(path) + 1, 16)
    if next_day != 0:
        date_overpass = date1 + datetime.timedelta(16 - next_day)
    else:
        date_overpass = date1
    return(date_overpass)

# "Unzip tgz file


def unzipimage(tgzfile, outputdir):
    success = False
    global general_log_filename
    tarFileExists = False
    tgzFileName = tgzfile
    targetDir = outputdir + '/' + tgzfile
    if (not tgzfile.endswith('.tgz')) and (not tgzfile.endswith('.tar.gz')):
        if (os.path.exists(outputdir + '/' + tgzfile + '.tgz')):
            tgzFileName = tgzfile + '.tgz'
            tarFileExists = True
        elif (os.path.exists(outputdir + '/' + tgzfile + '.tar.gz')):
            tgzFileName = tgzfile + '.tar.gz'
            tarFileExists = True
    else:
        if (os.path.exists(outputdir + '/' + tgzfile)):
            tarFileExists = True

    if tarFileExists:
        log(outputdir,  "decompressing...", general_log_filename)
        fullTgzFilePath = outputdir + '/' + tgzFileName
        try:
            if sys.platform.startswith('linux'):
                subprocess.call('mkdir ' + targetDir, shell=True)  # Unix
                subprocess.call('tar zxvf ' + fullTgzFilePath + ' -C ' + targetDir, shell=True)  # Unix
            elif sys.platform.startswith('win'):
                subprocess.call('tartool ' + fullTgzFilePath + ' ' + targetDir, shell=True)  # W32
            success = True
            log(outputdir,  "Removing downloaded file {}...".format(fullTgzFilePath), general_log_filename)
            os.remove(fullTgzFilePath)
            log(outputdir,  "decompress succeded. removing the file {}".format(fullTgzFilePath), general_log_filename)
        except TypeError:
            log(outputdir, "Failed to unzip {}".format(tgzFileName), general_log_filename)
            os.remove(outputdir)
    return success, targetDir

# "Read image metadata


def read_cloudcover_in_metadata(image_path):
    output_list = []
    fields = ['CLOUD_COVER']
    cloud_cover = 0
    imagename = os.path.basename(os.path.normpath(image_path))
    metadatafile = os.path.join(image_path, imagename + '_MTL.txt')
    metadata = open(metadatafile, 'r')
    # metadata.replace('\r','')
    for line in metadata:
        line = line.replace('\r', '')
        for f in fields:
            if line.find(f) >= 0:
                lineval = line[line.find('= ') + 2:]
                cloud_cover = lineval.replace('\n', '')
    return float(cloud_cover)

# "Check cloud cover limit


def check_cloud_limit(imagepath, limit):
    global general_log_path
    global general_log_filename
    removed = 0
    cloudcover = read_cloudcover_in_metadata(imagepath)
    if cloudcover > limit:
        shutil.rmtree(imagepath)
        log(general_log_path, "Image was removed because the cloud cover value of {} exceeded the limit defined by the user!".format(cloudcover), general_log_filename)
        removed = 1
    return removed


######################################################################################

#signal.signal(signal.SIGINT, signal_handler)

def landsat_download(aoiContext):
    # Iterate site seasons and download the files for each of them
    for seasonInfo in aoiContext.aoiSeasonInfos:
        log(general_log_path, "Performing download for season start: {} end: {}".format(seasonInfo.startSeasonDate, seasonInfo.endSeasonDate), general_log_filename)
        landsat_download_season(aoiContext, seasonInfo)


def landsat_download_season(aoiContext, seasonInfo):
    global general_log_filename
    global general_log_path
    print("START")
    general_log_filename = "landsat_download.log"
    general_log_path = aoiContext.writeDir
    manage_log_file(general_log_path, general_log_filename)
    usgsFile = aoiContext.remoteSiteCredentials
    proxy = {}
    # read password file
    try:
        f = file(usgsFile)
        (account, passwd) = f.readline().split(' ')
        if passwd.endswith('\n'):
            passwd = passwd[:-1]
        usgs = {'account': account, 'passwd': passwd}
        proxy_line = f.readline().strip('\n\t\r ')
        if(len(proxy_line) > 0):
            log(general_log_path, "Found proxy info: {}".format(proxy_line), general_log_filename)
            proxy_info = proxy_line.split(' ')
            if len(proxy_info) == 2:
                proxy = {'host': proxy_info[0], 'port': proxy_info[1]}
            elif len(proxy_info) == 4:
                proxy = {'host': proxy_info[0], 'port': proxy_info[1], 'user': proxy_info[2], 'pass': proxy_info[3]}
            else:
                log(general_log_path, "Proxy information erroneous in {} file, second line. It should have the following format: host port [user pass] ".format(aoiContext.remoteSiteCredentials), general_log_filename)
                f.close()
                return
            log(general_log_path, "Proxy info: {}".format(proxy), general_log_filename)
        f.close()
    except:
        log(general_log_path, "Error raised when reading the landsat 8 password file {} ".format(usgsFile), general_log_filename)
        return

    db = LandsatAOIInfo(aoiContext.configObj.host, aoiContext.configObj.database, aoiContext.configObj.user, aoiContext.configObj.password)

    start_date = datetime.datetime(seasonInfo.startSeasonYear, seasonInfo.startSeasonMonth, seasonInfo.startSeasonDay)
    end_date = datetime.datetime(seasonInfo.endSeasonYear, seasonInfo.endSeasonMonth, seasonInfo.endSeasonDay)
    log(general_log_path, "Number of tiles found {}".format(len(aoiContext.aoiTiles)), general_log_filename)
    if len(aoiContext.aoiTiles) == 0:
        log(general_log_path, "No tiles, nothing to do, exit".format(len(aoiContext.aoiTiles)), general_log_filename)
        return
    for tile in aoiContext.aoiTiles:
        log(aoiContext.writeDir, "Starting the process for tile {} for time interval [{} - {}]".format(tile, start_date, end_date), general_log_filename)
        if len(tile) != 6:
            log(aoiContext.writeDir, "The length for tile is not 6. There should be ppprrr, where ppp = path and rrr = row. The string is {}".format(tile), general_log_filename)
            continue

        product = "LC8"
        remoteDirsStr = '12864, 4923'
        stations = ['LGN']
        if aoiContext.landsatStation != None:
            stations = [aoiContext.landsatStation]
        if aoiContext.landsatDirNumbers != None:
            remoteDirsStr = aoiContext.landsatDirNumbers

        remoteDirsUnstriped = remoteDirsStr.split(',')
        remoteDirs = []
        for remoteDirStr in remoteDirsUnstriped:
            remoteDirs.append(remoteDirStr.strip())

        path = tile[0:3]
        row = tile[3:6]
        log(aoiContext.writeDir, "path={}|row={}".format(path, row), general_log_filename)

        downloaded_ids = []
        connection = False
        try:
            connection = connect_earthexplorer(proxy, usgs)
        except Exception, e:
            traceback.print_exc()
            log(aoiContext.writeDir, "Exception caught when trying to connect at USGS server: {}".format(e), general_log_filename)

        if connection == False:
            return
        curr_date = next_overpass(start_date, int(path), product)

        while (curr_date < end_date and curr_date <= datetime.datetime.now()):
            date_asc = curr_date.strftime("%Y%j")

            log(aoiContext.writeDir, "Searching for images on (julian date): {}...".format(date_asc), general_log_filename)
            curr_date = curr_date + datetime.timedelta(16)
            for station in stations:
                for version in ['00', '01', '02']:
                    nom_prod = product + tile + date_asc + station + version
                    lsdestdir = os.path.join(aoiContext.writeDir, nom_prod)

                    if aoiContext.fileExists(nom_prod):
                        log(aoiContext.writeDir, "File {} found in history so it's already downloaded".format(nom_prod), general_log_filename)
                        if not os.path.exists(lsdestdir):
                            log(aoiContext.writeDir, "Trying to decompress {}. If an error will be raised, means that the archived tgz/tar.gz file was phisically erased (manually or automatically) ".format(nom_prod), general_log_filename)
                            unzipSuccess, abs_prod_path = unzipimage(nom_prod, aoiContext.writeDir)
                            if unzipSuccess : 
                                break       # avoid downloading other version of the product if the current one was successful
                            continue
                        else :
                            break   # avoid downloading other version of the product if the current one was successful

                    # get the date by transforming it from doy to date
                    year = date_asc[0:4]
                    days = date_asc[4:]
                    prod_date = (datetime.datetime(int(year), 1, 1) + datetime.timedelta(int(days))).strftime("%Y%m%dT000000")
                    downloadSuccessful = False
                    for remoteDir in remoteDirs:
                        url = "https://earthexplorer.usgs.gov/download/{}/{}/STANDARD/EE".format(remoteDir, nom_prod)
                        if downloadChunks(url, nom_prod, prod_date, lsdestdir, aoiContext, db):
                            # if the file successfully downloaded, append it also to the history files list
                            aoiContext.appendHistoryFile(nom_prod)
                            downloaded_ids.append(nom_prod)
                            downloadSuccessful = True
                            break
                    # if successfully downloaded a product, then do not try the next version
                    if downloadSuccessful == True :
                        break
        if len(downloaded_ids) > 0:
            log(aoiContext.writeDir, "Downloaded product: {}".format(downloaded_ids), general_log_filename)
        else:
            log(aoiContext.writeDir, "No product has been downloaded ", general_log_filename)
