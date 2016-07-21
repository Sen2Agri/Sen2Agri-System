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

  VERSION: 01.00.00 | DATE: <14/03/2015> | COMMENTS: Creation of file.

  END-HISTORY
  _____________________________________________________________________________

  $Id: $
  _____________________________________________________________________________

Checks given DTM product.
Check structure and checks extent of each data.
The given product has to follows this structure:
    OUT/                                                        (1)
    └── Ukraine                                                 (2)
        └── 181025                                              (3)
            ├── Data............................................(4)
            │   ├── Tiles_Dir/                                  (5)
            └── MNT/                                            (6)
                ├── L8_TEST_AUX_REFDE2_181025_0001.DBL.DIR/     (7)
                └── L8_TEST_AUX_REFDE2_181025_0001.HDR

"""

# import system libraries
import argparse
import glob
import os
import sys
import time

# project libraries
import config
from DEM_common import display_parameters, usage, myGlob
from GDAL_Tools.DEM_gdalTools import get_image_info_with_gdal, get_extent


# import GDAL and QGIS libraries
from osgeo import gdal

gdal.UseExceptions()

#import logging for debug messages
import logging
logging.basicConfig()
# create logger
logger = logging.getLogger( 'Check_Product' )
logger.setLevel(config.level_working_modules)
fh = logging.StreamHandler()
formatter = logging.Formatter('%(asctime)s %(name)s %(levelname)s - %(message)s')
fh.setFormatter(formatter)
fh.setLevel(logging.DEBUG)
logger.addHandler(fh)

# hard-coded values
launcher_base = config.launcher
libs = config.app_directory
lib_otb = config.app_otb_directory




def get_arguments():
    """
    Manages inputs
    :return:
    """
    # ####### MANAGING ARGUMENTS ########
    # TODO : set path to downloadwb as an argument

    # check inputs
    logger.info("######## MANAGING ARGUMENTS ########")
    logger.debug("Checking arguments")

    argParser = argparse.ArgumentParser()
    argParser.add_argument('-i', '--input_directory',
                        help='Path to input directory')
    argParser.add_argument('-z', '--input_zone_directory',
                        help='Path to input directory')
    argParser.add_argument('-s', "--sensor", choices=['s2', 'l8'], required=True)

    input_directory = input_zone = None
    logger.debug(len(sys.argv))
    if len(sys.argv) != 3:
        print "Nb args = ", len(sys.argv), " required: 3."
        usage(argParser)

    args = argParser.parse_args(sys.argv[1:])
    input_directory = args.input_directory
    input_zone = args.input_zone_directory
    sensor = args.sensor
    logger.debug("Arguments ok")

    if input_directory:
        input_directory = os.path.realpath(input_directory)
        # creating directories if they don't exist
        logger.debug("Managing input directories")
        if not os.path.isdir(input_directory):
            usage(argParser)
            sys.exit(2)

    if input_zone:
        input_zone = os.path.realpath(input_zone)
        # creating directories if they don't exist
        logger.debug("Managing input directories")
        if not os.path.isdir(input_zone):
            usage(argParser)
            sys.exit(2)

    display_parameters(locals(), "checkdem_get_arguments")
    return input_directory, input_zone, sensor


def checkExtent(mntProductDirectory, extent, supportImages):
    if supportImages:
        extent = get_extent(supportImages["R1"])

    if supportImages:
        for key, image in supportImages.iteritems():
            tifExtent = get_extent(image)
            if extent != tifExtent:
                print "Extent does not match R1 and", key

    for demfile in myGlob(mntProductDirectory, ".TIF"):
        tifExtent = get_extent(demfile)
        if extent != tifExtent:
            print "Extent does not match R1 and", demfile




def checkMNTStructure(mntProductDirectory, pathRow, sensor):
    """
    It has to follow this structure:
    MNT/
    ├── L8_TEST_AUX_REFDE2_181025_0001.DBL.DIR
    │       ├── L8_TEST_AUX_REFDE2_181025_0001_ALC.TIF
    │       ├── L8_TEST_AUX_REFDE2_181025_0001_ALT.TIF
    │       ├── L8_TEST_AUX_REFDE2_181025_0001_ASC.TIF
    │       ├── L8_TEST_AUX_REFDE2_181025_0001_ASP.TIF
    │       ├── L8_TEST_AUX_REFDE2_181025_0001_SLC.TIF
    │       └── L8_TEST_AUX_REFDE2_181025_0001_SLP.TIF
    └── L8_TEST_AUX_REFDE2_181025_0001.HDR

    :param mntProductDirectory:
    :return:
    """
    if sensor=="l8":
        mntBasename = "L8_TEST_AUX_REFDE2_" + str(pathRow) + "_0001"
        expectedNumberOfFiles = 7
    elif sensor == "s2":
        mntBasename = "S2__TEST_AUX_REFDE2_" + str(pathRow) + "____5001"
        expectedNumberOfFiles = 10

    content = os.listdir(mntProductDirectory)
    # if not len(content) == 2:
    #     print "Error: MNT product must contain the HDR and the MNT directory"
    #     print "Found: ", len(content), "elements"
    #     sys.exit(2)

    hdrFile = os.path.join(mntProductDirectory, mntBasename + ".HDR")
    print hdrFile
    if not os.path.isfile(hdrFile):
        print "Error: MNT product should have a correct hdr"
        print "Expected: ", hdrFile
        hdrFiles = glob.glob(os.path.join(mntProductDirectory, ".HDR"))
        if hdrFiles:
            print "There may have a bad name."
            print "Some HDR found :"
            print " ".join(hdrFiles)
        sys.exit(2)

    mntDir = os.path.join(mntProductDirectory, mntBasename + ".DBL.DIR")
    if not os.path.isdir(mntDir):
        print "Error: MNT product should have a correct MNT directory"
        print "Expected: ", mntDir
        sys.exit(2)

    contentMntDir = glob.glob(os.path.join(mntDir, "*"))
    contentMntDir = [x for x in contentMntDir if not x.endswith(".aux.xml")]
    if not len(contentMntDir) == expectedNumberOfFiles:
        print "Error: Bad structure: MNT directory should contain 7 files"
        print "Found: ", len(contentMntDir)
        if len(contentMntDir) == expectedNumberOfFiles-1:
            print "Is the MSK missing ?"
            print "Did you forget to set the proxy ?"
        sys.exit(2)

    for mntfilecontent in contentMntDir:
        checkFile(mntfilecontent)


    if sensor=="l8":
        mntFile = os.path.join(mntDir, mntBasename + "_ALT.TIF")
    elif sensor == "s2":
        mntFile = os.path.join(mntDir, mntBasename + "_ALT_R1.TIF")
    if not os.path.isfile(mntFile):
        print "Error: Bad structure: MNT file seems to be missing"
        print "Expected: ", mntFile
        sys.exit(2)

    return mntFile, mntDir



def checkFile(inputFile):
    res = get_image_info_with_gdal(inputFile, get_stats=True)
    codeEPSG, a_x, a_y, b_x, b_y, spacing_x, spacing_y, sizeX, sizeY, stats = res

    if spacing_x == 0 or spacing_y == 0:
        print "ERROR: Wrong spacing.", inputFile
        sys.exit(4)
    if sizeX == 0 or sizeY == 0:
        print "ERROR: Wrong size.", inputFile
        sys.exit(4)
    if stats[2] == 0:
        print "WARNING: Null mean", inputFile


def run_check_mnt(MNTDir, pathRow, sensor, supportImages=None):

    display_parameters(locals(), "run_check_mnt")
    if not os.path.isdir(MNTDir):
        print "Bad structure"
        print "Required:", MNTDir

    #checking mnt
    mntFile, mntDir = checkMNTStructure(MNTDir, os.path.basename(pathRow), sensor)
    extent = get_extent(mntFile)
    print "\t Checking on extent", extent

    checkExtent(mntDir, extent, supportImages)


def run_check_data(dataDir):
        extent = [0, 0, 0, 0]
        #checking tiles
        for tile in [x for x in glob.glob(os.path.join(dataDir, "*")) if os.path.isdir(x)]:
            print "\t\t Checking", os.path.basename(tile)
            tifFiles = [x for x in glob.glob(os.path.join(tile, "*.TIF"))]
            for tifFile in tifFiles:
                extentTIF = get_extent(tifFile)
                # checkFile(tifFile)
                # res = get_image_info_with_gdal(tifFile, get_stats=True)
                # codeEPSG_TIF, a_x_TIF, a_y_TIF, b_x_TIF, b_y_TIF, spacing_x_TIF, spacing_y_TIF, sizeX_TIF, sizeY_TIF, stats = res
                if extentTIF != extent:
                    print "ERROR: tif file does not have the same extent !!"
                    print "Expected:", extent
                    print "Current: ", extentTIF
                    sys.exit(3)


def run_check_zone(zone, sensor):
    for pathRow in [x for x in glob.glob(os.path.join(zone, "*")) if os.path.isdir(x)]:
            print "\t Checking", os.path.basename(pathRow)
            dataDir = os.path.join(pathRow, "Data")
            MNTDir = os.path.join(pathRow, "MNT")
            if not os.path.isdir(dataDir):
                print "Bad structure"
                print "Required:", dataDir
            run_check_mnt(MNTDir, pathRow, sensor)
            run_check_data(dataDir)

def run_check_all(input_directory, sensor):
    """
    OUT/                                                        (1)
    └── Ukraine                                                 (2) = zone
        └── 181025                                              (3) = pathRow
            ├── Data............................................(4) = dataDir
            │   ├── Tiles_Dir/                                  (5) = tile
            └── MNT/                                            (6) = MNTDir
                ├── L8_TEST_AUX_REFDE2_181025_0001.DBL.DIR/     (7) =
                └── L8_TEST_AUX_REFDE2_181025_0001.HDR

    :param input_directory:
    :return:
    """

    time_start = time.time()
    # extent_wgs84_vector, extent_wgs84, extent_feature = get_extent_from_l8_extent(l8_vector, path, row, working_dir, "4326")
    for zone in [x for x in glob.glob(os.path.join(input_directory, "*")) if os.path.isdir(x)]:
        print "Checking", os.path.basename(zone)
        run_check_zone(zone, sensor)



    interval = time.time() - time_start
    logger.info('Total time in seconds:' + str(interval))


if __name__ == '__main__':
    input_directory, input_zone, sensor = get_arguments()
    if input_directory:
        run_check_all(input_directory, sensor)
    else:
        run_check_zone(input_zone, sensor)