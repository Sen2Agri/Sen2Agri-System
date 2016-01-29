##!/bin/bash
#USER modif

#add directories where SPOT products are to be found
/home/achenebert/src/sen2agri-processors/Composite/TestScripts/composite_processing.py --applocation /data/s2agri/sen2agri-processors-build \
--syntdate 20150524 --synthalf 25 --input \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150504_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150504_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150509_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150509_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150514_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150514_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150519_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150519_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150524_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150524_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150608_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150608_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150613_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150613_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150618_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150618_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150623_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150623_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150628_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150628_N2A_SouthAfricaD0000B0000.xml " \
--res 10 --t0 20150504 --tend 20150628 --outdir /data/s2agri/output/SouthAfrica/SouthAfrica/composite/Composite_SouthAfrica_May --bandsmap /home/achenebert/src/sen2agri/sen2agri-processors/Composite/TestScripts/bands_mapping_spot5.txt
