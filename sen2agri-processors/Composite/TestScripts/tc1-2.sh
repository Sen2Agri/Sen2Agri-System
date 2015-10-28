#! /bin/bash

if [ $# -lt 5 ]
then
  echo "Usage: $0 <otb apllication directory> <xml L2A> <resolution> <out folder name> <bands mapping file> [scattering coefficient file - S2 case only]"
  echo "The file with input xml should be given. The resolution for which the mask extraction will be performed should be given. The output directory should be given" 1>&2  
  exit
fi

source try_command.sh

RESOLUTION="$3"
OUT_FOLDER="$4"

COMPOSITE_OTB_LIBS_ROOT="$1"
OUT_SPOT_MASKS="$OUT_FOLDER/masks$RESOLUTION.tif"

OUT_IMG_BANDS="$OUT_FOLDER/res$RESOLUTION.tif"
OUT_CLD="$OUT_FOLDER/cld$RESOLUTION.tif"
OUT_WAT="$OUT_FOLDER/wat$RESOLUTION.tif"
OUT_SNOW="$OUT_FOLDER/snow$RESOLUTION.tif"
OUT_AOT="$OUT_FOLDER/aot$RESOLUTION.tif"

FULL_SCAT_COEFFS=""
BANDS_MAPPING="$5"

cp "$BANDS_MAPPING" "$OUT_FOLDER"

FULL_BANDS_MAPPING="$OUT_FOLDER/$BANDS_MAPPING"

if [ $# == 6 ] ; then
    SCAT_COEFFS="$5"
    cp "$SCAT_COEFFS" "$OUT_FOLDER"
    FULL_SCAT_COEFFS="-scatcoef $OUT_FOLDER/$SCAT_COEFFS"
fi

try otbcli CompositePreprocessing2 "$COMPOSITE_OTB_LIBS_ROOT/CompositePreprocessing/" -xml $2 -bmap "$FULL_BANDS_MAPPING" -res "$RESOLUTION" "$FULL_SCAT_COEFFS" -msk "$OUT_SPOT_MASKS" -outres "$OUT_IMG_BANDS" -outcmres "$OUT_CLD" -outwmres "$OUT_WAT" -outsmres "$OUT_SNOW" -outaotres "$OUT_AOT"

OUTPUT_IMAGE_BANDS_INFO="$(otbcli_ReadImageInfo -in $OUT_IMG_BANDS | grep "Number of bands")"
OUTPUT_IMAGE_CLD_INFO="$(otbcli_ReadImageInfo -in $OUT_CLD | grep "Number of bands")"
OUTPUT_IMAGE_WAT_INFO="$(otbcli_ReadImageInfo -in $OUT_WAT | grep "Number of bands")"
OUTPUT_IMAGE_SNOW_INFO="$(otbcli_ReadImageInfo -in $OUT_SNOW | grep "Number of bands")"
OUTPUT_IMAGE_AOT_INFO="$(otbcli_ReadImageInfo -in $OUT_AOT | grep "Number of bands")"

BANDS_NB="${OUTPUT_IMAGE_BANDS_INFO: -1}"
CLD_NB="${OUTPUT_IMAGE_CLD_INFO: -1}"
WAT_NB="${OUTPUT_IMAGE_WAT_INFO: -1}"
SNOW_NB="${OUTPUT_IMAGE_SNOW_INFO: -1}"
AOT_NB="${OUTPUT_IMAGE_AOT_INFO: -1}"

#if [[ "$RESOLUTION" == "10" && "$BANDS_NB" == "3" ]] ; then
#    echo "Reflectance number of bands: PASSED"
#else
#    if [[ "$RESOLUTION" == "20" && "$BANDS_NB" == "1" ]] ; then
#	echo "Reflectance number of bands: PASSED"
#    else
#	echo "Reflectance number of bands: FAILED"
#    fi
#fi
FILESIZE_REF="48016384"
FILESIZE_CLD="16016360"
FILESIZE_WAT="16016360"
FILESIZE_SNOW="16016360"
FILESIZE_AOT="16016360"
if [ $RESOLUTION == 20 ] ; then
    FILESIZE_REF="16008394"
    FILESIZE_CLD="4004360"
    FILESIZE_WAT="4004360"
    FILESIZE_SNOW="4004360"
    FILESIZE_AOT="4004360"
fi
ut_output_info "$OUT_IMG_BANDS" 4 "./qr_cmp_southafrica/res$RESOLUTION.tif" $FILESIZE_REF
ut_output_info "$OUT_CLD" 1 "./qr_cmp_southafrica/cld$RESOLUTION.tif" $FILESIZE_CLD
ut_output_info "$OUT_WAT" 1 "./qr_cmp_southafrica/wat$RESOLUTION.tif" $FILESIZE_WAT
ut_output_info "$OUT_SNOW" 1 "./qr_cmp_southafrica/snow$RESOLUTION.tif" $FILESIZE_SNOW
ut_output_info "$OUT_AOT" 1 "./qr_cmp_southafrica/aot$RESOLUTION.tif" $FILESIZE_AOT
exit

if [[ "$BANDS_NB" == "4" ]] ; then
    echo "Reflectance number of bands: PASSED"
    FILESIZE=$(stat -c%s "$OUT_IMG_BANDS")
    if [ "$FILESIZE" == "$FILESIZE_REF" ] ; then
	echo "Reflectance file size      : PASSED"
    else
	echo "Reflectance file size      : FAILED"
    fi
else
    echo "Reflectance number of bands: FAILED"
    echo "Reflectance file size      : FAILED"
fi

if [ $CLD_NB == 1 ] ; then
    echo "Cloud number of bands      : PASSED"
    FILESIZE=$(stat -c%s "$OUT_CLD")
    if [ "$FILESIZE" == "$FILESIZE_CLD" ] ; then
	echo "Cloud file size            : PASSED"
    else
	echo "Cloud file size            : FAILED"
    fi
else
    echo "Cloud number of bands      : FAILED"
    echo "Cloud file size            : FAILED"
fi

if [ $WAT_NB == 1 ] ; then
    echo "Water number of bands      : PASSED"
    FILESIZE=$(stat -c%s "$OUT_WAT")
    if [ "$FILESIZE" == "$FILESIZE_WAT" ] ; then
	echo "Water file size            : PASSED"
    else
	echo "Water file size            : FAILED"
    fi
else
    echo "Water number of bands      : FAILED"
    echo "Water file size            : FAILED"
fi

if [ $SNOW_NB == 1 ] ; then
    echo "Snow number of bands       : PASSED"
    FILESIZE=$(stat -c%s "$OUT_SNOW")
    if [ "$FILESIZE" == "$FILESIZE_SNOW" ] ; then
	echo "Snow file size             : PASSED"
    else
	echo "Snow file size             : FAILED"
    fi
else
    echo "Snow number of bands       : FAILED"
    echo "Snow file size             : FAILED"
fi

if [ $AOT_NB == 1 ] ; then
    echo "AOT number of bands        : PASSED"
    FILESIZE=$(stat -c%s "$OUT_AOT")
    if [ "$FILESIZE" == "$FILESIZE_AOT" ] ; then
	echo "AOT file size              : PASSED"
    else
	echo "AOT file size              : FAILED"
    fi
else
    echo "AOT number of bands        : FAILED"
    echo "AOT file size              : FAILED"
fi







