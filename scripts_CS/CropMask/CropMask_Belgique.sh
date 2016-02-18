#!/bin/bash

/home/achenebert/src/scripts/CropMask.py -refp /data/s2agri/input/InSituData/2015/Belgium/BE_HESB_LC_SM_2015.shp -ratio 0.75 -input \
/data/s2agri/input/EOData/2015/Belgium/Belgium/SPOT5_HRG2_XS_20150410_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150410_N2A_BelgiumD0000B0000.xml \
/data/s2agri/input/EOData/2015/Belgium/Belgium/SPOT5_HRG2_XS_20150415_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150415_N2A_BelgiumD0000B0000.xml \
/data/s2agri/input/EOData/2015/Belgium/Belgium/SPOT5_HRG2_XS_20150420_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150420_N2A_BelgiumD0000B0000.xml \
/data/s2agri/input/EOData/2015/Belgium/Belgium/SPOT5_HRG2_XS_20150505_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150505_N2A_BelgiumD0000B0000.xml \
/data/s2agri/input/EOData/2015/Belgium/Belgium/SPOT5_HRG2_XS_20150515_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150515_N2A_BelgiumD0000B0000.xml \
/data/s2agri/input/EOData/2015/Belgium/Belgium/SPOT5_HRG2_XS_20150520_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150520_N2A_BelgiumD0000B0000.xml \
/data/s2agri/input/EOData/2015/Belgium/Belgium/SPOT5_HRG2_XS_20150604_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150604_N2A_BelgiumD0000B0000.xml \
/data/s2agri/input/EOData/2015/Belgium/Belgium/SPOT5_HRG2_XS_20150614_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150614_N2A_BelgiumD0000B0000.xml \
/data/s2agri/input/EOData/2015/Belgium/Belgium/SPOT5_HRG2_XS_20150704_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150704_N2A_BelgiumD0000B0000.xml \
/data/s2agri/input/EOData/2015/Belgium/Belgium/SPOT5_HRG2_XS_20150709_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150709_N2A_BelgiumD0000B0000.xml \
/data/s2agri/input/EOData/2015/Belgium/Belgium/SPOT5_HRG2_XS_20150729_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150729_N2A_BelgiumD0000B0000.xml \
/data/s2agri/input/EOData/2015/Belgium/Belgium/SPOT5_HRG2_XS_20150813_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150813_N2A_BelgiumD0000B0000.xml \
/data/s2agri/input/EOData/2015/Belgium/Belgium/SPOT5_HRG2_XS_20150823_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150823_N2A_BelgiumD0000B0000.xml \
/data/s2agri/input/EOData/2015/Belgium/Belgium/SPOT5_HRG2_XS_20150902_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150902_N2A_BelgiumD0000B0000.xml \
/data/s2agri/input/EOData/2015/Belgium/Belgium/SPOT5_HRG2_XS_20150912_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150912_N2A_BelgiumD0000B0000.xml \
-nbtrsample 4000 -rseed 0 -window 6 -lmbd 2 -weight 1 -nbcomp 6 -spatialr 10 -ranger 0.65 -minsize 10 -rfnbtrees 100 -rfmax 25 -rfmin 25 -tilename T15SVC -pixsize 10 \
-outdir /data/s2agri/output/Belgique/Belgique/cropMask -targetfolder /data/s2agri/output/Belgique/Belgique/cropMask-product -buildfolder /data/s2agri/sen2agri-processors-build/
