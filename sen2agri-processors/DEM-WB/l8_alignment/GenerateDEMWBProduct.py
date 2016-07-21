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

  VERSION: 01.00.00 | DATE: <29/05/2015> | COMMENTS: Creation of file.

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

Usage :
    python processing.py -i path/to/my/product/directory - -o path/to/my/output/dir
"""

import argparse
import os
import sys

import config
#import project libraries
from DEM_Generator import GenerateDEML1, GenerateDEML2, GetSupportImage, GetTilesDEM
from WB_Generator import GenerateWB
from DEM_common import usage, getBasename
from GDAL_Tools.DEM_gdalTools import get_extent

#import logging for debug messages
import logging
logging.basicConfig()
# create logger
logger = logging.getLogger( 'GenerateDEMWBProduct' )
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
    argParser.add_argument('-i', '--input_metadata', required=True,
                                help='Path to input image')
    argParser.add_argument('-d', '--srtm_directory', required=True,
                                help='Path to SRTM directory')
    argParser.add_argument('-w', '--working_directory', required=True,
                                help='Path to working directory')
    argParser.add_argument('-s', "--sensor", choices=['s2', 'l8'], required=True)

    args = argParser.parse_args(sys.argv[1:])

    input_metadata = os.path.realpath(args.input_metadata)
    sensor = args.sensor
    srtm_dir = args.srtm_directory
    working_dir = args.working_directory

    # creating directories if they don't exist
    if not os.path.isdir(working_dir):
        os.mkdir(working_dir)
    # checking srtm directory existence
    if not os.path.isdir(srtm_dir):
        print "ERROR srtm dir is missing"
        usage(argParser)

    # checking if inputs exists
    if not input_metadata or not os.path.isfile(input_metadata):
        print "input directory missing"
        sys.exit(2)

    return input_metadata, srtm_dir, working_dir, sensor


def getDEMWBProduct(input_metadata, srtm_dir, working_dir, sensor):

    supportImages = GetSupportImage.get_support_images(input_metadata, sensor, working_dir)

    # input_image = ""
    extent_wgs84 = get_extent(supportImages["R1"], 4326)

    input_mnt = GetTilesDEM.get_tile_dem(srtm_dir, extent_wgs84, working_dir)
    outputFiles = {}

    for key, support_image in supportImages.iteritems():
        output_mnt = os.path.join(working_dir, getBasename(input_mnt) + "_" + key + ".TIF")
        GenerateDEML1.generate_dem_l1(input_mnt, support_image, output_mnt)
        outputFiles["MNT_"+key] = output_mnt
        slope, aspect = GenerateDEML2.generate_dem_l2(output_mnt, working_dir)
        outputFiles["Slope_"+key] = slope
        outputFiles["Aspect_"+key] = aspect
    if "MNT_Coarse" in outputFiles:
        outputFiles["WB_Coarse"] = GenerateWB.generate_wb(outputFiles["MNT_Coarse"], working_dir)
    else:
        print "ERROR: MNT coarse is missing."

    # print outputFiles
    return outputFiles, supportImages


if __name__ == '__main__':
    input_metadata, srtm_dir, working_dir, sensor = get_arguments()
    getDEMWBProduct(input_metadata, srtm_dir, working_dir, sensor)
