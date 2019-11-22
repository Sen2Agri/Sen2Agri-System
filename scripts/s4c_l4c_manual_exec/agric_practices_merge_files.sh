#!/bin/bash

function usage() {
    echo "Usage: ./agric_practices_merge_files.sh -c <COUNTRY_CODE - (NLD|CZE|LTU|ESP|ITA|ROU)> -y <YEAR> -p <PRD_TYPE>"
    exit 1
}

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -c|--country)
    COUNTRY_AND_REGION="$2"
    shift # past argument
    shift # past value
    ;;
    -y|--year)
    YEAR="$2"
    shift # past argument
    shift # past value
    ;;
    -p|--prd_type)
    PRD_TYPE="$2"
    shift # past argument
    shift # past value
    ;;
    -s|--compact)
    COMPACT_PARAM_VAL="$2"
    shift # past argument
    shift # past value
    ;;
    
    *)    # unknown option
    POSITIONAL+=("$1") # save it in an array for later
    shift # past argument
    ;;
esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

echo COUNTRY        = "${COUNTRY_AND_REGION}"
echo YEAR           = "${YEAR}"
echo PRD_TYPE       = "${PRD_TYPE}"
echo COMPACT_PARAM_VAL       = "${COMPACT_PARAM_VAL}"

if [ -z ${COUNTRY_AND_REGION} ] ; then
    echo "No country provided!"
    usage
fi 

if [ -z ${YEAR} ] ; then
    echo "No year provided!"
    usage
fi 

if [ -z ${PRD_TYPE} ] ; then
    echo "No product type provided (AMP, COHE or NDVI should be provided)!"
    usage
fi 

CSV_COMPACT=""
if [ "${COMPACT_PARAM_VAL}" == "0" ] ; then 
    CSV_COMPACT="--csvcompact 0"
fi

COUNTRY="${COUNTRY_AND_REGION%%_*}"
COUNTRY_REGION=""
if [ "${COUNTRY_AND_REGION}" != "$COUNTRY" ] ; then

    COUNTRY_REGION="${COUNTRY_AND_REGION##*_}"
    echo "Using country region = $COUNTRY_REGION for country ${COUNTRY}"
fi    

OUT_REL_PATH=""
OUT_FILE_NAME=""
OUTPUT_LEVEL1="DataExtractionResults"
OUT_SUBFOLDER="Compact"

OUTPUTS_ROOT="/mnt/archive/agric_practices/Outputs/${COUNTRY_AND_REGION}/"

OUT_REL_PATH="${PRD_TYPE}_SHP"
OUT_FILE_NAME="${COUNTRY_AND_REGION}_${YEAR}_${PRD_TYPE}_Extracted_Data.csv"

SORT_INPUT_FILES="--sort 1"
case "${COUNTRY}" in
    NLD)
        SORT_INPUT_FILES="--sort 0"
        ;;
    CZE)
        SORT_INPUT_FILES="--sort 0"
        ;;
    LTU)
        ;;
    ESP)
        SORT_INPUT_FILES="--sort 0"
        ;;
    ITA)
        ;;
    ROU)
        ;;
    *)
        echo $"Please provide a country code in {NLD|CZE|LTU|ESP|ITA|ROU}"
        exit 1
esac

FILTER_FILES_REGEX_OPT=""
case "${PRD_TYPE}" in
    NDVI)
        FILTER_FILES_REGEX_OPT="--in-file-regex (S2AGRI_L3B_SNDVI_A[0-9]{8})T([0-9]{6}_T[0-9]{2}[A-Z]{3})\."
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

echo "Executing: ./agric_practices_merge_files.py --input-dir $INPUT_PATH --output-file $OUTPUT_FILE_PATH $CSV_COMPACT $SORT_INPUT_FILES ${FILTER_FILES_REGEX_OPT}"

./agric_practices_merge_files.py --input-dir $INPUT_PATH --output-file $OUTPUT_FILE_PATH $CSV_COMPACT $SORT_INPUT_FILES ${FILTER_FILES_REGEX_OPT}

