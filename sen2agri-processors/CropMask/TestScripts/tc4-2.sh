#! /bin/bash

#Test for module StatisticFeatures

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

NDWI="$REFERENCE_FOLDER/ndwi.tif"
BRIGHTNESS="$REFERENCE_FOLDER/brightness.tif"

SF="$OUT_FOLDER/sf.tif"

try otbcli StatisticFeatures $CROPMASK_OTB_LIBS_ROOT/StatisticFeatures/ -ndwi $NDWI -brightness $BRIGHTNESS -sf $SF

validateRasterFile $SF "$REFERENCE_FOLDER/sf.tif"

