#! /bin/bash

#Test for module TemporalResampling

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

TOCR="$REFERENCE_FOLDER/tocr.tif"
MASK="$REFERENCE_FOLDER/mask.tif"
DATES="$REFERENCE_FOLDER/dates.txt"
SP=5
T0="20130402"
TEND="20130427"
RADIUS="25"
RTOCR="$OUT_FOLDER/rtocr.tif"
OUTDAYS="$OUT_FOLDER/outdays.txt"

try otbcli TemporalResampling $CROPTYPE_OTB_LIBS_ROOT/TemporalResampling/ -tocr $TOCR -mask $MASK -ind $DATES -sp $SP -t0 $T0 -tend $TEND -radius $RADIUS -outdays $OUTDAYS -rtocr $RTOCR

validateRasterFile $RTOCR "$REFERENCE_FOLDER/rtocr.tif"
validateTextFile $OUTDAYS "$REFERENCE_FOLDER/outdays.txt"

