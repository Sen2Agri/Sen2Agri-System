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

  VERSION: 01.00.00 | DATE: <01/06/2015> | COMMENTS: Creation of file.

  END-HISTORY
  _____________________________________________________________________________

  $Id: $
  _____________________________________________________________________________
"""


import logging
level_working_modules = logging.DEBUG
level_debug_modules = logging.DEBUG

L1_resolution = 30  # m
L2_resolution = 30  # m
S2_R1_resolution = 10  # m
S2_R2_resolution = 20  # m
L2_coarse_resolution = 240  # m

app_directory = "/home/amondot/S2Agri/Dev/S2AgriTB2/build/lib"
launcher = "/usr/bin/otbApplicationLauncherCommandLine"
app_otb_directory = "/home/amondot/S2Agri/OTB/build/bin"


tile_id_prefix = "L8_TEST_AUX_REFDE2_"
tile_id_suffix = "_0001"
