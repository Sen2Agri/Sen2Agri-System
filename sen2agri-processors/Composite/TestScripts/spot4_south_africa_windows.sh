#!/bin/bash
#USER modif
BUILD_FOLDER=$1
RES=$2
OUT_FOLDER=$3

#add directories where SPOT products are to be found
./composite_processing.py --applocation $BUILD_FOLDER --syntdate 20130228 --synthalf 15 --input \
"/mnt/Imagery_S2A/L2A/Spot4-T5/South Africa/SPOT4_HRVIR1_XS_20130220_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130220_N2A_ESouthAfricaD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/South Africa/SPOT4_HRVIR1_XS_20130225_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130225_N2A_ESouthAfricaD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/South Africa/SPOT4_HRVIR1_XS_20130302_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130302_N2A_ESouthAfricaD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/South Africa/SPOT4_HRVIR1_XS_20130312_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130312_N2A_ESouthAfricaD0000B0000.xml" \
--res $RES --t0 20130220 --tend 20130312 --outdir $OUT_FOLDER/20130228 --bandsmap bands_mapping_spot.txt

./composite_processing.py --applocation $BUILD_FOLDER --syntdate 20130318 --synthalf 15 --input \
"/mnt/Imagery_S2A/L2A/Spot4-T5/South Africa/SPOT4_HRVIR1_XS_20130317_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130317_N2A_ESouthAfricaD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/South Africa/SPOT4_HRVIR1_XS_20130322_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130322_N2A_ESouthAfricaD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/South Africa/SPOT4_HRVIR1_XS_20130401_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130401_N2A_ESouthAfricaD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/South Africa/SPOT4_HRVIR1_XS_20130406_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130406_N2A_ESouthAfricaD0000B0000.xml" \
--res $RES --t0 20130317 --tend 20130406 --outdir $OUT_FOLDER/20130328 --bandsmap bands_mapping_spot.txt

./composite_processing.py --applocation $BUILD_FOLDER --syntdate 20130425 --synthalf 15 --input \
"/mnt/Imagery_S2A/L2A/Spot4-T5/South Africa/SPOT4_HRVIR1_XS_20130416_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130416_N2A_ESouthAfricaD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/South Africa/SPOT4_HRVIR1_XS_20130421_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130421_N2A_ESouthAfricaD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/South Africa/SPOT4_HRVIR1_XS_20130426_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130426_N2A_ESouthAfricaD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/South Africa/SPOT4_HRVIR1_XS_20130501_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130501_N2A_ESouthAfricaD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/South Africa/SPOT4_HRVIR1_XS_20130506_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130506_N2A_ESouthAfricaD0000B0000.xml" \
--res $RES --t0 20130416 --tend 20130506 --outdir $OUT_FOLDER/20130425 --bandsmap bands_mapping_spot.txt
./composite_processing.py --applocation $BUILD_FOLDER --syntdate 20130523 --synthalf 15 --input \
"/mnt/Imagery_S2A/L2A/Spot4-T5/South Africa/SPOT4_HRVIR1_XS_20130511_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130511_N2A_ESouthAfricaD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/South Africa/SPOT4_HRVIR1_XS_20130516_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130516_N2A_ESouthAfricaD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/South Africa/SPOT4_HRVIR1_XS_20130521_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130521_N2A_ESouthAfricaD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/South Africa/SPOT4_HRVIR1_XS_20130526_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130526_N2A_ESouthAfricaD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/South Africa/SPOT4_HRVIR1_XS_20130531_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130531_N2A_ESouthAfricaD0000B0000.xml" \
"/mnt/Imagery_S2A/L2A/Spot4-T5/South Africa/SPOT4_HRVIR1_XS_20130605_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130605_N2A_ESouthAfricaD0000B0000.xml" \
--res $RES --t0 20130511 --tend 20130605 --outdir $OUT_FOLDER/20130523 --bandsmap bands_mapping_spot.txt
