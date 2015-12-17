#!/bin/bash

source set_build_folder.sh

./CropType.py -ref /mnt/Imagery_S2A/Belgium/TDS/InSitu/LC/BE_HESB_LC_II_2013.shp -ratio 0.75 -input \
/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000.xml \
/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130402_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130402_N2A_EBelgiumD0000B0000.xml \
/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130407_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130407_N2A_EBelgiumD0000B0000.xml \
/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130417_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130417_N2A_EBelgiumD0000B0000.xml \
/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130422_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130422_N2A_EBelgiumD0000B0000.xml \
/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130507_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130507_N2A_EBelgiumD0000B0000.xml \
/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130527_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130527_N2A_EBelgiumD0000B0000.xml \
/mnt/Imagery_S2A/L2A/Spot4-T5/Belgium/SPOT4_HRVIR1_XS_20130606_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130606_N2A_EBelgiumD0000B0000.xml \
-t0 20130318 -tend 20130606 -rate 5 -radius 100 -classifier rf -rfnbtrees 100 -rfmax 25 -rfmin 25 -rseed 0 -mask /mnt/data/belgium/Belgium-mask/crop_mask.tif -pixsize 20 \
-outdir /mnt/data/belgium/Belgium-type -fromstep 10 -buildfolder $BUILD_FOLDER
