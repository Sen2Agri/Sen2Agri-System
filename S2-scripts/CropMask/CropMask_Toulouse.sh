#!/bin/bash

source set_build_folder.sh

../../scripts/CropMask.py -refp /mnt/Sen2Agri_DataSets/In-Situ_TDS/France/LC/FR_MIPY_LC_SM_2013.shp -ratio 0.75 -input \
/mnt/output/L2A/Sentinel-2/Toulouse/S2A_OPER_SSC_L2VALD_31TCJ____20150706/S2A_OPER_SSC_L2VALD_31TCJ____20150706.HDR \
/mnt/output/L2A/Sentinel-2/Toulouse/S2A_OPER_SSC_L2VALD_31TCJ____20150716/S2A_OPER_SSC_L2VALD_31TCJ____20150716.HDR \
/mnt/output/L2A/Sentinel-2/Toulouse/S2A_OPER_SSC_L2VALD_31TCJ____20150726/S2A_OPER_SSC_L2VALD_31TCJ____20150726.HDR \
/mnt/output/L2A/Sentinel-2/Toulouse/S2A_OPER_SSC_L2VALD_31TCJ____20150805/S2A_OPER_SSC_L2VALD_31TCJ____20150805.HDR \
/mnt/output/L2A/Sentinel-2/Toulouse/S2A_OPER_SSC_L2VALD_31TCJ____20150825/S2A_OPER_SSC_L2VALD_31TCJ____20150825.HDR \
-t0 20150410 -tend 20150823 -rate 5 -radius 100 -nbtrsample 4000 -rseed 0 -window 6 -lmbd 2 -weight 1 -nbcomp 6 -spatialr 10 -ranger 0.65 -minsize 10 -rfnbtrees 100 -rfmax 25 -rfmin 25 -tilename T15SVC -pixsize 10 \
-outdir /mnt/output/ramona/S2-Tests/CropMask_Toulouse -targetfolder /mnt/output/L4A/Sentinel-2/Toulouse -buildfolder $BUILD_FOLDER
