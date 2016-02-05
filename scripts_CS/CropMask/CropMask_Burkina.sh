#!/bin/bash

source /home/achenebert/src/S5T5-scripts/CropMask/set_build_folder.sh

/home/achenebert/src/scripts/CropMask.py -refp /data/s2agri/input/InSituData/2015/BurkinaFaso/BF_KOUM_LC_FO_2015.shp -ratio 0.75 -input \
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
-t0 20150410 -tend 20150823 -rate 5 -radius 100 -nbtrsample 4000 -rseed 0 -window 6 -lmbd 2 -weight 1 -nbcomp 6 -spatialr 10 -ranger 0.65 -minsize 10 -rfnbtrees 100 -rfmax 25 -rfmin 25 -tilename T15SVC -pixsize 10 \
-outdir /data/s2agri/output/Burkina/Burkina/cropMask -targetfolder /data/s2agri/output/Burkina/Burkina/cropMask-product -buildfolder /data/s2agri/sen2agri-processors-build/
