#!/bin/bash

source set_build_folder.sh

/home/achenebert/src/scripts/CropType.py -ref /data/s2agri/input/InSituData/2015/BurkinaFaso/BF_KOUM_LC_FO_2015.shp -ratio 0.75 -input \
/data/s2agri/input/EOData/2015/Burkina/Burkina/SPOT5_HRG2_XS_20150410_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150410_N2A_BurkinaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Burkina/Burkina/SPOT5_HRG2_XS_20150415_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150415_N2A_BurkinaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Burkina/Burkina/SPOT5_HRG2_XS_20150420_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150420_N2A_BurkinaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Burkina/Burkina/SPOT5_HRG2_XS_20150425_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150425_N2A_BurkinaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Burkina/Burkina/SPOT5_HRG2_XS_20150430_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150430_N2A_BurkinaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Burkina/Burkina/SPOT5_HRG2_XS_20150505_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150505_N2A_BurkinaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Burkina/Burkina/SPOT5_HRG2_XS_20150510_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150510_N2A_BurkinaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Burkina/Burkina/SPOT5_HRG2_XS_20150515_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150515_N2A_BurkinaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Burkina/Burkina/SPOT5_HRG2_XS_20150520_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150520_N2A_BurkinaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Burkina/Burkina/SPOT5_HRG2_XS_20150525_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150525_N2A_BurkinaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Burkina/Burkina/SPOT5_HRG2_XS_20150604_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150604_N2A_BurkinaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Burkina/Burkina/SPOT5_HRG2_XS_20150614_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150614_N2A_BurkinaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Burkina/Burkina/SPOT5_HRG2_XS_20150624_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150624_N2A_BurkinaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Burkina/Burkina/SPOT5_HRG2_XS_20150629_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150629_N2A_BurkinaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Burkina/Burkina/SPOT5_HRG2_XS_20150709_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150709_N2A_BurkinaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Burkina/Burkina/SPOT5_HRG2_XS_20150714_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150714_N2A_BurkinaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Burkina/Burkina/SPOT5_HRG2_XS_20150729_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150729_N2A_BurkinaD0000B0000.xml \
/data/s2agri/input/EOData/2015/Burkina/Burkina/SPOT5_HRG2_XS_20150823_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150823_N2A_BurkinaD0000B0000.xml \
-rate 5 -classifier rf -rfnbtrees 100 -rfmax 25 -rfmin 25 -rseed 0 -mask /data/s2agri/output/Burkina/Burkina/cropMask/crop_mask.tif -tilename T15SVC -pixsize 10 \
-outdir /data/s2agri/output/Burkina/Burkina/cropType -targetfolder /data/s2agri/output/Burkina/Burkina/cropType-product -buildfolder /data/s2agri/sen2agri-processors-build/
