#!/bin/bash
set -e
source set_build_folder.sh

./CropTypeFused.py \
    -input @inputs/ukraine-s2-l8.txt \
    -refp /mnt/archive/insitu/ukraine-keep/UA_ALLC_LC_SM_201606_ALL.shp \
    -rseed 0 \
    -outdir /mnt/data/ukraine/Ukraine-type-prod \
    -strata /mnt/archive/insitu/ukraine-keep/strata/UKr_Strata_All.shp \
    -stratum-filter 3 \
    -buildfolder $BUILD_FOLDER
