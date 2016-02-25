#!/bin/bash
./sen2agri-ctl.py submit-job -s Ukraine crop-type -i \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Ukraine/SPOT4_HRVIR1_XS_20130206_N2A_EUkraineD0000B0000/" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Ukraine/SPOT4_HRVIR1_XS_20130226_N2A_EUkraineD0000B0000/" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Ukraine/SPOT4_HRVIR1_XS_20130318_N2A_EUkraineD0000B0000/" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Ukraine/SPOT4_HRVIR1_XS_20130402_N2A_EUkraineD0000B0000/" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Ukraine/SPOT4_HRVIR1_XS_20130412_N2A_EUkraineD0000B0000/" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Ukraine/SPOT4_HRVIR1_XS_20130417_N2A_EUkraineD0000B0000/" \
-r /mnt/Imagery_S2A/In-Situ_TDS/Ukraine/LC/UA_KYIV_LC_FO_2013.shp \
--resolution 20 -p processor.l4b.sampling-rate 5 --crop-mask /mnt/scratch/426/5755-image-compression/crop_mask.tif --date-start 20130206 --date-end 20130417
