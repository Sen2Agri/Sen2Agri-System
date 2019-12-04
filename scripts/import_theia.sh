#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: import_theia.sh <INPUT_PRDS_DIR> <SITE_SHORT_NAME>"
    exit 1
fi

INPUT_DIR=$1
SITE_NAME=$2

if [ ! -d ${INPUT_DIR} ] ; then 
    echo "No such input directory ${INPUT_DIR}. Exiting ..."
    exit 1
fi

INIT_PWD=`pwd`

cd ${INPUT_DIR}
INPUT_DIR_PATH=`pwd`

echo "Starting processing products in folder: ${INPUT_DIR_PATH}"

for zip_name in *.zip; do
    ZIP_FILE_PATH="${INPUT_DIR_PATH}/${zip_name}"
    echo "Processing zip file: $ZIP_FILE_PATH"
    filename=$(basename -- "$zip_name")
    tgt_dir="${INPUT_DIR_PATH}/${filename%.*}"
    echo "Target product path: ${tgt_dir}"    
    mkdir -p ${tgt_dir}
    unzip -o "$ZIP_FILE_PATH" -d ${tgt_dir}
    for prd_dir in ${tgt_dir}/*/ ; do
        echo "Unzipped dir full path: ${prd_dir}"
        if [ -d "${prd_dir}" ] ; then 
            echo "Product dir is : ${prd_dir}"
            mtd_file="${prd_dir}/*_MTD_ALL.xml"
            echo "Metadata file is : $mtd_file"
            target_dir=$(xmllint --xpath '//Muscate_Metadata_Document/Production_Informations/Processing_Jobs_List/Job/Products_List/Inputs_List/PRODUCT/PRODUCT_ID' ${mtd_file} | sed 's/<\/PRODUCT_ID>/\n/g' | sed 's/<PRODUCT_ID>//g' | grep _MSIL1C_)
            echo "Target dir is : $target_dir"
            TARGET_DIR_PATH="${INPUT_DIR_PATH}/${target_dir}"
            if [ -d "${TARGET_DIR_PATH}" ] ; then 
                rm -fR "${TARGET_DIR_PATH}"
            fi
            mkdir -p "${TARGET_DIR_PATH}"
            mv -f ${prd_dir} "${TARGET_DIR_PATH}"
            chmod -R a+r "${TARGET_DIR_PATH}"
        fi
    done
    if [ -d ${tgt_dir} ] ; then
        rm -fR ${tgt_dir}
    fi
done

echo "Importing products in folder ${INPUT_DIR_PATH}"
insert_l2a_product_to_db.py -p l2a -t l2a -s "${SITE_NAME}" -m true -d "${INPUT_DIR_PATH}/"

cd ${INIT_PWD}