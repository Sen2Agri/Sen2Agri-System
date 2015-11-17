#!/bin/bash
#USER modif
#add directories where SPOT products are to be found
./composite_processing.py --applocation /home/agrosu/sen2agri-processors-build --syntdate 20150228 --synthalf 20 --input \
"/mnt/output/L2A/Sentinel-2/Toulouse/31TCJ/S2A_OPER_SSC_L2VALD_31TCJ____20150716/S2A_OPER_SSC_L2VALD_31TCJ____20150716.HDR" \
"/mnt/output/L2A/Sentinel-2/Toulouse/31TCJ/S2A_OPER_SSC_L2VALD_31TCJ____20150726/S2A_OPER_SSC_L2VALD_31TCJ____20150726.HDR" \
--res 10 --t0 20150716 --tend 20150726 --outdir /mnt/scratch/composite_s2/20150228 --bandsmap /home/agrosu/sen2agri/sen2agri-processors/Composite/TestScripts/bands_mapping_s2.txt
