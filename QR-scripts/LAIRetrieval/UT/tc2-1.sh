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


OTB_APP="/home/cudroiu/sen2agri-processors-build/VegetationStatus/"
SCRIPT_PATH="/home/cudroiu/sen2agri/sen2agri-processors/VegetationStatus/TestScripts"
OUTPUT_PATH="/mnt/data/UT_Results/VegetationStatus/out_qr_lai_midp_e_tc"

echo "The OTB appliction will be launched from:"
echo "$OTB_APP"
echo "The used RSR file:"
echo "$SCRIPT_PATH/../otb-bv/data/spot4hrvir1.rsr"
echo "The output path"
echo "$OUTPUT_PATH"
mkdir -p "$OUTPUT_PATH"

"$SCRIPT_PATH/lai_retr_qr_midp_E.sh" "$OTB_APP" 0 "$SCRIPT_PATH/../otb-bv/data/spot4hrvir1.rsr" "$OUTPUT_PATH" "$SCRIPT_PATH" "tc2-1"
