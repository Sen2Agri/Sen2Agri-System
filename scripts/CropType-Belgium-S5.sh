#!/bin/bash

source set_build_folder.sh

./CropType.py \
    -ref /mnt/Imagery_S2A/Belgium/TDS/InSitu/LC/BE_HESB_LC_II_2013.shp \
    -input \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Belgium/SPOT5_HRG2_XS_20150410_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150410_N2A_BelgiumD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Belgium/SPOT5_HRG2_XS_20150415_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150415_N2A_BelgiumD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Belgium/SPOT5_HRG2_XS_20150420_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150420_N2A_BelgiumD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Belgium/SPOT5_HRG2_XS_20150505_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150505_N2A_BelgiumD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Belgium/SPOT5_HRG2_XS_20150515_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150515_N2A_BelgiumD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Belgium/SPOT5_HRG2_XS_20150520_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150520_N2A_BelgiumD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Belgium/SPOT5_HRG2_XS_20150604_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150604_N2A_BelgiumD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Belgium/SPOT5_HRG2_XS_20150614_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150614_N2A_BelgiumD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Belgium/SPOT5_HRG2_XS_20150704_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150704_N2A_BelgiumD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Belgium/SPOT5_HRG2_XS_20150709_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150709_N2A_BelgiumD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Belgium/SPOT5_HRG2_XS_20150729_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150729_N2A_BelgiumD0000B0000.xml \
    -t0 20150410 -tend 20150729 -rate 5 \
    -rseed 0 -pixsize 10 \
    -mask /mnt/output/L4A/SPOT5-T5/Belgium/work/crop_mask.tif \
    -outdir /mnt/output/L4B/SPOT5-T5/Belgium/work \
    -buildfolder $BUILD_FOLDER
