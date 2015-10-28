#! /bin/bash

#Test for module DataSmoothing

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

RTOCR="$REFERENCE_FOLDER/rtocr.tif"
NDVI="$REFERENCE_FOLDER/ndvi.tif"
LAMBDA=2
WEIGHT=1

NDVI_SMOOTH="$OUT_FOLDER/ndvi_smooth.tif"
RTOCR_SMOOTH="$OUT_FOLDER/rtocr_smooth.tif"

try otbcli DataSmoothing $CROPMASK_OTB_LIBS_ROOT/DataSmoothing/ -ts $NDVI -bands 1 -lambda $LAMBDA -weight $WEIGHT -sts $NDVI_SMOOTH
try otbcli DataSmoothing $CROPMASK_OTB_LIBS_ROOT/DataSmoothing/ -ts $RTOCR -bands 4 -lambda $LAMBDA -weight $WEIGHT -sts $RTOCR_SMOOTH

validateRasterFile $NDVI_SMOOTH "$REFERENCE_FOLDER/ndvi_smooth.tif"
validateRasterFile $RTOCR_SMOOTH "$REFERENCE_FOLDER/rtocr_smooth.tif"

