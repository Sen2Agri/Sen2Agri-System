# -*- coding: utf-8 -*-
"""
_____________________________________________________________________________

   Program:         DEM&WaterBodyModule

   Author:          CS SI, (Alexia Mondot alexia.mondot@c-s.fr)
   Copyright:       CS SI
   Licence:         See Licence.txt

   Language:        Python
  _____________________________________________________________________________

  HISTORY

  VERSION: 01.00.00 | DATE: <02/05/2016> | COMMENTS: Creation of file.

  END-HISTORY
  _____________________________________________________________________________

  $Id: $
  _____________________________________________________________________________

This script allows to create a DTM product according to an input image.
It produces the following files :
    - DTM at L2 resolution
    - DTM at L2_Coarse resolution
    - WaterBodies  at L2_Coarse resolution
    - Slope at L2 resolution
    - Slope at L2_Coarse resolution
    - Aspect at L2 resolution
    - Aspect at L2_Coarse resolution
"""


import sys
import os
import config
import re
import time
import argparse
import shutil

#import project libraries
from s2agri_dem_product import getS2AgriDEMWBProduct
from DEM_common import usage, display_parameters, display_parameters_p, searchOneFile
from l8_align import l8_align
from hdr_creation import hdr_creation
from GenerateDEMWBProduct import getDEMWBProduct
from DEM_checkProduct import run_check_mnt


#import logging for debug messages
import logging
logging.basicConfig()
# create logger
logger = logging.getLogger( 'S2Agri_DEM_Product' )
logger.setLevel(config.level_working_modules)
fh = logging.StreamHandler()
formatter = logging.Formatter('%(asctime)s %(name)s %(levelname)s - %(message)s')
fh.setFormatter(formatter)
fh.setLevel(logging.DEBUG)
logger.addHandler(fh)


def get_arguments():
    """
    Manages inputs
    :return:
    """
    argParser = argparse.ArgumentParser()
    argParser.add_argument('-i', '--input_zone', required=True,
                                help='Path to input zone ex: /media/Input/China/')
    argParser.add_argument('-o', '--output_directory', required=True,
                                help='Path to output directory ex: /tmp/')
    argParser.add_argument('-d', '--srtm_directory', required=True,
                                help='Path to SRTM directory ex: /media/mysrtm ')
    argParser.add_argument('-w', '--working_directory', required=True,
                                help='Path to working directory ex: /tmp/wd')
    argParser.add_argument('-s', "--sensor", choices=['s2', 'l8'], required=True)
    argParser.add_argument('-v', '--l8_vector',
                            help='Path to l8 vector ex: wrs2_descending/wrs2_descending.shp')

    args = argParser.parse_args(sys.argv[1:])

    input_zone = os.path.realpath(args.input_zone)
    sensor = args.sensor
    srtm_dir = args.srtm_directory
    working_dir = args.working_directory
    l8_vector = args.l8_vector
    output_directory = os.path.realpath(args.output_directory)

    # creating directories if they don't exist
    wd_zone = os.path.join(working_dir, os.path.basename(input_zone))
    if not os.path.isdir(wd_zone):
        os.makedirs(wd_zone)

    out_zone = os.path.join(output_directory, os.path.basename(input_zone))
    if not os.path.isdir(out_zone):
        os.makedirs(out_zone)
    # checking srtm directory existence
    if not os.path.isdir(srtm_dir):
        print "ERROR srtm dir is missing"
        usage(argParser)

    if sensor == "l8" and not args.l8_vector:
        print "Missing L8 extent vector"
        usage(argParser)


    # checking if inputs exists
    if not input_zone or not os.path.isdir(input_zone):
        print "input metadata are missing"
        sys.exit(2)

    return input_zone, srtm_dir, wd_zone, sensor, l8_vector, out_zone




def getAllL8S2AgriDEMWBProduct(input_zone, srtm_dir, working_dir, sensor, l8_vector, output_directory):

    time_start = time.time()
    display_parameters_p(locals(), "getAllL8S2AgriDEMWBProduct")

    listdir = os.listdir(input_zone)
    rePathRow = "\d{6}$"
    inputPathRowDirs = [os.path.join(input_zone,f) for f in listdir if re.search(rePathRow, f) and
                os.path.isdir(os.path.join(input_zone,f))]

    print inputPathRowDirs
    for pathrow in inputPathRowDirs:
        wd_pathrow = os.path.join(working_dir, os.path.basename(pathrow))
        if not os.path.isdir(wd_pathrow):
            os.mkdir(wd_pathrow)
        print wd_pathrow
        out_pathrow = os.path.join(output_directory, os.path.basename(pathrow))
        print out_pathrow
        if not os.path.isdir(out_pathrow):
            os.mkdir(out_pathrow)

        for tile in [os.path.join(pathrow, x) for x in os.listdir(pathrow)]:
            metadata = searchOneFile(tile, "*_MTL.txt")
            print "metadata", metadata
            if not metadata:
                print "WARNING ! Missing input metadata"
                print "Search path", tile
            else:
                getS2AgriDEMWBProduct(metadata, srtm_dir, wd_pathrow, "l8", l8_vector, out_pathrow)

    interval = time.time() - time_start
    logger.info('Total time in seconds:' + str(interval))




if __name__ == '__main__':
    input_zone, srtm_dir, working_dir, sensor, l8_vector, output_directory = get_arguments()
    getAllL8S2AgriDEMWBProduct(input_zone, srtm_dir, working_dir, sensor, l8_vector, output_directory)


