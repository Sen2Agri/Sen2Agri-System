#! /usr/bin/env python
# -*- coding: iso-8859-1 -*-
"""
    Landsat Data download from earth explorer. Original source code from Olivier Hagolle
    https://github.com/olivierhagolle/LANDSAT-Download
    Incorporates jake-Brinkmann improvements
"""

import glob,os,sys,math,urllib2,urllib,time,math,shutil
import subprocess

import datetime
import csv
import signal
import osgeo.ogr as ogr
import osgeo.osr as osr
from sen2agri_common_db import *

general_log_path = "/tmp/"
general_log_filename = "landsat_download.log"

#############################################################################
# CONSTANTS
MONTHS_FOR_REQUESTING_AFTER_SEASON_FINSIHED = int(1)
DEBUG = True

#############################"Connection to Earth explorer with proxy

def connect_earthexplorer_proxy(proxy_info,usgs):
     print "Establishing connection to Earthexplorer with proxy..."
     # contruction d'un "opener" qui utilise une connexion proxy avec autorisation
     proxy_support = urllib2.ProxyHandler({"http" : "http://%(user)s:%(pass)s@%(host)s:%(port)s" % proxy_info,
     "https" : "http://%(user)s:%(pass)s@%(host)s:%(port)s" % proxy_info})
     opener = urllib2.build_opener(proxy_support, urllib2.HTTPCookieProcessor)

     # installation
     urllib2.install_opener(opener)

     # parametres de connection
     params = urllib.urlencode(dict(username=usgs['account'], password=usgs['passwd']))

     # utilisation
     f = opener.open('https://ers.cr.usgs.gov/login', params)
     data = f.read()
     f.close()

     if data.find('You must sign in as a registered user to download data or place orders for USGS EROS products')>0 :
        print "Authentification failed"
        sys.exit(-1)

     return


#############################"Connection to Earth explorer without proxy

def connect_earthexplorer_no_proxy(usgs):
     global general_log_path
     global general_log_filename
     log(general_log_path, "Establishing connection to Earthexplorer...", general_log_filename)
     opener = urllib2.build_opener(urllib2.HTTPCookieProcessor())
     urllib2.install_opener(opener)
     params = urllib.urlencode(dict(username=usgs['account'],password= usgs['passwd']))
     f = opener.open("https://ers.cr.usgs.gov/login", params)
     data = f.read()
     f.close()
     if data.find('You must sign in as a registered user to download data or place orders for USGS EROS products')>0 :
          log(general_log_path, "Authentification failed !", general_log_filename)
          sys.exit(-1)
     return

#############################

def sizeof_fmt(num):
    for x in ['bytes', 'KB', 'MB', 'GB', 'TB']:
        if num < 1024.0:
            return "%3.1f %s" % (num, x)
        num /= 1024.0
