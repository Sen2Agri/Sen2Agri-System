#!/bin/bash
./sen2agri-ctl.py submit-job -s Ukraine crop-mask -i \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Ukraine/SPOT4_HRVIR1_XS_20130206_N2A_EUkraineD0000B0000/" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Ukraine/SPOT4_HRVIR1_XS_20130226_N2A_EUkraineD0000B0000/" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Ukraine/SPOT4_HRVIR1_XS_20130318_N2A_EUkraineD0000B0000/" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Ukraine/SPOT4_HRVIR1_XS_20130402_N2A_EUkraineD0000B0000/" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Ukraine/SPOT4_HRVIR1_XS_20130412_N2A_EUkraineD0000B0000/" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Ukraine/SPOT4_HRVIR1_XS_20130417_N2A_EUkraineD0000B0000/" \
-r "" -refr /mnt/data/reference/Reference_Ukraine.tif \
--resolution 20 --date-start 20130206 --date-end 20130417
