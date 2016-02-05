#!/bin/bash


/home/achenebert/src/scripts/CropType.py -ref /data/s2agri/input/InSituData/2015/Russia/RU_TULA_LC_SM_2015.shp -ratio 0.75 -input \
/data/s2agri/input/EOData/2015/Russia/Toula/SPOT5_HRG2_XS_20150412_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150412_N2A_ToulaRussiaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Russia/Toula/SPOT5_HRG2_XS_20150417_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150417_N2A_ToulaRussiaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Russia/Toula/SPOT5_HRG2_XS_20150507_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150507_N2A_ToulaRussiaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Russia/Toula/SPOT5_HRG2_XS_20150512_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150512_N2A_ToulaRussiaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Russia/Toula/SPOT5_HRG2_XS_20150522_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150522_N2A_ToulaRussiaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Russia/Toula/SPOT5_HRG2_XS_20150606_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150606_N2A_ToulaRussiaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Russia/Toula/SPOT5_HRG2_XS_20150611_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150611_N2A_ToulaRussiaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Russia/Toula/SPOT5_HRG2_XS_20150701_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150701_N2A_ToulaRussiaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Russia/Toula/SPOT5_HRG2_XS_20150706_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150706_N2A_ToulaRussiaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Russia/Toula/SPOT5_HRG2_XS_20150716_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150716_N2A_ToulaRussiaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Russia/Toula/SPOT5_HRG2_XS_20150726_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150726_N2A_ToulaRussiaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Russia/Toula/SPOT5_HRG2_XS_20150731_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150731_N2A_ToulaRussiaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Russia/Toula/SPOT5_HRG2_XS_20150805_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150805_N2A_ToulaRussiaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Russia/Toula/SPOT5_HRG2_XS_20150810_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150810_N2A_ToulaRussiaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Russia/Toula/SPOT5_HRG2_XS_20150815_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150815_N2A_ToulaRussiaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Russia/Toula/SPOT5_HRG2_XS_20150820_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150820_N2A_ToulaRussiaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Russia/Toula/SPOT5_HRG2_XS_20150825_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150825_N2A_ToulaRussiaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Russia/Toula/SPOT5_HRG2_XS_20150830_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150830_N2A_ToulaRussiaD0000B0000.xml \
-t0 20150414 -tend 20150911 -rate 5 -radius 100 -classifier rf -rfnbtrees 100 -rfmax 25 -rfmin 25 -rseed 0 -mask /data/s2agri/output/Russia/Toula/cropMask/crop_mask.tif -tilename T15SVC -pixsize 10 \
-outdir /data/s2agri/output/Russia/Toula/cropType -targetfolder /data/s2agri/output/Russia/Toula/cropType-product -buildfolder /data/s2agri/sen2agri-processors-build/
