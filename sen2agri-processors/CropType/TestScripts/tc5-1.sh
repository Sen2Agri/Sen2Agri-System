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

INPUT_DESCRIPTORS="/mnt/Satellite_Imagery/S2-QR/Ukraine/SPOT4_HRVIR1_XS_20130402_N2A_EUkraineD0000B0000/SPOT4_HRVIR1_XS_20130402_N2A_EUkraineD0000B0000.xml "
INPUT_DESCRIPTORS+="/mnt/Satellite_Imagery/S2-QR/Ukraine/SPOT4_HRVIR1_XS_20130412_N2A_EUkraineD0000B0000/SPOT4_HRVIR1_XS_20130412_N2A_EUkraineD0000B0000.xml "
INPUT_DESCRIPTORS+="/mnt/Satellite_Imagery/S2-QR/Ukraine/SPOT4_HRVIR1_XS_20130417_N2A_EUkraineD0000B0000/SPOT4_HRVIR1_XS_20130417_N2A_EUkraineD0000B0000.xml "
INPUT_DESCRIPTORS+="/mnt/Satellite_Imagery/S2-QR/Ukraine/SPOT4_HRVIR1_XS_20130422_N2A_EUkraineD0000B0000/SPOT4_HRVIR1_XS_20130422_N2A_EUkraineD0000B0000.xml "
INPUT_DESCRIPTORS+="/mnt/Satellite_Imagery/S2-QR/Ukraine/SPOT4_HRVIR1_XS_20130427_N2A_EUkraineD0000B0000/SPOT4_HRVIR1_XS_20130427_N2A_EUkraineD0000B0000.xml "

OUT_TOCR="$OUT_FOLDER/tocr.tif"
OUT_MASK="$OUT_FOLDER/mask.tif"
OUT_DATE="$OUT_FOLDER/dates.txt"
OUT_SHAPE="$OUT_FOLDER/shape.shp"
RESOLUTION=20

try otbcli BandsExtractor $CROPTYPE_OTB_LIBS_ROOT/BandsExtractor/ -il $INPUT_DESCRIPTORS -out $OUT_TOCR -mask $OUT_MASK -outdate $OUT_DATE -shape $OUT_SHAPE -pixsize $RESOLUTION

echo "Information for $OUT_TOCR file:"
OUTPUT_TOCR_INFO="$(otbcli_ReadImageInfo -in $OUT_TOCR | grep "Number of bands")"

TOCR_BANDS_NB="${OUTPUT_TOCR_INFO: -2}"
TOCR_COMPARISION_FILE="./Reference/tocr.tif"

if [ $TOCR_BANDS_NB == 20 ] ; then
    echo "Number of bands: PASSED"
    FILESIZE=$(stat -c%s "$OUT_TOCR")
    if [ $FILESIZE == 80008490 ] ; then    
	echo "File size      : PASSED"
	if [[ ! $(diff "$OUT_TOCR" "$TOCR_COMPARISION_FILE") ]] ; then
    	    echo "File content   : PASSED"
	else
	    echo "File content   : FAILED"
	fi
    else
	echo "File size      : FAILED"
        echo "File content   : FAILED"
    fi
else
    echo "Number of bands: FAILED"
    echo "File size      : FAILED"
    echo "File content   : FAILED"
fi

echo "Information for $OUT_MASK file:"
OUTPUT_MASK_INFO="$(otbcli_ReadImageInfo -in $OUT_MASK | grep "Number of bands")"

MASK_BANDS_NB="${OUTPUT_MASK_INFO: -1}"
MASK_COMPARISION_FILE="./Reference/mask.tif"

if [ $MASK_BANDS_NB == 5 ] ; then
    echo "Number of bands: PASSED"
    FILESIZE=$(stat -c%s "$OUT_MASK")
    if [ $FILESIZE == 20008400 ] ; then    
	echo "File size      : PASSED"
	if [[ ! $(diff "$OUT_MASK" "$MASK_COMPARISION_FILE") ]] ; then
    	    echo "File content   : PASSED"
	else
	    echo "File content   : FAILED"
	fi
    else
	echo "File size      : FAILED"
        echo "File content   : FAILED"
    fi
else
    echo "Number of bands: FAILED"
    echo "File size      : FAILED"
    echo "File content   : FAILED"
fi

echo "Information for $OUT_DATE file:"
DATE_COMPARISION_FILE="./Reference/dates.txt"

FILESIZE=$(stat -c%s "$OUT_DATE")
if [ $FILESIZE == 53 ] ; then    
    echo "File size      : PASSED"
    if [[ ! $(diff "$OUT_DATE" "$DATE_COMPARISION_FILE") ]] ; then
        echo "File content   : PASSED"
    else
	echo "File content   : FAILED"
    fi
else
    echo "File size      : FAILED"
    echo "File content   : FAILED"
fi

echo "Information for $OUT_SHAPE file:"
SHAPE_COMPARISION_FILE="./Reference/shape.shp"

FILESIZE=$(stat -c%s "$OUT_SHAPE")
if [ $FILESIZE == 236 ] ; then    
    echo "File size      : PASSED"
    if [[ ! $(diff "$OUT_SHAPE" "$SHAPE_COMPARISION_FILE") ]] ; then
        echo "File content   : PASSED"
    else
	echo "File content   : FAILED"
    fi
else
    echo "File size      : FAILED"
    echo "File content   : FAILED"
fi

