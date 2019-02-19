#!/bin/bash

if [ "$#" -lt 2 ]; then
    echo "Please provide the country code (NLD|CZE|LTU|ESP|ITA|ROU), the product type (NDVI|AMP|COHE) and, optionally, the polarisation (VV|VH)"
    exit 1
fi

SITE_ID=""
POOL_SIZE=2
PRDS_PER_GROUP=20

WORKING_DIR_ROOT="/mnt/archive/agric_practices"
INSITU_ROOT="$WORKING_DIR_ROOT/insitu/"
OUTPUTS_ROOT="$WORKING_DIR_ROOT/Outputs/"
INPUTS_FILES_ROOT="/mnt/archive/agric_practices/Inputs/exec_cmds/"
FILTER_IDS_FILE="$INSITU_ROOT/PracticesInfos/Sen4CAP_L4C_2018_FilterIDs.csv"

INSITU_REL_PATH=""
OUT_REL_PATH=""
INPUT_FILE_NAME=""
FILE_TYPE=""

INSITU_BUFFER_SIZE="10m"

# TODO: Uncomment the lines below to launch the execution using S2 rasters

case "$2" in
    NDVI)
        FILE_TYPE="NDVI"
        OUT_REL_PATH="NDVI_SHP"
        #OUT_REL_PATH="NDVI_S2"
        INPUT_FILE_NAME="2018_prds_ndvi"
        INSITU_BUFFER_SIZE="5m"
        ;;
    AMP)
        FILE_TYPE="AMP"
        OUT_REL_PATH="AMP_SHP"
        #OUT_REL_PATH="AMP_S2"
        INPUT_FILE_NAME="2018_prds_s1_amp"
        ;;
    COHE)
        FILE_TYPE="COHE"
        OUT_REL_PATH="COHE_SHP"
        #OUT_REL_PATH="COHE_S2"
        INPUT_FILE_NAME="2018_prds_s1_cohe"
        ;;
    *)
        echo $"Usage: $0 {NLD|CZE|LTU|ESP|ITA|ROU} (NDVI|AMP|COHE)"
        exit 1
esac        

case "$1" in
    NLD)
        INSITU_REL_PATH="nld_2018_32631_buf_${INSITU_BUFFER_SIZE}.shp"
        SITE_ID="4"
        ;;
    CZE)
        INSITU_REL_PATH="cze_2018_32633_buf_${INSITU_BUFFER_SIZE}.shp"
        ;;
    LTU)
        INSITU_REL_PATH="ltu_2018_32635_buf_${INSITU_BUFFER_SIZE}.shp"
        ;;
    ESP)
        INSITU_REL_PATH="gsaa2018_32629_full_buf_${INSITU_BUFFER_SIZE}.shp"
        ;;
    ITA)
        INSITU_REL_PATH="TODO.shp"
        ;;
    ROU)
        INSITU_REL_PATH="TODO.shp"
        ;;
    *)
        echo $"Usage: $0 {NLD|CZE|LTU|ESP|ITA|ROU}"
        exit 1
esac

S2_RASTERS_PATH="/d38/UCL/InSituData/07_2018_final/$1/Rasters"

POLARISATION=""
if [ "$#" -eq 3 ] ; then
    POLARISATION="--polarisation $3"
    INPUT_FILE_NAME="$INPUT_FILE_NAME""_$3"
fi

INSITU_FULL_PATH="$INSITU_ROOT$INSITU_REL_PATH"
OUT_FULL_PATH="$OUTPUTS_ROOT$OUT_REL_PATH"
INPUT_FILE_NAME="$INPUT_FILE_NAME.txt"
INPUT_FILE_PATH="$INPUTS_FILES_ROOT$INPUT_FILE_NAME"

python agric_practices_data_extraction.py --lpis-shp $INSITU_FULL_PATH --file-type $FILE_TYPE --inputs-file $INPUT_FILE_PATH -p $OUT_FULL_PATH --prds-per-group $PRDS_PER_GROUP --use-shapefile-only 1 --uid-field NewID --pool-size $POOL_SIZE --filter-ids $FILTER_IDS_FILE

#python agric_practices_data_extraction.py -s $SITE_ID --season-start "2018-01-01" --season-end "2019-01-01" --lpis-name nld_2018_decl --lpis-path $S2_RASTERS_PATH --lpis-shp $INSITU_FULL_PATH --file-type $FILE_TYPE -p $OUT_FULL_PATH --prds-per-group $PRDS_PER_GROUP --use-shapefile-only 1 --uid-field NewID --pool-size $POOL_SIZE --filter-ids $FILTER_IDS_FILE $POLARISATION

