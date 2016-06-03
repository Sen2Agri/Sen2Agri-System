#!/usr/bin/env python
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

  VERSION: 01.00.00 | DATE: <29/06/2015> | COMMENTS: Creation of file.

  END-HISTORY
  _____________________________________________________________________________

  $Id: $
  _____________________________________________________________________________

This script allows to align product on the bb of the given shapefile

Usage :
    python processing.py -i path/to/my/product/directory - -o path/to/my/output/dir
"""
# import system libraries
import argparse
import glob
import os
import re
import sys
import time

# project libraries
import config

# import GDAL and QGIS libraries
from osgeo import gdal

gdal.UseExceptions()
from DEM_common import display_parameters, usage, searchOneFile
from GDAL_Tools.DEM_gdalTools import (crop_with_mask, crop_with_extent, get_image_info_with_gdal,
                                      get_feature_enveloppe, convert_extent, get_extent_from_l8_extent)
from GDAL_Tools.gdalinfoO import gdalinfoO
from DEM_Generator.DEMGeneratorCommon import  bandmath

# import logging for debug messages
import logging
logging.basicConfig()
# create logger
logger = logging.getLogger('l8_align')
logger.setLevel(config.level_debug_modules)
fh = logging.StreamHandler()
formatter = logging.Formatter('%(asctime)s %(name)s %(levelname)s - %(message)s')
fh.setFormatter(formatter)
fh.setLevel(logging.DEBUG)
logger.addHandler(fh)

# hard-coded values
launcher_base = config.launcher
libs = config.app_directory
lib_otb = config.app_otb_directory



def update_txt(txt_file, shape_env_points, output_dir, epsg_code):
    """
    Update MTL file with new extent

    :param txt_file:
    :param shape_env_points:
    :param output_dir:
    :param epsg_code:
    :return:
    """
    # convert shape_env_points in meters in lat/long
    shape_env_points_wgs84 = convert_extent(shape_env_points, epsg_code)
    logger.debug(shape_env_points)

    logger.debug("reading " + txt_file)
    f_in = open(txt_file, 'r')

    file_out = os.path.join(output_dir, os.path.basename(txt_file))
    logger.debug(file_out)
    f_out = open(file_out, 'w')

    for line in f_in:
        # UTM coordinates
        if "CORNER_UL_PROJECTION_X_PRODUCT" in line:
            line = "    CORNER_UL_PROJECTION_X_PRODUCT = " + str(shape_env_points[0]) + "\n"
        if "CORNER_UL_PROJECTION_Y_PRODUCT" in line:
            line = "    CORNER_UL_PROJECTION_Y_PRODUCT = " + str(shape_env_points[1]) + "\n"
        if "CORNER_UR_PROJECTION_X_PRODUCT" in line:
            line = "    CORNER_UR_PROJECTION_X_PRODUCT = " + str(shape_env_points[2]) + "\n"
        if "CORNER_UR_PROJECTION_Y_PRODUCT" in line:
            line = "    CORNER_UR_PROJECTION_Y_PRODUCT = " + str(shape_env_points[1]) + "\n"
        if "CORNER_LL_PROJECTION_X_PRODUCT" in line:
            line = "    CORNER_LL_PROJECTION_X_PRODUCT = " + str(shape_env_points[0]) + "\n"
        if "CORNER_LL_PROJECTION_Y_PRODUCT" in line:
            line = "    CORNER_LL_PROJECTION_Y_PRODUCT = " + str(shape_env_points[3]) + "\n"
        if "CORNER_LR_PROJECTION_X_PRODUCT" in line:
            line = "    CORNER_LR_PROJECTION_X_PRODUCT = " + str(shape_env_points[2]) + "\n"
        if "CORNER_LR_PROJECTION_Y_PRODUCT" in line:
            line = "    CORNER_LR_PROJECTION_Y_PRODUCT = " + str(shape_env_points[3]) + "\n"
        # latlong coordinates
        if "CORNER_UL_LAT_PRODUCT" in line:
            line = "    CORNER_UL_LAT_PRODUCT = " + str(shape_env_points_wgs84[0]) + "\n"
        if "CORNER_UL_LON_PRODUCT" in line:
            line = "    CORNER_UL_LON_PRODUCT = " + str(shape_env_points_wgs84[1]) + "\n"
        if "CORNER_UR_LAT_PRODUCT" in line:
            line = "    CORNER_UR_LAT_PRODUCT = " + str(shape_env_points_wgs84[2]) + "\n"
        if "CORNER_UR_LON_PRODUCT" in line:
            line = "    CORNER_UR_LON_PRODUCT = " + str(shape_env_points_wgs84[1]) + "\n"
        if "CORNER_LL_LAT_PRODUCT" in line:
            line = "    CORNER_LL_LAT_PRODUCT = " + str(shape_env_points_wgs84[0]) + "\n"
        if "CORNER_LL_LON_PRODUCT" in line:
            line = "    CORNER_LL_LON_PRODUCT = " + str(shape_env_points_wgs84[3]) + "\n"
        if "CORNER_LR_LAT_PRODUCT" in line:
            line = "    CORNER_LR_LAT_PRODUCT = " + str(shape_env_points_wgs84[2]) + "\n"
        if "CORNER_LR_LON_PRODUCT" in line:
            line = "    CORNER_LR_LON_PRODUCT = " + str(shape_env_points_wgs84[3]) + "\n"

        f_out.write(line)

    f_in .close()
    f_out.close()
    print "MTD ok"



def getPathRowFromMTL(mtlFile):
    if not os.path.isfile(mtlFile):
        print "Input prodict incomplete. Missing txt", mtlFile
        sys.exit()
    else:
        path, row = "", ""
        with open(mtlFile, 'r') as fd:
            for line in fd:
                if re.search("\s*WRS_PATH = (\d*)", line) is not None:
                    path = int(re.search("\s*WRS_PATH = (\d*)", line).group(1))
                elif re.search("\s*WRS_ROW = (\d*)", line) is not None:
                    row = int(re.search("\s*WRS_ROW = (\d*)", line).group(1))
            logging.debug("path {path}, row {row}".format(path=path, row=row))
    return path, row


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
    required_arguments = argParser.add_argument_group('required arguments')
    required_arguments.add_argument('-i', '--input_directory', required=True,
                                    help='Path to input image')
    required_arguments.add_argument('-o', '--output_directory', required=True,
                                    help='Path to output directory')
    required_arguments.add_argument('-v', '--l8_vector', required=True,
                                    help='Path to l8 vector ex: wrs2_descending/wrs2_descending.shp')
    required_arguments.add_argument('-w', '--working_directory', required=True,
                                    help='Path to working directory')
    required_arguments.add_argument('-t', '--tile_id', required=True,
                                    help='Current product tile id')

    logger.debug(len(sys.argv))
    if len(sys.argv) != 11:
        usage(argParser)

    args = argParser.parse_args(sys.argv[1:])
    input_directory = os.path.realpath(args.input_directory)
    output_directory = os.path.realpath(args.output_directory)
    l8_vector = args.l8_vector
    working_dir = args.working_directory
    tile_id = args.tile_id

    logger.debug("Arguments ok")

    # crating directories if they don't exist
    logger.debug("Managing output directories")
    if not os.path.isdir(output_directory):
        os.mkdir(output_directory)
    if not os.path.isdir(working_dir):
        os.mkdir(working_dir)
    if not os.path.isdir(input_directory):
        print "Error, input dir is missing "
        usage(argParser, 2)
    logger.debug("Output directories ok")

    # checking if inputs exist
    if not input_directory:
        print "input image missing"
        sys.exit(2)
    if not l8_vector:
        print "l8_vector missing"
        sys.exit(2)

    # get path, row of the product
    txt = os.path.join(input_directory, tile_id + "_MTL.txt")
    path, row = getPathRowFromMTL(txt)

    display_parameters(locals(), "l8_align_get_arguments")
    return input_directory, l8_vector, output_directory, working_dir, path, row


def preprocessl8(tile_directory, l8_vector,
                         output_directory_path_row_data, working_dir, path, row):
    """
    Generates l8 data structure and call l8_align

    OUT/                                                        (1) NA
    └── Ukraine                                                 (2) NA
        └── 181025                                              (3) NA
            ├── Data............................................(4) = output_directory_path_row_data
            │   ├── Tiles_Dir/                                  (5) = output_product_dir_data
            └── MNT/                                            (6) NA
                ├── L8_TEST_AUX_REFDE2_181025_0001.DBL.DIR/     (7) NA
                └── L8_TEST_AUX_REFDE2_181025_0001.HDR

    :param tile_directory:
    :param l8_vector:
    :param output_directory_path_row_data:
    :param working_dir:
    :param path:
    :param row:
    :return:
    """
    pass



def l8_align(input_dir, l8_vector, output_directory_path_row_data, working_dir, path=0, row=0):
    """
    Runs the crop of all input data
    # crop all input tiles to match l8 real extent
    :param input_dir:
    :param l8_vector:
    :param output_data_directory:
    :param working_dir:
    :param path:
    :param row:
    :return:
    """
    display_parameters(locals(), "l8_align")
    time_start = time.time()

    extent_wgs84_vector, extent_wgs84, extent_feature = get_extent_from_l8_extent(l8_vector, path, row, working_dir, "4326")

    #creates the output data directory containing cropped images (5)
    output_product_dir_data = os.path.join(output_directory_path_row_data,
                                          os.path.basename(os.path.normpath(input_dir)))
    logging.debug("ouput_product_dir_data {}".format(output_product_dir_data))
    if not os.path.isdir(output_product_dir_data):
        os.makedirs(output_product_dir_data)

        image = searchOneFile(input_dir, "*_B1.TIF")
        if not image:
            print "B1 is missing"
            sys.exit(2)
        logger.debug(image)

        # txt file
        logging.debug(glob.glob(os.path.join(input_dir, "*_MTL.txt")))
        txt_file = glob.glob(os.path.join(input_dir, "*_MTL.txt"))[0]
        logger.debug(txt_file)

        epsg_code = get_image_info_with_gdal(image, get_only_epsg = True)
        # print epsg_code

        logging.debug("epsg code{}".format(epsg_code))
        shape_temp = os.path.join(working_dir, os.path.splitext(os.path.basename(l8_vector))[0] + "_reprojected" + os.path.splitext(os.path.basename(l8_vector))[1])
        logger.debug(shape_temp)
        #ogr_command = "ogr2ogr -lco \"OGR_ENABLE_PARTIAL_REPROJECTION\"=TRUE -t_srs EPSG:"  + str(epsg_code) + " -s_srs EPSG:4326" " " + shape_temp + " " + l8_vector #
        ogr_command = 'ogr2ogr -where \'"PATH"=' + str(path) + ' and "ROW"=' + str(row) + '\' -t_srs EPSG:'  + str(epsg_code) + " -s_srs EPSG:4326" " " + shape_temp + " " + l8_vector

        logging.debug(ogr_command)
        os.system(ogr_command)

        #shape_env = os.path.join(working_dir, os.path.splitext(os.path.basename(l8_vector))[0] + "_env" + os.path.splitext(os.path.basename(l8_vector))[1])
        shape_env, shape_env_points = get_feature_enveloppe(shape_temp, working_dir, int(epsg_code))
        update_txt(txt_file, shape_env_points, output_product_dir_data, epsg_code)

        bqa_temp = ""
        bqd_out = ""
        for tif_file in glob.glob(os.path.join(input_dir, "*.TIF")):
            data_out = os.path.join(output_product_dir_data, os.path.basename(tif_file))
            resolution = gdalinfoO(tif_file).getPixelSize()[0]
            scale_factor = float(config.L2_coarse_resolution)
            extent, shape_env_points = get_feature_enveloppe(shape_temp, working_dir, int(epsg_code), scale_factor)

            if "BQA" in tif_file:
                bqa_temp = os.path.join(working_dir, os.path.basename(tif_file))
                bqd_out = data_out
                crop_with_extent(tif_file, shape_env_points, bqa_temp)
            else:
                crop_with_extent(tif_file, shape_env_points, data_out)

            # crop_with_mask(tif_file, extent, data_out)
        if os.path.isfile(bqa_temp):
            image_support = searchOneFile(output_product_dir_data, "*_B1.TIF")
            if not image_support:
                print "B1 is missing in output directory"
                sys.exit(2)
            bandmath([image_support, bqa_temp], "(im1b1==0&&im2b1==0?0:im2b1)", bqd_out)
    else:
        logging.info("{} already processed.".format(output_product_dir_data))

    interval = time.time() - time_start
    logger.info('Total time in seconds:' + str(interval))
    return output_product_dir_data



if __name__ == '__main__':
    input_dir, l8_vector, output_data_directory, working_dir, path, row = get_arguments()
    l8_align(input_dir, l8_vector, output_data_directory, working_dir, path, row)
