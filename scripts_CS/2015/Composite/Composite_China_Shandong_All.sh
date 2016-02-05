#!/bin/bash -l
#USER modif


dateList="20150401 20150501 20150601 20150701 20150801 20150901"   
for date in $dateList    
do   

echo "synthesis date =" $date

# generate the configuration file with list of product to use
python generate_file_list_composite.py \
--inputdir /data/s2agri/input/EOData/2015/China/Shandong/ \
--syntdate $date --synthalf 25 --instrument HRG2 \
--outfile /data/s2agri/output/2015/China/composite_$date/configuration.file

#run composite processors
python /home/msavinaud/dev/s2agri/src/sen2agri-processors/Composite/TestScripts/composite_processing_CS.py \
--applocation /data/s2agri/sen2agri-processors-build-thor \
--configfile /data/s2agri/output/2015/China/composite_$date/configuration.file \
--res 10 \
--outdir /data/s2agri/output/2015/China/composite_$date \
--bandsmap /home/msavinaud/dev/s2agri/src/sen2agri-processors/Composite/TestScripts/bands_mapping_spot5.txt
           
done
