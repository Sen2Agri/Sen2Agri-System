#USER modif
#add directories where L8 products are to be found
./composite_processing.py --applocation ~/sen2agri-processors-build --syntdate 20130501 --synthalf 30 --input \
"/mnt/Sen2Agri_DataSets/L2A/Landsat8/Ukraine/MACCS_Manual_Format/EUkraineS2A_20130416_L8_181_025/EUkraineS2A_20130416_L8_181_025.HDR" \
"/mnt/Sen2Agri_DataSets/L2A/Landsat8/Ukraine/MACCS_Manual_Format/EUkraineS2A_20130502_L8_181_025/EUkraineS2A_20130502_L8_181_025.HDR" \
"/mnt/Sen2Agri_DataSets/L2A/Landsat8/Ukraine/MACCS_Manual_Format/EUkraineS2A_20130518_L8_181_025/EUkraineS2A_20130518_L8_181_025.HDR" \
--res 30 --outdir /mnt/output/L3A/Landsat/Ukraine --bandsmap ~/sen2agri/sen2agri-processors/Composite/TestScripts/bands_mapping_L8.txt
