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
import sys
import os
import time
import argparse
import lxml.etree as ET

# project libraries
import config
from DEM_common import display_parameters, usage, getBasename, searchOneFile
from GDAL_Tools.gdalinfoO import gdalinfoO

# import logging for debug messages
import logging
logging.basicConfig()
# create logger
logger = logging.getLogger('Get_Support_Image')
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
                           help='Path to input MNT')
    argParser.add_argument('-s', "--sensor", choices=['s2', 'l8'], required=True)
    argParser.add_argument('-w', '--working_directory', required=True,
                           help='Path to working directory')

    args = argParser.parse_args(sys.argv[1:])

    input_metadata = os.path.realpath(args.input_metadata)
    sensor = args.sensor
    working_dir = args.working_directory

    logger.debug("Arguments ok")

    logger.debug("Managing output directories")
    # checking existence of input mnt
    if not os.path.isfile(input_metadata):
        print "ERROR: Missing input metadata"
        usage(argParser)
    # creating directory if it does not exist
    if not os.path.isdir(working_dir):
        os.mkdir(working_dir)

    return input_metadata, sensor, working_dir


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



def otb_ressampling(input_image, output_image, ratio_for_otb):
    """
    Applies a ressampling on the given image with the given ratio

    :param input_image:
    :param output_image:
    :param ratio_for_otb:
    :return:
    """
    # ratio 2 : -> 200%

    command = 'otbcli_RigidTransformResample -in {inIm} -out {outIm} -transform.type id ' \
              '-transform.type.id.scalex {scalex} -transform.type.id.scaley {scaley}'.format(inIm=input_image,
                                                                                             outIm=output_image,
                                                                                             scalex=ratio_for_otb,
                                                                                             scaley=ratio_for_otb)
    logger.info("command : " + command)
    os.system(command)


def getBoundingVRT(image_in, vrt_out, scale_factor):
    command = 'gdalbuildvrt {} {}'.format(vrt_out, image_in)
    os.system(command)

    sizeX, sizeY = gdalinfoO(image_in).getSize()
    newSizeX = sizeX
    newSizeY = sizeY
    scale_factor = 1/scale_factor
    if (sizeX % scale_factor) != 0:
        newSizeX = sizeX + (scale_factor-(sizeX%scale_factor))
    if (sizeY % scale_factor) != 0:
        newSizeY = sizeY + (scale_factor-(sizeY%scale_factor))

    dom = ET.parse(vrt_out)
    root = dom.getroot()
    # <VRTDataset rasterXSize="914" rasterYSize="813">
    # print root.items()
    root.set("rasterXSize", str(int(newSizeX)))
    root.set("rasterYSize", str(int(newSizeY)))
    # print root.items()

    tree = ET.ElementTree(root)

    # print ET.tostring(tree)

    vrt2 = os.path.splitext(vrt_out)[0] + "_" + os.path.splitext(vrt_out)[1]
    f = open(vrt2, "w")
    f.write(ET.tostring(tree, pretty_print=True, xml_declaration=True, standalone='No',
                        encoding="UTF-8"))
    return vrt2


def get_support_images(input_metadata, sensor, working_dir):
    """

    :param input_image:
    :param sensor:
    :param working_dir:
    :return:
    """
    time_start = time.time()
    display_parameters(locals(), "get_support_images")

    supportImages = {}

    ratio_l2 = {}
    if sensor in ["l8"]:
        input_image = searchOneFile(os.path.dirname(input_metadata), "*B1.TIF")
        if input_image:
            supportImages["R1"] = input_image
        else:
            print "Missing 30m resolution"
        # ratio_l2["R1"] = float(config.L1_resolution) / float(config.L2_resolution)
        ratio_l2_coarse = float(config.L1_resolution) / float(config.L2_coarse_resolution)
    elif sensor in ["s2"]:
        directory_s2 = os.path.join(os.path.dirname(input_metadata), "IMG_DATA")
        input_image_r1 = searchOneFile(directory_s2, "*B02.jp2")
        if input_image_r1:
            supportImages["R1"] = input_image_r1
        else:
            print "Missing 10m resolution"
        input_image_r2 = searchOneFile(directory_s2, "*B05.jp2")
        if input_image_r2:
            supportImages["R2"] = input_image_r2
        else:
            print "Missing 20m resolution"
        # ratio_l2["R1"] = float(config.S2_L1_resolution) / float(config.S2_R1_resolution)
        ratio_l2_coarse = float(config.S2_R1_resolution) / float(config.L2_coarse_resolution)

    b1VRT = os.path.join(working_dir, getBasename(supportImages["R1"]) + "_bb.VRT")
    b1VRT_up = getBoundingVRT(supportImages["R1"], b1VRT, ratio_l2_coarse)
    output = os.path.join(working_dir, getBasename(supportImages["R1"]) + "_coarse.TIF")
    #otb_ressampling(supportImages["R1"], output, ratio_l2_coarse)
    otb_ressampling(b1VRT_up, output, ratio_l2_coarse)
    supportImages["Coarse"] = output

    interval = time.time() - time_start
    logger.info('Total time in seconds:' + str(interval))
    logger.info("Generated support images:{}".format(str(supportImages)))
    return supportImages


if __name__ == '__main__':
    input_metadata, sensor, working_dir = get_arguments()
    get_support_images(input_metadata, sensor, working_dir)
