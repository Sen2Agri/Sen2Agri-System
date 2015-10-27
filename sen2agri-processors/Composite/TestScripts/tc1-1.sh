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

COMPOSITE_OTB_LIBS_ROOT="$1"
OUT_SPOT_MASKS="$OUT_FOLDER/masks$RESOLUTION.tif"

try otbcli MaskHandler $COMPOSITE_OTB_LIBS_ROOT/MaskHandler/ -xml $2 -out $OUT_SPOT_MASKS -sentinelres $RESOLUTION

ut_output_info "$OUT_SPOT_MASKS" 3 "./qr_cmp_southafrica/masks$RESOLUTION.tif" 6008384

exit

echo "Information for $OUT_SPOT_MASKS file:"
OUTPUT_IMAGE_INFO="$(otbcli_ReadImageInfo -in $OUT_SPOT_MASKS | grep "Number of bands")"

BANDS_NB="${OUTPUT_IMAGE_INFO: -1}"
COMPARISION_FILE="./qr_cmp_southafrica/msk$RESOLUTION.tif"

if [ $BANDS_NB == 3 ] ; then
    echo "Number of bands: PASSED"
    FILESIZE=$(stat -c%s "$OUT_SPOT_MASKS")
    if [ $FILESIZE == 6008384 ] ; then    
	echo "File size      : PASSED"
	if [[ ! $(diff "$OUT_SPOT_MASKS" "$COMPARISION_FILE") ]] ; then
            echo "Comp ref file  : PASSED"
	else
            echo "Comp ref file  : FAILED"
	fi
    else
	echo "File size      : FAILED"
    fi
else
    echo "Number of bands: FAILED"
    echo "File size      : FAILED"
    echo "Comp ref file  : FAILED"
fi

