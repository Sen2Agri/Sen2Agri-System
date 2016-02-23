#!/bin/bash
#USER modif
#add directories where SPOT products are to be found
./composite_processing.py --applocation ~/sen2agri-processors-build --syntdate 20150801 --synthalf 25 --input \
"/mnt/output/L2A/Sentinel-2/Toulouse/S2A_OPER_SSC_L2VALD_31TCJ____20150706/S2A_OPER_SSC_L2VALD_31TCJ____20150706.HDR" \
"/mnt/output/L2A/Sentinel-2/Toulouse/S2A_OPER_SSC_L2VALD_31TCJ____20150716/S2A_OPER_SSC_L2VALD_31TCJ____20150716.HDR" \
"/mnt/output/L2A/Sentinel-2/Toulouse/S2A_OPER_SSC_L2VALD_31TCJ____20150726/S2A_OPER_SSC_L2VALD_31TCJ____20150726.HDR" \
"/mnt/output/L2A/Sentinel-2/Toulouse/S2A_OPER_SSC_L2VALD_31TCJ____20150805/S2A_OPER_SSC_L2VALD_31TCJ____20150805.HDR" \
"/mnt/output/L2A/Sentinel-2/Toulouse/S2A_OPER_SSC_L2VALD_31TCJ____20150825/S2A_OPER_SSC_L2VALD_31TCJ____20150825.HDR" \
--res 10 --outdir /mnt/scratch/composite_s2/20150801 \
--bandsmap ~/sen2agri/sen2agri-processors/Composite/TestScripts/bands_mapping_s2.txt \
--scatteringcoef ~/sen2agri/sen2agri-processors/Composite/TestScripts/scattering_coeffs_10m.txt
