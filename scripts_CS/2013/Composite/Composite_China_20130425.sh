#!/bin/bash
#USER modif

python generate_file_list_composite.py \
--inputdir /data/s2agri/input/EOData/2013/China/Shandong/ \
--syntdate 20130425 --synthalf 25 --instrument HRVIR1 \
--outfile /data/s2agri/output/2013/China/composite_20130425/configuration.file


#run composite processors
python /home/msavinaud/dev/s2agri/src/sen2agri-processors/Composite/TestScripts/composite_processing_CS.py \
--applocation /home/msavinaud/dev/s2agri/build/ \
--configfile /data/s2agri/output/2013/China/composite_20130425/configuration.file \
--res 20 \
--outdir /data/s2agri/output/2013/China/composite_20130425 \
--bandsmap /home/msavinaud/dev/s2agri/src/sen2agri-processors/Composite/TestScripts/bands_mapping_spot.txt
