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

  VERSION: 01.00.00 | DATE: <10/03/2016> | COMMENTS: Creation of file.

  END-HISTORY
  _____________________________________________________________________________

  $Id: $
  _____________________________________________________________________________

"""

import sys
import os
import glob

import config

#import logging for debug messages
import logging
logging.basicConfig()
# create logger
logger = logging.getLogger( 'DEMWB_Common' )
logger.setLevel(config.level_debug_modules)
fh = logging.StreamHandler()
formatter = logging.Formatter('%(asctime)s %(name)s %(levelname)s - %(message)s')
fh.setFormatter(formatter)
fh.setLevel(logging.DEBUG)
logger.addHandler(fh)




def usage(argParser, return_code = 1):
    """
    Usage
    :param argParser:
    :param return_code:
    :return:
    """
    print argParser.format_usage()
    sys.exit(return_code)


def display_parameters(dictParameters, functionName):
    """
    Display parameters of the functionName.
    Used as follows :
        display_parameters(locals(), "get_arguments")
    :param dictParameters:
    :param functionName:
    :return:
    """
    # print "dictParameters", dictParameters
    logging.debug("--------------------------------")
    logging.debug("\t{}".format(functionName))
    for argument, value in dictParameters.iteritems():
        logging.debug( "\t\t {argument}: {value}".format(argument=argument, value=value))
    logging.debug("--------------------------------")


def display_parameters_p(dictParameters, functionName):
    """
    Display parameters of the functionName.
    Used as follows :
        display_parameters(locals(), "get_arguments")
    :param dictParameters:
    :param functionName:
    :return:
    """
    # print "dictParameters", dictParameters
    print("--------------------------------")
    print("\t{}".format(functionName))
    for argument, value in dictParameters.iteritems():
        print( "\t\t {argument}: {value}".format(argument=argument, value=value))
    print("--------------------------------")

def getBasename(inputFilename):
    return os.path.splitext(os.path.basename(inputFilename))[0]


def unzip_files(dtm_folder):
    """
    This function unzip all zip of the given folder
    """
    # unzip downloaded files
    for zip_file in glob.glob(os.path.join(dtm_folder, "*.zip")):
        # add -o option to overwrite files without asking
        command_unzip = "unzip -o " + zip_file + " -d " + dtm_folder
        logger.debug("command : " + command_unzip)
        os.system(command_unzip)


def myGlob(directory, filePattern):
    """
    Creates an automatic path.join with given arguments
    :param directory:
    :param filePattern:
    :return:
    """
    return glob.glob(os.path.join(directory, filePattern))


def searchOneFile(directory, filePattern):
    """
    Use glob to find a file matching the given filePattern
    :param directory:
    :param filePattern:
    :return:
    """
    #print "directory, filePattern", directory, filePattern
    resList = myGlob(directory, filePattern)
    #print resList
    if resList:
        if len(resList)>1:
            print "Warning, more than one value matching the pattern", filePattern, "in", directory
        return resList[0]
    return None