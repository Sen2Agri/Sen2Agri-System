#!/bin/bash

THREAD_SIZE_POOL=10
SIMULATE_CMDS=0
FORCE_REINIT=0

SCRIPTS_DIR="/mnt/archive/agric_practices/Inputs/exec_cmds/"
LOG_FILES_DIR="/mnt/archive/agric_practices/Inputs/exec_cmds/"
PRD_LIST_FILES_DIR="/mnt/archive/agric_practices/Inputs/exec_cmds/"
OUTPUTS_DIR="/mnt/archive/agric_practices/Outputs/"

function usage() {
    echo "Usage: ./agric_practices_tsa.sh -c <COUNTRY_CODE - (NLD|CZE|LTU|ESP|ITA|ROU)> -y <YEAR> -d <PROD_DATE> -p <PREV_PRODUCT> -e <PREV_DATA_EXTR_DATE> -s <SITE_ID> -f <FORCE_REINIT>"
    exit 1
}

function getProductId() {
    local  prdId=''
    declare -i idx=0
    res=$(sudo -u postgres psql sen4cap -c "SELECT id from product where name = '$1'")
    linesCnt=$(echo "$res" | wc -l)
    if [ $linesCnt -ge 4 ] ; then
        for rec in ${res} ; do 
            idx=$((idx+1))
            if [ "$idx" == "3" ] ; then 
                prdId="$rec"
            fi
        done ;
    fi
    echo "$prdId"
}

