#! /bin/bash

#Test for module SpectralFeatures

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

RTOCR_SMOOTH="$REFERENCE_FOLDER/rtocr_smooth.tif"
TF_NOINSITU="$REFERENCE_FOLDER/tf_noinsitu.tif"

SF_NOINSITU="$OUT_FOLDER/sf_noinsitu.tif"

try otbcli SpectralFeatures $CROPMASK_OTB_LIBS_ROOT/SpectralFeatures/ -ts $RTOCR_SMOOTH -tf $TF_NOINSITU -sf $SF_NOINSITU

validateRasterFile $SF_NOINSITU "$REFERENCE_FOLDER/sf_noinsitu.tif"



