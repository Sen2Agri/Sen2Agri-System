#!/bin/bash

./CropMask.py \
    -refp /mnt/Imagery_S2A/In-Situ_TDS/Ukraine/LC/UA_KYIV_LC_FO_2013.shp \
    -input \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150410_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150410_N2A_UkraineD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150425_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150425_N2A_UkraineD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150430_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150430_N2A_UkraineD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150510_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150510_N2A_UkraineD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150520_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150520_N2A_UkraineD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150525_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150525_N2A_UkraineD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150604_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150604_N2A_UkraineD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150609_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150609_N2A_UkraineD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150614_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150614_N2A_UkraineD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150619_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150619_N2A_UkraineD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150624_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150624_N2A_UkraineD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150629_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150629_N2A_UkraineD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150704_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150704_N2A_UkraineD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150709_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150709_N2A_UkraineD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150719_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150719_N2A_UkraineD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150724_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150724_N2A_UkraineD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150729_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150729_N2A_UkraineD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150808_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150808_N2A_UkraineD0000B0000.xml \
    -rseed 0 -pixsize 10 \
    -outdir /mnt/output/L4A/SPOT5-T5/Ukraine/work
