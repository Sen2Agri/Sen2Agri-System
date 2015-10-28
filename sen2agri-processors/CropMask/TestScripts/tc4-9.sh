#! /bin/bash

#Test for module MajorityVoting

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

RAW_CROP_MASK="$REFERENCE_FOLDER/raw_crop_mask.tif"
SEGMENTED="$REFERENCE_FOLDER/segmented.tif"

CROP_MASK="$OUT_FOLDER/crop_mask.tif"

try otbcli MajorityVoting $CROPMASK_OTB_LIBS_ROOT/MajorityVoting/ -nodatasegvalue 0 -nodataclassifvalue -10000 -inclass $RAW_CROP_MASK -inseg $SEGMENTED -rout $CROP_MASK 

validateRasterFile $CROP_MASK "$REFERENCE_FOLDER/crop_mask.tif"

