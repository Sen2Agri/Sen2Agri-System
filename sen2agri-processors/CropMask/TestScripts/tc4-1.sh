#! /bin/bash

#Test for module TemporalFeatures

if [ $# -lt 3 ]
then
  echo "Usage: $0 <otb apllication directory> <reference files directory> <out folder name>"
  echo "The output directory should be given" 1>&2  
  exit
fi

source functions.sh

CROPMASK_OTB_LIBS_ROOT="$1"
REFERENCE_FOLDER="$2"
OUT_FOLDER="$3"


NDVI="$REFERENCE_FOLDER/ndvi.tif"
DATES="$REFERENCE_FOLDER/outdays.txt"

TF="$OUT_FOLDER/tf.tif"


try otbcli TemporalFeatures $CROPMASK_OTB_LIBS_ROOT/TemporalFeatures/ -ndvi $NDVI -dates $DATES -tf $TF

validateRasterFile $TF "$REFERENCE_FOLDER/tf.tif"
