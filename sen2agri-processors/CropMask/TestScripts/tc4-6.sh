#! /bin/bash

#Test for module Erosion

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

REFERENCE="$REFERENCE_FOLDER/reference.tif"
RADIUS=3

ERODED_REFERENCE="$OUT_FOLDER/eroded_reference.tif"

try otbcli Erosion $CROPMASK_OTB_LIBS_ROOT/Erosion/ -in $REFERENCE -out $ERODED_REFERENCE -radius $RADIUS

validateRasterFile $ERODED_REFERENCE "$REFERENCE_FOLDER/eroded_reference.tif"



