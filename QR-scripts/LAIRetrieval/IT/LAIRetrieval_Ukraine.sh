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


OTB_APP="/home/agrosu/sen2agri-processors-build/VegetationStatus"
SCRIPT_PATH="/home/agrosu/sen2agri/sen2agri-processors/VegetationStatus/TestScripts"
OUTPUT_PATH="/mnt/data/QR_Results/LAIRetrieval_Ukraine/"

echo "The OTB application will be launched from:"
echo "$OTB_APP"
echo "The output path:"
echo "$OUTPUT_PATH"
mkdir "$OUTPUT_PATH"
try cd "$SCRIPT_PATH"

./lai_retr_qr_ukraine.sh "$OTB_APP" 0 ../otb-bv/data/spot4hrvir1.rsr "$OUTPUT_PATH"

