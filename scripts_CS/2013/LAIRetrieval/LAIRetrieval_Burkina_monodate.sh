#! /bin/bash -l

python /home/msavinaud/dev/s2agri/src/sen2agri-processors/VegetationStatus/TestScripts/lai_retrieve_processing_CS.py \
--applocation /data/s2agri/sen2agri-processors-build-thor \
--input "/data/s2agri/input/EOData/2015/Burkina/Burkina/SPOT5_HRG2_XS_20150410_N2A_BurkinaD0000B0000/SPOT5_HRG2_XS_20150410_N2A_BurkinaD0000B0000.xml" \
--res 0 \
--t0 20150410 \
--tend 20150410 \
--generatemodel YES \
--outdir /data/s2agri/output/2013/Burkina/LAI_Burkina_monodate \
--rsrfile /home/msavinaud/dev/s2agri/src/sen2agri-processors/VegetationStatus/otb-bv/data/spot5hrg1.rsr
