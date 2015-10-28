#! /bin/bash

#Test for module PrincipalComponentAnalysis

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
NBCOMP=6

PCA="$OUT_FOLDER/pca.tif"


try otbcli PrincipalComponentAnalysis $CROPMASK_OTB_LIBS_ROOT/PrincipalComponentAnalysis/ -ndvi $NDVI -nc $NBCOMP -out $PCA 

validateRasterFile $PCA "$REFERENCE_FOLDER/pca.tif"

