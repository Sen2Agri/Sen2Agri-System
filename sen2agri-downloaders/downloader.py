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

"""

import glob,os,sys,math,urllib2,urllib,time,math,shutil
import subprocess

import datetime
import csv
import signal
import osgeo.ogr as ogr
import osgeo.osr as osr
from sen2agri_common_db import *
from sentinel_download import *
from landsat_download import *
from multiprocessing import Pool

#signal.signal(signal.SIGINT, signal_handler)

if len(sys.argv) == 1:
    prog = os.path.basename(sys.argv[0])
    print '      '+sys.argv[0]+' [options]'
    print "     Help : ", prog, " --help"
    print "        or : ", prog, " -h"
    sys.exit(-1)
else:
    usage = "usage: %prog [options] "
    parser = OptionParser(usage=usage)

    parser.add_option("-r", "--remote-host", dest="remote_host", action="store", type="string", \
                      help="Type of the downloader. Can be s2 or l8")

    parser.add_option("-s", "--site-credentials", dest="site_credentials", action="store",type="string", \
                        help="File with credentials for the server")

    parser.add_option("-p","--proxy-passwd", dest="proxy", action="store", type="string", \
                        help="Proxy account and password file",default=None)

    parser.add_option("-c","--config", dest="config", action="store",type="string",  \
                          help="File with credentials for the database",default="/etc/sen2agri/sen2agri.conf")

    # for sentinel only
    parser.add_option("-l","--location", dest="location", action="store",type="string", \
                help="The location from where the product should be donwloaded: scihub or amazon", default=None)

    #for landsat only
    parser.add_option("--dir", dest="dir", action="store", type="string", \
                    help="Dir number where files  are stored at USGS",default=None)
    parser.add_option("--station", dest="station", action="store", type="string", \
                    help="Station acronym (3 letters) of the receiving station from where the file is downloaded",default=None)

    #manually launch for a site and a period
    parser.add_option("--site-to-dwn", dest="site_to_dwn", action="store", type="string", \
                    help="ID site from db",default=None)
    parser.add_option("--start-date", dest="start_date", action="store", type="string", \
                    help="Start date of the wanted time interval",default=None)
    parser.add_option("--end-date", dest="end_date", action="store", type="string", \
                    help="Start date of the wanted time interval",default=None)
    (options, args) = parser.parse_args()


    general_log_filename = "downloader.log"
    general_log_path = "/tmp/"
    config = Config()
    if not config.loadConfig(options.config):
        log(general_log_path, "Could not load the config file", general_log_filename)
        sys.exit(-1)
    manual_site_to_dwn = -1
    manual_start_date = ""
    manual_end_date = ""
    if options.site_to_dwn is not None:
        manual_site_to_dwn = int(options.site_to_dwn)
    if options.start_date is not None:
        parser.check_required("--end-date")
        manual_start_date = options.start_date
    if options.end_date is not None:
        parser.check_required("--start-date")
        manual_end_date = options.end_date

    database = None
    sites_aoi_database = []
    if options.remote_host == "s2":
        parser.check_required("-l")
        database = SentinelAOIInfo(config.host, config.database, config.user, config.password)
        sites_aoi_database = database.getSentinelAOI(manual_site_to_dwn, manual_start_date, manual_end_date)
        for aoi in sites_aoi_database:
            aoi.setSentinelLocation(options.location)
    elif options.remote_host == "l8":
        database = LandsatAOIInfo(config.host, config.database, config.user, config.password)
        sites_aoi_database = database.getLandsatAOI(manual_site_to_dwn, manual_start_date, manual_end_date)
        for aoi in sites_aoi_database:
            aoi.setLandsatDirNumber(options.dir)
            aoi.setLandsatStation(options.station)
    else:
        log(general_log_path, "Unkown remote host. Has to be 's2' or 'l8'. The received input is {}".format(options.remote_host), general_log_filename)
        sys.exit(-1)

    if len(sites_aoi_database) <= 0:
        log(general_log_path, "Could not get DB info", general_log_filename)
        sys.exit(-1)
    
    print("LEN = {} ------------------------".format(len(sites_aoi_database)))
    for aoiContext in sites_aoi_database:
        if not create_recursive_dirs(aoiContext.writeDir):
            log(general_log_path, "Could not create the output directory", general_log_filename)
            sys.exit(-1)
        aoiContext.setConfigObj(config)
        aoiContext.setRemoteSiteCredentials(options.site_credentials)
        aoiContext.setProxy(options.proxy)    
        aoiContext.printInfo()
        print("------------------------")

    p = Pool(len(sites_aoi_database))
    if options.remote_host == "s2":        
        p.map(sentinel_download, sites_aoi_database)
        #used only in debug mode
        #for site in sites_aoi_database:
        #    sentinel_download(site)
    else:
        p.map(landsat_download, sites_aoi_database)
        #used only in debug mode
        #for site in sites_aoi_database:
        #    landsat_download(site)
    print("downloader exit !")

'''

    #this works for KeyboardInterrupt. Due to a python bug, there is no other way to catch a SIGINT and terminate all processes launched with Pool
    original_sigint_handler = signal.signal(signal.SIGINT, signal.SIG_IGN)
    p = Pool(len(sites_aoi_database))
    signal.signal(signal.SIGINT, original_sigint_handler)
    try:        
        if options.remote_host == "s2":        
            res = p.map_async(sentinel_download, sites_aoi_database)
        else:
            res = p.map_async(landsat_download, sites_aoi_database)
        res.get(99999999999999999999999999999) # Without the timeout this blocking call ignores all signals.
    except KeyboardInterrupt:
        print("Caught KeyboardInterrupt, terminating workers")
        p.terminate()
    else:
        print("Normal termination")
        p.join()

'''



