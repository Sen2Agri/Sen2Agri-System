#!/bin/bash
set -e
source set_build_folder.sh

./CropTypeFused.py \
    -refp /mnt/Imagery_S2A/In-Situ_TDS/Ukraine/LC/UA_KYIV_LC_FO_2013.shp \
    -input @inputs/ukraine-s4-l8.txt \
    -rseed 0 -pixsize 20 \
    -outdir /mnt/data/ukraine/Ukraine-type \
    -buildfolder "$BUILD_FOLDER"
