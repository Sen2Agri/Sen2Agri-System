#!/bin/bash

function usage() {
    echo "Usage: ./agric_practices_data_extraction_2019.sh -c <COUNTRY_CODE - (NLD|CZE|LTU|ESP|ITA|ROU)> -t <PRODUCT_TYPE> [-p <POLARISATION>] -y <YEAR> -g <PRDS_PER_GROUP> -s <THREADS_POOL_SIZE>"
    exit 1
}

POOL_SIZE=2
PRDS_PER_GROUP=20

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
    -t|--prdtype)
    PRODUCT_TYPE="$2"
    shift # past argument
    shift # past value
    ;;
    -p|--polarisation)
    POLARISATION="$2"
    shift # past argument
    shift # past value
    ;;
    -y|--year)
    YEAR="$2"
    shift # past argument
    shift # past value
    ;;
    -g|--prds_per_group)
    PRDS_PER_GROUP="$2"
    shift # past argument
    shift # past value
    ;;
    -s|--threads_pool_size)
    POOL_SIZE="$2"
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
echo PRODUCT TYPE   = "${PRODUCT_TYPE}"
echo POLARISATION   = "${POLARISATION}"
echo YEAR           = "${YEAR}"
echo PRDS_PER_GROUP = "${PRDS_PER_GROUP}"
echo POOL_SIZE      = "${POOL_SIZE}"

if [ -z ${COUNTRY_AND_REGION} ] ; then
    echo "No country provided!"
    usage
fi 

if [ -z ${PRODUCT_TYPE} ] ; then
    echo "No product type provided!"
    usage
fi 

if [ -z ${YEAR} ] ; then
    echo "No year provided!"
    usage
fi 

COUNTRY="${COUNTRY_AND_REGION%%_*}"
COUNTRY_REGION=""
if [ "${COUNTRY_AND_REGION}" != "$COUNTRY" ] ; then

    COUNTRY_REGION="${1##*_}"
fi    

SITE_ID=""

WORKING_DIR_ROOT="/mnt/archive/agric_practices"
INSITU_ROOT="$WORKING_DIR_ROOT/insitu/"
OUTPUTS_ROOT="$WORKING_DIR_ROOT/Outputs/${COUNTRY_AND_REGION}/"
INPUTS_FILES_ROOT="/mnt/archive/agric_practices/Inputs/exec_cmds/"

if [ -z $COUNTRY_REGION ] ; then 
    FILTER_IDS_FILE="$INSITU_ROOT/PracticesInfos/Sen4CAP_L4C_${YEAR}_FilterIDs.csv"
else 
    FILTER_IDS_FILE="$INSITU_ROOT/PracticesInfos/Sen4CAP_L4C_${YEAR}_${COUNTRY_REGION}_FilterIDs.csv"
fi

SHP_EXT=".shp"
PROJ_SUFFIX=""
INSITU_REL_PATH=""
OUT_REL_PATH=""
INPUT_FILE_NAME=""
FILE_TYPE=""

INSITU_BUFFER_SIZE="10m"

# TODO: Uncomment the lines below to launch the execution using S2 rasters

case "${PRODUCT_TYPE}" in
    NDVI)
        FILE_TYPE="NDVI"
        OUT_REL_PATH="NDVI_SHP"
        INPUT_FILE_NAME="${YEAR}_prds_ndvi"
        INSITU_BUFFER_SIZE="5m"
        ;;
    AMP)
        FILE_TYPE="AMP"
        OUT_REL_PATH="AMP_SHP"
        INPUT_FILE_NAME="${YEAR}_prds_s1_amp"
        if [ "$YEAR" == "2019" ] ; then
            PROJ_SUFFIX="_LAEA"
        fi
        ;;
    COHE)
        FILE_TYPE="COHE"
        OUT_REL_PATH="COHE_SHP"
        INPUT_FILE_NAME="${YEAR}_prds_s1_cohe"
        if [ "$YEAR" == "2019" ] ; then
            PROJ_SUFFIX="_LAEA"
        fi
        ;;
    *)
        echo $"Usage: $0 {NLD|CZE|LTU|ESP|ITA|ROU} (NDVI|AMP|COHE)"
        exit 1