function getNdviProductFiles() {
    echo "Created timestamp is $1"
    MAX_DATE=$1
    NDVI_PRDS_FILE="${PRD_LIST_FILES_DIR}/NDVI_ALL_${PROD_DATE}_Products.txt"
    execute_command "sudo -u postgres psql sen4cap -c \"select full_path from product where site_id=${SITE_ID} and product_type_id = 3 and created_timestamp>='$YEAR-01-01' and created_timestamp<='${MAX_DATE}';\" > ${PRD_LIST_FILES_DIR}/NDVI_ALL_${PROD_DATE}_Products.txt"
    NDVI_OUT_FILE="${PRD_LIST_FILES_DIR}/NDVI_ALL_${PROD_DATE}.txt"
    # truncate or create file
    :> ${NDVI_OUT_FILE}
    while IFS= read -r line; do
        line=${line//[$'\t\r\n ']}
        if [ -d "${line}" ] ; then 
            for entry in "${line}"/*/*/IMG_DATA/*_SNDVI_*.TIF
            do
                echo "$entry" >> ${NDVI_OUT_FILE}
            done
        fi
    done < "${NDVI_PRDS_FILE}"
}

function execute_command() {
    doExecute=1
    # $2 = 1 means simulate execution
    if [[ "$#" -gt 1 && "$2" == "1" ]]; then
        doExecute=0
    fi
    if [ "${doExecute}" == "1" ] ; then 
        echo "######################"
        echo "Real execution of: $1"    
        eval $1
        echo "######################"
    else 
        echo "######################"
        echo "Simulation execution of: $1"
        echo "######################"
    fi
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
    -p|--prev-product)
    PREV_PRODUCT="$2"
    shift # past argument
    shift # past value
    ;;
    -d|--prod-date)
    PROD_DATE="$2"
    shift # past argument
    shift # past value
    ;;
    -e|--prev-data-extr-date)
    PREV_DATA_EXTR_DATE="$2"
    shift # past argument
    shift # past value
    ;;
    -s|--site-id)
    SITE_ID="$2"
    shift # past argument
    shift # past value
    ;;
    -f|--force-reinit)
    FORCE_REINIT_VAL="$2"
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

if [ -z ${COUNTRY_AND_REGION} ] ; then
    echo "No country provided!"
    usage
fi 

if [ -z ${YEAR} ] ; then
    YEAR=`date -d $PROD_DATE +%Y`
    echo "Using automatically detected year $YEAR"
fi 

if [ -z ${PROD_DATE} ] ; then
    echo "No product date provided!"
    usage
fi 

if [ -z ${PREV_DATA_EXTR_DATE} ] ; then 
    PREV_PRD_DATE=`date -d "$PROD_DATE -7 days" +"%Y%m%d"`
else
    PREV_PRD_DATE=${PREV_DATA_EXTR_DATE}
fi

if [ -z ${FORCE_REINIT_VAL} ] ; then 
    FORCE_REINIT=0
else
    if [[ "$FORCE_REINIT_VAL" == "1" || "$FORCE_REINIT_VAL" == "true" ]] ; then 
        FORCE_REINIT=1
    fi
fi


echo "Prev product date is : $PREV_PRD_DATE"
echo "Force reinit is : $FORCE_REINIT"

if [ -z ${PREV_PRODUCT} ] ; then
    echo "No previous product will be used!"
    PREV_PRODUCT_OPTION=""
else
    if [ "${PREV_PRODUCT}" == "TSA" ] ; then 
        PREV_TSA_DIR_PATH="${OUTPUTS_DIR}/${COUNTRY}/TimeSeriesAnalysisResults_${PREV_PRD_DATE}"
        if [ -d ${PREV_TSA_DIR_PATH} ] ; then
            PREV_PRODUCT_OPTION="-p ${PREV_TSA_DIR_PATH}"
        fi
    else
        echo "Checking the product $PREV_PRODUCT in database ..."
        declare -i idx=0
        for rec in $(sudo -u postgres psql sen4cap -c "SELECT full_path, geog, tiles, created_timestamp FROM product WHERE name = '$PREV_PRODUCT' OR full_path = '$PREV_PRODUCT'") ; do 
            #echo "Echo $rec";
            if [ "$idx" == "8" ] ; then 
                PREV_PRODUCT="$rec"
                if [ -d ${PREV_PRODUCT} ] ; then
                    PREV_PRODUCT_OPTION="-p ${PREV_PRODUCT}/VECTOR_DATA/"
                fi
            elif [ "$idx" == "10" ] ; then 
                PREV_PRODUCT_GEOG="$rec"
            elif [ "$idx" == "12" ] ; then 
                PREV_PRODUCT_TILES="$rec"
            elif [ "$idx" == "14" ] ; then 
                PREV_PRD_DATE=$(date -d "$rec" +'%Y%m%d')
                echo "Prev product date is now: $PREV_PRD_DATE"
            fi
            idx=$((idx+1))            
        done;  
    fi
    if [[ -z ${PREV_PRODUCT_OPTION} ]] ; then 
        echo "Could not detetermine the previous product from the parameter given. It must be TSA, the short name of the product in the database or the full path of the product. Exiting ..."
        exit 1
    fi
fi 

# Get the geog and site tiles from the site definition 
if [ -z ${PREV_PRODUCT_GEOG} ] ; then
    declare -i idx=0
    for rec in $(sudo -u postgres psql sen4cap -c "SELECT geog from site where id = $SITE_ID;") ; do 
        #echo "Echo $rec"; 
        if [ "$idx" == "2" ] ; then 
            PREV_PRODUCT_GEOG="$rec"
        fi
        idx=$((idx+1))        
    done;
    PREV_PRODUCT_TILES="{}"
fi


COUNTRY="${COUNTRY_AND_REGION%%_*}"
COUNTRY_REGION=""
if [ "$1" != "$COUNTRY" ] ; then

    COUNTRY_REGION="${1##*_}"
fi   
# Get the country as lower case
COUNTRY_LC="${COUNTRY,,}"

# Change dir location with the one from DB for this site
SITE_ARCH_PATH="/mnt/archive/${COUNTRY_LC}_${YEAR}"
declare -i idx=0
for rec in $(sudo -u postgres psql sen4cap -c "select short_name from site where id = $SITE_ID;") ; do 
    #echo "Echo $rec"; 
    if [ "$idx" == "2" ] ; then 
        SITE_ARCH_PATH="$rec"
        SITE_ARCH_PATH="/mnt/archive/$rec"
    fi
    idx=$((idx+1))        
done;

MAX_DATE="${PROD_DATE}T23:59:59"

getNdviProductFiles $MAX_DATE

if [[ -f "${PRD_LIST_FILES_DIR}/NDVI_ALL_${PREV_PRD_DATE}.txt" && $FORCE_REINIT == 0 ]] ; then
    execute_command "comm -2 -3 <(sort  ${PRD_LIST_FILES_DIR}/NDVI_ALL_${PROD_DATE}.txt) <(sort ${PRD_LIST_FILES_DIR}/NDVI_ALL_${PREV_PRD_DATE}.txt) > ${PRD_LIST_FILES_DIR}/diff_ndvi_${PROD_DATE}.txt"
else
    execute_command "cp -f ${PRD_LIST_FILES_DIR}/NDVI_ALL_${PROD_DATE}.txt ${PRD_LIST_FILES_DIR}/diff_ndvi_${PROD_DATE}.txt"
fi

execute_command "cp -f ${PRD_LIST_FILES_DIR}/diff_ndvi_${PROD_DATE}.txt ${PRD_LIST_FILES_DIR}/${YEAR}_prds_ndvi.txt"

execute_command "sudo -u postgres psql sen4cap -c \"select full_path from product where site_id=${SITE_ID} and satellite_id=3 and created_timestamp>='$YEAR-01-01' and created_timestamp<='${MAX_DATE}';\" > ${PRD_LIST_FILES_DIR}/S1_all_$PROD_DATE.txt"

if [[ -f S1_all_${PREV_PRD_DATE}.txt && $FORCE_REINIT == 0 ]] ; then
    execute_command "comm -2 -3 <(sort  ${PRD_LIST_FILES_DIR}/S1_all_${PROD_DATE}.txt) <(sort ${PRD_LIST_FILES_DIR}/S1_all_${PREV_PRD_DATE}.txt) > ${PRD_LIST_FILES_DIR}/diff_S1_${PROD_DATE}.txt"
else
    execute_command "cp -f ${PRD_LIST_FILES_DIR}/S1_all_${PROD_DATE}.txt ${PRD_LIST_FILES_DIR}/diff_S1_${PROD_DATE}.txt"
fi

execute_command "cp -f ${PRD_LIST_FILES_DIR}/diff_S1_${PROD_DATE}.txt ${PRD_LIST_FILES_DIR}/${YEAR}_prds_s1_amp_VH.txt"
execute_command "cp -f ${PRD_LIST_FILES_DIR}/diff_S1_${PROD_DATE}.txt ${PRD_LIST_FILES_DIR}/${YEAR}_prds_s1_amp_VV.txt"
execute_command "cp -f ${PRD_LIST_FILES_DIR}/diff_S1_${PROD_DATE}.txt ${PRD_LIST_FILES_DIR}/${YEAR}_prds_s1_cohe_VV.txt"

sed -ni '/_AMP.tif/p' "${PRD_LIST_FILES_DIR}/${YEAR}_prds_s1_amp_VH.txt"
sed -ni '/_VH_/p' "${PRD_LIST_FILES_DIR}/${YEAR}_prds_s1_amp_VH.txt"

sed -ni '/_AMP.tif/p' "${PRD_LIST_FILES_DIR}/${YEAR}_prds_s1_amp_VV.txt"
sed -ni '/_VV_/p' "${PRD_LIST_FILES_DIR}/${YEAR}_prds_s1_amp_VV.txt"

sed -ni '/_COHE.tif/p' "${PRD_LIST_FILES_DIR}/${YEAR}_prds_s1_cohe_VV.txt"
sed -ni '/_VV_/p' "${PRD_LIST_FILES_DIR}/${YEAR}_prds_s1_cohe_VV.txt"

# before executing the data extraction, perform backup to the existing files
if [[ -d ${OUTPUTS_DIR}/${COUNTRY}/DataExtractionResults/ && ! -d "${OUTPUTS_DIR}/${COUNTRY}/DataExtractionResults_${PREV_PRD_DATE}/" ]] ; then 
    echo "Backing up the previous DataExtractionResults to ${OUTPUTS_DIR}/${COUNTRY}/DataExtractionResults_${PREV_PRD_DATE}/"
    execute_command "mv ${OUTPUTS_DIR}/${COUNTRY}/DataExtractionResults/ ${OUTPUTS_DIR}/${COUNTRY}/DataExtractionResults_${PREV_PRD_DATE}/" $SIMULATE_CMDS
fi

if [[ -d ${OUTPUTS_DIR}/${COUNTRY}/TimeSeriesAnalysisResults/ && ! -d "${OUTPUTS_DIR}/${COUNTRY}/TimeSeriesAnalysisResults_${PREV_PRD_DATE}/" ]] ; then 
    echo "Backing up the previous TimeSeriesAnalysisResults to ${OUTPUTS_DIR}/${COUNTRY}/TimeSeriesAnalysisResults_${PREV_PRD_DATE}/"
    execute_command "mv ${OUTPUTS_DIR}/${COUNTRY}/TimeSeriesAnalysisResults/ ${OUTPUTS_DIR}/${COUNTRY}/TimeSeriesAnalysisResults_${PREV_PRD_DATE}/" $SIMULATE_CMDS
fi 
    
if [ ! -d ${OUTPUTS_DIR}/${COUNTRY}/Individual_Results_${PREV_PRD_DATE}/ ] ; then     
    echo "Backing up the previous dirs into ${OUTPUTS_DIR}/${COUNTRY}/Individual_Results_${PREV_PRD_DATE}/"
    execute_command "mkdir ${OUTPUTS_DIR}/${COUNTRY}/Individual_Results_${PREV_PRD_DATE}/" $SIMULATE_CMDS
    if [ $FORCE_REINIT == 0 ] ; then 
        execute_command "cp -fR ${OUTPUTS_DIR}/${COUNTRY}/AMP_SHP ${OUTPUTS_DIR}/${COUNTRY}/Individual_Results_${PREV_PRD_DATE}/" $SIMULATE_CMDS
        execute_command "cp -fR ${OUTPUTS_DIR}/${COUNTRY}/COHE_SHP ${OUTPUTS_DIR}/${COUNTRY}/Individual_Results_${PREV_PRD_DATE}/" $SIMULATE_CMDS
        execute_command "cp -fR ${OUTPUTS_DIR}/${COUNTRY}/NDVI_SHP ${OUTPUTS_DIR}/${COUNTRY}/Individual_Results_${PREV_PRD_DATE}/" $SIMULATE_CMDS
    else
        execute_command "mv ${OUTPUTS_DIR}/${COUNTRY}/AMP_SHP ${OUTPUTS_DIR}/${COUNTRY}/Individual_Results_${PREV_PRD_DATE}/" $SIMULATE_CMDS
        execute_command "mv ${OUTPUTS_DIR}/${COUNTRY}/COHE_SHP ${OUTPUTS_DIR}/${COUNTRY}/Individual_Results_${PREV_PRD_DATE}/" $SIMULATE_CMDS
        execute_command "mv ${OUTPUTS_DIR}/${COUNTRY}/NDVI_SHP ${OUTPUTS_DIR}/${COUNTRY}/Individual_Results_${PREV_PRD_DATE}/" $SIMULATE_CMDS
    fi
fi

execute_command "${SCRIPTS_DIR}/agric_practices_data_extraction.sh --country ${COUNTRY} --prdtype AMP --polarisation VH --year ${YEAR} --prds_per_group 1 --threads_pool_size ${THREAD_SIZE_POOL} > ${LOG_FILES_DIR}/res_amp_vh_${PROD_DATE}.txt" $SIMULATE_CMDS
execute_command "${SCRIPTS_DIR}/agric_practices_data_extraction.sh --country ${COUNTRY} --prdtype AMP --polarisation VV --year ${YEAR} --prds_per_group 1 --threads_pool_size ${THREAD_SIZE_POOL} > ${LOG_FILES_DIR}/res_amp_vv_${PROD_DATE}.txt" $SIMULATE_CMDS
execute_command "${SCRIPTS_DIR}/agric_practices_data_extraction.sh --country ${COUNTRY} --prdtype COHE --polarisation VV --year ${YEAR} --prds_per_group 1 --threads_pool_size ${THREAD_SIZE_POOL} > ${LOG_FILES_DIR}/res_cohe_vv_${PROD_DATE}.txt" $SIMULATE_CMDS
execute_command "${SCRIPTS_DIR}/agric_practices_data_extraction.sh --country ${COUNTRY} --prdtype NDVI --year ${YEAR} --prds_per_group 1 --threads_pool_size ${THREAD_SIZE_POOL} > ${LOG_FILES_DIR}/res_ndvi_${PROD_DATE}.txt" $SIMULATE_CMDS

execute_command "${SCRIPTS_DIR}/agric_practices_merge_files.sh -c ${COUNTRY} -y ${YEAR} -p AMP --compact 1 ; ${SCRIPTS_DIR}/agric_practices_merge_files.sh -c ${COUNTRY} -y ${YEAR} -p AMP ; ${SCRIPTS_DIR}/agric_practices_merge_files.sh -c ${COUNTRY} -y ${YEAR} -p COHE --compact 1 ; ${SCRIPTS_DIR}/agric_practices_merge_files.sh -c ${COUNTRY} -y ${YEAR} -p COHE ; ${SCRIPTS_DIR}/agric_practices_merge_files.sh -c ${COUNTRY} -y ${YEAR} -p NDVI --compact 1 ; ${SCRIPTS_DIR}/agric_practices_merge_files.sh -c ${COUNTRY} -y ${YEAR} -p NDVI" $SIMULATE_CMDS

if [ -z ${PREV_PRODUCT_OPTION} ] ; then 
    execute_command "${SCRIPTS_DIR}/agric_practices_tsa.sh -c ${COUNTRY} -y ${YEAR}" $SIMULATE_CMDS
else
    execute_command "${SCRIPTS_DIR}/agric_practices_tsa.sh -c ${COUNTRY} -y ${YEAR} ${PREV_PRODUCT_OPTION}" $SIMULATE_CMDS
fi

# Create the product in the database
NEW_PRD_DIR="S2AGRI_S4C_L4C_PRD_S${SITE_ID}_${PROD_DATE}T235959_V${YEAR}0101T000000_${PROD_DATE}T235959"
NEW_PRD_FULL_PATH="${SITE_ARCH_PATH}/s4c_l4c/${NEW_PRD_DIR}/"
VECTOR_DATA_DIR="${NEW_PRD_FULL_PATH}VECTOR_DATA/"  

if [ ! -d "${VECTOR_DATA_DIR}/SHP" ]; then
    execute_command "mkdir -p \"${VECTOR_DATA_DIR}/SHP\"" $SIMULATE_CMDS
fi
execute_command "cp \"${OUTPUTS_DIR}/${COUNTRY}/TimeSeriesAnalysisResults/\"* \"${VECTOR_DATA_DIR}\"" $SIMULATE_CMDS

# now insert the product in the database
#Here we parse the date to get year, month and day
MONTH=`date -d $PROD_DATE +%m`
DAY=`date -d $PROD_DATE +%d`

EXISTING_PRD_ID=$(getProductId ${NEW_PRD_DIR})
if [[ $EXISTING_PRD_ID != "" ]]; then
    execute_command "sudo -u postgres psql sen4cap -c \"delete from product_details_l4c where product_id = ${EXISTING_PRD_ID};\"" $SIMULATE_CMDS
    execute_command "sudo -u postgres psql sen4cap -c \"delete from product where id = ${EXISTING_PRD_ID};\"" $SIMULATE_CMDS
fi

execute_command "sudo -u postgres psql sen4cap -c \"insert into product (product_type_id, processor_id, site_id, full_path, created_timestamp, inserted_timestamp, name, job_id, geog, tiles) values (6, 6, ${SITE_ID}, '${NEW_PRD_FULL_PATH}', '${YEAR}-${MONTH}-${DAY} 23:59:59.405973+02', '${YEAR}-${MONTH}-${DAY} 23:59:59.405973+02', '${NEW_PRD_DIR}', 50, '${PREV_PRODUCT_GEOG}', '${PREV_PRODUCT_TILES}');\"" $SIMULATE_CMDS

NEW_PRD_ID=$(getProductId ${NEW_PRD_DIR})

if [ -z ${NEW_PRD_ID} ] ; then
    echo "Cannot get new product id from database. Exiting ..."
    exit 1
fi

# Cleanup 
if [ -d ${VECTOR_DATA_DIR} ] ; then
    execute_command "rm -f \"${VECTOR_DATA_DIR}\"/*.gpkg"  $SIMULATE_CMDS
fi

# execute the import 
execute_command "/usr/bin/import-product-details.py -p $NEW_PRD_ID" $SIMULATE_CMDS
execute_command "/usr/bin/export-product.py -p $NEW_PRD_ID ${VECTOR_DATA_DIR}/Sen4CAP_L4C_PRACTICE_${COUNTRY}_${YEAR}.gpkg" $SIMULATE_CMDS

if [ -f ${VECTOR_DATA_DIR}/Sen4CAP_L4C_CatchCrop_${COUNTRY}_${YEAR}.gpkg ]; then
    execute_command "ogr2ogr ${VECTOR_DATA_DIR}/Sen4CAP_L4C_CatchCrop_${COUNTRY}_${YEAR}.shp ${VECTOR_DATA_DIR}/Sen4CAP_L4C_CatchCrop_${COUNTRY}_${YEAR}.gpkg -lco ENCODING=UTF-8" $SIMULATE_CMDS
fi    
if [ -f ${VECTOR_DATA_DIR}/Sen4CAP_L4C_NFC_${COUNTRY}_${YEAR}.gpkg ]; then
    execute_command "ogr2ogr ${VECTOR_DATA_DIR}/Sen4CAP_L4C_NFC_${COUNTRY}_${YEAR}.shp ${VECTOR_DATA_DIR}/Sen4CAP_L4C_NFC_${COUNTRY}_${YEAR}.gpkg -lco ENCODING=UTF-8" $SIMULATE_CMDS
fi
if [ -f ${VECTOR_DATA_DIR}/Sen4CAP_L4C_Fallow_${COUNTRY}_${YEAR}.gpkg ]; then
    execute_command "ogr2ogr ${VECTOR_DATA_DIR}/Sen4CAP_L4C_Fallow_${COUNTRY}_${YEAR}.shp ${VECTOR_DATA_DIR}/Sen4CAP_L4C_Fallow_${COUNTRY}_${YEAR}.gpkg -lco ENCODING=UTF-8" $SIMULATE_CMDS
fi
if [ -f ${VECTOR_DATA_DIR}/Sen4CAP_L4C_NA_${COUNTRY}_${YEAR}.gpkg ]; then
    execute_command "ogr2ogr ${VECTOR_DATA_DIR}/Sen4CAP_L4C_NA_${COUNTRY}_${YEAR}.shp ${VECTOR_DATA_DIR}/Sen4CAP_L4C_NA_${COUNTRY}_${YEAR}.gpkg -lco ENCODING=UTF-8" $SIMULATE_CMDS
fi

# Cleanup 
if [ -d ${VECTOR_DATA_DIR}/SHP ] ; then
    execute_command "rm -f \"${VECTOR_DATA_DIR}\"/SHP/*.cpg" $SIMULATE_CMDS
    execute_command "rm -f \"${VECTOR_DATA_DIR}\"/SHP/*.dbf" $SIMULATE_CMDS
    execute_command "rm -f \"${VECTOR_DATA_DIR}\"/SHP/*.prj" $SIMULATE_CMDS
    execute_command "rm -f \"${VECTOR_DATA_DIR}\"/SHP/*.shp" $SIMULATE_CMDS
    execute_command "rm -f \"${VECTOR_DATA_DIR}\"/SHP/*.shx" $SIMULATE_CMDS
fi

execute_command "mv ${VECTOR_DATA_DIR}/*.cpg ${VECTOR_DATA_DIR}/SHP/" $SIMULATE_CMDS
execute_command "mv ${VECTOR_DATA_DIR}/*.dbf ${VECTOR_DATA_DIR}/SHP/" $SIMULATE_CMDS
execute_command "mv ${VECTOR_DATA_DIR}/*.prj ${VECTOR_DATA_DIR}/SHP/" $SIMULATE_CMDS
execute_command "mv ${VECTOR_DATA_DIR}/*.shp ${VECTOR_DATA_DIR}/SHP/" $SIMULATE_CMDS
execute_command "mv ${VECTOR_DATA_DIR}/*.shx ${VECTOR_DATA_DIR}/SHP/" $SIMULATE_CMDS

 