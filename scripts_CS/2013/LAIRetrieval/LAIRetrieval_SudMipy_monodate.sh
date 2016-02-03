#! /bin/bash -l

python /home/msavinaud/dev/s2agri/src/sen2agri-processors/VegetationStatus/TestScripts/lai_retrieve_processing_CS.py \
--applocation /data/s2agri/sen2agri-processors-build-thor \
--input "/data/s2agri/input/EOData/2013/France/SudOuest/SPOT4_HRVIR1_XS_20130503_N2A_CSudmipy-ED0000B0000/SPOT4_HRVIR1_XS_20130503_N2A_CSudmipy-ED0000B0000.xml" \
--res 0 \
--t0 20130503 \
--tend 20130503 \
--generatemodel YES \
--outdir /data/s2agri/output/2013/France/LAI_France_monodate_20130503 \
--rsrfile /home/msavinaud/dev/s2agri/src/sen2agri-processors/VegetationStatus/otb-bv/data/spot4hrvir1.rsr
