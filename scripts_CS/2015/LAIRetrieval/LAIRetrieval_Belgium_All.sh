#! /bin/bash -l

# Retrieve the list of SPOT4 file
python generate_file_list.py --inputdir /data/s2agri/input/EOData/2015/Belgium/Belgium/ \
--instrument HRG2 --outfile /data/s2agri/output/2015/Belgium/LAI_conf_file.txt

# Compute the LAI for each product

while read F  ; do
echo $F
python /home/msavinaud/dev/s2agri/src/sen2agri-processors/VegetationStatus/TestScripts/lai_retrieve_processing_CS.py \
--applocation /data/s2agri/sen2agri-processors-build-thor \
--input $F \
--res 0 \
--t0 20150503 \
--tend 20150503 \
--generatemodel YES \
--outdir /data/s2agri/output/2015/Belgium/LAI_Belgium_monodate \
--rsrfile /home/msavinaud/dev/s2agri/src/sen2agri-processors/VegetationStatus/otb-bv/data/spot5hrg1.rsr
        
done </data/s2agri/output/2015/Belgium/LAI_conf_file.txt
