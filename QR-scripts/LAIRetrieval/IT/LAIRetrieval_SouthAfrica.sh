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


OTB_APP="/home/cudroiu/sen2agri-processors-build/VegetationStatus"
SCRIPT_PATH="/home/cudroiu/sen2agri/sen2agri-processors/VegetationStatus/TestScripts"
OUTPUT_PATH="/mnt/data/QR_Results/LAIRetrieval_SouthAfrica/"

echo "The OTB application will be launched from:"
echo "$OTB_APP"
echo "The script will be launched from:"
echo "$SCRIPT_PATH"
echo "The output path:"
echo "$OUTPUT_PATH"
#mkdir "$OUTPUT_PATH"
#try cd "$SCRIPT_PATH"

"$SCRIPT_PATH/lai_retr_qr_southafrica.sh" "$OTB_APP" 0 "$SCRIPT_PATH/../otb-bv/data/spot4hrvir1.rsr" "$OUTPUT_PATH" "$SCRIPT_PATH"
