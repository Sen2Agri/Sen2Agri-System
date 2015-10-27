#! /bin/bash

#Test for module TemporalFeaturesNoInsitu

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

NDVI_SMOOTH="$REFERENCE_FOLDER/ndvi_smooth.tif"
RTOCR_SMOOTH="$REFERENCE_FOLDER/rtocr_smooth.tif"
DATES="$REFERENCE_FOLDER/outdays.txt"

TF_NOINSITU="$OUT_FOLDER/tf_noinsitu.tif"

try otbcli TemporalFeaturesNoInsitu $CROPMASK_OTB_LIBS_ROOT/TemporalFeaturesNoInsitu/ -ndvi $NDVI_SMOOTH -ts $RTOCR_SMOOTH -dates $DATES -tf $TF_NOINSITU

validateRasterFile $TF_NOINSITU "$REFERENCE_FOLDER/tf_noinsitu.tif"



