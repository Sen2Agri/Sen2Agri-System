##!/bin/bash
#USER modif

#add directories where SPOT products are to be found
/home/achenebert/src/sen2agri-processors/Composite/TestScripts/composite_processing.py --applocation /data/s2agri/sen2agri-processors-build \
--syntdate 20150827 --synthalf 25 --input \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150807_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150807_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150817_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150817_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150822_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150822_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150827_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150827_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150901_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150901_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150911_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150911_N2A_SouthAfricaD0000B0000.xml " \
--res 10 --t0 20150807 --tend 20150911 --outdir /data/s2agri/output/SouthAfrica/SouthAfrica/composite/Composite_SouthAfrica_August --bandsmap /home/achenebert/src/sen2agri/sen2agri-processors/Composite/TestScripts/bands_mapping_spot5.txt
