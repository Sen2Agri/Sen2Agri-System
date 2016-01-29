#!/bin/bash
#USER modif

#add directories where SPOT products are to be found
/home/achenebert/src/sen2agri-processors/Composite/TestScripts/composite_processing.py --applocation /data/s2agri/sen2agri-processors-build \
--syntdate 20150430 --synthalf 25 --input \
"/data/s2agri/input/Spot5-T5/Russia/Toula/SPOT5_HRG2_XS_20150412_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150412_N2A_ToulaRussiaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Russia/Toula/SPOT5_HRG2_XS_20150417_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150417_N2A_ToulaRussiaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Russia/Toula/SPOT5_HRG2_XS_20150507_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150507_N2A_ToulaRussiaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Russia/Toula/SPOT5_HRG2_XS_20150512_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150512_N2A_ToulaRussiaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Russia/Toula/SPOT5_HRG2_XS_20150522_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150522_N2A_ToulaRussiaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Russia/Toula/SPOT5_HRG2_XS_20150606_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150606_N2A_ToulaRussiaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Russia/Toula/SPOT5_HRG2_XS_20150611_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150611_N2A_ToulaRussiaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Russia/Toula/SPOT5_HRG2_XS_20150701_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150701_N2A_ToulaRussiaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Russia/Toula/SPOT5_HRG2_XS_20150706_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150706_N2A_ToulaRussiaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Russia/Toula/SPOT5_HRG2_XS_20150716_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150716_N2A_ToulaRussiaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Russia/Toula/SPOT5_HRG2_XS_20150726_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150726_N2A_ToulaRussiaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Russia/Toula/SPOT5_HRG2_XS_20150731_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150731_N2A_ToulaRussiaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Russia/Toula/SPOT5_HRG2_XS_20150805_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150805_N2A_ToulaRussiaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Russia/Toula/SPOT5_HRG2_XS_20150810_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150810_N2A_ToulaRussiaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Russia/Toula/SPOT5_HRG2_XS_20150815_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150815_N2A_ToulaRussiaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Russia/Toula/SPOT5_HRG2_XS_20150820_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150820_N2A_ToulaRussiaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Russia/Toula/SPOT5_HRG2_XS_20150825_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150825_N2A_ToulaRussiaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/Russia/Toula/SPOT5_HRG2_XS_20150830_N2A_ToulaRussiaD0000B0000/SPOT5_HRG2_XS_20150830_N2A_ToulaRussiaD0000B0000.xml" \
--res 10 --t0 20150410 --tend 20150525 --outdir /data/s2agri/output/Russia/Toula/composite/Composite_Russia_April --bandsmap /home/achenebert/src/sen2agri/sen2agri-processors/Composite/TestScripts/bands_mapping_spot5.txt
