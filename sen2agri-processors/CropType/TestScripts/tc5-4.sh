#! /bin/bash

#Test for module FeatureExtraction

if [ $# -lt 3 ]
then
  echo "Usage: $0 <otb apllication directory> <reference files directory> <out folder name>"
  echo "The output directory should be given" 1>&2  
  exit
fi

source functions.sh

CROPTYPE_OTB_LIBS_ROOT="$1"
REFERENCE_FOLDER="$2"
OUT_FOLDER="$3"

RTOCR="$REFERENCE_FOLDER/rtocr.tif"

FTS="$OUT_FOLDER/fts.tif"
NDVI="$OUT_FOLDER/ndvi.tif"
NDWI="$OUT_FOLDER/ndwi.tif"
BRIGHTNESS="$OUT_FOLDER/brightness.tif"

try otbcli FeatureExtraction $CROPTYPE_OTB_LIBS_ROOT/FeatureExtraction/ -rtocr $RTOCR -fts $FTS -ndvi $NDVI -ndwi $NDWI -brightness $BRIGHTNESS

validateRasterFile $FTS "$REFERENCE_FOLDER/fts.tif"
validateRasterFile $NDVI "$REFERENCE_FOLDER/ndvi.tif"
validateRasterFile $NDWI "$REFERENCE_FOLDER/ndwi.tif"
validateRasterFile $BRIGHTNESS "$REFERENCE_FOLDER/brightness.tif"



