#!/bin/bash
#USER modif

#python generate_file_list_composite.py \
#--inputdir $SEN2AGRI_DATA_IN_EO_2013/Argentina/Argentina/ \
#--syntdate 20130228 --synthalf 15 --instrument HRVIR1 \
#--outfile $SEN2AGRI_DATA_OUT_2013/Argentina/composite_20130228_bis/configuration.file


#run composite processors
#python /home/msavinaud/dev/s2agri/src/sen2agri-processors/Composite/TestScripts/composite_processing_CS.py \

composite_processing_CS.py \
--inputdir $SEN2AGRI_DATA_IN_EO_2013/Argentina/Argentina/ \
--syntdate 20130228 --synthalf 15 --satellite SPOT4 --instrument HRVIR1 \
--res 20 \
--bandsmap $SEN2AGRI_SHARE_DATA/bands_mapping_spot.txt \
--siteid 01 \
--outdir $SEN2AGRI_DATA_OUT_2013/Argentina/composite_20130228_bis 
