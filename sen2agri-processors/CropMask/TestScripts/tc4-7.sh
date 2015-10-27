#! /bin/bash

#Test for module Trimming

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

SF_NOINSITU="$REFERENCE_FOLDER/sf_noinsitu.tif"
ERODED_REFERENCE="$REFERENCE_FOLDER/reference.tif"
ALPHA=0.01

REF_SHAPE="$OUT_FOLDER/reference_shape.shp"


try otbcli Trimming $CROPMASK_OTB_LIBS_ROOT/Trimming/ -feat $SF_NOINSITU -ref $ERODED_REFERENCE -out $REF_SHAPE -alpha $ALPHA 

validateShapeFile $REF_SHAPE "$REFERENCE_FOLDER/reference_shape.shp"



