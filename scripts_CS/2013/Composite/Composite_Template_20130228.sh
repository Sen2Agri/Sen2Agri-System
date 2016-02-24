#!/bin/bash -l
#USER modif

#add directories where SPOT products are to be found
python /home/msavinaud/dev/s2agri/src/sen2agri-processors/Composite/TestScripts/composite_processing.py \
--applocation /home/msavinaud/dev/s2agri/build/ \
--syntdate 20130228 --synthalf 15 --input \
"/data/s2agri/input/EOData/2013/Argentina/Argentina//.xml" \
--res 20 --t0 20150228 --tend 20150315 \
--outdir /data/s2agri/output/2013/Argentina/composite_20130228 \
--bandsmap /home/msavinaud/dev/s2agri/src/sen2agri-processors/Composite/TestScripts/bands_mapping_spot.txt


