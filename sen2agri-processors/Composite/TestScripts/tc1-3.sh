#! /bin/bash

if [ $# -lt 4 ]
then
  echo "Usage: $0 <otb apllication directory> <xml L2A> <resolution> <out folder name>"
  echo "The file with input xml should be given. The resolution for which the mask extraction will be performed should be given. The output directory should be given" 1>&2  
  exit
fi

source try_command.sh

RESOLUTION=$3
OUT_FOLDER=$4
xml=$2

COMPOSITE_OTB_LIBS_ROOT="$1"
WEIGHT_OTB_LIBS_ROOT="$COMPOSITE_OTB_LIBS_ROOT/WeightCalculation"

OUT_AOT="$OUT_FOLDER/aot$RESOLUTION.tif"
OUT_WEIGHT_AOT_FILE="$OUT_FOLDER/WeightAot.tif"

OUT_CLD="$OUT_FOLDER/cld$RESOLUTION.tif"

OUT_WEIGHT_CLOUD_FILE="$OUT_FOLDER/WeightCloud.tif"
OUT_TOTAL_WEIGHT_FILE="$OUT_FOLDER/WeightTotal.tif"

WEIGHT_AOT_MIN="0.33"
WEIGHT_AOT_MAX="1"
AOT_MAX="50"

COARSE_RES="240"
SIGMA_SMALL_CLD="10"
SIGMA_LARGE_CLD="50"

WEIGHT_SENSOR="0.33"
WEIGHT_DATE_MIN="0.10"

L3A_DATE=20130131
HALF_SYNTHESIS=50

try otbcli WeightAOT "$WEIGHT_OTB_LIBS_ROOT/WeightAOT/" -in "$OUT_AOT" -xml "$xml" -waotmin $WEIGHT_AOT_MIN -waotmax $WEIGHT_AOT_MAX -aotmax $AOT_MAX -out "$OUT_WEIGHT_AOT_FILE"
ut_output_info "$OUT_WEIGHT_AOT_FILE" 1 "./qr_cmp_southafrica/WeightAot.tif" 4004360

try otbcli WeightOnClouds "$WEIGHT_OTB_LIBS_ROOT/WeightOnClouds/" -incldmsk "$OUT_CLD" -coarseres $COARSE_RES -sigmasmallcld $SIGMA_SMALL_CLD -sigmalargecld $SIGMA_LARGE_CLD -out "$OUT_WEIGHT_CLOUD_FILE"
ut_output_info "$OUT_WEIGHT_CLOUD_FILE" 1 "./qr_cmp_southafrica/WeightCloud.tif" 4004360

try otbcli TotalWeight "$WEIGHT_OTB_LIBS_ROOT/TotalWeight/" -xml "$xml" -waotfile "$OUT_WEIGHT_AOT_FILE" -wcldfile "$OUT_WEIGHT_CLOUD_FILE" -wsensor $WEIGHT_SENSOR -l3adate "$L3A_DATE" -halfsynthesis $HALF_SYNTHESIS -wdatemin $WEIGHT_DATE_MIN -out "$OUT_TOTAL_WEIGHT_FILE"
ut_output_info "$OUT_TOTAL_WEIGHT_FILE" 1 "./qr_cmp_southafrica/WeightTotal.tif" 4004360
