#!/bin/bash

function try {
    echo
    echo
    echo "$@"
    "$@"
    code=$?
    if [ $code -ne 0 ]
    then
        echo "$1 did not work: exit status $code"
        exit 1
    fi
}

CROPTYPE_APP="/home/agrosu/sen2agri-processors-build/CropType"
OTB_APP="/home/agrosu/sen2agri-processors-build/VegetationStatus/phenotb/src/Applications/"
SCRIPT_PATH="/home/agrosu/sen2agri/sen2agri-processors/VegetationStatus/TestScripts"
OUTPUT_PATH="/mnt/data/QR_Results/PhenoNDVI_Ukraine/"

echo "The OTB appliction will be launched from:"
echo "$OTB_APP"
echo "The output path"
echo "$OUTPUT_PATH"
echo "The script path:"
echo "$SCRIPT_PATH"
mkdir "$OUTPUT_PATH"
try cd "$SCRIPT_PATH"

"./qr_pheno_ndvi_ukraine.sh" "$CROPTYPE_APP" "$OTB_APP" "$OUTPUT_PATH"


