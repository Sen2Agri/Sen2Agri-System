#! /bin/bash -l

python /home/msavinaud/dev/s2agri/src/sen2agri-processors/VegetationStatus/TestScripts/lai_retrieve_processing_CS.py \
--applocation /data/s2agri/sen2agri-processors-build-thor \
--input \
"/data/s2agri/input/EOData/2013/Belgium/Belgium/SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000.xml" \
"/data/s2agri/input/EOData/2013/Belgium/Belgium/SPOT4_HRVIR1_XS_20130402_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130402_N2A_EBelgiumD0000B0000.xml" \
"/data/s2agri/input/EOData/2013/Belgium/Belgium/SPOT4_HRVIR1_XS_20130407_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130407_N2A_EBelgiumD0000B0000.xml" \
"/data/s2agri/input/EOData/2013/Belgium/Belgium/SPOT4_HRVIR1_XS_20130417_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130417_N2A_EBelgiumD0000B0000.xml" \
"/data/s2agri/input/EOData/2013/Belgium/Belgium/SPOT4_HRVIR1_XS_20130422_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130422_N2A_EBelgiumD0000B0000.xml" \
"/data/s2agri/input/EOData/2013/Belgium/Belgium/SPOT4_HRVIR1_XS_20130507_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130507_N2A_EBelgiumD0000B0000.xml" \
"/data/s2agri/input/EOData/2013/Belgium/Belgium/SPOT4_HRVIR1_XS_20130527_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130527_N2A_EBelgiumD0000B0000.xml" \
"/data/s2agri/input/EOData/2013/Belgium/Belgium/SPOT4_HRVIR1_XS_20130606_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130606_N2A_EBelgiumD0000B0000.xml" \
--res 0 \
--t0 20130503 \
--tend 20130503 \
--generatemodel YES \
--outdir /data/s2agri/output/2013/Belgium/LAI_Belgium_monodate \
--rsrfile /home/msavinaud/dev/s2agri/src/sen2agri-processors/VegetationStatus/otb-bv/data/spot4hrvir1.rsr










