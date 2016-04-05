#!/bin/bash

declare -a nameArr=("Morocco"
"Pakistan" 
"Russia"
"SouthAfrica"
"Ukraine"
"US-Maricopa"
)

declare -a pathsArr

for name in "${nameArr[@]}"
do
    pathsArr+=("/mnt/archive/Sen2Agri_DataSets/2013/$name/EO/L8/")
done

OUT_MACCS_FOLDER_NAME="MACCS_Format_New"
PREV_MACCS_FOLDER_NAME="MACCS_Format"
REF_MACCS_FOLDER="/home/cudroiu/sen2agri/scripts/maccs/convert_maccs_l8_products/ReferenceProduct/BelgiumS2A_20130728_L8_198_025"
OUT_FOLDER_FOR_ARCHIVES="/mnt/output/L2A/Landsat8"

for root_folder in "${pathsArr[@]}"
do
    out_maccs_folder="${root_folder}/${OUT_MACCS_FOLDER_NAME}"
    echo "Converting ${root_folder} to ${out_maccs_folder}"
    mkdir -p $out_maccs_folder
    ./convert_maccs_l8_products.py --indir $root_folder --refdir $REF_MACCS_FOLDER --outdir $out_maccs_folder
done

for root_folder in "${pathsArr[@]}"
do
   echo "Comparing $root_folder"
   maccs_folder_new="${root_folder}/${OUT_MACCS_FOLDER_NAME}"
   maccs_folder_prev="${root_folder}/${PREV_MACCS_FOLDER_NAME}"
   diff -qr $maccs_folder_new $maccs_folder_prev
done

MyPwd=`pwd`
for folderName in "${nameArr[@]}"
do
   mkdir -p "${OUT_FOLDER_FOR_ARCHIVES}/${folderName}"
   root_folder="/mnt/archive/Sen2Agri_DataSets/2013/$folderName/EO/L8/"
   echo "Zipping in ${root_folder}/${OUT_MACCS_FOLDER_NAME}"
   cd "${root_folder}/${OUT_MACCS_FOLDER_NAME}"
   for FOLDER in *; do
        if [[ -d $FOLDER ]]; then
            dirname="$(basename "${FOLDER}")"
            echo "executing zip -R ${dirname}.zip $dirname"
            zip -r "${dirname}.zip" $dirname
            mv "${dirname}.zip" "${OUT_FOLDER_FOR_ARCHIVES}/${folderName}"
        fi
    done
done

cd $MyPwd
