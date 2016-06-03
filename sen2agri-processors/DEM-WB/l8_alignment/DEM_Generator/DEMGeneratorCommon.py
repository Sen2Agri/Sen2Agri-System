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
import os
import glob

# project libraries
import config

#import logging for debug messages
import logging
logging.basicConfig()
# create logger
logger = logging.getLogger( 'DEMGeneratorCommon' )
logger.setLevel(config.level_working_modules)
fh = logging.StreamHandler()
formatter = logging.Formatter('%(asctime)s %(name)s %(levelname)s - %(message)s')
fh.setFormatter(formatter)
fh.setLevel(logging.DEBUG)
logger.addHandler(fh)





def bandmath(images, expression, output_filename, out_type = ""):
    """
    This function applies otbcli_BandMath to image with the given expression

    Keyword arguments:
        image               --    raster layer to process
        expression          --    expression to apply
        output_filename     --    output raster image
        type                --    type of the output image
    """
    command = ('%s '
                '-il %s '
                '-exp "%s" '
                '-out "%s"'
                % ("otbcli_BandMath",
                    " ".join(images),
                    expression,
                    output_filename))
    if out_type:
        command += " " + out_type

    logger.info("command : " + command)
    os.system(command)