esac        

case "${COUNTRY}" in
    NLD)
        INSITU_FILE_NAME="nld_2018_32631_buf_${INSITU_BUFFER_SIZE}"
        ;;
    CZE)
        INSITU_FILE_NAME="cze_${YEAR}_32633_buf_${INSITU_BUFFER_SIZE}"
        ;;
    LTU)
        INSITU_FILE_NAME="ltu_${YEAR}_32635_buf_${INSITU_BUFFER_SIZE}"
        ;;
    ESP)
        INSITU_FILE_NAME="gsaa${YEAR}_32629_full_buf_${INSITU_BUFFER_SIZE}"
        ;;
    ITA)
        if [ "$COUNTRY_REGION" == "FML" ] ; then
            INSITU_FILE_NAME="ITA_FRIULI_MARCHE_LAZIO_${YEAR}_32632_buf_${INSITU_BUFFER_SIZE}"
        elif [ "$COUNTRY_REGION" == "CP1" ] || [ "$COUNTRY_REGION" == "CP2" ] ; then
            INSITU_FILE_NAME="ITA_CAMPANIA_PUGLIA_${YEAR}_32633_buf_${INSITU_BUFFER_SIZE}"
        else 
            echo "Error executing data extraction for ITA. Unknown region ${COUNTRY_REGION}"
            exit 1
        fi    
        ;;
    ROU)
        INSITU_FILE_NAME="ROM_2018_32634_buf_${INSITU_BUFFER_SIZE}"
        ;;
    *)
        echo $"Usage: $0 {NLD|CZE|LTU|ESP|ITA|ROU}"
        exit 1
esac

S2_RASTERS_PATH="/d38/UCL/InSituData/07_2018_final/$1/Rasters"

POLARISATION_OPT=""
if [ ! -z ${POLARISATION} ] ; then
    POLARISATION_OPT="--polarisation ${POLARISATION}"
    INPUT_FILE_NAME="$INPUT_FILE_NAME""_${POLARISATION}"
fi

INSITU_REL_PATH=${INSITU_FILE_NAME}${SHP_EXT}
INSITU_FULL_REL_PATH=${INSITU_FILE_NAME}${PROJ_SUFFIX}${SHP_EXT}

INSITU_FULL_PATH="${INSITU_ROOT}${INSITU_FULL_REL_PATH}"
INSITU_FULL_PATH_NO_PROJ="${INSITU_ROOT}${INSITU_REL_PATH}"

if [ "${INSITU_REL_PATH}" != "${INSITU_FULL_REL_PATH}" ]; then
    if [ ! -f ${INSITU_FULL_PATH} ] ; then 
        echo "Reprojecting to LAEA ..."
        ogr2ogr -f "ESRI Shapefile" -t_srs "EPSG:3035" "${INSITU_FULL_PATH}" "${INSITU_FULL_PATH_NO_PROJ}"
    fi
fi

OUT_FULL_PATH="$OUTPUTS_ROOT$OUT_REL_PATH"
INPUT_FILE_NAME="$INPUT_FILE_NAME.txt"
INPUT_FILE_PATH="$INPUTS_FILES_ROOT$INPUT_FILE_NAME"



CMD="python agric_practices_data_extraction.py --lpis-shp $INSITU_FULL_PATH --file-type $FILE_TYPE --inputs-file $INPUT_FILE_PATH -p $OUT_FULL_PATH --prds-per-group $PRDS_PER_GROUP --use-shapefile-only 1 --uid-field NewID --pool-size $POOL_SIZE --filter-ids $FILTER_IDS_FILE"

echo "Executing ${CMD}"
    
#Execute the command
eval $CMD    

#python agric_practices_data_extraction.py -s $SITE_ID --season-start "2018-01-01" --season-end "2019-01-01" --lpis-name nld_2018_decl --lpis-path $S2_RASTERS_PATH --lpis-shp $INSITU_FULL_PATH --file-type $FILE_TYPE -p $OUT_FULL_PATH --prds-per-group $PRDS_PER_GROUP --use-shapefile-only 1 --uid-field NewID --pool-size $POOL_SIZE --filter-ids $FILTER_IDS_FILE $POLARISATION_OPT


