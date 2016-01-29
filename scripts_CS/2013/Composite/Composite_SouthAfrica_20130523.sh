#!/bin/bash -l
#USER modif

#add directories where SPOT products are to be found
python /home/msavinaud/dev/s2agri/src/sen2agri-processors/Composite/TestScripts/composite_processing.py \
--applocation /data/s2agri/sen2agri-processors-build-thor \
--syntdate 20130523 --synthalf 15 --input \
"/data/s2agri/input/EOData/2013/SouthAfrica/SouthAfrica/SPOT4_HRVIR1_XS_20130511_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130511_N2A_ESouthAfricaD0000B0000.xml" \
"/data/s2agri/input/EOData/2013/SouthAfrica/SouthAfrica/SPOT4_HRVIR1_XS_20130516_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130516_N2A_ESouthAfricaD0000B0000.xml" \
"/data/s2agri/input/EOData/2013/SouthAfrica/SouthAfrica/SPOT4_HRVIR1_XS_20130521_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130521_N2A_ESouthAfricaD0000B0000.xml" \
"/data/s2agri/input/EOData/2013/SouthAfrica/SouthAfrica/SPOT4_HRVIR1_XS_20130526_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130526_N2A_ESouthAfricaD0000B0000.xml" \
"/data/s2agri/input/EOData/2013/SouthAfrica/SouthAfrica/SPOT4_HRVIR1_XS_20130531_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130531_N2A_ESouthAfricaD0000B0000.xml" \
"/data/s2agri/input/EOData/2013/SouthAfrica/SouthAfrica/SPOT4_HRVIR1_XS_20130605_N2A_ESouthAfricaD0000B0000/SPOT4_HRVIR1_XS_20130605_N2A_ESouthAfricaD0000B0000.xml" \
--res 20 --t0 20150511 --tend 20150605 \
--outdir /data/s2agri/output/2013/SouthAfrica/composite_20130523 \
--bandsmap /home/msavinaud/dev/s2agri/src/sen2agri-processors/Composite/TestScripts/bands_mapping_spot.txt







