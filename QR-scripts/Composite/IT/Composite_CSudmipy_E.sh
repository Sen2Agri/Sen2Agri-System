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


OTB_APP="/home/cudroiu/sen2agri-processors-build/Composite"
SCRIPT_PATH="/home/cudroiu/sen2agri/sen2agri-processors/Composite/TestScripts"
OUTPUT_PATH="/mnt/data/QR_Results/Composite_CSudmipy_E/"

echo "The OTB application will be launched from:"
echo "$OTB_APP"
echo "Resolution 20 meters"
echo "The output path:"
echo "$OUTPUT_PATH"
mkdir -p "$OUTPUT_PATH"
try cd "$SCRIPT_PATH"


./qr_composite_midpir_E.sh "$OTB_APP" 20 "$OUTPUT_PATH" bands_mapping_spot.txt
