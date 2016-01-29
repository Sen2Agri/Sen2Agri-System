##!/bin/bash
#USER modif

#add directories where SPOT products are to be found
/home/achenebert/src/sen2agri-processors/Composite/TestScripts/composite_processing.py --applocation /data/s2agri/sen2agri-processors-build \
--syntdate 20150628 --synthalf 25 --input \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150608_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150608_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150613_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150613_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150618_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150618_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150623_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150623_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150628_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150628_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150703_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150703_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150708_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150708_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150718_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150718_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150728_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150728_N2A_SouthAfricaD0000B0000.xml " \
--res 10 --t0 20150608 --tend 20150728 --outdir /data/s2agri/output/SouthAfrica/SouthAfrica/composite/Composite_SouthAfrica_June --bandsmap /home/achenebert/src/sen2agri/sen2agri-processors/Composite/TestScripts/bands_mapping_spot5.txt
