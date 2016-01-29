#!/bin/bash


/home/achenebert/src/scripts/CropType.py -ref /data/s2agri/input/InSituData/2015/Ukraine/UA_KYIV_LC_FO_2015.shp -ratio 0.75 -input \
/data/s2agri/input/EOData/2015/Ukraine/Ukraine/SPOT5_HRG2_XS_20150410_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150410_N2A_UkraineD0000B0000.xml \
/data/s2agri/input/EOData/2015/Ukraine/Ukraine/SPOT5_HRG2_XS_20150425_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150425_N2A_UkraineD0000B0000.xml \
/data/s2agri/input/EOData/2015/Ukraine/Ukraine/SPOT5_HRG2_XS_20150430_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150430_N2A_UkraineD0000B0000.xml \
/data/s2agri/input/EOData/2015/Ukraine/Ukraine/SPOT5_HRG2_XS_20150510_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150510_N2A_UkraineD0000B0000.xml \
/data/s2agri/input/EOData/2015/Ukraine/Ukraine/SPOT5_HRG2_XS_20150520_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150520_N2A_UkraineD0000B0000.xml \
/data/s2agri/input/EOData/2015/Ukraine/Ukraine/SPOT5_HRG2_XS_20150525_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150525_N2A_UkraineD0000B0000.xml \
/data/s2agri/input/EOData/2015/Ukraine/Ukraine/SPOT5_HRG2_XS_20150604_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150604_N2A_UkraineD0000B0000.xml \
/data/s2agri/input/EOData/2015/Ukraine/Ukraine/SPOT5_HRG2_XS_20150609_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150609_N2A_UkraineD0000B0000.xml \
/data/s2agri/input/EOData/2015/Ukraine/Ukraine/SPOT5_HRG2_XS_20150614_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150614_N2A_UkraineD0000B0000.xml \
/data/s2agri/input/EOData/2015/Ukraine/Ukraine/SPOT5_HRG2_XS_20150619_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150619_N2A_UkraineD0000B0000.xml \
/data/s2agri/input/EOData/2015/Ukraine/Ukraine/SPOT5_HRG2_XS_20150624_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150624_N2A_UkraineD0000B0000.xml \
/data/s2agri/input/EOData/2015/Ukraine/Ukraine/SPOT5_HRG2_XS_20150629_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150629_N2A_UkraineD0000B0000.xml \
/data/s2agri/input/EOData/2015/Ukraine/Ukraine/SPOT5_HRG2_XS_20150704_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150704_N2A_UkraineD0000B0000.xml \
/data/s2agri/input/EOData/2015/Ukraine/Ukraine/SPOT5_HRG2_XS_20150709_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150709_N2A_UkraineD0000B0000.xml \
/data/s2agri/input/EOData/2015/Ukraine/Ukraine/SPOT5_HRG2_XS_20150719_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150719_N2A_UkraineD0000B0000.xml \
/data/s2agri/input/EOData/2015/Ukraine/Ukraine/SPOT5_HRG2_XS_20150724_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150724_N2A_UkraineD0000B0000.xml \
/data/s2agri/input/EOData/2015/Ukraine/Ukraine/SPOT5_HRG2_XS_20150729_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150729_N2A_UkraineD0000B0000.xml \
/data/s2agri/input/EOData/2015/Ukraine/Ukraine/SPOT5_HRG2_XS_20150808_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150808_N2A_UkraineD0000B0000.xml \
-t0 20150410 -tend 20150808 -rate 5 -radius 100 -classifier rf -rfnbtrees 100 -rfmax 25 -rfmin 25 -rseed 0 -mask /data/s2agri/output/Ukraine/Ukraine/cropMask/crop_mask.tif -tilename T15SVC -pixsize 10 \
-outdir /data/s2agri/output/Ukraine/Ukraine/cropType -targetfolder /data/s2agri/output/Ukraine/Ukraine/cropType-product -buildfolder /data/s2agri/sen2agri-processors-build/
