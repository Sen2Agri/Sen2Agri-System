#!/bin/bash
#USER modif
#add directories where SPOT products are to be found
./composite_processing.py --applocation ~/sen2agri-processors-build --syntdate 20131002 --synthalf 90 --input \
"/mnt/Sen2Agri_DataSets/L2A/Landsat8/Belgium/MACCS_ManualFormat/BelgiumS2A_20130728_L8_198_025/BelgiumS2A_20130728_L8_198_025.HDR" \
"/mnt/Sen2Agri_DataSets/L2A/Landsat8/Belgium/MACCS_ManualFormat/BelgiumS2A_20130930_L8_198_025/BelgiumS2A_20130930_L8_198_025.HDR" \
"/mnt/Sen2Agri_DataSets/L2A/Landsat8/Belgium/MACCS_ManualFormat/BelgiumS2A_20131203_L8_198_025/BelgiumS2A_20131203_L8_198_025.HDR" \
--res 30 --outdir /mnt/output/L3A/Landsat/Belgium --bandsmap ~/sen2agri/sen2agri-processors/Composite/TestScripts/bands_mapping_L8.txt
