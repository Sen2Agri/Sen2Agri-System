#!/bin/bash
#USER modif

#add directories where SPOT products are to be found
/home/achenebert/src/sen2agri-processors/Composite/TestScripts/composite_processing.py --applocation /data/s2agri/sen2agri-processors-build \
--syntdate 20150430 --synthalf 25 --input \
"/data/s2agri/input/Spot5-T5/Belgium/Belgium/SPOT5_HRG2_XS_20150505_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150505_N2A_BelgiumD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Belgium/Belgium/SPOT5_HRG2_XS_20150515_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150515_N2A_BelgiumD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Belgium/Belgium/SPOT5_HRG2_XS_20150520_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150520_N2A_BelgiumD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Belgium/Belgium/SPOT5_HRG2_XS_20150604_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150604_N2A_BelgiumD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Belgium/Belgium/SPOT5_HRG2_XS_20150614_N2A_BelgiumD0000B0000/SPOT5_HRG2_XS_20150614_N2A_BelgiumD0000B0000.xml" \
--res 10 --t0 20150410 --tend 20150525 --outdir /data/s2agri/output/Belgique/Belgique/composite/Composite_Belgique_April --bandsmap /home/achenebert/src/sen2agri/sen2agri-processors/Composite/TestScripts/bands_mapping_spot5.txt
