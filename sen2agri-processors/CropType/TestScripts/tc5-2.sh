#! /bin/bash

if [ $# -lt 2 ]
then
  echo "Usage: $0 <otb apllication directory> <out folder name>"
  echo "The output directory should be given" 1>&2  
  exit
fi

source try_command.sh

CROPTYPE_OTB_LIBS_ROOT="$1"
OUT_FOLDER=$2

REFERENCE_SHAPE="./Reference/reference.shp"
RATIO=0.75
SEED=0

TRAINING_POLYGONS="$OUT_FOLDER/training_polygons.shp"
VALIDATION_POLYGONS="$OUT_FOLDER/validation_polygons.shp"

try otbcli SampleSelection $CROPTYPE_OTB_LIBS_ROOT/SampleSelection/ -ref $REFERENCE_SHAPE -ratio $RATIO -seed $SEED -tp $TRAINING_POLYGONS -vp $VALIDATION_POLYGONS

echo "Information for $TRAINING_POLYGONS file:"
TRAINING_COMPARISION_FILE="./Reference/training_polygons.shp"

FILESIZE=$(stat -c%s "$TRAINING_POLYGONS")
if [ $FILESIZE == 7260 ] ; then    
    echo "File size      : PASSED"
    if [[ ! $(diff "$TRAINING_POLYGONS" "$TRAINING_COMPARISION_FILE") ]] ; then
        echo "File content   : PASSED"
    else
	echo "File content   : FAILED"
    fi
else
    echo "File size      : FAILED"
    echo "File content   : FAILED"
fi

echo "Information for $VALIDATION_POLYGONS file:"
VALIDATION_COMPARISION_FILE="./Reference/validation_polygons.shp"

FILESIZE=$(stat -c%s "$VALIDATION_POLYGONS")
if [ $FILESIZE == 3780 ] ; then    
    echo "File size      : PASSED"
    if [[ ! $(diff "$VALIDATION_POLYGONS" "$VALIDATION_COMPARISION_FILE") ]] ; then
        echo "File content   : PASSED"
    else
	echo "File content   : FAILED"
    fi
else
    echo "File size      : FAILED"
    echo "File content   : FAILED"
fi

