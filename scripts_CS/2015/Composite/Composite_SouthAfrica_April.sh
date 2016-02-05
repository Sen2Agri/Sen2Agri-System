##!/bin/bash
#USER modif

#add directories where SPOT products are to be found
/home/achenebert/src/sen2agri-processors/Composite/TestScripts/composite_processing.py --applocation /data/s2agri/sen2agri-processors-build \
--syntdate 20150429 --synthalf 25 --input \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150414_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150414_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150429_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150429_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150504_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150504_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150509_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150509_N2A_SouthAfricaD0000B0000.xml " \
"/data/s2agri/input/Spot5-T5/SouthAfrica/SouthAfrica/SPOT5_HRG2_XS_20150514_N2A_SouthAfricaD0000B0000/SPOT5_HRG2_XS_20150514_N2A_SouthAfricaD0000B0000.xml " \
--res 10 --t0 20150414 --tend 20150514 --outdir /data/s2agri/output/SouthAfrica/SouthAfrica/composite/Composite_SouthAfrica_April --bandsmap /home/achenebert/src/sen2agri/sen2agri-processors/Composite/TestScripts/bands_mapping_spot5.txt
