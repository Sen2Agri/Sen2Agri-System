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
from DEM_common import usage, display_parameters, display_parameters_p, searchOneFile, myGlob
from GDAL_Tools.DEM_gdalTools import get_extent


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

    args = argParser.parse_args(sys.argv[1:])

    input_zone = os.path.realpath(args.input_zone)

    # checking if inputs exists
    if not input_zone or not os.path.isdir(input_zone):
        print "input zone is missing"
        sys.exit(2)

    return input_zone



def checkAllL8S2AgriDEMWBProduct(input_zone):

    time_start = time.time()
    display_parameters_p(locals(), "getAllL8S2AgriDEMWBProduct")

    listdir = os.listdir(input_zone)
    rePathRow = "\d{6}$"
    inputPathRowDirs = [os.path.join(input_zone,f) for f in listdir if re.search(rePathRow, f) and
                os.path.isdir(os.path.join(input_zone,f))]

    print inputPathRowDirs
    for pathrow in inputPathRowDirs:
        data_dir = os.path.join(pathrow, "Data")
        if not os.path.isdir(data_dir):
            print "Data dir missing"
            print "expected {}".format(data_dir)
            sys.exit(2)

        mnt_up_dir = os.path.join(pathrow, "MNT")
        if not os.path.isdir(mnt_up_dir):
            print "MNT dir missing"
            print "expected {}".format(mnt_up_dir)
            sys.exit(2)
        mnt_dir = searchOneFile(mnt_up_dir, "*.DBL.DIR")
        alt_file = searchOneFile(mnt_dir, "*ALT.TIF")

        extent = get_extent(alt_file)

        # check all extent
        for imageFile in myGlob(mnt_dir, "*.TIF") + myGlob(data_dir, "*/*.TIF"):
            extent_ = get_extent(imageFile)
            if extent_ != extent:
                print "Extent does not match !"
                print "Extent of ALT file : {}".format(extent)
                print "Extent of {} : {}".format(os.path.basename(imageFile), extent_)

    interval = time.time() - time_start
    logger.info('Total time in seconds:' + str(interval))




if __name__ == '__main__':
    input_zone = get_arguments()
    checkAllL8S2AgriDEMWBProduct(input_zone)


