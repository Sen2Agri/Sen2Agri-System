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


import sys
import os
import config
import re
import time
import argparse
import shutil

#import project libraries
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
    argParser.add_argument('-i', '--input_metadata', required=True,
                                help='Path to input metadata ex: /media/Input/China/122034/LC81220342014288LGN00/LC81220342014288LGN00_MTL.txt')
    argParser.add_argument('-o', '--output_directory', required=True,
                                help='Path to output directory ex: /tmp/China')
    argParser.add_argument('-d', '--srtm_directory', required=True,
                                help='Path to SRTM directory ex: /media/mysrtm ')
    argParser.add_argument('-w', '--working_directory', required=True,
                                help='Path to working directory ex: /tmp/wd')
    argParser.add_argument('-s', "--sensor", choices=['s2', 'l8'], required=True)
    argParser.add_argument('-v', '--l8_vector',
                            help='Path to l8 vector ex: wrs2_descending/wrs2_descending.shp')

    args = argParser.parse_args(sys.argv[1:])

    input_metadata = os.path.realpath(args.input_metadata)
    sensor = args.sensor
    srtm_dir = args.srtm_directory
    working_dir = args.working_directory
    l8_vector = args.l8_vector
    output_directory = os.path.realpath(args.output_directory)

    # creating directories if they don't exist
    if not os.path.isdir(working_dir):
        os.mkdir(working_dir)
    if not os.path.isdir(output_directory):
        os.mkdir(output_directory)
    # checking srtm directory existence
    if not os.path.isdir(srtm_dir):
        print "ERROR srtm dir is missing"
        usage(argParser)

    if sensor == "l8" and not args.l8_vector:
        print "Missing L8 extent vector"
        usage(argParser)


    # checking if inputs exists
    if not input_metadata or not os.path.isfile(input_metadata):
        print "input metadata are missing"
        sys.exit(2)

    return input_metadata, srtm_dir, working_dir, sensor, l8_vector, output_directory


def getPathRow(metadata):

    # get path, row of the product
    #txt = os.path.join(os.path.dirname(input_image), tile_id + "_MTL.txt")
    if not os.path.isfile(metadata):
        print "Input product incomplete. Missing txt", metadata
        sys.exit()
    else:
        path, row = "", ""
        with open(metadata, 'r') as fd:
            for line in fd:
                if re.search("\s*WRS_PATH = (\d*)", line) is not None:
                    path = int(re.search("\s*WRS_PATH = (\d*)", line).group(1))
                elif re.search("\s*WRS_ROW = (\d*)", line) is not None:
                    row = int(re.search("\s*WRS_ROW = (\d*)", line).group(1))
            logging.debug("path {path}, row {row}".format(path=path, row=row))

            return path, row


def composeMNTProductFromOutputFiles(outputFiles, output_directory, tileId):

    # create outputDIr/MNT
    # create outputDIr/MNT/XX.DBL.DIR
    mnt_output_dir = os.path.join(output_directory, tileId + ".DBL.DIR")
    if not os.path.isdir(mnt_output_dir):
        os.makedirs(mnt_output_dir)

    logging.debug(outputFiles)
    filenames = { 'MNT_Coarse': "_".join([tileId, "ALC.TIF"]),
                  'Slope_Coarse': "_".join([tileId, "SLC.TIF"]),
                  'Aspect_Coarse': "_".join([tileId, "ASC.TIF"]),
                  'WB_Coarse': "_".join([tileId, "MSK.TIF"])
                }

    if len(outputFiles)>7:
        #multi resolution
        filenames['MNT_R1'] = "_".join([tileId, "ALT_R1.TIF"])
        filenames['Slope_R1'] = "_".join([tileId, "SLP_R1.TIF"])
        filenames['Aspect_R1'] = "_".join([tileId, "ASP_R1.TIF"])
        filenames['MNT_R2'] = "_".join([tileId, "ALT_R2.TIF"])
        filenames['Slope_R2'] = "_".join([tileId, "SLP_R2.TIF"])
        filenames['Aspect_R2'] = "_".join([tileId, "ASP_R2.TIF"])
    else:
        #one resolution
        filenames['MNT_R1'] = "_".join([tileId, "ALT.TIF"])
        filenames['Slope_R1'] = "_".join([tileId, "SLP.TIF"])
        filenames['Aspect_R1'] = "_".join([tileId, "ASP.TIF"])

    # for each file from output file
    for key, value in filenames.iteritems():
        # move it to outputDIr/MNT/XX.DBL.DIR with the good name
        newFile = os.path.join(mnt_output_dir, value)
        print "Move", outputFiles[key], "to", newFile
        shutil.move(outputFiles[key], newFile)

    return mnt_output_dir


def getS2AgriDEMWBProduct(input_metadata, srtm_dir, working_dir, sensor, l8_vector, output_directory):

    time_start = time.time()
    display_parameters_p(locals(), "getS2AgriDEMWBProduct")

    if not input_metadata:
        print "WARNING ! Missing input metadata !"
        return


    if sensor == "l8":
        path, row = getPathRow(input_metadata)
        print "path row",path, row
        data_output_dir = os.path.join(output_directory, "Data")
        if not os.path.isdir(data_output_dir):
            os.makedirs(data_output_dir)
        output_product_dir_data = l8_align(os.path.dirname(input_metadata), l8_vector,
                                           data_output_dir,
                                           working_dir, path, row)
        keyPathRow = "%03d"%(int(path))+"%03d"%(int(row))
        input_metadata = searchOneFile(output_product_dir_data, "*.txt")
        tileId = "L8_TEST_AUX_REFDE2_{}_0001".format(keyPathRow)
    elif sensor == 's2':
        s2tileId = (os.path.splitext(os.path.basename(input_metadata))[0]).split("_")[-1][1:]
        keyPathRow = s2tileId
        tileId = "S2__TEST_AUX_REFDE2_{}____5001".format(s2tileId)


    mnt_output_dir = os.path.join(output_directory, "MNT")
    if not os.path.isdir(mnt_output_dir):
        os.makedirs(mnt_output_dir)
    else:
        print "existing output mnt", mnt_output_dir

    if not os.path.isdir(os.path.join(mnt_output_dir, tileId + ".DBL.DIR")):

        outputFiles, supportImages = getDEMWBProduct(input_metadata, srtm_dir, working_dir, sensor)

        mnt_output_dir_dbl_dir = composeMNTProductFromOutputFiles(outputFiles, mnt_output_dir, tileId)

        hdr_filename = os.path.join(mnt_output_dir, tileId + ".HDR")
        logging.debug("hdr_filename {}".format(hdr_filename))

        if not os.path.isfile(hdr_filename):
            hdr_creation(mnt_output_dir_dbl_dir, hdr_filename, sensor)


        run_check_mnt(mnt_output_dir, keyPathRow, sensor, supportImages)

    interval = time.time() - time_start
    logger.info('Total time in seconds:' + str(interval))




if __name__ == '__main__':
    input_metadata, srtm_dir, working_dir, sensor, l8_vector, output_directory = get_arguments()
    getS2AgriDEMWBProduct(input_metadata, srtm_dir, working_dir, sensor, l8_vector, output_directory)


