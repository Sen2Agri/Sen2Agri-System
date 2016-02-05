##!/bin/bash
#USER modif

#add directories where SPOT products are to be found
/home/achenebert/src/sen2agri-processors/Composite/TestScripts/composite_processing.py --applocation /data/s2agri/sen2agri-processors-build \
--syntdate 20150525 --synthalf 25 --input \
"/data/s2agri/input//Ukraine/SPOT5_HRG2_XS_20150510_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150510_N2A_UkraineD0000B0000.xml " \
"/data/s2agri/input//Ukraine/SPOT5_HRG2_XS_20150520_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150520_N2A_UkraineD0000B0000.xml " \
"/data/s2agri/input//Ukraine/SPOT5_HRG2_XS_20150525_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150525_N2A_UkraineD0000B0000.xml " \
"/data/s2agri/input//Ukraine/SPOT5_HRG2_XS_20150604_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150604_N2A_UkraineD0000B0000.xml " \
"/data/s2agri/input//Ukraine/SPOT5_HRG2_XS_20150609_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150609_N2A_UkraineD0000B0000.xml " \
"/data/s2agri/input//Ukraine/SPOT5_HRG2_XS_20150614_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150614_N2A_UkraineD0000B0000.xml " \
"/data/s2agri/input//Ukraine/SPOT5_HRG2_XS_20150619_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150619_N2A_UkraineD0000B0000.xml " \
"/data/s2agri/input//Ukraine/SPOT5_HRG2_XS_20150624_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150624_N2A_UkraineD0000B0000.xml " \
"/data/s2agri/input//Ukraine/SPOT5_HRG2_XS_20150629_N2A_UkraineD0000B0000/SPOT5_HRG2_XS_20150629_N2A_UkraineD0000B0000.xml " \
--res 10 --t0 20150510 --tend 20150629 --outdir /data/s2agri/output/Ukraine/Ukraine/composite/Composite_Ukraine_May --bandsmap /home/achenebert/src/sen2agri/sen2agri-processors/Composite/TestScripts/bands_mapping_spot5.txt
