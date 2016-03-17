#!/bin/bash

CUR_DATE=`date +%Y-%m-%d`

OUT_FOLDER_PATH="/mnt/output/L3B/SPOT4-T5/Ukraine/composite_${CUR_DATE}"

./spot4_ukraine_windows.sh ~/sen2agri-processors-build 20 "/mnt/output/L3A/SPOT4-T5/Ukraine/composite_${CUR_DATE}"
./spot4_south_africa_windows.sh ~/sen2agri-processors-build 20 "/mnt/output/L3A/SPOT4-T5/SouthAfrica/composite_${CUR_DATE}"
./midp_W_windows.sh ~/sen2agri-processors-build 20 "/mnt/output/L3A/SPOT4-T5/Midp_W/composite_${CUR_DATE}"
