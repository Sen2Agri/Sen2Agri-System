# -*- coding: utf-8 -*-
"""
_____________________________________________________________________________

   Program:         MACCS

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
level_working_modules = logging.INFO
level_debug_modules = logging.DEBUG
fichier_log = "/tmp/s2agri_log.txt"

L1_resolution = 30  # m
L2_resolution = 30  # m
L2_coarse_resolution = 240  # m

app_directory = "path to lib in build directory"
launcher = "otbApplicationLauncherCommandLine"
app_otb_directory = ""


tile_id_prefix = "L8_TEST_AUX_REFDE2_"
tile_id_suffix = "_0001"