#############################
def downloadChunks(url,rep,nom_fic):

  """ Downloads large files in pieces
   inspired by http://josh.gourneau.com
  """
  global exitFlag
  print("INFO START")
  print("url: {}".format(url))
  print("rep: {}".format(rep))
  print("nom_fic: {}".format(nom_fic))
  print("INFO STOP")
  log(rep, "Trying to download {0}".format(nom_fic), general_log_filename)
  try:
    req = urllib2.urlopen(url, timeout=600)
    #taille du fichier
    if (req.info().gettype()=='text/html'):
      log(rep, 'Error: the file has a html format for '.format(nom_fic), general_log_filename)
      lignes=req.read()
      if lignes.find('Download Not Found')>0 :
            raise TypeError
      else:
          log(rep, lignes, general_log_filename)
          log(rep, 'Download not found for '.format(nom_fic), general_log_filename)
	  return False
    total_size = int(req.info().getheader('Content-Length').strip())

    if (total_size<50000):
       log(rep, "Error: The file is too small to be a Landsat Image: {0}".format(nom_fic), general_log_filename)
       log(rep, "The used url which generated this error was: {}".format(url), general_log_filename)
       return False

    log(rep, "The filename {} has a total size of {}".format(nom_fic,total_size), general_log_filename)

    total_size_fmt = sizeof_fmt(total_size)
    fullFilename = rep+'/'+nom_fic
    if os.path.isfile(fullFilename) and os.stat(fullFilename).st_size == total_size:
        log(rep, "downloadChunks:File {} already downloaded, returning true".format(fullFilename), general_log_filename)
        return True

    downloaded = 0
    CHUNK = 1024 * 1024 *8
    with open(rep+'/'+nom_fic, 'wb') as fp:
        start = time.clock()
        log(rep, 'Downloading {0} ({1})'.format(nom_fic, total_size_fmt), general_log_filename)
	while True and not exitFlag:
	     chunk = req.read(CHUNK)
	     downloaded += len(chunk)
	     done = int(50 * downloaded / total_size)
	     sys.stdout.write('\r[{1}{2}]{0:3.0f}% {3}ps'
                             .format(math.floor((float(downloaded)
                                                 / total_size) * 100),
                                     '=' * done,
                                     ' ' * (50 - done),
                                     sizeof_fmt((downloaded // (time.clock() - start)) / 8)))
	     sys.stdout.flush()
	     if not chunk: break
	     fp.write(chunk)
    if exitFlag:
        log(rep, "SIGINT signal caught", general_log_filename)
        sys.exit(0)
  except urllib2.HTTPError, e:
       if e.code == 500:
            log(rep, "File doesn\'t exist: {0}".format(nom_fic), general_log_filename)
       else:
            log(rep, "HTTP Error for file {0}. Error code: {1}. Url: {2}".format(nom_fic, e.code, url), general_log_filename)
       return False
  except urllib2.URLError, e:
    log(rep, "URL Error for file {0} . Reason: {1}. Url: {2}".format(nom_fic, e.reason,url), general_log_filename)
    return False
  size = os.stat(rep+'/'+nom_fic).st_size
  log(rep, "File {0} downloaded with size {1} from a total size of {2}".format(nom_fic,str(size), str(total_size)), general_log_filename)
  if int(total_size) != int(size):
    log(rep, "File {0} has a different size {1} than the expected one {2}. Will not be marked as downloaded".format(nom_fic,str(size), str(total_size)), general_log_filename)
    return False
  #all went well...hopefully
  return True

##################
def cycle_day(path):
    """ provides the day in cycle given the path number
    """
    cycle_day_path1  = 5
    cycle_day_increment = 7
    nb_days_after_day1=cycle_day_path1+cycle_day_increment*(path-1)

    cycle_day_path=math.fmod(nb_days_after_day1,16)
    if path>=98: #change date line
	cycle_day_path+=1
    return(cycle_day_path)



###################
def next_overpass(date1,path,sat):
    """ provides the next overpass for path after date1
    """
    date0_L5 = datetime.datetime(1985,5,4)
    date0_L7 = datetime.datetime(1999,1,11)
    date0_L8 = datetime.datetime(2013,5,1)
    if sat=='LT5':
        date0=date0_L5
    elif sat=='LE7':
        date0=date0_L7
    elif sat=='LC8':
        date0=date0_L8
    next_day=math.fmod((date1-date0).days-cycle_day(path)+1,16)
    if next_day!=0:
        date_overpass=date1+datetime.timedelta(16-next_day)
    else:
        date_overpass=date1
    return(date_overpass)

#############################"Unzip tgz file

def unzipimage(tgzfile, outputdir):
    success=0
    global general_log_filename
    if (os.path.exists(outputdir+'/'+tgzfile+'.tgz')):
        log(outputdir,  "decompressing...", general_log_filename)
        try:
            if sys.platform.startswith('linux'):
                subprocess.call('mkdir '+ outputdir+'/'+tgzfile, shell=True)   #Unix
                subprocess.call('tar zxvf '+outputdir+'/'+tgzfile+'.tgz -C '+ outputdir+'/'+tgzfile, shell=True)   #Unix
            elif sys.platform.startswith('win'):
                subprocess.call('tartool '+outputdir+'/'+tgzfile+'.tgz '+ outputdir+'/'+tgzfile, shell=True)  #W32
            success=1
            os.remove(outputdir+'/'+tgzfile+'.tgz')
            log(outputdir,  "decompress succeded. removing the .tgz file {}".format(outputdir+'/'+tgzfile), general_log_filename)
        except TypeError:
            log(outputdir, "Failed to unzip {}".format(tgzfile), general_log_filename)
            os.remove(outputdir)
    return success

#############################"Read image metadata
def read_cloudcover_in_metadata(image_path):
    output_list=[]
    fields = ['CLOUD_COVER']
    cloud_cover=0
    imagename=os.path.basename(os.path.normpath(image_path))
    metadatafile= os.path.join(image_path,imagename+'_MTL.txt')
    metadata = open(metadatafile, 'r')
    # metadata.replace('\r','')
    for line in metadata:
        line = line.replace('\r', '')
        for f in fields:
            if line.find(f)>=0:
                lineval = line[line.find('= ')+2:]
                cloud_cover=lineval.replace('\n','')
    return float(cloud_cover)

#############################"Check cloud cover limit

def check_cloud_limit(imagepath,limit):
     global general_log_path
     global general_log_filename
     removed=0
     cloudcover=read_cloudcover_in_metadata(imagepath)
     if cloudcover>limit:
          shutil.rmtree(imagepath)
          log(general_log_path, "Image was removed because the cloud cover value of {} exceeded the limit defined by the user!".format(cloudcover), general_log_filename)
          removed=1
     return removed

def signal_handler(signal, frame):
    global exitFlag
    global general_log_path
    global general_log_filename
    log(general_log_path, "You pressed Ctrl+C!", general_log_filename)
    exitFlag = True
    sys.exit(0)

######################################################################################
###############                       main                    ########################
######################################################################################

################Lecture des arguments################
def main():
    global exitFlag
    global general_log_path
    global general_log_filename
    exitFlag = False
    signal.signal(signal.SIGINT, signal_handler)
    if len(sys.argv) == 1:
        prog = os.path.basename(sys.argv[0])
        print '      '+sys.argv[0]+' [options]'
        print "      Help : ", prog, " --help"
        print "        or : ", prog, " -h"
        print ("example (scene): {} -u usgs.txt ".format(sys.argv[0]))
        print ("Inside the input directory all the files with .dwn extension will be checked")
        sys.exit(-1)
    else:
        usage = "usage: %prog [options] "
        parser = OptionParser(usage=usage)
        parser.add_option("-u","--usgs_passwd", dest="usgs", action="store", type="string", \
                help="USGS earthexplorer account and password file")
        parser.add_option("-p","--proxy_passwd", dest="proxy", action="store", type="string", \
                help="Proxy account and password file")
        parser.add_option("-z","--unzip", dest="unzip", action="store", type="string", \
                help="Unzip downloaded tgz file", default=None)
        parser.add_option("-c","--config", dest="config", action="store",type="string",  \
                          help="File with credentials for the local database",default="/etc/sen2agri/sen2agri.conf")
        parser.add_option("--dir", dest="dir", action="store", type="string", \
                help="Dir number where files  are stored at USGS",default=None)
        parser.add_option("--station", dest="station", action="store", type="string", \
                help="Station acronym (3 letters) of the receiving station where the file is downloaded",default=None)

    (options, args) = parser.parse_args()
    parser.check_required("-c")

    fullFilename = os.path.realpath(__file__)
    dirname = fullFilename[0:fullFilename.rfind('/') + 1]
    config = Config()
    if not config.loadConfig(options.config):
        log(general_log_path, "Could not load the config file", general_log_filename)
        sys.exit(-1)
    db = LandsatAOIInfo(config.host, config.database, config.user, config.password)
    aoiDatabase = db.getLandsatAOI()
    print("------------------------")
    for aoi in aoiDatabase:
        aoi.printInfo()
        print("------------------------")

    if len(aoiDatabase) <= 0:
        log(general_log_path, "Could not get DB info", general_log_filename)
        sys.exit(-1)

    # read password files
    try:
        f = file(options.usgs)
        (account,passwd)=f.readline().split(' ')
        if passwd.endswith('\n'):
            passwd=passwd[:-1]
        usgs={'account':account,'passwd':passwd}
        f.close()
    except :
        log(general_log_path, "Error with usgs password file", general_log_filename)
        sys.exit(-2)

    if options.proxy != None :
        try:
            f=file(options.proxy)
            (user,passwd)=f.readline().split(' ')
            if passwd.endswith('\n'):
                passwd=passwd[:-1]
            host=f.readline()
            if host.endswith('\n'):
                host=host[:-1]
            port=f.readline()
            if port.endswith('\n'):
                port=port[:-1]
            proxy={'user':user,'pass':passwd,'host':host,'port':port}
            f.close()
        except :
            log(general_log_path, "Error with proxy password file", general_log_filename)
            sys.exit(-3)

##########Telechargement des produits par scene
for aoiFile in aoiDatabase:
        if not createRecursiveDirs(aoiFile.writeDir):
             print("Could not create the output directory")
             sys.exit(-1)
        general_log_path = aoiFile.writeDir

        startSeasonMonth = int(0)
        startSeasonDay = int(0)
        endSeasonMonth = int(0)
        endSeasonDay = int(0)
        # first position is the startSeasonYear, the second is the endPositionYear
        currentYearArray = []

        print("SITE NAME:{}".format(aoiFile.siteName))
        if check_if_season(aoiFile.startSummerSeason, aoiFile.endSummerSeason, MONTHS_FOR_REQUESTING_AFTER_SEASON_FINSIHED, currentYearArray, aoiFile.writeDir, general_log_filename):
            startSeasonMonth = int(aoiFile.startSummerSeason[0:2])
            startSeasonDay = int(aoiFile.startSummerSeason[2:4])
            endSeasonMonth = int(aoiFile.endSummerSeason[0:2])
            endSeasonDay = int(aoiFile.endSummerSeason[2:4])
        elif check_if_season(aoiFile.startWinterSeason, aoiFile.endWinterSeason, MONTHS_FOR_REQUESTING_AFTER_SEASON_FINSIHED, currentYearArray, aoiFile.writeDir, general_log_filename):
            startSeasonMonth = int(aoiFile.startWinterSeason[0:2])
            startSeasonDay = int(aoiFile.startWinterSeason[2:4])
            endSeasonMonth = int(aoiFile.endWinterSeason[0:2])
            endSeasonDay = int(aoiFile.endWinterSeason[2:4])
        else:
            log(aoiFile.writeDir, "Out of season ! No request will be made for {}".format(aoiFile.siteName), general_log_filename)
            continue
        if len(currentYearArray) == 0:
            log(aoiFile.writeDir, "Something went wrong in check_if_season function", general_log_filename)
            continue

        start_date=str(currentYearArray[0])+str(startSeasonMonth)+str(startSeasonDay)
        end_date=str(currentYearArray[1])+str(endSeasonMonth)+str(endSeasonDay)

        for tile in aoiFile.aoiTiles:
            log(aoiFile.writeDir, "Starting the process for tile {}".format(tile), general_log_filename)
            if len(tile) != 6:
                log(aoiFile.writeDir, "The length for tile is not 6. There should be ppprrr, where ppp = path and rrr = row. The string is {}".format(tile), general_log_filename)
                continue

            product="LC8"
            remoteDir='4923'
            stations=['LGN']
            if options.station !=None:
                stations=[options.station]
            if options.dir !=None:
                remoteDir=options.dir

            path=tile[0:3]
            row=tile[3:6]
            log(aoiFile.writeDir, "path={}|row={}".format(path, row), general_log_filename)

            year_start =int(currentYearArray[0])
            month_start=int(startSeasonMonth)
            day_start  =int(startSeasonDay)
            date_start=datetime.datetime(year_start,month_start, day_start)

            year_end =int(currentYearArray[1])
            month_end=int(endSeasonMonth)
            day_end  =int(endSeasonDay)
            date_end =datetime.datetime(year_end,month_end, day_end)

            global downloaded_ids
            downloaded_ids=[]

            if options.proxy!=None:
                connect_earthexplorer_proxy(proxy,usgs)
            else:
                connect_earthexplorer_no_proxy(usgs)

            curr_date=next_overpass(date_start,int(path),product)

            while (curr_date < date_end):
                date_asc=curr_date.strftime("%Y%j")
                notfound = False

                log(aoiFile.writeDir, "Searching for images on (julian date): {}...".format(date_asc), general_log_filename)
                curr_date=curr_date+datetime.timedelta(16)
                for station in stations:
                    for version in ['00','01','02']:
                                            if exitFlag:
                                                log(aoiFile.writeDir, "SIGINT was caught")
                                                sys.exit(0)
                                            nom_prod=product + tile + date_asc + station + version
                                            tgzfile=os.path.join(aoiFile.writeDir,nom_prod+'.tgz')
                                            lsdestdir=os.path.join(aoiFile.writeDir,nom_prod)
                                            url = "http://earthexplorer.usgs.gov/download/{}/{}/STANDARD/EE".format(remoteDir,nom_prod)

                                            if aoiFile.fileExists(nom_prod):
                                                log(aoiFile.writeDir, "File {} found in history so it's already downloaded".format(nom_prod), general_log_filename))
                                                if not os.path.exists(lsdestdir) and options.unzip!= None:
                                                    log(aoiFile.writeDir, "Trying to decompress {}. If an error will be raised, means that the archived tgz file was phisically erased (manually or automatically) ".format(nom_prod), general_log_filename)
                                                    unzipimage(nom_prod, aoiFile.writeDir)
                                                continue
                                            try:
                                                if downloadChunks(url, aoiFile.writeDir, "{}.tgz".format(nom_prod)):
                                                    downloaded_ids.append(nom_prod)
                                                    if options.unzip!= None:
                                                        unzipimage(nom_prod, aoiFile.writeDir)
                                                    #write the filename in history
                                                    try:
                                                        if not db.updateLandsatHistory(aoiFile.siteId, nom_prod,  "{}/{}".format(aoiFile.writeDir, nom_prod)):
                                                            log(aoiFile.writeDir, "Could not insert into database site_id {} the product name {}".format(aoiFile.siteId, s2Obj.filename), general_log_filename)
                                                    except:
                                                        log(aoiFile.writeDir, "db.updateSentinelHistory throwed an exception", general_log_filename)
                                            except:
                                                log(aoiFile.writeDir, "downloadChunks throwed an exception", general_log_filename)
            log(aoiFile.writeDir, downloaded_ids, general_log_filename)
if __name__ == "__main__":
    main()
