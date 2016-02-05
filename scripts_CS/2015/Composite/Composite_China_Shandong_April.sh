#!/bin/bash
#USER modif

#add directories where SPOT products are to be found
/home/achenebert/src/sen2agri-processors/Composite/TestScripts/composite_processing.py --applocation /data/s2agri/sen2agri-processors-build \
--syntdate 20150430 --synthalf 25 --input \
"/data/s2agri/input/Spot5-T5/China/Shandong/SPOT5_HRG2_XS_20150423_N2A_ShandongChinaD0000B0000/SPOT5_HRG2_XS_20150423_N2A_ShandongChinaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/China/Shandong/SPOT5_HRG2_XS_20150508_N2A_ShandongChinaD0000B0000/SPOT5_HRG2_XS_20150508_N2A_ShandongChinaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/China/Shandong/SPOT5_HRG2_XS_20150513_N2A_ShandongChinaD0000B0000/SPOT5_HRG2_XS_20150513_N2A_ShandongChinaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/China/Shandong/SPOT5_HRG2_XS_20150523_N2A_ShandongChinaD0000B0000/SPOT5_HRG2_XS_20150523_N2A_ShandongChinaD0000B0000.xml" \
"/data/s2agri/input/Spot5-T5/China/Shandong/SPOT5_HRG2_XS_20150528_N2A_ShandongChinaD0000B0000/SPOT5_HRG2_XS_20150528_N2A_ShandongChinaD0000B0000.xml" \
--res 10 --t0 20150410 --tend 20150525 --outdir /data/s2agri/output/China/Shangdong/composite/Composite_China_April --bandsmap /home/achenebert/src/sen2agri/sen2agri-processors/Composite/TestScripts/bands_mapping_spot5.txt
