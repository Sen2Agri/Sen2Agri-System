#! /bin/bash

#Test for module SampleSelection

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

REFERENCE_SHAPE="$REFERENCE_FOLDER/reference.shp"
RATIO=0.75
SEED=0

TRAINING_POLYGONS="$OUT_FOLDER/training_polygons.shp"
VALIDATION_POLYGONS="$OUT_FOLDER/validation_polygons.shp"

try otbcli SampleSelection $CROPTYPE_OTB_LIBS_ROOT/SampleSelection/ -ref $REFERENCE_SHAPE -ratio $RATIO -seed $SEED -tp $TRAINING_POLYGONS -vp $VALIDATION_POLYGONS

validateShapeFile $TRAINING_POLYGONS "$REFERENCE_FOLDER/training_polygons.shp"
validateShapeFile $VALIDATION_POLYGONS "$REFERENCE_FOLDER/validation_polygons.shp"

