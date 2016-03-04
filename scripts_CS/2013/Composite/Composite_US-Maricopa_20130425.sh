#!/bin/bash -l
#USER modif

python generate_file_list_composite.py \
--inputdir /data/s2agri/input/EOData/2013/US-Maricopa/US-Maricopa/ \
--syntdate 20130425 --synthalf 15 --instrument HRVIR \
--outfile /data/s2agri/output/2013/US-Maricopa/composite_20130425/configuration.file \
--orbitday J1


#run composite processors
python /home/msavinaud/dev/s2agri/src/sen2agri-processors/Composite/TestScripts/composite_processing_CS.py \
--applocation /home/msavinaud/dev/s2agri/build/ \
--configfile /data/s2agri/output/2013/US-Maricopa/composite_20130425/configuration.file \
--res 20 \
--outdir /data/s2agri/output/2013/US-Maricopa/composite_20130425 \
--bandsmap /home/msavinaud/dev/s2agri/src/sen2agri-processors/Composite/TestScripts/bands_mapping_spot.txt

