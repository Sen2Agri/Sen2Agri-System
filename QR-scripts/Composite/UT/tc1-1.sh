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
OUTPUT_PATH="/mnt/data/UT_Results/Composite/out_qr_l3a_south_africa_tests"
XML="/mnt/Satellite_Imagery/S2-QR/SouthAfrica/SPOT4_HRVIR1_XS_20130131_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130131_N2A_ESouthAfricaD0000B0000.xml"

echo "The OTB application will be launched from:"
echo "$OTB_APP"
echo "Resolution 20 meters"
echo "The output path:"
echo "$OUTPUT_PATH"
mkdir -p "$OUTPUT_PATH"
try cd "$SCRIPT_PATH"

"./tc1-1.sh" "$OTB_APP" $XML 20 "$OUTPUT_PATH"
