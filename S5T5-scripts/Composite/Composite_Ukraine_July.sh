##!/bin/bash
#USER modif
#add directories where SPOT products are to be found
../../sen2agri-processors/Composite/TestScripts/composite_processing.py --applocation /home/ramona/sen2agri-processors-build \
--syntdate 20150729 --synthalf 25 --input \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150704_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150704_N2A_UkraineD0000B0000.xml " \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150709_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150709_N2A_UkraineD0000B0000.xml " \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150719_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150719_N2A_UkraineD0000B0000.xml " \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150724_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150724_N2A_UkraineD0000B0000.xml " \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150729_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150729_N2A_UkraineD0000B0000.xml " \
"/mnt/Sen2Agri_DataSets/L2A/Spot5-T5/Ukraine/SPOT5_HRG2_XS_20150808_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150808_N2A_UkraineD0000B0000.xml " \
--res 10 --t0 20150704 --tend 20150808 --outdir /mnt/output/ramona/Composite_Ukraine_July --bandsmap /home/ramona/sen2agri/sen2agri-processors/Composite/TestScripts/bands_mapping_spot5.txt