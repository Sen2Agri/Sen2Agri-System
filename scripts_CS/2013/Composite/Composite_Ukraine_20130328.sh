#!/bin/bash -l
#USER modif

#add directories where SPOT products are to be found
python /home/msavinaud/dev/s2agri/src/sen2agri-processors/Composite/TestScripts/composite_processing.py  \
--applocation /data/s2agri/sen2agri-processors-build-thor \
--syntdate 20130328 --synthalf 20 --input \
"/data/s2agri/input/EOData/2013/Ukraine/Ukraine/SPOT4_HRVIR1_XS_20130318_N2A_EUkraineD0000B0000/SPOT4_HRVIR1_XS_20130318_N2A_EUkraineD0000B0000.xml" \
"/data/s2agri/input/EOData/2013/Ukraine/Ukraine/SPOT4_HRVIR1_XS_20130402_N2A_EUkraineD0000B0000/SPOT4_HRVIR1_XS_20130402_N2A_EUkraineD0000B0000.xml" \
"/data/s2agri/input/EOData/2013/Ukraine/Ukraine/SPOT4_HRVIR1_XS_20130412_N2A_EUkraineD0000B0000/SPOT4_HRVIR1_XS_20130412_N2A_EUkraineD0000B0000.xml" \
"/data/s2agri/input/EOData/2013/Ukraine/Ukraine/SPOT4_HRVIR1_XS_20130417_N2A_EUkraineD0000B0000/SPOT4_HRVIR1_XS_20130417_N2A_EUkraineD0000B0000.xml" \
--res 20 --t0 20130318 --tend 20130417 \
--outdir /data/s2agri/output/2013/Ukraine/composite__20130328 \
--bandsmap /home/msavinaud/dev/s2agri/src/sen2agri-processors/Composite/TestScripts/bands_mapping_spot.txt

