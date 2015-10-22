#! /bin/bash

if [ $# -lt 4 ]
then
  echo "Usage: $0 <xml L2A> <out folder name> <bands mapping> [previous weights l3a product] [previous dates l3a product] [previous reflectances l3a product] [previous flags l3a product]"
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

OUT_WEIGHTS="$OUT_FOLDER/L3AResult_weights.tif"
OUT_DATES="$OUT_FOLDER/L3AResult_dates.tif"
OUT_REFLS="$OUT_FOLDER/L3AResult_refls.tif"
OUT_FLAGS="$OUT_FOLDER/L3AResult_flags.tif"
OUT_RGB="$OUT_FOLDER/L3AResult_rgb.tif"

out_w="$OUT_WEIGHTS"
out_d="$OUT_DATES"
out_r="$OUT_REFLS"
out_f="$OUT_FLAGS"
out_rgb="-outrgb $OUT_RGB"

PREV_L3A=""
if [ $# == 8 ] ; then
    PREV_L3A="-prevl3aw $4 -prevl3ad $5 -prevl3ar $6 -prevl3af $7"
fi

try otbcli CompositeSplitter2 "$COMPOSITE_OTB_LIBS_ROOT/CompositeSplitter/" -in "$OUT_L3A_FILE" -xml "$xml" -bmap "$FULL_BANDS_MAPPING" -outweights "$out_w" -outdates "$out_d" -outrefls "$out_r" -outflags "$out_f" $out_rgb

echo "Information for $OUT_WEIGHTS:"
OUTPUT_IMAGE_INFO="$(otbcli_ReadImageInfo -in $OUT_WEIGHTS | grep "Number of bands")"

BANDS_NB="${OUTPUT_IMAGE_INFO: -1}"

if [ $BANDS_NB == 4 ] ; then
    echo "Number of bands: PASSED"
    FILESIZE=$(stat -c%s "$OUT_WEIGHTS")
    if [ $FILESIZE == 8008394 ] ; then
	echo "File size      : PASSED"
    else
	echo "File size      : FAILED"
    fi
else
    echo "Number of bands: FAILED"
    echo "File size      : FAILED"
fi


OUT_DATES="$OUT_FOLDER/L3AResult_dates.tif"
OUT_REFLS="$OUT_FOLDER/L3AResult_refls.tif"
OUT_FLAGS="$OUT_FOLDER/L3AResult_flags.tif"
OUT_RGB="$OUT_FOLDER/L3AResult_rgb.tif"

echo "Information for $OUT_DATES:"
OUTPUT_IMAGE_INFO="$(otbcli_ReadImageInfo -in $OUT_DATES | grep "Number of bands")"

BANDS_NB="${OUTPUT_IMAGE_INFO: -1}"

if [ $BANDS_NB == 1 ] ; then
    echo "Number of bands: PASSED"
    FILESIZE=$(stat -c%s "$OUT_DATES")
    if [ $FILESIZE == 2002360 ] ; then
	echo "File size      : PASSED"
    else
	echo "File size      : FAILED"
    fi
else
    echo "Number of bands: FAILED"
    echo "File size      : FAILED"
fi

echo "Information for $OUT_REFLS:"
OUTPUT_IMAGE_INFO="$(otbcli_ReadImageInfo -in $OUT_REFLS | grep "Number of bands")"

BANDS_NB="${OUTPUT_IMAGE_INFO: -1}"

if [ $BANDS_NB == 4 ] ; then
    echo "Number of bands: PASSED"
    FILESIZE=$(stat -c%s "$OUT_REFLS")
    if [ $FILESIZE == 8008394 ] ; then
	echo "File size      : PASSED"
    else
	echo "File size      : FAILED"
    fi
else
    echo "Number of bands: FAILED"
    echo "File size      : FAILED"
fi

echo "Information for $OUT_FLAGS:"
OUTPUT_IMAGE_INFO="$(otbcli_ReadImageInfo -in $OUT_FLAGS | grep "Number of bands")"

BANDS_NB="${OUTPUT_IMAGE_INFO: -1}"

if [ $BANDS_NB == 1 ] ; then
    echo "Number of bands: PASSED"
    FILESIZE=$(stat -c%s "$OUT_FLAGS")
    if [ $FILESIZE == 1001360 ] ; then
	echo "File size      : PASSED"
    else
	echo "File size      : FAILED"
    fi
else
    echo "Number of bands: FAILED"
    echo "File size      : FAILED"
fi

echo "Information for $OUT_RGB:"
OUTPUT_IMAGE_INFO="$(otbcli_ReadImageInfo -in $OUT_RGB | grep "Number of bands")"

BANDS_NB="${OUTPUT_IMAGE_INFO: -1}"

if [ $BANDS_NB == 3 ] ; then
    echo "Number of bands: PASSED"
    FILESIZE=$(stat -c%s "$OUT_RGB")
    if [ $FILESIZE == 6008384 ] ; then
	echo "File size      : PASSED"
    else
	echo "File size      : FAILED"
    fi
else
    echo "Number of bands: FAILED"
    echo "File size      : FAILED"
fi
