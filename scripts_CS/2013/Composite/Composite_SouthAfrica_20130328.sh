#!/bin/bash -l
#USER modif

#add directories where SPOT products are to be found
python /home/msavinaud/dev/s2agri/src/sen2agri-processors/Composite/TestScripts/composite_processing.py \
--applocation /data/s2agri/sen2agri-processors-build-thor \
--syntdate 20130328 --synthalf 15 --input \
"/data/s2agri/input/EOData/2013/SouthAfrica/SouthAfrica/SPOT4_HRVIR1_XS_20130317_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130317_N2A_ESouthAfricaD0000B0000.xml" \
"/data/s2agri/input/EOData/2013/SouthAfrica/SouthAfrica/SPOT4_HRVIR1_XS_20130322_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130322_N2A_ESouthAfricaD0000B0000.xml" \
"/data/s2agri/input/EOData/2013/SouthAfrica/SouthAfrica/SPOT4_HRVIR1_XS_20130401_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130401_N2A_ESouthAfricaD0000B0000.xml" \
"/data/s2agri/input/EOData/2013/SouthAfrica/SouthAfrica/SPOT4_HRVIR1_XS_20130406_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130406_N2A_ESouthAfricaD0000B0000.xml" \
--res 20 --t0 20150317 --tend 20150406 \
--outdir /data/s2agri/output/2013/SouthAfrica/composite_20130328 \
--bandsmap /home/msavinaud/dev/s2agri/src/sen2agri-processors/Composite/TestScripts/bands_mapping_spot.txt




