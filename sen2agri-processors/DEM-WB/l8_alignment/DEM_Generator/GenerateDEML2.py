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
import time
import argparse

# project libraries
import config
from DEM_common import display_parameters, usage, getBasename
from DEMGeneratorCommon import bandmath

# import logging for debug messages
import logging
logging.basicConfig()
# create logger
logger = logging.getLogger('Generate_DEM_L2')
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
    argParser.add_argument('-i', '--input_mnt', required=True,
                           help='Path to input MNT')
    argParser.add_argument('-w', '--working_directory', required=True,
                           help='Path to working directory')
    # argParser.add_argument('-s', '--slope', required=True,
    #                        help='Path to output slope file')
    # argParser.add_argument('-a', '--aspect', required=True,
    #                        help='Path to output aspect file')

    args = argParser.parse_args(sys.argv[1:])

    input_mnt = os.path.realpath(args.input_mnt)
    # slope = os.path.realpath(args.slope)
    # aspect = os.path.realpath(args.aspect)
    working_dir = os.path.realpath(args.working_directory)

    # checking input mnt
    if not input_mnt or not os.path.isfile(input_mnt):
        print "Input mnt is missing"
        usage(argParser)
        sys.exit(2)
    # creating directory if it does not exist
    if not os.path.isdir(working_dir):
        os.mkdir(working_dir)
    logger.debug("Arguments ok")

    return input_mnt, working_dir  # slope, aspect


def setSeuil(inputImage, seuil, outputFile):
    bandmath([inputImage], "(im1b1<" + str(seuil) + "?0:im1b1)", outputFile)


def get_slope(dtm_l2, working_dir, scale=True):
    """
    Get slope at dtm_l2 resolution
    :param dtm_l2:
    :param working_dir:
    :param scale:
    :return:
    """
    display_parameters(locals(), "get_slope")
    # running gdaldem to get slope
    logger.debug("gdal dem")
    slope_raw = os.path.join(working_dir, getBasename(dtm_l2) + "_slope_raw.tif")
    logger.debug("slope file : " + slope_raw)
    commandGDAL = "gdaldem slope " + dtm_l2 + " "
    if scale:
        commandGDAL += " -compute_edges "
    commandGDAL += slope_raw
    logger.debug("command : " + commandGDAL)
    os.system(commandGDAL)
    logger.debug("gdal dem ok")

    slope_radian = os.path.join(working_dir, getBasename(dtm_l2) + "_slope_rad.tif")
    # la pente est exprimée en degre float 32, on remet en radians * 100 codé sur 16 bits
    # les degres vont de 0 à 90, onr emet entre 0 et pi/2*100
    cmd = "gdal_translate -ot Int16 -scale 0 90 0 157 " + slope_raw + " " + slope_radian
    os.system(cmd)

    # Necessary step ?
    seuil = -500
    output_slope = os.path.join(working_dir, getBasename(dtm_l2) + "_slope.tif")
    bandmath([slope_radian], "(im1b1<" + str(seuil) + "?0:im1b1)", output_slope)

    logger.info("Output Slope")
    logger.info("\t Slope : " + output_slope)

    return output_slope


def get_aspect(dtm_l2, working_dir, scale=True):
    """
    Get aspect at dtm_l2 resolution
    :param dtm_l2:
    :param working_dir:
    :param scale:
    :return:
    """
    display_parameters(locals(), "get_aspect")
    # running gdaldem to get aspect
    logger.debug("gdal dem")
    aspect_raw = os.path.join(working_dir, getBasename(dtm_l2) + "_aspect_raw.tif")
    logger.debug("slope file : " + aspect_raw)
    commandGDAL = "gdaldem aspect " + dtm_l2 + " "
    if scale:
        commandGDAL += " -compute_edges "
    commandGDAL += aspect_raw
    logger.debug("command : " + commandGDAL)
    os.system(commandGDAL)
    logger.debug("gdal dem ok")

    aspect_rad = os.path.join(working_dir, getBasename(dtm_l2) + "_aspect_rad.tif")
    # l'orientation est exprimée en degre float 32, on remet en radians * 100 codé sur 16 bits
    # les degres vont de 0 à 360, on remet entre 0 et 2pi*100
    cmd = "gdal_translate -ot Int16 -scale 0 360 0 628 " + aspect_raw + " " + aspect_rad
    os.system(cmd)

    # Necessary step ?
    seuil = -500
    output_aspect = os.path.join(working_dir, getBasename(dtm_l2) + "_aspect.tif")
    bandmath([aspect_rad], "(im1b1<" + str(seuil) + "?0:im1b1)", output_aspect)

    logger.info("Output aspect")
    logger.info("\t aspect : " + output_aspect)

    return output_aspect


def generate_dem_l2(input_mnt, working_dir):
    """

    :param input_mnt:
    :param working_dir:
    :return:
    """

    time_start = time.time()
    display_parameters(locals(), "get_tile_dem")

    slope = get_slope(input_mnt, working_dir)
    aspect = get_aspect(input_mnt, working_dir)

    interval = time.time() - time_start
    logger.info('Total time in seconds:' + str(interval))
    logger.info("Generated slope: {}".format(slope))
    logger.info("Generated aspect:{}".format(aspect))
    return slope, aspect


if __name__ == '__main__':
    # input_mnt, slope, aspect = get_arguments()
    input_mnt, working_dir = get_arguments()
    generate_dem_l2(input_mnt, working_dir)
