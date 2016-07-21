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

# project libraries
import config
from DEM_common import display_parameters, usage

# import logging for debug messages
import logging
logging.basicConfig()
# create logger
logger = logging.getLogger('Generate_DEM_L1')
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
    argParser.add_argument('-s', '--support_image', required=True,
                           help='Image at RX resolution')
    argParser.add_argument('-o', '--output_mnt', required=True,
                           help='Path to output mnt')

    args = argParser.parse_args(sys.argv[1:])

    input_mnt = os.path.realpath(args.input_mnt)
    support_image = os.path.realpath(args.support_image)
    working_dir = args.working_directory

    logger.debug("Arguments ok")

    logger.debug("Managing output directories")
    # checking existence of input mnt
    if not os.path.isfile(input_mnt):
        print "ERROR: Missing input MNT"
        usage(argParser)
    # checking support image
    if not os.path.isdir(support_image):
        print "ERROR: Support image is missing"
        usage(argParser)

    return input_mnt, support_image, output_mnt


def superimpose(image_in, image_ref, image_out, lms=4, out_type=""):
    """
    Runs the OTB Application Superimpose

    :param image_in:
    :param image_ref:
    :param image_out:
    :param lms:
    :param out_type:
    :return:
    """
    command = ('%s '
               '-inr "%s" '
               '-inm "%s" '
               '-interpolator "linear" '
               '-lms "%s" '
               '-out "%s"'
               % ("otbcli_Superimpose",
                   image_ref,
                   image_in,
                   str(lms),
                   image_out))

    if out_type:
        command += " " + out_type

    logger.info("command : " + command)
    os.system(command)


def generate_dem_l1(input_mnt, support_image, dtm_output):
    """

    :param input_mnt:
    :param support_image:
    :param dtm_output:
    :return:
    """

    time_start = time.time()
    display_parameters(locals(), "generate_dem_l2")

    # superimpose the vrt on the input data
    if not os.path.isfile(dtm_output):
        superimpose(input_mnt, support_image, dtm_output, 40)

    logger.info("Output DTM")
    logger.info("\t DTM L1 resolution          : " + dtm_output)

    interval = time.time() - time_start
    logger.info('Total time in seconds:' + str(interval))
    logger.info("Generated raw mnt: {}".format(dtm_output))


if __name__ == '__main__':
    input_mnt, support_image, output_mnt = get_arguments()
    generate_dem_l1(input_mnt, support_image, output_mnt)
