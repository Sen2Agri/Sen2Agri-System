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

  VERSION: 01.00.00 | DATE: <18/03/2016> | COMMENTS: Creation of file.

  END-HISTORY
  _____________________________________________________________________________

  $Id: $
  _____________________________________________________________________________

"""
# import system libraries
import argparse
import datetime
import glob
import os
import sys

# project libraries
import config
from DEM_common import display_parameters, usage, getBasename, unzip_files
from DEM_Generator.DEMGeneratorCommon import bandmath
from GDAL_Tools.DEM_gdalTools import get_image_info_with_gdal

# import logging for debug messages
import logging
logging.basicConfig()
# create logger
logger = logging.getLogger('Generate_WB')
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

    logger.debug("Checking arguments")

    argParser = argparse.ArgumentParser()
    argParser.add_argument('-i', '--input_mnt', required=True,
                           help='Path to input MNT')
    argParser.add_argument('-w', '--working_directory', required=True,
                           help='Path to working directory')

    args = argParser.parse_args(sys.argv[1:])

    input_mnt = os.path.realpath(args.input_mnt)
    working_dir = args.working_directory

    logger.debug("Arguments ok")

    logger.debug("Managing output directories")
    # checking existence of input mnt
    if not os.path.isfile(input_mnt):
        print "ERROR: Missing input image"
        usage(argParser)
    # creating directory if it does not exist
    if not os.path.isdir(working_dir):
        os.mkdir(working_dir)

    return input_mnt, working_dir




def DownloadSWBDTiles_dl(image, output_directory):
    """
    This function calls the otb application DownloadSRTMTiles with the given image.
    The STRM tiles will be downloaded in the outputDirectory.

    Keyword arguments:
        image               --    raster layer to process
        outputDirectory     --    working directory
    """
    if image and output_directory:

        launcher = launcher_base + " DownloadSWBDTiles " + libs
        command = ('%s '
                   '-il "%s" '
                   '-mode "download" '
                   '-mode.download.outdir "%s" '
                   % (launcher,
                      image,
                      output_directory))

        logger.info("command : " + command)
        os.system(command)


def gdal_ressampling(input_image, output_image, ratio_for_otb):
    """
    Applies a ressampling on the given image with the given ratio

    :param input_image:
    :param output_image:
    :param ratio_for_otb:
    :return:
    """
    # ratio 2 : -> 200%

    command = 'gdal_translate -outsize ' + str(ratio_for_otb * 100) + '% ' + \
        str(ratio_for_otb * 100) + '% ' + input_image + ' ' + output_image

    logger.info("command : " + command)
    os.system(command)


def rasterizationWithSupportImage(vectorIn, supportImage, outputDirectory, field = None):
    """
    Runs the Rasterization OTB application.

    Keyword arguments:
        vectorIn         --    vector to rasterize
        supportImage     --    support image
        outputDirectory  --    working directory
        field            --    specific field to rasterize
    """

    if vectorIn and supportImage and outputDirectory:
        # create the output
        if not field:
            field = ""
        filenameWithoutExtension = os.path.splitext(vectorIn)[0]
        output = os.path.join(outputDirectory, filenameWithoutExtension + "_" + \
                               field + "_raster.tif")  # + temp[index:]

        # create the Segmentation application
        command = ('%s '
                    '-in "%s" '
                    '-out "%s" uint8 '
                    '-im "%s" '
                    '-background "%s" '
                    % ("otbcli_Rasterization",
                        vectorIn,
                        output,
                        supportImage,
                        str(0)))
        if field:
            command += '-mode "attribute" '
            command += '-mode.attribute.field ' + field
        else:
            command += '-mode "binary" '
            command += '-mode.binary.foreground "1"'

        logger.info("command : " + command)
        os.system(command)

        return output
    return None


def clipVectorWithOGR_Extent(vectorIn, extent, outputDirectory, keyword = "", sqlite = False):
    """
    Calls the command ogr2ogr to clip the vector layer to the given extent.

    Keyword Arguments :
        vectorIn        --    vector to crop
        extent          --    list of the 4 coordinates to define the extent
        outputDirectory --    directory to save the cropped file

    Returns the path of the cropped shapefile
    """
    if sqlite:
        newvector = os.path.join(outputDirectory, os.path.basename(vectorIn)[:-7] + "_cropped_" + datetime.datetime.now().strftime("%H%M%S") + keyword + ".shp")
    else:
        newvector = os.path.join(outputDirectory, os.path.basename(vectorIn)[:-4] + "_cropped_" + datetime.datetime.now().strftime("%H%M%S") + keyword + ".shp")
    commandOGR = "ogr2ogr -clipsrc " + str(extent[0]) + " " + str(extent[1]) + " " + str(extent[2]) + " " + str(extent[3]) + " " + newvector + " " + vectorIn
    os.system(commandOGR)
    return newvector


def generate_wb(dtm_l2_coarse, working_dir):
    """
    Behaviour for downloaded tiles from DownloadSRTMTiles application.

    It runs the following steps:
        -unzip all downloaded tiles
        -creates a vrt with all hgt files
        -crop the vrt with the given image as reference
    """
    display_parameters(locals(), "get_wb")

    working_dir_wb = os.path.join(working_dir, "working_dir_wb")
    if os.path.isdir(working_dir_wb):
        #empty wd directory
        [os.remove(x) for x in glob.glob(os.path.join(working_dir_wb, "*"))]
    else:
        os.mkdir(working_dir_wb)

    # download wb tiles
    logger.debug("Downloading srtm wb tiles")
    # DownloadSWBDTiles_dl(data, working_dir_wb)
    DownloadSWBDTiles_dl(dtm_l2_coarse, working_dir_wb)
    logger.debug("Srtm wb tiles ok")

    logger.debug("Unziping srtm tiles")

    epsg_code, a_x, a_y, b_x, b_y, _, _, _, _ = get_image_info_with_gdal(dtm_l2_coarse)

    unzip_files(working_dir_wb)
    logger.debug("Unzip ok")

    logger.debug("Reproject DTM coarse in wgs84")
    # set a projection to all shapefiles and rasterize on dtm at L2_Coarse resolution
    logger.debug("Assigning projection to wb")
    listWaterBodies = []
    for shapefile in glob.glob(os.path.join(working_dir_wb, "*.shp")):
        # set projection to wb
        projfile = os.path.splitext(shapefile)[0] + ".prj"
        logger.debug("projfile :" + projfile)
        my_proj = open(projfile, 'w')
        my_proj.write('GEOGCS["GCS_WGS_1984",DATUM["D_WGS_1984",SPHEROID["WGS_1984",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["Degree",0.017453292519943295]]')
        my_proj.close()
        logger.debug("rasterizing wb")
        shape_temp = os.path.splitext(shapefile)[0] + "_reproject" + os.path.splitext(shapefile)[1]

        ogr_command = "ogr2ogr -t_srs EPSG:" + str(epsg_code) + " -s_srs EPSG:4326 " + shape_temp + " " + shapefile
        os.system(ogr_command)

        shape_clipped = clipVectorWithOGR_Extent(shape_temp, [a_x, a_y, b_x, b_y], working_dir_wb)

        temp = rasterizationWithSupportImage(shape_clipped, dtm_l2_coarse, working_dir_wb)
        logger.debug("rasterizing wb ok")
        listWaterBodies.append(temp)
    logger.debug("Projection to wb ok")
    logger.debug(listWaterBodies)

    wb = os.path.join(working_dir, getBasename(dtm_l2_coarse) + "_MSK.TIF")
    logger.debug("OR between images")
    if not listWaterBodies == []:
        formula = "im1b1 "
        for i in range(1, len(listWaterBodies)):
            formula += 'or im' + str(i + 1) + "b1 "
        bandmath(listWaterBodies, formula, wb, "uint8")

    logger.info("Output WB")
    logger.info("\t WB : " + wb)
    return wb
    # ####### END MANAGING WB ########


if __name__ == '__main__':
    dtm_l2_coarse, working_dir = get_arguments()
    generate_wb(dtm_l2_coarse, working_dir)
