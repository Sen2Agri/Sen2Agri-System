#!/bin/bash

if [ "$#" -lt 2 ]; then
    echo "Please provide the the country code (NLD|CZE|LTU|ESP|ITA|ROU), the product type (NDVI|AMP|COHE) and optionally the compact option"
    exit 1
fi

CSV_COMPACT=""
if [ "$#" -eq "3" ] ; then 
    CSV_COMPACT="--csvcompact 0"
fi

COUNTRY="$1"
PRD_TYPE="$2"
OUT_REL_PATH=""
OUT_FILE_NAME=""
OUTPUT_LEVEL1="DataExtractionResults"
OUT_SUBFOLDER="Compact"

OUTPUTS_ROOT="/mnt/archive/agric_practices/Outputs/${COUNTRY}/"

OUT_REL_PATH="${PRD_TYPE}_SHP"
OUT_FILE_NAME="${COUNTRY}_2018_${PRD_TYPE}_Extracted_Data.csv"

case "$1" in
    NLD)
        ;;
    CZE)
        ;;
    LTU)
        ;;
    ESP)
        ;;
    ITA)
        ;;
    ITA_CP1)
        ;;
    ITA_CP2)
        ;;
    ITA_FML)
        ;;        
    ROU)
        ;;
    *)
        echo $"Please provide a country code in {NLD|CZE|LTU|ESP|ITA|ROU}"
        exit 1
esac

case "$2" in
    NDVI)
        ;;
    AMP)
        ;;
    COHE)
        ;;
    *)
        echo $"Usage: $0 {NLD|CZE|LTU|ESP|ITA|ROU} (NDVI|AMP|COHE)"
        exit 1
esac   

if [ -n "$CSV_COMPACT" ]; then 
    OUT_SUBFOLDER="NotCompact"
fi    

INPUT_PATH="${OUTPUTS_ROOT}${OUT_REL_PATH}"
OUTPUT_FILE_PATH="${OUTPUTS_ROOT}/${OUTPUT_LEVEL1}/${OUT_SUBFOLDER}/${OUT_FILE_NAME}"

echo "Executing: ./agric_practices_merge_files.py --input-dir $INPUT_PATH --output-file $OUTPUT_FILE_PATH $CSV_COMPACT"

./agric_practices_merge_files.py --input-dir $INPUT_PATH --output-file $OUTPUT_FILE_PATH $CSV_COMPACT

