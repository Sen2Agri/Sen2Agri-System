#! /bin/bash -l

python /home/msavinaud/dev/s2agri/src/sen2agri-processors/VegetationStatus/TestScripts/lai_retrieve_processing_mul.py \
--applocation /home/msavinaud/dev/s2agri/build \
--inputdir /data/s2agri/input/EOData/2013/Belgium/Belgium/ \
--outdir /data/s2agri/output/2013/Belgium/LAI_Belgium_monodate \
--backwardwindow 2 \
--forwardwindow 0 \
--sat SPOT4 \
--inst HRVIR1
