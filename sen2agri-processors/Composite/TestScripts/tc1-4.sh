#! /bin/bash

if [ $# -lt 4 ]
then
  echo "Usage: $0 <xml L2A> <resolution> <out folder name> <bands mapping> [previous weights l3a product] [previous dates l3a product] [previous reflectances l3a product] [previous flags l3a product]"
  echo "The file with input xml should be given. The resolution for which the mask extraction will be performed should be given. The output directory should be given" 1>&2  
  exit
fi

source try_command.sh

RESOLUTION=$2
OUT_FOLDER=$3
xml=$1

COMPOSITE_OTB_LIBS_ROOT="~/sen2agri-processors-build/Composite"
WEIGHT_OTB_LIBS_ROOT="$COMPOSITE_OTB_LIBS_ROOT/WeightCalculation"

OUT_IMG_BANDS="$OUT_FOLDER/res$RESOLUTION.tif"

OUT_AOT="$OUT_FOLDER/aot$RESOLUTION.tif"
OUT_WEIGHT_AOT_FILE="$OUT_FOLDER/WeightAot.tif"

OUT_CLD="$OUT_FOLDER/cld$RESOLUTION.tif"
OUT_WAT="$OUT_FOLDER/wat$RESOLUTION.tif"
OUT_SNOW="$OUT_FOLDER/snow$RESOLUTION.tif"
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

BANDS_MAPPING="$4"

cp "$BANDS_MAPPING" "$OUT_FOLDER"

FULL_BANDS_MAPPING="$OUT_FOLDER/$BANDS_MAPPING"

OUT_L3A_FILE="$OUT_FOLDER/L3AResult_$RESOLUTION.tif"

PREV_L3A=""
if [ $# == 8 ] ; then
    PREV_L3A="-prevl3aw $4 -prevl3ad $5 -prevl3ar $6 -prevl3af $7"
fi

#try otbcli UpdateSynthesis "$COMPOSITE_OTB_LIBS_ROOT/UpdateSynthesis/" -in "$OUT_IMG_BANDS" -allinone 1 -xml "$xml" $PREV_L3A -csm "$OUT_CLD" -wm "$OUT_WAT" -sm "$OUT_SNOW" -wl2a "$OUT_TOTAL_WEIGHT_FILE" -out "$OUT_L3A_FILE" 
try otbcli UpdateSynthesis2 $COMPOSITE_OTB_LIBS_ROOT/UpdateSynthesis/ -in "$OUT_IMG_BANDS" -bmap "$FULL_BANDS_MAPPING" -xml "$xml" $PREV_L3A -csm "$OUT_CLD" -wm "$OUT_WAT" -sm "$OUT_SNOW" -wl2a "$OUT_TOTAL_WEIGHT_FILE" -out "$OUT_L3A_FILE"

echo "Information for $OUT_L3A_FILE file:"
OUTPUT_IMAGE_INFO="$(otbcli_ReadImageInfo -in $OUT_L3A_FILE | grep "Number of bands")"

BANDS_NB="${OUTPUT_IMAGE_INFO: -2}"

if [ $BANDS_NB == 10 ] ; then
    echo "Number of bands: PASSED"
    FILESIZE=$(stat -c%s "$OUT_L3A_FILE")
    if [ $FILESIZE == 20008430 ] ; then
	echo "File size      : PASSED"
    else
	echo "File size      : FAILED"
    fi
else
    echo "Number of bands: FAILED"
    echo "File size      : FAILED"
fi

