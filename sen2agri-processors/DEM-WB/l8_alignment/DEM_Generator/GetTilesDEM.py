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

  VERSION: 01.00.00 | DATE: <17/03/2016> | COMMENTS: Creation of file.

  END-HISTORY
  _____________________________________________________________________________

  $Id: $
  _____________________________________________________________________________

"""
# import system libraries
import sys
import os
import glob
import shutil
import math
import time
import argparse
import uuid

# project libraries
import config
from DEM_common import display_parameters, usage, unzip_files, getBasename
from DEMGeneratorCommon import  bandmath

# import logging for debug messages
import logging

logging.basicConfig()
# create logger
logger = logging.getLogger('Get_Tiles_DEM')
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
    argParser.add_argument('-d', '--srtm_directory', required=True,
                           help='Path to SRTM directory')
    argParser.add_argument('-e', '--extent_wgs84', required=True,
                           help='Extent of the required MNT in WGS84')
    argParser.add_argument('-w', '--working_directory', required=True,
                           help='Path to working directory')

    args = argParser.parse_args(sys.argv[1:])

    srtm_dir = os.path.realpath(args.srtm_directory)
    extentWgs84 = args.extent_wgs84
    working_dir = args.working_directory

    logger.debug("Arguments ok")

    logger.debug("Managing output directories")
    # checking existence of input srtm dir
    if not os.path.isdir(srtm_dir):
        print "ERROR Missing input srtm directory"
        usage(argParser)
    # creating directory if it does not exist
    if not os.path.isdir(working_dir):
        os.mkdir(working_dir)
    # checking extent
    if not extentWgs84:
        print "Extent is missing"
        usage(argParser)
        sys.exit(2)

    return srtm_dir, extent_wgs84, working_dir


def get_tiles(points):
    """
    Returns a list of dtm tiles covering the given extent

    :param points:
    :return:
    """
    # extract
    a_x, a_y, b_x, b_y = points
    logger.debug("points ")
    logger.debug(points)

    # check a is upper left corner
    if a_x < b_x and a_y > b_y:
        a_bb_x = int(math.floor(a_x / 5) * 5)
        a_bb_y = int(math.floor((a_y + 5) / 5) * 5)
        b_bb_x = int(math.floor((b_x + 5) / 5) * 5)
        b_bb_y = int(math.floor(b_y / 5) * 5)

        #print "bounding box", a_bb_x, a_bb_y, b_bb_x, b_bb_y

        # get list of zip
        x_numbers_list = [((x + 180) / 5) + 1 for x in range(min(a_bb_x, b_bb_x), max(a_bb_x, b_bb_x), 5)]
        x_numbers_list_format = ["%02d" % (x,) for x in x_numbers_list]
        y_numbers_list = [(60 - x) / 5 for x in range(min(a_bb_y, b_bb_y), max(a_bb_y, b_bb_y), 5)]
        y_numbers_list_format = ["%02d" % (x,) for x in y_numbers_list]

        logger.debug(x_numbers_list_format)
        logger.debug(y_numbers_list_format)

        srtm_zips = ["srtm_" + str(x) + "_" + str(y) + ".zip" for x in x_numbers_list_format for y in
                     y_numbers_list_format]

        logger.debug("zip to take ")
        logger.debug(srtm_zips)

        return srtm_zips


def getSRTMTiles(extent_wgs84, srtm_dir, working_dir):
    """
    Get the list of SRTM tiles covering the given extent, copy each in the working dir and unzip all
    :param extent_wgs84:
    :param srtm_dir:
    :param working_dir:
    :return:
    """
    logger.debug(extent_wgs84)
    tiles = get_tiles(extent_wgs84)

    for tile in tiles:
        full_path = os.path.join(srtm_dir, tile)
        logger.debug(full_path)
        if not os.path.isfile(full_path):
            logger.info(tile + " is missing")
        else:
            shutil.copyfile(full_path, os.path.join(working_dir, tile))


def createsDEMVRT(workingDir, extent_wgs84):
    """

    :param workingDir:
    :return:
    """
    extent_string = [str(x).replace(".", "o") for x in extent_wgs84]

    # make a vrt with all srtm tif tiles
    output_vrt = os.path.join(workingDir, "MNT_" +  "-".join(extent_string) + ".VRT")
    if not os.path.isfile(output_vrt):
        logger.debug("output_vrt : " + output_vrt)
        logger.debug("Bulding vrt")
        command_build_vrt = "gdalbuildvrt " + output_vrt
        for srtm in glob.glob(os.path.join(workingDir, "*.tif")):
            command_build_vrt += " " + srtm
        logger.info(command_build_vrt)
        os.system(command_build_vrt)
    logger.debug("VRT ok")
    return output_vrt


def get_tile_dem(srtm_dir, extent_wgs84, working_dir):
    """

    :param srtm_dir:
    :param extent_wgs84:
    :param working_dir:
    :return: VRT of SRTM tiles corresponding to given extent
    """

    time_start = time.time()
    display_parameters(locals(), "get_tile_dem")

    # manage arguments
    if not os.path.isdir(srtm_dir):
        print "ERROR: input SRTM dir is missing"
        sys.exit(1)
    if not os.path.isdir(working_dir):
        os.makedirs(working_dir)

    # get srtm tiles for the given extent
    getSRTMTiles(extent_wgs84, srtm_dir, working_dir)
    unzip_files(working_dir)
    output_vrt = createsDEMVRT(working_dir, extent_wgs84)

    dtm_temp_nodata = os.path.join(working_dir, getBasename(output_vrt) + "_DTM_no_data.tif")
    bandmath([output_vrt], "(im1b1==-32768?0:im1b1)", dtm_temp_nodata)


    interval = time.time() - time_start
    logger.info('Total time in seconds:' + str(interval))
    logger.info("Generated MNT: {}".format(dtm_temp_nodata))
    return dtm_temp_nodata


if __name__ == '__main__':
    srtm_dir, extent_wgs84, working_dir = get_arguments()
    get_tile_dem(srtm_dir, extent_wgs84, working_dir)
