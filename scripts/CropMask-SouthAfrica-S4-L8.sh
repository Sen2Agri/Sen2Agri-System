#!/bin/bash

source set_build_folder.sh

./CropMask.py \
    -refp /mnt/Sen2Agri_DataSets/In-Situ_TDS/SouthAfrica/ZA_FST_LC_FO_2013.shp \
    -input \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130131_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130131_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130205_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130205_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130220_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130220_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130225_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130225_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130302_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130302_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130312_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130312_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130317_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130317_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130322_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130322_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130401_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130401_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130406_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130406_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130416_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130416_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130421_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130421_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130426_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130426_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130501_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130501_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130506_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130506_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130511_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130511_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130516_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130516_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130521_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130521_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130526_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130526_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130531_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130531_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130605_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130605_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130610_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130610_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Spot4-T5/South\ Africa/SPOT4_HRVIR1_XS_20130615_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130615_N2A_ESouthAfricaD0000B0000.xml \
    /mnt/Sen2Agri_DataSets/L2A/Landsat8/South\ Africa/MACCS_Manual_Format/SouthAfricaS2A_20130426_L8_171_079/SouthAfricaS2A_20130426_L8_171_079.HDR \
    /mnt/Sen2Agri_DataSets/L2A/Landsat8/South\ Africa/MACCS_Manual_Format/SouthAfricaS2A_20130512_L8_171_079/SouthAfricaS2A_20130512_L8_171_079.HDR \
    /mnt/Sen2Agri_DataSets/L2A/Landsat8/South\ Africa/MACCS_Manual_Format/SouthAfricaS2A_20130528_L8_171_079/SouthAfricaS2A_20130528_L8_171_079.HDR \
    /mnt/Sen2Agri_DataSets/L2A/Landsat8/South\ Africa/MACCS_Manual_Format/SouthAfricaS2A_20130613_L8_171_079/SouthAfricaS2A_20130613_L8_171_079.HDR \
    /mnt/Sen2Agri_DataSets/L2A/Landsat8/South\ Africa/MACCS_Manual_Format/SouthAfricaS2A_20130629_L8_171_079/SouthAfricaS2A_20130629_L8_171_079.HDR \
    /mnt/Sen2Agri_DataSets/L2A/Landsat8/South\ Africa/MACCS_Manual_Format/SouthAfricaS2A_20130715_L8_171_079/SouthAfricaS2A_20130715_L8_171_079.HDR \
    /mnt/Sen2Agri_DataSets/L2A/Landsat8/South\ Africa/MACCS_Manual_Format/SouthAfricaS2A_20130731_L8_171_079/SouthAfricaS2A_20130731_L8_171_079.HDR \
    /mnt/Sen2Agri_DataSets/L2A/Landsat8/South\ Africa/MACCS_Manual_Format/SouthAfricaS2A_20130816_L8_171_079/SouthAfricaS2A_20130816_L8_171_079.HDR \
    /mnt/Sen2Agri_DataSets/L2A/Landsat8/South\ Africa/MACCS_Manual_Format/SouthAfricaS2A_20130901_L8_171_079/SouthAfricaS2A_20130901_L8_171_079.HDR \
    /mnt/Sen2Agri_DataSets/L2A/Landsat8/South\ Africa/MACCS_Manual_Format/SouthAfricaS2A_20130917_L8_171_079/SouthAfricaS2A_20130917_L8_171_079.HDR \
    /mnt/Sen2Agri_DataSets/L2A/Landsat8/South\ Africa/MACCS_Manual_Format/SouthAfricaS2A_20131019_L8_171_079/SouthAfricaS2A_20131019_L8_171_079.HDR \
    /mnt/Sen2Agri_DataSets/L2A/Landsat8/South\ Africa/MACCS_Manual_Format/SouthAfricaS2A_20131104_L8_171_079/SouthAfricaS2A_20131104_L8_171_079.HDR \
    /mnt/Sen2Agri_DataSets/L2A/Landsat8/South\ Africa/MACCS_Manual_Format/SouthAfricaS2A_20131120_L8_171_079/SouthAfricaS2A_20131120_L8_171_079.HDR \
    /mnt/Sen2Agri_DataSets/L2A/Landsat8/South\ Africa/MACCS_Manual_Format/SouthAfricaS2A_20131222_L8_171_079/SouthAfricaS2A_20131222_L8_171_079.HDR \
    -t0 20130131 -tend 20130615 -rate 5 \
    -rseed 0 -pixsize 20 \
    -outdir /mnt/data/southafrica/SouthAfrica-mask/ \
    -buildfolder $BUILD_FOLDER
